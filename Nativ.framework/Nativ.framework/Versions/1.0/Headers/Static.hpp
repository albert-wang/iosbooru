

/* This is generated code. Do not modify. */
#pragma once
#ifndef GENERATED_MODEL_Static
#define GENERATED_MODEL_Static

#include <string>
#include <boost/cstdint.hpp>
#include <boost/optional.hpp>
#include <boost/lexical_cast.hpp>
#include <algorithm>
#include <map>

#include "forwardenum.hpp"
#include "ormbacking.hpp"
#include "ormcore.hpp"
#include "Mutation.hpp"



class Static
{
	ORawrM::GUID pid;
	boost::optional< boost::int64_t > lastUpdatedTimestamp;
	std::string mime;
	std::string path;
	std::string SHA1Hash;
	std::string thumb;

public:
		static const bool hasGeneratedKey = false;

private:


	ORawrM::UploadRequestBacking reserved_backing;


	friend class ORawrM::Access;
	friend class Tag;
	friend class Comment;
	friend class TagBridge;
	friend class Image;
	friend class UploadMetadata;
	friend class Rating;

	Static();
public:

	enum 
	{



		RelationCount
	};

	static std::string getRelationName(size_t id);
	static size_t getRelationID(const std::string& name);

	void deleted();
	ORawrM::UploadRequestBacking& getBacking();

	template<typename T>
	void setBackingStore(ORawrM::Datastore<T> * ds) 
	{
		reserved_backing.setStore(*this, ds);
	}

	void setPid(const ORawrM::GUID &value);
	void setCleanPid(const ORawrM::GUID &value); 
	const ORawrM::GUID& getPid() const;
	void setLastUpdatedTimestamp(const boost::optional< boost::int64_t > &value);
	void setCleanLastUpdatedTimestamp(const boost::optional< boost::int64_t > &value); 
	const boost::optional< boost::int64_t >& getLastUpdatedTimestamp() const;
	void setMime(const std::string &value);
	void setCleanMime(const std::string &value); 
	const std::string& getMime() const;
	void setPath(const std::string &value);
	void setCleanPath(const std::string &value); 
	const std::string& getPath() const;
	void setSHA1Hash(const std::string &value);
	void setCleanSHA1Hash(const std::string &value); 
	const std::string& getSHA1Hash() const;
	void setThumb(const std::string &value);
	void setCleanThumb(const std::string &value); 
	const std::string& getThumb() const;

	template<typename Archive>
	void serialize(Archive& arch) 
	{
		arch.start(*this);


		ORawrM::Constraints constraint_for_pid = ORawrM::Constraints::none();
		constraint_for_pid.combine(ORawrM::Constraints::key());
		arch & ::ORawrM::makeEntry("pid", pid, constraint_for_pid);
		ORawrM::Constraints constraint_for_lastUpdatedTimestamp = ORawrM::Constraints::none();
		constraint_for_lastUpdatedTimestamp.combine(ORawrM::Constraints::nullable());
		arch & ::ORawrM::makeEntry("lastUpdatedTimestamp", lastUpdatedTimestamp, constraint_for_lastUpdatedTimestamp);
		ORawrM::Constraints constraint_for_mime = ORawrM::Constraints::none();
		arch & ::ORawrM::makeEntry("mime", mime, constraint_for_mime);
		ORawrM::Constraints constraint_for_path = ORawrM::Constraints::none();
		constraint_for_path.combine(ORawrM::Constraints::serverOnly());
		constraint_for_path.combine(ORawrM::Constraints::createsUploadRequest());
		arch & ::ORawrM::makeEntry("path", path, constraint_for_path);
		ORawrM::Constraints constraint_for_SHA1Hash = ORawrM::Constraints::none();
		arch & ::ORawrM::makeEntry("SHA1Hash", SHA1Hash, constraint_for_SHA1Hash);
		ORawrM::Constraints constraint_for_thumb = ORawrM::Constraints::none();
		constraint_for_thumb.combine(ORawrM::Constraints::serverOnly());
		arch & ::ORawrM::makeEntry("thumb", thumb, constraint_for_thumb);
		
		arch.end(*this);
	}

	const ORawrM::GUID& getPrimaryKey() const;
	void setPrimaryKey(const ORawrM::GUID& value); 
	void setCleanPrimaryKey(const ORawrM::GUID& value);

	void regenerateKey(); 
};

REGISTER_MODEL(Static);

#endif
