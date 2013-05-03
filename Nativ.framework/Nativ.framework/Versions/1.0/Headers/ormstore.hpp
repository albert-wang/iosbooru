/* Copyright (c) 2011 Ironclad Mobile, Inc. All rights reserved */

#ifndef ICM_ORMSTORE_HPP
#define ICM_ORMSTORE_HPP
#pragma once

#include <boost/utility/enable_if.hpp>

#include "ormcore.hpp"
#include "ormpredicate.hpp"
#include "ormlogger.hpp"
#include "ormnetworking.hpp"
#include "ormstorevisitors.hpp"
#include "forwardenum.hpp"
#include "Mutation.hpp"


class Static;
namespace ORawrM
{
	template<typename Adapter>
	class Datastore
	{
		typedef typename Adapter::ModelCacheEntry CacheEntry;
		static const size_t MAXIMUM_CACHE_ENTRIES = 8;
	public:
		typedef typename Adapter::OpenParameters OpenParameters;

		class Transaction
		{
		public:
			explicit Transaction(Datastore& ds)
				:ds(ds)
				,commit(true)
			{
				ds.startTransaction();
			}
			
			/*
				Rollback and Commit are mutually exclusive.
			*/
			void commitTransaction()
			{
				if (commit) 
				{
					ds.commitTransaction();
					commit = false;
				}
			}

			void rollbackTransaction()
			{
				if (commit)
				{
					ds.rollbackTransaction();
					commit = false;
				}
			}

			~Transaction()
			{
				commitTransaction();
			}

		private:
			Datastore& ds;
			bool commit;
		};

		Datastore()
			:caches(new CacheEntry[MAXIMUM_CACHE_ENTRIES])
			,logger(nullLogger())
			,networkHandler(new NullNetwork())
		{
			seed();
			adapter.setLogger(logger);
			networkHandler->setLogger(logger);
		}

		explicit Datastore(ILogger * logger)
			:caches(new CacheEntry[MAXIMUM_CACHE_ENTRIES])
			,logger(logger)
			,networkHandler(new NullNetwork())
		{
			seed();
			adapter.setLogger(logger);
			networkHandler->setLogger(logger);
		} 

		explicit Datastore(const Adapter& a)
			:adapter(a)
			,caches(new CacheEntry[MAXIMUM_CACHE_ENTRIES])
			,logger(nullLogger())
			,networkHandler(new NullNetwork())
		{
			seed();
			adapter.setLogger(logger);
			networkHandler->setLogger(logger);
		}

		~Datastore()
		{
			delete logger;
			delete [] caches;
			delete networkHandler;
		}

		//This should be used only for testing purposes.
		Adapter& getAdapter() { return adapter; }

		void setLogger(ILogger * l)
		{
			assert(l);
			delete logger;

			emplaceLogger(l);
		}

		ILogger * emplaceLogger(ILogger * l)
		{
			ILogger * original = logger;

			logger = l;
			adapter.setLogger(logger);
			networkHandler->setLogger(logger);

			return original;
		}

		ILogger * getLogger()
		{
			return logger;
		}

		void startTransaction()
		{
#ifndef DISABLE_TRANSACTIONS
			adapter.startTransaction();
#endif
		}

		void commitTransaction()
		{
#ifndef DISABLE_TRANSACTIONS
			adapter.commitTransaction();
#endif
		}

		void rollbackTransaction()
		{
#ifndef DISABLE_TRANSACTIONS
			adapter.rollbackTransaction();
#endif
		}

		void setDeviceID(const char * id, size_t length)
		{
			assert(length >= 2);
			deviceID[0] = id[0];
			deviceID[1] = id[1];

			for (size_t i = 2; i < length; i++)
			{
				deviceID[i % 2] ^= id[i];
			}

			if (length % 2 == 1)
			{
				deviceID[1] ^= 0xDA;
			}
		}

		void open(const OpenParameters& parameters)
		{
			adapter.connect(parameters);
			adapter.associateCaches(caches, MAXIMUM_CACHE_ENTRIES);
		}

		template<typename T>
		T create()
		{
			BOOST_STATIC_ASSERT(Metadata<T>::requiresLink);

			T result = Access::create<T>();

			if (!T::hasGeneratedKey) {
				insert(result);
			}

			return result;
		}

		template<typename T>
		T createWithGUID(const GUID& in)
		{
			BOOST_STATIC_ASSERT(Metadata<T>::requiresLink);

			T result = Access::create<T>();

			if (!T::hasGeneratedKey) {
				result.setCleanPrimaryKey(in);
				insert(result, false);
			}

			return result;
		}

