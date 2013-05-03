/* Copyright (c) 2011 Ironclad Mobile, Inc. All rights reserved */

#include "json/json.h"
#include <boost/lexical_cast.hpp>
#include <string>
#include <map>
#include <vector>

#include "ormcore.hpp"
#include "ormlogger.hpp"

#ifndef ICM_ORMJSON_HPP
#define ICM_ORMJSON_HPP

namespace ORawrM
{
	//GUID serialization stuff, used for all javascript stuff.
	std::string stringFromGUID(const GUID& g);

	//TERMINATES ON FAILURE. (lol!)
	GUID GUIDFromString(const std::string&, bool * successful = NULL);

	//The no-allocation versions of the above functions.
	void stringFromGUID(const GUID& in, char * out, size_t length); //out must be at least 33 bytes long.
	
	struct JSONSerializer : public ArchiveAdapter<JSONSerializer>
	{
		JSONSerializer(Json::Value& node, size_t recursionDepth = 0);
		void operator()(const char * name, boost::int32_t& i, const Constraints& c);
		void operator()(const char * name, boost::int64_t& i, const Constraints& c);
	
		void operator()(const char * name, float& f, const Constraints& c);
		void operator()(const char * name, double& d, const Constraints& c);

		void operator()(const char * name, std::string& s, const Constraints& c);
		void operator()(const char * name, GUID& g, const Constraints& c);

		template<typename T>
		void operator()(const char * name, boost::optional<T>& o, const Constraints& c)
		{
			if (c.isServerOnly) 
			{
				return; 
			}

			if (!o)
			{
				root[name] = Json::Value::null;
			} else 
			{
				(*this)(name, *o, c);
			}
		}

		template<typename T>
		typename boost::enable_if_c<Metadata<T>::requiresLink>::type operator()(const char * name, std::vector<T>& v, const Constraints& c)
		{
			if (c.isServerOnly) 
			{
				return; 
			}

			if (recursionDepth > 0)
			{
				//Serialize the stuff within.
				for (size_t i = 0; i < v.size(); ++i)
				{
					Json::Value value; 
					JSONSerializer serializer(value, recursionDepth - 1);

					v[i].serialize(serializer);

					root[name].append(value);
				}
			} else 
			{
				for (size_t i = 0; i < v.size(); ++i)
				{
					root[name].append(stringFromGUID(v[i].getPrimaryKey()));
				}
			}
		}

		template<typename T>
		typename boost::enable_if_c<Metadata<T>::requiresLink>::type operator()(const char * name, T& v, const Constraints& c)
		{
			if (c.isServerOnly) 
			{
				return; 
			}

			if (recursionDepth > 0)
			{
				Json::Value value;
				JSONSerializer serializer(value, recursionDepth - 1);

				v.serialize(serializer);

				root[name] = value;
			} else 
			{
				GUID g = v.getPrimaryKey();
				(*this)(name, g, c);
			}
		}

		template<typename T>
		void start(T& u)
		{
		}

		Json::Value& root;
		size_t recursionDepth;
	};

	template<typename I>
	typename boost::disable_if<
		boost::is_pointer<typename std::iterator_traits<I>::value_type>,
		size_t>::type 
	serializeToJSONRaw(Json::Value& out, I begin, I end, size_t recursion = 0)
	{
		if (Metadata<typename std::iterator_traits<I>::value_type>::isForbidden)
		{
			//Cannot serialize forbidden types.
			out = Json::arrayValue;
			return 0;
		}

		int index = 0;
		while(begin != end)
		{
			JSONSerializer serializer(out[index], recursion);
			begin->serialize(serializer);
			++begin;
			++index;
		}
		return index;
	}

	template<typename I>
	typename boost::enable_if<
		boost::is_pointer<typename std::iterator_traits<I>::value_type>, 
		size_t>::type
	serializeToJSONRaw(Json::Value& out, I begin, I end, size_t recursion = 0)
	{
		typedef typename boost::remove_pointer<typename std::iterator_traits<I>::value_type>::type value_type;

		if (Metadata<value_type>::isForbidden)
		{
			out = Json::arrayValue;
			return 0;
		}

		int index = 0;
		while (begin != end)
		{
			JSONSerializer serializer(out[index], recursion);

			(*begin)->serialize(serializer);
			++begin;
			++index;
		}
		return index;
	}

