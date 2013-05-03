

/* This is generated code. Do not modify. */
#pragma once
#ifndef GENERATED_MODEL_Rating
#define GENERATED_MODEL_Rating

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



class Rating
{
	ORawrM::GUID pid;
	ORawrM::GUID image_id;
	boost::int32_t rating;
	std::string raterEmail;

public:
		static const bool hasGeneratedKey = false;

private:


	ORawrM::NullBacking reserved_backing;


	friend class ORawrM::Access;
	friend class Tag;
	friend class Comment;
	friend class TagBridge;
	friend class Image;
	friend class UploadMetadata;
	friend class Static;

	Rating();
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
	void setRating(const boost::int32_t &value);
	void setCleanRating(const boost::int32_t &value); 
	const boost::int32_t& getRating() const;
	void setRaterEmail(const std::string &value);
	void setCleanRaterEmail(const std::string &value); 
	const std::string& getRaterEmail() const;

	template<typename Archive>
	void serialize(Archive& arch) 
	{
		arch.start(*this);


		ORawrM::Constraints constraint_for_pid = ORawrM::Constraints::none();
		constraint_for_pid.combine(ORawrM::Constraints::key());
		arch & ::ORawrM::makeEntry("pid", pid, constraint_for_pid);
		ORawrM::Constraints constraint_for_image_id = ORawrM::Constraints::none();
		constraint_for_image_id.combine(ORawrM::Constraints::isParentReference(ForwardImage_ratingsRelation));
		arch & ::ORawrM::makeEntry("image_id", image_id, constraint_for_image_id);
		ORawrM::Constraints constraint_for_rating = ORawrM::Constraints::none();
		arch & ::ORawrM::makeEntry("rating", rating, constraint_for_rating);
		ORawrM::Constraints constraint_for_raterEmail = ORawrM::Constraints::none();
		constraint_for_raterEmail.combine(ORawrM::Constraints::index());
		arch & ::ORawrM::makeEntry("raterEmail", raterEmail, constraint_for_raterEmail);
		
		arch.end(*this);
	}

	const ORawrM::GUID& getPrimaryKey() const;
	void setPrimaryKey(const ORawrM::GUID& value); 
	void setCleanPrimaryKey(const ORawrM::GUID& value);

	void regenerateKey(); 
};

REGISTER_MODEL(Rating);

#endif