		template<typename T>
		void registerModel()
		{
			BOOST_STATIC_ASSERT(Metadata<T>::requiresLink);

			const char * name = Metadata<T>::name;
			std::stringstream mainbuffer;

			typename Adapter::ModelGenerator generator(name, &mainbuffer);
			generator.setLogger(logger);
			
			Access::serializeTemporary<T>(generator);

			const std::vector<std::string>& qs = generator.getModelString();
			for (size_t i = 0; i < qs.size(); ++i)
			{
				adapter.execute(qs[i].c_str());
			}
		}

		void afterRegistration()
		{
			adapter.afterRegistration();
		}

		void updateSchema()
		{
			typedef typename Adapter::SchemaMigration SchemaMigration;
			SchemaMigration migrate(&adapter);
			migrate.setLogger(logger);

			typedef typename Adapter::ModelGenerator ModelGenerator;
			typedef typename Adapter::DropIndices    DropIndices;

			size_t currentVersion = migrate.getSchemaVersion(1);
			
			switch(currentVersion)
			{
				default:
					break;
			}
		}

		template<typename T>
		bool update(T& t) //Returns true if the update succeeded, or if an update was not required.
		{
			BOOST_STATIC_ASSERT(Metadata<T>::requiresLink);

			CacheEntry& cache = caches[Metadata<T>::modelID];

			if (T::hasGeneratedKey) {
				if (t.getPrimaryKey() == INVALID_GUID) {
					return false;
				}

				if (!exists(t)) {
					insert(t, false);
				}
			}

			std::stringstream out;
			typename Adapter::UpdateGenerator update(Metadata<T>::name, &out, adapter, cache);
			update.setLogger(logger);
			if (update.needsSerialize())
			{
				t.serialize(update);
				update.finalize();
			}


			t.getBacking().preupdate(t);

			typename Adapter::UpdateQuery query(adapter, cache);
			query.setLogger(logger);
			t.serialize(query);

			bool result = query.finalize();
			if (result)
			{
				t.getBacking().updated(t);
				t.getBacking().setStore(t, this, true);
			}
			return result;
		}

		template<typename T>
		void remove(const GUID& guid)
		{
			BOOST_STATIC_ASSERT(Metadata<T>::requiresLink);

			CacheEntry& cache = caches[Metadata<T>::modelID];

			std::stringstream out;
			typename Adapter::DeleteGenerator query(Metadata<T>::name, &out, adapter, cache);
			query.setLogger(logger);
			
			if (query.needsSerialize())
			{
				Access::serializeTemporary<T>(query);
				query.finalize();
			}

			typename Adapter::DeleteQuery del(adapter, cache);
			del.setLogger(logger);

			del.setTarget(guid);
			del.finalize();
		}

		//Deletes a given object. This one cascades.
		template<typename T>
		void remove(T& in)
		{
			Transaction trans(*this);

			BOOST_STATIC_ASSERT(Metadata<T>::requiresLink);

			in.deleted();
			in.getBacking().setStore(in, this, true);

			remove<T>(in.getPrimaryKey());

			CascadeDelete< Datastore<Adapter> > cascade(this);
			in.serialize(cascade);
		}

		//Deletes a range where a column matches a GUID.
		template<typename T>
		void removeWhere(const char * column, const GUID& targetGUID)
		{
			KeyPredicate predicate;
			predicate.whereGUID(column, targetGUID);

			removeWhere<T>(predicate);
		}

		template<typename T>
		void removeWhere(const KeyPredicate& predi)
		{
			BOOST_STATIC_ASSERT(Metadata<T>::requiresLink);

			CacheEntry& cache = caches[Metadata<T>::modelID];

			KeyPredicate pkey = predi;
			sanitize<T>(pkey.predicates, logger);

			GetPrimaryKeyName pkeyName;
			Access::serializeTemporary<T>(pkeyName);

			typename Adapter::KeyQuery q(pkey, pkeyName.name, Metadata<T>::name, adapter, cache, false, NULL, true);
			q.setLogger(logger);

			typename Adapter::KeyQueryMultiple placeholder;
			q.step(placeholder);
		}

		//Inherently not portable.
		template<typename T, typename Out>
		Out query(const char * q, Out o)
		{
			BOOST_STATIC_ASSERT(Metadata<T>::requiresLink);

			typename Adapter::ArbitraryQuery query(q, adapter);
			query.setLogger(logger);

			typename Adapter::KeyQueryMultiple m;

			T result = Access::create<T>();
			while(query.step(m))
			{
				m.setLogger(logger);
				result.serialize(m);
				result.setBackingStore(this);
				*o = result;
				++o;
			}
			return o;
		}

		std::string execute(const std::string& s)
		{
			return execute(s.c_str());
		}

