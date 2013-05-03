/* Copyright (c) 2011 Ironclad Mobile, Inc. All rights reserved */

#include <boost/lexical_cast.hpp>
#include <boost/optional.hpp>
#include <vector>
#include <ctime>

#include "ormcore.hpp"
#include "ormjson.hpp"
#include "ormdefer.hpp"

#ifndef ICM_ORMBACKING_HPP
#define ICM_ORMBACKING_HPP
//Backings
class Mutation;
namespace ORawrM
{
	//Forward declare the datastore.
	template<typename T>
	class Datastore;

	class NullBacking
	{
	public:
		NullBacking();

		//Return false if the replacement is invalid.
		template<typename Self, typename T>
		bool mutationReplace(Self&, const char * field, const T& prev, const T& next)
		{
			return true;
		}

		template<typename Self, typename T>
		void mutationAdd(Self&, const char * field, const std::vector<T>& arr, const T& add)
		{
		}

		template<typename Self, typename T>
		void mutationRemove(Self&, const char * field, const std::vector<T>& arr, const T& rm)
		{
		}

		template<typename Self>
		void deleted(Self&)
		{}

		template<typename Self>
		void inserted(Self&)
		{}

		template<typename Self>
		void updated(Self&)
		{}

		template<typename Self>
		void preupdate(Self&)
		{}

		template<typename T, typename Self>
		void setStore(Self&, T * d, bool updateOrDelete = false)
		{}

		void serialize(Json::Value&)
		{}

		void deserialize(Json::Value&)
		{}
	};

	//Any model with this backing will try to set the model's lastUpdatedTimestamp field on update.
	class AutomaticLastUpdatedTimestampBacking : public NullBacking
	{
	public:
		template<typename Self>
		void preupdate(Self& s)
		{
			s.setCleanLastUpdatedTimestamp(time(NULL));
		}
	};


	//Any model with this backing will create a Mutation with operation = 'UploadRequest' when 
	//the marked field is changed.
	class UploadRequestBacking : public NullBacking
	{
	public:
		UploadRequestBacking()
			:requestingUpload(false)
		{}

		template<typename Self>
		bool mutationReplace(Self& s, const char * field, const std::string& prev, const std::string& next)
		{
			ConstraintsForColumn getConstraints(field);
			Access::serializeTemporary<Self>(getConstraints);

			if (getConstraints.constraints.uploadRequest) 
			{
				requestingUpload = true;
			}

			if (prev != next)
			{
				addMutationOnField(Metadata<Self>::name, field, next, s.getPrimaryKey());
			}

			//Modifying this field does not cause an upload request, so allow it.
			return true;
		}

		template<typename Self>
		bool mutationReplace(Self& s, const char * field, const boost::optional<std::string>& prev, const boost::optional<std::string>& next)
		{
			ConstraintsForColumn getConstraints(field);
			Access::serializeTemporary<Self>(getConstraints);

			if (getConstraints.constraints.uploadRequest && next)
			{
				return mutationReplace(s, field, "", *next);
			}
			return true;
		}

		template<typename Self, typename T>
		bool mutationReplace(Self&, const char *, const T&, const T&)
		{
			//Allow other replacements.
			return true;
		}

		template<typename Self>
		void updated(Self& s)
		{
			//XXX: THIS IS HARDCODED. THE MIME AND PATH FIELDS SHOULD BE DERIVED THROUGH CONSTRAINT
			if (requestingUpload)
			{
				addUploadRequest(Metadata<Self>::name, s.getPrimaryKey(), s.getMime(), s.getPath(), s.getThumb());
			}
			requestingUpload = false;
		}

		template<typename Self>
		void inserter(Self& s)
		{
			addCreatedMutation(Metadata<Self>::name, s.getPrimaryKey());
		}

		template<typename T, typename Self>
		void setStore(Self&, T * store, bool updateOrDelete = false)
		{
			if (updateOrDelete)
			{
				store->startTransaction();

				std::vector<Mutation> pmc;
				pmc.swap(pendingUploads);

				for (size_t i = 0; i < pmc.size(); ++i)
				{
					store->insert(pmc[i]);
				}

				store->commitTransaction();
			}
		}
		
		void serialize(Json::Value&);
		void deserialize(Json::Value&);
	private:
		void addCreatedMutation(const char * name, const GUID& pkey);
		void addMutationOnField(const char * tblname, const std::string&, const std::string&, const GUID& g);
		void addUploadRequest(const char * name, const GUID& g, const std::string& mime, const std::string& path, const std::string& thumb);
		bool requestingUpload;
		std::vector<Mutation> pendingUploads;
	};


