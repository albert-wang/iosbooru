

/* This is generated code. Do not modify. */
#pragma once
#ifndef GENERATED_MODEL_UploadMetadata
#define GENERATED_MODEL_UploadMetadata

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



class UploadMetadata
{
	ORawrM::GUID pid;
	ORawrM::GUID imageGUID;
	std::string uploadedBy;
	std::string originalExtension;

public:
		static const bool hasGeneratedKey = false;

private:


	ORawrM::NullBacking reserved_backing;


	friend class ORawrM::Access;
	friend class Tag;
	friend class Comment;
	friend class TagBridge;
	friend class Image;
	friend class Rating;
	friend class Static;

	UploadMetadata();
public:

	enum 
	{



		RelationCount
	};

	static std::string getRelationName(size_t id);
	static size_t getRelationID(const std::string& name);

	void deleted();
	ORawrM::NullBacking& getBacking();

	template<typename T>
	void setBackingStore(ORawrM::Datastore<T> * ds) 
	{
		reserved_backing.setStore(*this, ds);
	}

	void setPid(const ORawrM::GUID &value);
	void setCleanPid(const ORawrM::GUID &value); 
	const ORawrM::GUID& getPid() const;
	void setImageGUID(const ORawrM::GUID &value);
	void setCleanImageGUID(const ORawrM::GUID &value); 
	const ORawrM::GUID& getImageGUID() const;
	void setUploadedBy(const std::string &value);
	void setCleanUploadedBy(const std::string &value); 
	const std::string& getUploadedBy() const;
	void setOriginalExtension(const std::string &value);
	void setCleanOriginalExtension(const std::string &value); 
	const std::string& getOriginalExtension() const;

	template<typename Archive>
	void serialize(Archive& arch) 
	{
		arch.start(*this);


		ORawrM::Constraints constraint_for_pid = ORawrM::Constraints::none();
		constraint_for_pid.combine(ORawrM::Constraints::key());
		arch & ::ORawrM::makeEntry("pid", pid, constraint_for_pid);
		ORawrM::Constraints constraint_for_imageGUID = ORawrM::Constraints::none();
		constraint_for_imageGUID.combine(ORawrM::Constraints::index());
		arch & ::ORawrM::makeEntry("imageGUID", imageGUID, constraint_for_imageGUID);
		ORawrM::Constraints constraint_for_uploadedBy = ORawrM::Constraints::none();
		arch & ::ORawrM::makeEntry("uploadedBy", uploadedBy, constraint_for_uploadedBy);
		ORawrM::Constraints constraint_for_originalExtension = ORawrM::Constraints::none();
		arch & ::ORawrM::makeEntry("originalExtension", originalExtension, constraint_for_originalExtension);
		
		arch.end(*this);
	}

	const ORawrM::GUID& getPrimaryKey() const;
	void setPrimaryKey(const ORawrM::GUID& value); 
	void setCleanPrimaryKey(const ORawrM::GUID& value);

	void regenerateKey(); 
};

REGISTER_MODEL(UploadMetadata);

#endif
