

/* This is generated code. Do not modify. */
#pragma once
#ifndef GENERATED_MODEL_TagBridge
#define GENERATED_MODEL_TagBridge

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



class TagBridge
{
	ORawrM::GUID pid;
	ORawrM::GUID image_id;
	ORawrM::GUID tag_id;

public:
		static const bool hasGeneratedKey = false;

private:


	ORawrM::NullBacking reserved_backing;

	void reserved_setFirst(const ORawrM::GUID& f);
	void reserved_setSecond(const ORawrM::GUID& s);

	friend class ORawrM::Access;
	friend class Tag;
	friend class Comment;
	friend class Image;
	friend class UploadMetadata;
	friend class Rating;
	friend class Static;

	TagBridge();
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
	void setImage_id(const ORawrM::GUID &value);
	void setCleanImage_id(const ORawrM::GUID &value); 
	const ORawrM::GUID& getImage_id() const;
	void setTag_id(const ORawrM::GUID &value);
	void setCleanTag_id(const ORawrM::GUID &value); 
	const ORawrM::GUID& getTag_id() const;

	template<typename Archive>
	void serialize(Archive& arch) 
	{
		arch.start(*this);


		ORawrM::Constraints constraint_for_pid = ORawrM::Constraints::none();
		constraint_for_pid.combine(ORawrM::Constraints::key());
		arch & ::ORawrM::makeEntry("pid", pid, constraint_for_pid);
		ORawrM::Constraints constraint_for_image_id = ORawrM::Constraints::none();
		constraint_for_image_id.combine(ORawrM::Constraints::references("Image"));
		constraint_for_image_id.combine(ORawrM::Constraints::index());
		arch & ::ORawrM::makeEntry("image_id", image_id, constraint_for_image_id);
		ORawrM::Constraints constraint_for_tag_id = ORawrM::Constraints::none();
		constraint_for_tag_id.combine(ORawrM::Constraints::references("Tag"));
		constraint_for_tag_id.combine(ORawrM::Constraints::index());
		arch & ::ORawrM::makeEntry("tag_id", tag_id, constraint_for_tag_id);
		
		arch.end(*this);
	}

	const ORawrM::GUID& getPrimaryKey() const;
	void setPrimaryKey(const ORawrM::GUID& value); 
	void setCleanPrimaryKey(const ORawrM::GUID& value);

	void regenerateKey(); 
};

REGISTER_MODEL(TagBridge);

REGISTER_BRIDGE_MODEL(TagBridge, Image, Tag);
#endif
