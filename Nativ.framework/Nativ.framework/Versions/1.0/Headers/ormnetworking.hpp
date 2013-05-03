#include <boost/cstdint.hpp>
#include <vector>
#include "json/json.h"

#include "ormlogger.hpp"
#include "ormpredicate.hpp"
#include "ormjson.hpp"

#ifdef USING_CURL_NETWORK
#include <curl/curl.h>
#endif

#ifndef ORM_NETWORKING_HPP
#define ORM_NETWORKING_HPP

class Mutation;
class Tag;
class Comment;
class TagBridge;
class Image;
class UploadMetadata;
class Rating;
class Static;
namespace ORawrM
{
	class INetworkHandler;
	class INetworkCallbacks;

	enum NetworkError
	{
		NETWORK_ERROR_OK = 0,
		NETWORK_ERROR_FAILED_INIT,
		NETWORK_ERROR_NO_CONNECTION,
		NETWORK_ERROR_API_MISUSE,
		NETWORK_ERROR_INTERNAL, 
		NETWORK_ERROR_FILE_NOT_FOUND
	};

	class INetworkHandler
	{
	public:
		typedef boost::uint64_t ErrorHandle;

		INetworkHandler();
		virtual ~INetworkHandler();
		
		//Utility functions.
		void setLogger(ILogger * logger);
		ILogger * getLogger();

		bool hasAuthenticationToken() const;
		void setAuthenticationToken(const std::string& authenticationToken);
		void gatherJSON(Json::Value& out, const Json::Value& root, const std::string& operation) const;
		const std::string& getAuthenticationToken() const;

		//The output should be the ASCII hex representation of a SHA-1 digest.
		virtual ErrorHandle computeSHA1HashOfFile(const std::string& path, std::string& out) = 0;
		virtual ErrorHandle createDeviceIndependentResources() = 0;
		virtual ErrorHandle sendJSONData(const Json::Value& root, const std::string& operation, boost::optional<std::string> outputPath, INetworkCallbacks * callbacks) = 0;
		virtual ErrorHandle uploadFileWithJSON(const Json::Value& root, const std::string& localPath, const std::string& uid, INetworkCallbacks * callbacks) = 0;
	protected:
		ILogger * logger;
		std::string authenticationToken;

		//Default implementation of computeSHA1HashOfFile
		static ErrorHandle computeSHA1(const std::string& path, std::string& out); 
	};


	/*
		This is the network callback interface.
	*/
	class INetworkCallbacks
	{
	public:
		virtual ~INetworkCallbacks();
		virtual void onReceivedData(INetworkHandler * network, const Json::Value& value) = 0;
		virtual void onError(INetworkHandler * handle, INetworkHandler::ErrorHandle err, const std::string& msg, const Json::Value& data) = 0;
		virtual void finished(boost::optional<std::string> extras) = 0;
	};

	/*
		This adapts any class into an INetworkCallbacks instance.
	*/
	template<typename T>
	class NetworkCallbackWrapper : public INetworkCallbacks
	{
	public:
		NetworkCallbackWrapper(const T& in)
			:decor(in)
		{}

		void onError(INetworkHandler * handle, INetworkHandler::ErrorHandle err, const std::string& msg, const Json::Value& data)
		{
			decor.onError(handle, err, msg, data);
		}

		void onReceivedData(INetworkHandler * network, const Json::Value& value)
		{
			decor.onReceivedData(network, value);
		}

		void finished(boost::optional<std::string> extra)
		{
			decor.finished(extra);
			delete this;
		}
	private:
		T decor;
	};

	/*
		This is used for mutation uploads.
		Will delete successful mutations.
	*/
	template<typename DS, typename T>
	class MutationCallbackWrapper : public NetworkCallbackWrapper<T>
	{
	public:
		MutationCallbackWrapper(const T& in, const std::vector<Mutation>& mutations, DS * datastore)
			:NetworkCallbackWrapper<T>(in)
			,mutations(mutations)
			,datastore(datastore)
			,hadError(false)
		{}

		void onError(INetworkHandler * handle, INetworkHandler::ErrorHandle err, const std::string& msg, const Json::Value& data)
		{
			hadError = true;

			//Well, see which ones were denied.
			if (data.isArray())
			{
				deniedObjects.clear();
				deniedObjects.reserve(data.size());

				for (Json::Value::ArrayIndex i = 0; i < data.size(); ++i)
				{
					deniedObjects.push_back(GUIDFromString(data[i]["guid"].asString()));
				}
			}

			NetworkCallbackWrapper<T>::onError(handle, err, msg, data);
		}