		std::string execute(const char * c)
		{
			return adapter.execute(c);
		}

		template<typename T> 
		T select(const GUID& id)
		{
			T result = Access::create<T>();
			select<T>(primaryKeys<T>(id).limit(1), &result);
			return result;
		}

		//Sanitizes a set of predicates.
		template<typename T>
		bool sanitize(std::vector<ColumnPredicate>& preds, ILogger * log)
		{
			return adapter.template sanitizePredicates<T>(preds);
		}

		template<typename T>
		size_t count(const KeyPredicate& pkeyi)
		{
			BOOST_STATIC_ASSERT(Metadata<T>::requiresLink);

			CacheEntry& cache = caches[Metadata<T>::modelID];

			//Sanitize and query the where clauses.
			KeyPredicate pkey = pkeyi;
			sanitize<T>(pkey.predicates, logger);

			GetPrimaryKeyName pkeyName;
			Access::serializeTemporary<T>(pkeyName);

			GetBridgeRelationship bridge(pkey.relationID);
			if (pkey.isBridgeRelationship)
			{
				Access::serializeTemporary<T>(bridge);
			}

			typename Adapter::KeyQuery q(pkey, pkeyName.name, Metadata<T>::name, adapter, cache, true, &bridge);
			q.setLogger(logger);

			return q.count();
		}

		template<typename T, typename Out>
		Out select(const KeyPredicate& pkeyi, Out o)
		{
			BOOST_STATIC_ASSERT(Metadata<T>::requiresLink);

			CacheEntry& cache = caches[Metadata<T>::modelID];

			//Sanitize and query the where clauses.
			KeyPredicate pkey = pkeyi;
			sanitize<T>(pkey.predicates, logger);

			GetPrimaryKeyName pkeyName;
			Access::serializeTemporary<T>(pkeyName);

			GetBridgeRelationship bridge(pkey.relationID);
			if (pkey.isBridgeRelationship)
			{
				Access::serializeTemporary<T>(bridge);
			}

			typename Adapter::KeyQuery q(pkey, pkeyName.name, Metadata<T>::name, adapter, cache, false, &bridge, false);
			q.setLogger(logger);

			typename Adapter::KeyQueryMultiple m;
			
			T result = Access::create<T>();
			while(q.step(m))
			{
				m.setLogger(logger);
				result.serialize(m);

				result.setBackingStore(this);
				*o = result;
				++o;
			}

			return o;
		}
		
		template<typename T>
		bool exists(const GUID& id)
		{
			BOOST_STATIC_ASSERT(Metadata<T>::requiresLink);
			CacheEntry& cache = caches[Metadata<T>::modelID];
			
			std::stringstream out;
			typename Adapter::QueryGenerator query(Metadata<T>::name, &out, adapter, cache);
			query.setLogger(logger);

			if (query.needsSerialize())
			{
				Access::serializeTemporary<T>(query);
				query.finalize();
			}

			typename Adapter::Query select(adapter, cache);
			select.setLogger(logger);
			select.setTarget(id); 
			return select.exists();
		}

		//Returns true if this UID is in the database.
		template<typename T>
		bool exists(const T& t)
		{
			return exists<T>(t.getPrimaryKey());
		}

		//Used to insert NEW entries into the model. These are not checked for uniqueness or anything.
		template<typename T>
		void insert(T& entry, bool forceGen = true)
		{
			BOOST_STATIC_ASSERT(Metadata<T>::requiresLink);

			entry.setBackingStore(this);
			CacheEntry& cache = caches[Metadata<T>::modelID];
			std::stringstream out;
			
			typename Adapter::InsertGenerator query(Metadata<T>::name, &out, adapter, cache);
			query.setLogger(logger);

			if (query.needsSerialize())
			{
				entry.serialize(query);
				query.finalize();
			}

			if (entry.getPrimaryKey() == INVALID_GUID || forceGen)
			{
				entry.setCleanPrimaryKey(generateGUID(randomness, deviceID, Metadata<T>::modelID));
			}
		
			typename Adapter::InsertQuery insert(adapter, cache);
			insert.setLogger(logger);

			entry.serialize(insert);
			insert.finalize();

			entry.getBacking().inserted(entry);
		}

		//Returns true if the thing exists.
		template<typename T>
		bool insertOrUpdate(T& entry)
		{
			BOOST_STATIC_ASSERT(Metadata<T>::requiresLink);

			if (exists(entry))
			{
				update(entry);
				return true;
			} else 
			{
				insert(entry, false);
				return false;
			}
		}