	//Any model with this backing will create mutations. These mutations
	//will be persisted to the datastore on a call to .update or .delete
	class BufferedMutationBacking : public NullBacking
	{
	public:
		~BufferedMutationBacking();

		template<typename Self, typename T>
		typename boost::enable_if_c<Metadata<T>::requiresLink, bool>::type
		mutationReplace(Self& self, const char * field, const T& prev, const T& next)
		{
			if (prev.getPrimaryKey() != next.getPrimaryKey())
			{
				mutationReplace(self, field, prev.getPrimaryKey(), next.getPrimaryKey());
			}
			return true;
		}

		template<typename Self, typename T>
		typename boost::disable_if_c<Metadata<T>::requiresLink, bool>::type 
		mutationReplace(Self& self, const char * field, const T& prev, const T& next)
		{
			if (prev != next)
			{
				addMutationOnField(Metadata<Self>::name, field, boost::lexical_cast<std::string>(next), self.getPrimaryKey());
			}
			return true;
		}

		template<typename Self>
		bool mutationReplace(Self& self, const char * field, const GUID& prev, const GUID& next)
		{
			if (prev != next)
			{
				addMutationOnField(Metadata<Self>::name, field, stringFromGUID(next), self.getPrimaryKey());
			}
			return true;
		}

		template<typename Self>
		void deleted(Self& s)
		{
			addDeleteMutation(Metadata<Self>::name, s.getPrimaryKey());
		}

		template<typename Self>
		void inserted(Self& s)
		{
			addCreatedMutation(Metadata<Self>::name, s.getPrimaryKey());
		}

		template<typename Self>
		void updated(Self& s)
		{
		}

		//Removing a owned one-to-many creates a deferred deletion, full delete on update to datastore?
		template<typename Self, typename T>
		void mutationRemove(Self&, const char * field, const std::vector<T>& arr, T& removed)
		{
			ConstraintsForColumn constraints(field);
			Access::serializeTemporary<Self>(constraints);
			
			if (constraints.constraints.ownsChild)
			{
				DeferredDelete del;
				del.memberName = field;
				del.guid = removed.getPrimaryKey();
				deletes.push_back(del);
			}
		}

		template<typename Self, typename T>
		bool mutationReplace(Self& self, const char * field, const boost::optional<T>& prev, const boost::optional<T>& next)
		{
			if (!prev && next)
			{
				mutationReplace(self, field, T(), *next);
			}
			else if (prev && !next)
			{
				addMutationToNullOnField(Metadata<Self>::name, field, self.getPrimaryKey());
			}
			else if (prev && next && *prev != *next)
			{
				mutationReplace(self, field, *prev, *next);
			}
			return true;
		}

		template<typename Self>
		bool mutationReplace(Self& self, const char * field, const boost::optional<GUID>& prev, const boost::optional<GUID>& next)
		{
			if (!prev && next)
			{
				mutationReplace(self, field, GUID(), *next);
			} 
			else if (prev && !next)
			{
				addMutationToNullOnField(Metadata<Self>::name, field, self.getPrimaryKey()); 
			} 
			else if (prev && next && *prev != *next)
			{
				mutationReplace(self, field, *prev, *next);
			}
			return true;
		}

		template<typename T, typename Self>
		void setStore(Self& self, T * store, bool updateOrDelete = false)
		{
			if (updateOrDelete)
			{
				typename T::Transaction transact(*store);

				std::vector<Mutation> pmc;
				pmc.swap(pendingMutations);

				for (size_t i = 0; i < pmc.size(); ++i)
				{
					store->insert(pmc[i]);
				}

				for (size_t i = 0; i < deletes.size(); ++i)
				{
					DeferredExecute<T> executor(store, &deletes[i]);
					self.serialize(executor);
				}

				deletes.clear();
			}
		}

		void serialize(Json::Value&);
		void deserialize(Json::Value&);
	private:
		void addMutationToNullOnField(const char * tblname, const std::string& field, const GUID& g);
		void addMutationOnField(const char * tblname, const std::string&, const std::string&, const GUID& g);
		void addDeleteMutation(const char * tblname, const GUID& g);
		void addCreatedMutation(const char * tblname, const GUID& g);
		
		std::vector<Mutation> pendingMutations;
		std::vector<DeferredDelete> deletes;
	};
}

#endif
