

/* This is generated code. Do not modify. */
#pragma once
#ifndef GENERATED_MODEL_Comment
#define GENERATED_MODEL_Comment

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



class Comment
{
	ORawrM::GUID pid;
	ORawrM::GUID parent_id;
	boost::int64_t dateCreated;
	std::string contents;

public:
		static const bool hasGeneratedKey = false;

private:


	ORawrM::NullBacking reserved_backing;


	friend class ORawrM::Access;
	friend class Tag;
	friend class TagBridge;
	friend class Image;
	friend class UploadMetadata;
	friend class Rating;
	friend class Static;

	Comment();
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
	void setParent_id(const ORawrM::GUID &value);
	void setCleanParent_id(const ORawrM::GUID &value); 
	const ORawrM::GUID& getParent_id() const;
	void setDateCreated(const boost::int64_t &value);
	void setCleanDateCreated(const boost::int64_t &value); 
	const boost::int64_t& getDateCreated() const;
	void setContents(const std::string &value);
	void setCleanContents(const std::string &value); 
	const std::string& getContents() const;

	template<typename Archive>
	void serialize(Archive& arch) 
	{
		arch.start(*this);


		ORawrM::Constraints constraint_for_pid = ORawrM::Constraints::none();
		constraint_for_pid.combine(ORawrM::Constraints::key());
		arch & ::ORawrM::makeEntry("pid", pid, constraint_for_pid);
		ORawrM::Constraints constraint_for_parent_id = ORawrM::Constraints::none();
		constraint_for_parent_id.combine(ORawrM::Constraints::isParentReference(ForwardImage_commentsRelation));
		arch & ::ORawrM::makeEntry("parent_id", parent_id, constraint_for_parent_id);
		ORawrM::Constraints constraint_for_dateCreated = ORawrM::Constraints::none();
		arch & ::ORawrM::makeEntry("dateCreated", dateCreated, constraint_for_dateCreated);
		ORawrM::Constraints constraint_for_contents = ORawrM::Constraints::none();
		arch & ::ORawrM::makeEntry("contents", contents, constraint_for_contents);
		
		arch.end(*this);
	}

	const ORawrM::GUID& getPrimaryKey() const;
	void setPrimaryKey(const ORawrM::GUID& value); 
	void setCleanPrimaryKey(const ORawrM::GUID& value);

	void regenerateKey(); 
};

REGISTER_MODEL(Comment);

#endif