		//Links two models, assuming that they have a bridge relationship.
		//The second argument is used for SFINAE disabling of this function.
		template<typename F, typename S>
		void link(const F& f, const S& s, const typename BridgeModelType<F, S>::value * = NULL)
		{
			typedef typename BridgeModelType<F, S>::value BridgeType;
			static const bool firstThenSecond = BridgeModelType<F, S>::forwardOrdering;

			Transaction transact(*this);

			if (f.getPrimaryKey() == INVALID_GUID)
			{
				ORAWRM_LOG_ERROR(logger, std::string("The first parameter of type ") + Metadata<F>::name + " had an invalid id.");
				ORAWRM_LOG_ERROR(logger, "This may cause unexpected behavior.");
			} 

			if (s.getPrimaryKey() == INVALID_GUID)
			{
				ORAWRM_LOG_ERROR(logger, std::string("The second parameter of type ") + Metadata<S>::name + " had an invalid id.");
				ORAWRM_LOG_ERROR(logger, "This may cause unexpected behavior.");
			}

			unlink(f, s);
			
			BridgeType br = create<BridgeType>();

			if (firstThenSecond)
			{
				Access::setFirst(br, f);
				Access::setSecond(br, s);
			} else 
			{
				Access::setFirst(br, s);
				Access::setSecond(br, f);
			}

			update(br);
			return;
		}

		//The inverse of the above - unlinks two values.
		template<typename F, typename S>
		void unlink(const F& f, const S& s, const typename BridgeModelType<F, S>::value * = NULL)
		{
			typedef typename BridgeModelType<F, S>::value BridgeType;

			GetDeclarationReferencing firstName(Metadata<F>::name);
			Access::serializeTemporary<BridgeType>(firstName);

			GetDeclarationReferencing secondName(Metadata<S>::name);
			Access::serializeTemporary<BridgeType>(secondName);

			Transaction transaction(*this);

			KeyPredicate p = KeyPredicate();
			p.whereGUID(firstName.result, f.getPrimaryKey());
			p.whereGUID(secondName.result, s.getPrimaryKey());

			removeWhere<BridgeType>(p);

			ORawrM::GUID proxyGUID = generateGUIDWithID(Metadata<Mutation>::modelID);
			Mutation mutation;
			mutation.setTableName(Metadata<BridgeType>::name);
			mutation.setTarget(proxyGUID);
			mutation.setField(firstName.result);
			mutation.setData(stringFromGUID(f.getPrimaryKey()));
			mutation.setOperation("Update");
			insert(mutation, true);

			mutation.setField(secondName.result);
			mutation.setData(stringFromGUID(s.getPrimaryKey()));
			insert(mutation, true);

			mutation.setOperation("Delete");
			mutation.setField("");
			mutation.setData(std::string("Bridged"));
			insert(mutation, true);
		}

		//Networking
		void setNetworkHandler(INetworkHandler * network)
		{
			delete networkHandler;
			networkHandler = network;
			if (!network)
			{
				networkHandler = new NullNetwork();
				ORAWRM_LOG_MESSAGE(logger, "Null pointer passed in, using to a null network handler.");
			}
			networkHandler->setLogger(logger);

			if (networkHandler->createDeviceIndependentResources() != NETWORK_ERROR_OK)
			{
				delete networkHandler;
				networkHandler = new NullNetwork();
				ORAWRM_LOG_MESSAGE(logger, "Failed initalization of device indepdendent resources. Reverting to a null network handler.");
			}
		}

		INetworkHandler * getNetworkHandler()
		{
			return networkHandler;
		}

		void setDownloadPathPrefix(const std::string& prefix)
		{
			pathPrefix = prefix;
		}

		const std::string getDownloadPathPrefix() const
		{
			return pathPrefix;
		}

		template<typename T, typename C>
		void externalQuery(const KeyPredicate& preds, const Subquery * q, size_t n, C c)
		{
			externalQuery(preds, q, n, Metadata<T>::name, c);
		}

		//The modelName parameter must be valid until the call to c.finished.
		template<typename C>
		void externalQuery(const KeyPredicate& preds, const Subquery * q, size_t n, const char * modelName, C c)
		{
			if (!hasAuthenticationToken())
			{
				ORAWRM_LOG_ERROR(logger, "Attempted to call externalQuery without first authenticating.");
				c.onError(networkHandler, 1, "You must authenticate before trying to call externalQuery", Json::Value());
				c.finished(boost::none);
				return;
			}

			ORawrM::executeRemoteQuery(preds, modelName, q, n, networkHandler, *this, c); 
		}

