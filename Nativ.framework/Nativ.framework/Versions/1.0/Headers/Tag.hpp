

/* This is generated code. Do not modify. */
#pragma once
#ifndef GENERATED_MODEL_Tag
#define GENERATED_MODEL_Tag

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



class Tag
{
	ORawrM::GUID pid;
	std::string name;

public:
		static const bool hasGeneratedKey = true;

private:


	ORawrM::NullBacking reserved_backing;


	friend class ORawrM::Access;
	friend class Comment;
	friend class TagBridge;
	friend class Image;
	friend class UploadMetadata;
	friend class Rating;
	friend class Static;

	Tag();
public:

	enum 
	{


		ImageTagsRelation = 131072, 
		ImageTagsBridgeRelation = 1073872896,

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
	void setName(const std::string &value);
	void setCleanName(const std::string &value); 
	const std::string& getName() const;

	template<typename Archive>
	void serialize(Archive& arch) 
	{
		arch.start(*this);

		arch.template registerBridgeRelationship<Tag, TagBridge, Image, Tag>(*this, 131072);

		ORawrM::Constraints constraint_for_pid = ORawrM::Constraints::none();
		constraint_for_pid.combine(ORawrM::Constraints::key());
		constraint_for_pid.combine(ORawrM::Constraints::none());
		arch & ::ORawrM::makeEntry("pid", pid, constraint_for_pid);
		ORawrM::Constraints constraint_for_name = ORawrM::Constraints::none();
		constraint_for_name.combine(ORawrM::Constraints::index());
		constraint_for_name.combine(ORawrM::Constraints::keyGenerationInput());
		arch & ::ORawrM::makeEntry("name", name, constraint_for_name);
		
		arch.end(*this);
	}

	const ORawrM::GUID& getPrimaryKey() const;
	void setPrimaryKey(const ORawrM::GUID& value); 
	void setCleanPrimaryKey(const ORawrM::GUID& value);

	void regenerateKey(); 
};

REGISTER_MODEL(Tag);

#endif