	template<typename I>
	size_t serializeToJSON(Json::Value& out, I begin, I end, size_t recursion = 0)
	{
		Json::Value& proxy = out[
			Metadata<
				typename boost::remove_const<
					typename boost::remove_pointer<
						typename std::iterator_traits<I>::value_type
					>::type
				>::type
			>::name];

		return serializeToJSONRaw(proxy, begin, end, recursion);
	}

	template<typename I>
	size_t serializeToJSONRaw(Json::Value& out, I& v, size_t recurse = 0)
	{
		return serializeToJSON(out, boost::begin(v), boost::end(v), recurse);
	}

	template<typename I>
	size_t serializeToJSON(Json::Value& out, I& v, size_t recurse = 0)
	{
		return serializeToJSON(out, boost::begin(v), boost::end(v), recurse);
	}

	struct JSONReader : public ArchiveAdapter<JSONReader>
	{
		JSONReader(const Json::Value& node);

		ORawrM::GUID GUIDFromNode(const Json::Value&);

		void operator()(const char * name, boost::int32_t& i, const Constraints& c);
		void operator()(const char * name, boost::int64_t& i, const Constraints& c);
		
		void operator()(const char * name, float& f, const Constraints& c);
		void operator()(const char * name, double& d, const Constraints& c);

		void operator()(const char * name, std::string& s, const Constraints& c);
		void operator()(const char * name, GUID& g, const Constraints& c);

		template<typename T>
		void operator()(const char * name, boost::optional<T>& o, const Constraints& c)
		{
			if (c.isServerOnly)
			{
				return; 
			}

			if (root[name].isNull())
			{
				o = boost::none;
			} else 
			{
				o = T();
				(*this)(name, *o, c);
			}
		}

		template<typename T>
		MODEL_ENABLE(T) operator()(const char * name, std::vector<T>& v, const Constraints& c)
		{
			if (c.isServerOnly)
			{
				return; 
			}

			const Json::Value& arr = root[name];
			if (arr.isArray())
			{
				v.clear();
				v.reserve(arr.size());
				for (size_t i = 0; i < arr.size(); ++i)
				{
					const Json::Value& obj = arr[(int) i];
				
					T result = Access::create<T>();
					if (obj.isString()) // Its a GUID
					{
						GUID uid = GUIDFromNode(obj);	
						result.setCleanPrimaryKey(uid);
					} else if (obj.isObject())
					{
						GetPrimaryKeyName pkey;
						Access::serializeTemporary<T>(pkey);

						const Json::Value& arr = obj[pkey.name];

						if (!arr.isString())
						{
							if (!firstFailure) firstFailure = name;
						}

						GUID uid = GUIDFromNode(arr);
						result.setCleanPrimaryKey(uid);
					}

					v.push_back(result);
				}
			}
		}

		template<typename T>
		typename boost::enable_if_c<Metadata<T>::requiresLink>::type operator()(const char * name, T& v, const Constraints& c)
		{
			if (c.isServerOnly)
			{
				return; 
			}

			const Json::Value& obj = root[name];
			if (obj.isString()) // GUID
			{
				GUID result = GUIDFromNode(obj);
				v.setCleanPrimaryKey(result);
			} else if (obj.isObject())
			{
				GetPrimaryKeyName pkey;
				Access::serializeTemporary<T>(pkey);

				const Json::Value& arr = obj[pkey.name];
				if (!arr.isString())
				{
					if (!firstFailure) firstFailure = name;
				}

				GUID result = GUIDFromNode(arr);
				v.setCleanPrimaryKey(result);
			}
		}
		
		template<typename T>
		void start(T&)
		{
		}

		void checkNode(const char * name);

		const Json::Value& root;
		const char * firstFailure; 
	};

	template<typename T>
	size_t count(Json::Value& in)
	{
		const char * model = Metadata<T>::name;
		return in[model].size();
	}

	template<typename T>
	void readIntoValue(T& out, const Json::Value& in, bool * success, ILogger * log = NULL)
	{
		*success = false;

		T fallback = out;

		JSONReader reader(in);
		fallback.serialize(reader);

		if (reader.firstFailure) 
		{
			if (log) 
			{
				std::stringstream error;
				error << "Failed to parse JSON node: " << reader.firstFailure << ". This is the first failure only.";
				ORAWRM_LOG_ERROR(log, error.str());
			}
		} 
		else 
		{
			out = fallback;
			*success = true;
		}
	}

	template<typename T>
	T readSingleValue(const Json::Value& in, bool * success, ILogger * log = NULL)
	{
		*success = false;

		JSONReader reader(in);
		T result = Access::create<T>();

		readIntoValue(result, in, success, log);

		return result;
	}