		//A combined local query and external queries
		template<typename T, typename Network, typename Local>
		void hybridQuery(const KeyPredicate& preds, const Subquery * q, size_t n, Network ncbs, Local lcbs)
		{
			if (!hasAuthenticationToken())
			{
				ORAWRM_LOG_MESSAGE(logger, "Calling hybridQuery without an authentication token will only do local queries.");
				ncbs.onError(networkHandler, 1, "Without an authentication token, hybrid query will only do local queries.", Json::Value());
				ncbs.finished(boost::none);
			} else
			{
				ORawrM::executeRemoteQuery(preds, Metadata<T>::name, q, n, networkHandler, *this, ncbs);
			}
			this->template selectWithSubqueries<T>(preds, q, q + n, lcbs);
		}

		template<typename Network, typename Local>
		void hybridQuery(const KeyPredicate& preds, const Subquery * q, size_t n, const char * modelName, Network ncbs, Local lcbs)
		{
			if (!hasAuthenticationToken())
			{
				ORAWRM_LOG_MESSAGE(logger, "Calling hybridQuery without an authentication token will only do local queries.");
				ncbs.onError(networkHandler, 1, "Without an authentication token, hybrid query will only do local queries.", Json::Value());
				ncbs.finished(boost::none);
			} else 
			{
				ORawrM::executeRemoteQuery(preds, modelName, q, n, networkHandler, *this, ncbs);
			}

			if (modelName == std::string("Tag"))
			{
				this->template selectWithSubqueries<Tag>(preds, q, q + n, lcbs);
			}
			if (modelName == std::string("Comment"))
			{
				this->template selectWithSubqueries<Comment>(preds, q, q + n, lcbs);
			}
			if (modelName == std::string("TagBridge"))
			{
				this->template selectWithSubqueries<TagBridge>(preds, q, q + n, lcbs);
			}
			if (modelName == std::string("Image"))
			{
				this->template selectWithSubqueries<Image>(preds, q, q + n, lcbs);
			}
			if (modelName == std::string("UploadMetadata"))
			{
				this->template selectWithSubqueries<UploadMetadata>(preds, q, q + n, lcbs);
			}
			if (modelName == std::string("Rating"))
			{
				this->template selectWithSubqueries<Rating>(preds, q, q + n, lcbs);
			}
			if (modelName == std::string("Static"))
			{
				this->template selectWithSubqueries<Static>(preds, q, q + n, lcbs);
			}
		}

		template<typename C>
		void authenticate(const std::string& user, const std::string& pwrd, C c)
		{
			INetworkCallbacks * cbs = new NetworkCallbackWrapper<C>(c);
			ORawrM::authenticateWithUser(user, pwrd, networkHandler, cbs);
		}

		bool hasAuthenticationToken() const
		{
			return networkHandler->hasAuthenticationToken();
		}

		template<typename C>
		void uploadMutations(C c)
		{
			if (!hasAuthenticationToken())
			{
				ORAWRM_LOG_ERROR(logger, "Calling uploadMutations without first having an authentication token is an error.");
				c.onError(networkHandler, 1, "You must authenticate before trying to call uploadMutations", Json::Value());
				c.finished(boost::none);
				return;
			}
			KeyPredicate query;
			std::vector<Mutation> mutations;

			select<Mutation>(query, std::back_inserter(mutations));

			INetworkCallbacks * cbs = new MutationCallbackWrapper<Datastore, C>(c, mutations, this);
			ORawrM::uploadMutations(mutations, networkHandler, cbs);
		}

		static std::string pathForStatic(const std::string& pathPrefix, const std::string& sha1, const DownloadParameters& params)
		{
			std::stringstream result;
			if (params.screenWidth != 0 && params.screenHeight != 0) 
			{
				result << pathPrefix << sha1 << "-" << params.screenWidth << "x" << params.screenHeight;
			} 
			else 
			{
				result << pathPrefix << sha1;
			}
			return result.str();
		}

		template<typename S>
		std::string pathForStatic(const S& s, const DownloadParameters& params)
		{
			BOOST_STATIC_ASSERT((boost::is_same<S, Static>::value));

			if (s.getSHA1Hash().empty()) 
			{
				if (!s.getPath().empty()) 
				{
					std::string sha;
					networkHandler->computeSHA1HashOfFile(s.getPath(), sha);

					return pathForStatic(pathPrefix, sha, params);
				} 
				else 
				{
					return "";
				}
			}
			
			return pathForStatic(pathPrefix, s.getSHA1Hash(), params);
		}

		template<typename S, typename C>
		void downloadFile(const S& s, const DownloadParameters& params, C c)
		{
			BOOST_STATIC_ASSERT((boost::is_same<S, Static>::value));

			std::string path = pathForStatic(s, params);
			downloadFile(s.getPrimaryKey(), s.getMime(), path, params, c);
		}