		void onReceivedData(INetworkHandler * network, const Json::Value& value)
		{
			NetworkCallbackWrapper<T>::onReceivedData(network, value);
		}

		void finished(boost::optional<std::string> extra)
		{
			if (!hadError)
			{
				typename DS::Transaction transact(*datastore);
				for (size_t i = 0; i < mutations.size(); ++i)
				{
					datastore->remove(mutations[i]);
				}
			}
			else 
			{
				typename DS::Transaction transsact(*datastore);
				for (size_t i = 0; i < deniedObjects.size(); ++i)
				{
					//Remove all the offending mutations.
					ORawrM::KeyPredicate predicate;
					predicate.whereGUID("target", deniedObjects[i]);
#ifndef _WIN32
					datastore->template removeWhere<Mutation>(predicate);
#else
					datastore->removeWhere<Mutation>(predicate);
#endif
					ORAWRM_LOG_MESSAGE(datastore->getLogger(), "Mutations targeting " + stringFromGUID(deniedObjects[i]) +
						" were invalid, and will be removed from the next request. This may cause the user to lose data.");
				}

				ORAWRM_LOG_ERROR(datastore->getLogger(), "Mutations were not sent correctly, so the same mutations"
					" will be sent in the next request as well");
			}

			NetworkCallbackWrapper<T>::finished(extra);
		}
	private:
		bool hadError;
		DS * datastore;

		std::vector<Mutation> mutations;
		std::vector<GUID> deniedObjects;
	};


	//Used in the template below
	template<typename T, typename A, typename O>
	size_t insertOrUpdateModelsIntoDatastoreFromJson(const Json::Value& array, A& ds, O output)
	{
		typename A::Transaction transaction(ds);
		size_t count = 0;
		for (size_t i = 0; i < array.size(); ++i)
		{
			bool success = false;
			T value = ORawrM::readSingleValue<T>(array[static_cast<int>(i)], &success, ds.getLogger());

			if (success)
			{
				KeyPredicate mutationLookup;
				mutationLookup.whereGUID("target", value.getPrimaryKey());

				size_t outstandingMutations = ds.template count<Mutation>(mutationLookup);
				if (outstandingMutations)
				{
					std::stringstream warn;
					warn << outstandingMutations << " outstanding mutations exist on " << Metadata<T>::name << " with primary key = " 
						<< value.getPrimaryKey() << ", the server changes to this object will be ignored.";

					ORAWRM_LOG_WARNING(ds.getLogger(), warn.str());
				} 
				else 
				{
					if (ds.template exists<T>(value.getPrimaryKey()))
					{
						T target = ds.template select<T>(value.getPrimaryKey());
						ORawrM::readIntoValue(target, array[static_cast<int>(i)], &success, ds.getLogger());
						ds.update(target);

						*output = target;
					} 
					else 
					{
						ds.insert(value, false);
						*output = value;
					}

					++output;
					++count;
				}
			}
		}
		return count;
	}


	/*
		This takes in something that implements INetworkCallbacks, and extends it
		For every model that it receives, it will invoke operator()(Datastore *ds, const std::string& modelName, std::vector<Model> models, size_t count);
	*/
	template<typename DS, typename C>
	struct NetworkInsertion : public INetworkCallbacks
	{
		NetworkInsertion(DS& d, C c)
			:datastore(d)
			,callbacks(c)
		{}

		void onReceivedData(INetworkHandler * network, const Json::Value& value)
		{
			callbacks.onReceivedData(network, value);

			const std::vector<std::string> members = value.getMemberNames();
			for (size_t i = 0; i < members.size(); ++i)
			{
				std::string member = members[i];
				std::string model = value[member]["model"].asString();

				if (model == "Tag")
				{
					std::vector<Tag> result;
					insertOrUpdateModelsIntoDatastoreFromJson<Tag>(value[member]["slice"], datastore, std::back_inserter(result));
					size_t count = value[member]["total"].asUInt();

					callbacks(datastore, member, result, count);
				}
				if (model == "Comment")
				{
					std::vector<Comment> result;
					insertOrUpdateModelsIntoDatastoreFromJson<Comment>(value[member]["slice"], datastore, std::back_inserter(result));
					size_t count = value[member]["total"].asUInt();

					callbacks(datastore, member, result, count);
				}
				if (model == "TagBridge")
				{
					std::vector<TagBridge> result;
					insertOrUpdateModelsIntoDatastoreFromJson<TagBridge>(value[member]["slice"], datastore, std::back_inserter(result));
					size_t count = value[member]["total"].asUInt();

					callbacks(datastore, member, result, count);
				}
				if (model == "Image")
				{
					std::vector<Image> result;
					insertOrUpdateModelsIntoDatastoreFromJson<Image>(value[member]["slice"], datastore, std::back_inserter(result));
					size_t count = value[member]["total"].asUInt();

					callbacks(datastore, member, result, count);
				}
				if (model == "UploadMetadata")
				{
					std::vector<UploadMetadata> result;
					insertOrUpdateModelsIntoDatastoreFromJson<UploadMetadata>(value[member]["slice"], datastore, std::back_inserter(result));
					size_t count = value[member]["total"].asUInt();

					callbacks(datastore, member, result, count);
				}
				if (model == "Rating")
				{
					std::vector<Rating> result;
					insertOrUpdateModelsIntoDatastoreFromJson<Rating>(value[member]["slice"], datastore, std::back_inserter(result));
					size_t count = value[member]["total"].asUInt();

					callbacks(datastore, member, result, count);
				}
				if (model == "Static")
				{
					std::vector<Static> result;
					insertOrUpdateModelsIntoDatastoreFromJson<Static>(value[member]["slice"], datastore, std::back_inserter(result));
					size_t count = value[member]["total"].asUInt();

					callbacks(datastore, member, result, count);
				}
			}
		}