	//returns how many were inserted.
	template<typename T, typename O>
	size_t readFromJSON(Json::Value& in, O output, ILogger * log = NULL) //The In value must be the root node.
	{
		size_t inserted = 0;
		const char * model = Metadata<T>::name;
		Json::Value& value = in[model];
		for (size_t i = 0; i < value.size(); ++i)
		{
			bool success = false;
			T result = readSingleValue<T>(value[(int)i], &success, log);

			if (success)
			{
				*output = result;
				++output;
				inserted++;
			}
		}
		return inserted;
	}
}

namespace ORawrM
{
	namespace Detail
	{
		template<typename I>
		void serializeBackingsToJSON(Json::Value& out, I begin, I end)
		{
			Json::Value& proxy = out[
				Metadata<
					typename boost::remove_const<
						typename boost::remove_pointer<
							typename std::iterator_traits<I>::value_type
						>::type
					>::type
				>::name];

			int index = 0;
			while (begin != end)
			{
				begin->getBacking().serialize(proxy[index]);
				++begin;
				++index;
			}
		}
	}

	//These will attempt to serialize backing state as well.
	//These are used for IPC and IPC-like communication, not network operations.
	template<typename I>
	void serializeToJSONWithBacking(Json::Value& out, I begin, I end, size_t depth = 0) {
		Json::Value& values = out["data"];
		serializeToJSON(values, begin, end, depth);
		
		Json::Value& backings = out["backings"];
		Detail::serializeBackingsToJSON(backings, begin, end);
	}

	template<typename T, typename O>
	void readFromJSONWithBacking(Json::Value& out, O output, ILogger * log = NULL) {
		Json::Value& values = out["data"];
		Json::Value& backings = out["backings"];

		const char * model = Metadata<T>::name;
		Json::Value& value = values[model];
		Json::Value& vback = backings[model];

		for (size_t i = 0; i < value.size(); ++i)
		{
			bool success = false;
			T result = readSingleValue<T>(value[(int)i], &success, log);
			result.getBacking().deserialize(vback[(int)i]);

			if (success)
			{
				*output = result;
				++output;
			}
		}
	}
}


//Mostly node utilities
namespace ORawrM
{
	struct CleanMutateWithMap : public ArchiveAdapter<CleanMutateWithMap>
	{
		typedef std::map<std::string, boost::optional<std::string> > PropertyMap;
	public:
		explicit CleanMutateWithMap(const PropertyMap& properties);
		
		template<typename T>
		typename boost::disable_if_c<Metadata<T>::requiresLink>::type operator()(const char * name, T& v, const Constraints&)
		{
			PropertyMap::iterator it = properties.find(name);
			if (it != properties.end())
			{
				try {
					v = boost::lexical_cast<T>(*it->second);
				} catch (boost::bad_lexical_cast& e) {
					error = std::string("Bad lexical cast on: ") + name + " was given: " + *it->second;
				}
				properties.erase(it);
			}
		}
		
		void operator()(const char * name, std::string&, const Constraints& c);
		void operator()(const char * name, GUID&, const Constraints&);

		template<typename T>
		void operator()(const char * name, boost::optional<T>& o, const Constraints& c)
		{
			PropertyMap::iterator it = properties.find(name);
			if (it != properties.end())
			{
				if (!it->second)
				{
					o = boost::none;
					properties.erase(it);
				}
				else 
				{
					o = T();
					(*this)(name, *o, c);
				}
			}
		}

		template<typename T>
		typename boost::enable_if_c<Metadata<T>::requiresLink>::type operator()(const char * name, T& value, const Constraints& c)
		{
			PropertyMap::iterator it = properties.find(name);
			if (it != properties.end())
			{
				value.setCleanPrimaryKey(GUIDFromString(*it->second));
				properties.erase(it);
			}
		}

		template<typename T>
		typename boost::enable_if_c<Metadata<T>::requiresLink>::type operator()(const char * name, std::vector<T>& value, const Constraints& c)
		{
			PropertyMap::iterator it = properties.find(name);
			if (it != properties.end())
			{
				error = std::string("You cannot mutate ") + name + std::string(", an array of objects with this call."); 
				properties.erase(it);
			}
		}

		template<typename U>
		void end(U& self)
		{
			self.regenerateKey();
			finalize();
		}

		void finalize();

		boost::optional<std::string> error;
	private:
		PropertyMap properties;
	};
}

#endif 