		template<typename C>
		void downloadFile(const ORawrM::GUID& id, const std::string& mime, const std::string& path, const DownloadParameters& params, C c)
		{
			if (!hasAuthenticationToken())
			{
				ORAWRM_LOG_MESSAGE(logger, "Calling downloadFile without first having an authentication token is an error.");
				c.onError(networkHandler, 1, "You must authnenticate before trying to call downloadFile", Json::Value());
				c.finished(boost::none);
				return;
			}
			NetworkInsertion<Datastore, C> * cbs = new NetworkInsertion<Datastore, C>(*this, c);
			ORawrM::downloadFileToPath(id, mime, path, params, networkHandler, cbs);
		}

		template<typename C>
		ORawrM::GUID uploadFile(const std::string& localPath, const std::string& mime, C c)
		{
			if (!hasAuthenticationToken())
			{
				ORAWRM_LOG_MESSAGE(logger, "Calling uploadFile without first having an authentication token is an error.");
				c.onError(networkHandler, 1, "You must authenticate before trying to call uploadFile", Json::Value());
				c.finished(boost::none);
				return ORawrM::INVALID_GUID;
			}

			NetworkInsertion<Datastore, C> * cbs = new NetworkInsertion<Datastore, C>(*this, c);
			ORawrM::GUID s = generateGUID(randomness, deviceID, 0xFF);
			ORawrM::uploadFile(localPath, mime, s, NULL, networkHandler, cbs);
			return s;
		}

		template<typename C>
		void sendCustomOperation(const std::string& operation, Json::Value& data, C c)
		{
			NetworkCallbackWrapper<C> * callbackWrapper = new NetworkCallbackWrapper<C>(c);
			networkHandler->sendJSONData(data, operation, boost::none, callbackWrapper);
		}

		//Generic generate GUID
		ORawrM::GUID generateGUIDWithID(size_t pid)
		{
			return generateGUID(randomness, deviceID, pid);
		}