		void onError(INetworkHandler * handle, INetworkHandler::ErrorHandle err, const std::string& msg, const Json::Value& data)
		{
			ORAWRM_LOG_ERROR(handle->getLogger(), msg);
			callbacks.onError(handle, err, msg, data);
		}

		void finished(boost::optional<std::string> v)
		{
			callbacks.finished(v);
			delete this;
		}

		DS& datastore;
		C callbacks;
	};

	Json::Value queryToJson(const KeyPredicate& q, const std::string& modelName, const Subquery *, size_t);

//Networking calls.
	INetworkHandler::ErrorHandle uploadMutations(const std::vector<Mutation>& mutation, INetworkHandler * network, INetworkCallbacks * cbs);
	INetworkHandler::ErrorHandle authenticateWithUser(const std::string& user, const std::string& pwrd, INetworkHandler * network, INetworkCallbacks * cbs);

	template<typename T, typename U>
	INetworkHandler::ErrorHandle executeRemoteQuery(const KeyPredicate& query, const std::string& modelName, const Subquery * subs, size_t subCount, INetworkHandler * network, T& datastore, U typedCallbacks)
	{
		Json::Value q = queryToJson(query, modelName, subs, subCount);
		
		NetworkInsertion<T, U> * cbs = new NetworkInsertion<T, U>(datastore, typedCallbacks);
		return network->sendJSONData(q, "query", boost::none, cbs);
	}

	struct ThumbnailPoint
	{
		size_t x;
		size_t y;
	};

	void uploadFile(const std::string& localPath, const std::string& mime, const ORawrM::GUID& identifier, const ThumbnailPoint * point, INetworkHandler * handler, INetworkCallbacks * cbs);

	struct DownloadParameters
	{
		DownloadParameters(boost::uint16_t x, boost::uint16_t y, const std::string& quality);
		boost::uint16_t screenWidth;
		boost::uint16_t screenHeight;

		std::string connectionQuality;
	};


	INetworkHandler::ErrorHandle downloadFileToPath(ORawrM::GUID staticUID, const std::string& mime, const std::string& outputPath, const DownloadParameters& params, INetworkHandler * network, INetworkCallbacks * cbs);

	class NullNetwork : public INetworkHandler
	{
	public:
		ErrorHandle createDeviceIndependentResources();
		ErrorHandle computeSHA1HashOfFile(const std::string&, std::string& out);
		ErrorHandle uploadFileWithJSON(const Json::Value&, const std::string& file, const std::string& uid, INetworkCallbacks *);
		ErrorHandle sendJSONData(const Json::Value&, const std::string&, boost::optional<std::string>, INetworkCallbacks *);
	};

#ifdef USING_CURL_NETWORK
	class CURLNetwork : public INetworkHandler
	{
	public:
		CURLNetwork(const std::string& baseURI, size_t);
		~CURLNetwork(); 
		
		ErrorHandle createDeviceIndependentResources(); 
		ErrorHandle computeSHA1HashOfFile(const std::string& path, std::string& out);
		ErrorHandle uploadFileWithJSON(const Json::Value&, const std::string& file, const std::string& uid, INetworkCallbacks *);
		ErrorHandle sendJSONData(const Json::Value& root, const std::string& operation, boost::optional<std::string>, INetworkCallbacks * cbs);
	private:
		CURL * curlHandle;

		void regenerateCurlHandle();

		std::string baseURI;
		size_t port;
	};
#endif
}

#endif