		/*
			Callbacks must implement
				operator()(Datastore& ds, string queryname, std::vector<T>& buffer, size_t count)
		*/
		template<typename T, typename Iter, typename Callbacks>
		void selectWithSubqueries(const KeyPredicate& pred, Iter b, Iter e, Callbacks& typedCallbacks)
		{
			//How to return? :(
			std::vector<T> * primary = new std::vector<T>();
			this->template select<T>(pred, std::back_inserter(*primary));
			size_t count = this->template count<T>(pred);
			typedCallbacks(*this, Metadata<T>::name, *primary, count);

			//:3
			std::map<std::string, void *> results;
			std::map<std::string, std::string> modelTypes;

			results[Metadata<T>::name] = static_cast<void *>(primary);
			modelTypes[Metadata<T>::name] = Metadata<T>::name;
			
			while(b != e)
			{
				std::map<std::string, void *>::iterator rit = results.find(b->subgraph);
				if (rit == results.end())
				{
					getLogger()->log(ILogger::Error, "You have specified a subgraph that was not yet evaluated.", "<Generated File>", 0);
				}
				void * parent = rit->second;

				std::map<std::string, std::string>::iterator mit = modelTypes.find(b->subgraph);
				std::string parentType = mit->second;

				KeyPredicate subgraphPredicate;
				if (parentType == "Tag")
				{
					std::vector<Tag> * p = static_cast<std::vector<Tag> *>(parent);
					
					//Sometimes, the later parts of this can be generated with no content.
					//This will mean that p will be unused.
					//So, surpress that warning.
					(void)p;

					bool found = false;






					if (b->relation == "ImageTags")
					{
						found = true;
						std::vector<Image> * child = new std::vector<Image>();

						size_t rel = 131072;
						subgraphPredicate = relationKeys<Image>(rel, p->begin(), p->end());
						subgraphPredicate.limit(b->limit);
						subgraphPredicate.offset(b->offset);

						this->template select<Image>(subgraphPredicate, std::back_inserter(*child));
						results[b->name] = static_cast<void *>(child);
						modelTypes[b->name] = "Image";

						size_t count = this->template count<Image>(subgraphPredicate);
						typedCallbacks(*this, b->name, *child, count);
					} 

					if (b->relation == "ImageTagsBridge")
					{
						found = true;
						std::vector<TagBridge> * child = new std::vector<TagBridge>();

						size_t rel = 1073872896;
						subgraphPredicate = relationKeys<TagBridge>(rel, p->begin(), p->end());
						subgraphPredicate.limit(b->limit);
						subgraphPredicate.offset(b->offset);

						this->template select<TagBridge>(subgraphPredicate, std::back_inserter(*child));
						results[b->name] = static_cast<void *>(child);
						modelTypes[b->name] = "TagBridge";

						size_t count = this->template count<TagBridge>(subgraphPredicate);
						typedCallbacks(*this, b->name, *child, count);
					}

					if (!found)
					{
						ORAWRM_LOG_ERROR(logger, "The relation (" + b->relation + ") was not found in " + parentType + ".");
					}
				}
				if (parentType == "Comment")
				{
					std::vector<Comment> * p = static_cast<std::vector<Comment> *>(parent);
					
					//Sometimes, the later parts of this can be generated with no content.
					//This will mean that p will be unused.
					//So, surpress that warning.
					(void)p;

					bool found = false;



					if (b->relation == "parent_id") 
					{
						found = true;
						
						std::vector<Image> * child = new std::vector<Image>();

						//This would be better if we used the constants, 
						//But that would involve having a full definition of the model classes avaliable here
						//so we're using the generated constants instead. 

						size_t rel = ForwardImage_commentsRelation;
						subgraphPredicate = inverseRelation<Image>(rel, p->begin(), p->end());
						subgraphPredicate.limit(b->limit);
						subgraphPredicate.offset(b->offset);
						
						this->template select<Image>(subgraphPredicate, std::back_inserter(*child));
						results[b->name] = static_cast<void *>(child);
						modelTypes[b->name] = "Image";
						
						size_t count = this->template count<Image>(subgraphPredicate);
						typedCallbacks(*this, b->name, *child, count);
					}






					if (!found)
					{
						ORAWRM_LOG_ERROR(logger, "The relation (" + b->relation + ") was not found in " + parentType + ".");
					}
				}
				if (parentType == "TagBridge")
				{
					std::vector<TagBridge> * p = static_cast<std::vector<TagBridge> *>(parent);
					
					//Sometimes, the later parts of this can be generated with no content.
					//This will mean that p will be unused.
					//So, surpress that warning.
					(void)p;

					bool found = false;







					if (!found)
					{
						ORAWRM_LOG_ERROR(logger, "The relation (" + b->relation + ") was not found in " + parentType + ".");
					}
				}
				if (parentType == "Image")
				{
					std::vector<Image> * p = static_cast<std::vector<Image> *>(parent);
					
					//Sometimes, the later parts of this can be generated with no content.
					//This will mean that p will be unused.
					//So, surpress that warning.
					(void)p;

					bool found = false;




					if (b->relation == "comments")
					{
						found = true;
						
						std::vector<Comment> * child = new std::vector<Comment>();

						//This would be better if we used the constants, 
						//But that would involve having a full definition of the model classes avaliable here
						//so we're using the generated constants instead. 

						size_t rel = 50331652;
						subgraphPredicate = relationKeys<Comment>(rel, p->begin(), p->end());
						subgraphPredicate.limit(b->limit);
						subgraphPredicate.offset(b->offset);
						
						this->template select<Comment>(subgraphPredicate, std::back_inserter(*child));
						results[b->name] = static_cast<void *>(child);
						modelTypes[b->name] = "Comment";
						
						size_t count = this->template count<Comment>(subgraphPredicate);
						typedCallbacks(*this, b->name, *child, count);
					}

					if (b->relation == "ratings")
					{
						found = true;
						
						std::vector<Rating> * child = new std::vector<Rating>();

						//This would be better if we used the constants, 
						//But that would involve having a full definition of the model classes avaliable here
						//so we're using the generated constants instead. 

						size_t rel = 50331653;
						subgraphPredicate = relationKeys<Rating>(rel, p->begin(), p->end());
						subgraphPredicate.limit(b->limit);
						subgraphPredicate.offset(b->offset);
						
						this->template select<Rating>(subgraphPredicate, std::back_inserter(*child));
						results[b->name] = static_cast<void *>(child);
						modelTypes[b->name] = "Rating";
						
						size_t count = this->template count<Rating>(subgraphPredicate);
						typedCallbacks(*this, b->name, *child, count);
					}






					if (b->relation == "ImageTags")
					{
						found = true;
						std::vector<Tag> * child = new std::vector<Tag>();

						size_t rel = 131072;
						subgraphPredicate = relationKeys<Tag>(rel, p->begin(), p->end());
						subgraphPredicate.limit(b->limit);
						subgraphPredicate.offset(b->offset);

						this->template select<Tag>(subgraphPredicate, std::back_inserter(*child));
						results[b->name] = static_cast<void *>(child);
						modelTypes[b->name] = "Tag";

						size_t count = this->template count<Tag>(subgraphPredicate);
						typedCallbacks(*this, b->name, *child, count);
					} 

					if (b->relation == "ImageTagsBridge")
					{
						found = true;
						std::vector<TagBridge> * child = new std::vector<TagBridge>();

						size_t rel = 1073872896;
						subgraphPredicate = relationKeys<TagBridge>(rel, p->begin(), p->end());
						subgraphPredicate.limit(b->limit);
						subgraphPredicate.offset(b->offset);

						this->template select<TagBridge>(subgraphPredicate, std::back_inserter(*child));
						results[b->name] = static_cast<void *>(child);
						modelTypes[b->name] = "TagBridge";

						size_t count = this->template count<TagBridge>(subgraphPredicate);
						typedCallbacks(*this, b->name, *child, count);
					}

					if (!found)
					{
						ORAWRM_LOG_ERROR(logger, "The relation (" + b->relation + ") was not found in " + parentType + ".");
					}
				}
				if (parentType == "UploadMetadata")
				{
					std::vector<UploadMetadata> * p = static_cast<std::vector<UploadMetadata> *>(parent);
					
					//Sometimes, the later parts of this can be generated with no content.
					//This will mean that p will be unused.
					//So, surpress that warning.
					(void)p;

					bool found = false;








					if (!found)
					{
						ORAWRM_LOG_ERROR(logger, "The relation (" + b->relation + ") was not found in " + parentType + ".");
					}
				}
				if (parentType == "Rating")
				{
					std::vector<Rating> * p = static_cast<std::vector<Rating> *>(parent);
					
					//Sometimes, the later parts of this can be generated with no content.
					//This will mean that p will be unused.
					//So, surpress that warning.
					(void)p;

					bool found = false;



					if (b->relation == "image_id") 
					{
						found = true;
						
						std::vector<Image> * child = new std::vector<Image>();

						//This would be better if we used the constants, 
						//But that would involve having a full definition of the model classes avaliable here
						//so we're using the generated constants instead. 

						size_t rel = ForwardImage_ratingsRelation;
						subgraphPredicate = inverseRelation<Image>(rel, p->begin(), p->end());
						subgraphPredicate.limit(b->limit);
						subgraphPredicate.offset(b->offset);
						
						this->template select<Image>(subgraphPredicate, std::back_inserter(*child));
						results[b->name] = static_cast<void *>(child);
						modelTypes[b->name] = "Image";
						
						size_t count = this->template count<Image>(subgraphPredicate);
						typedCallbacks(*this, b->name, *child, count);
					}






					if (!found)
					{
						ORAWRM_LOG_ERROR(logger, "The relation (" + b->relation + ") was not found in " + parentType + ".");
					}
				}
				if (parentType == "Static")
				{
					std::vector<Static> * p = static_cast<std::vector<Static> *>(parent);
					
					//Sometimes, the later parts of this can be generated with no content.
					//This will mean that p will be unused.
					//So, surpress that warning.
					(void)p;

					bool found = false;










					if (!found)
					{
						ORAWRM_LOG_ERROR(logger, "The relation (" + b->relation + ") was not found in " + parentType + ".");
					}
				}
				++b;
			}

			std::map<std::string, void *>::iterator beg = results.begin();
			while (beg != results.end())
			{
				std::string type = modelTypes[beg->first];
				void * p = beg->second;

				if (type == "Tag")
				{
					delete static_cast<std::vector<Tag> *>(p);
				}
				if (type == "Comment")
				{
					delete static_cast<std::vector<Comment> *>(p);
				}
				if (type == "TagBridge")
				{
					delete static_cast<std::vector<TagBridge> *>(p);
				}
				if (type == "Image")
				{
					delete static_cast<std::vector<Image> *>(p);
				}
				if (type == "UploadMetadata")
				{
					delete static_cast<std::vector<UploadMetadata> *>(p);
				}
				if (type == "Rating")
				{
					delete static_cast<std::vector<Rating> *>(p);
				}
				if (type == "Static")
				{
					delete static_cast<std::vector<Static> *>(p);
				}
				++beg;
			}

			typedCallbacks.finished(boost::none);
		}
	
	private:
		void seed()
		{
			//Uninitalized memory for randomness.
			char random[sizeof(boost::int32_t)];
			boost::uint32_t cvalue = clock();
			for (size_t i = 0; i < sizeof(boost::int32_t); ++i)
			{
				reinterpret_cast<char *>(&cvalue)[i] ^= random[i];	  
			}
			randomness.seed(cvalue);

			//Set the device ID to random values for now
			deviceID[0] = randomness() & 0xFF;
			deviceID[1] = randomness() & 0xFF;
		}

		char deviceID[2];
		boost::mt19937 randomness;
		Adapter adapter;
		CacheEntry * caches;
		ILogger * logger;
		INetworkHandler * networkHandler;
		std::string pathPrefix;
	};
}

#endif

