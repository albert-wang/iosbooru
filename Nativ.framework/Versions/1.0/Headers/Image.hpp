

/* This is generated code. Do not modify. */
#pragma once
#ifndef GENERATED_MODEL_Image
#define GENERATED_MODEL_Image

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


#include "Rating.hpp"
#include "Comment.hpp"

class Image
{
	ORawrM::GUID pid;
	std::string filehash;
	std::string mime;
	boost::int64_t uploadedDate;
	std::vector< Comment > comments;
	std::vector< Rating > ratings;
	float ratingsAverage;

public:
		static const bool hasGeneratedKey = false;

private:


	ORawrM::NullBacking reserved_backing;


	friend class ORawrM::Access;
	friend class Tag;
	friend class Comment;
	friend class TagBridge;
	friend class UploadMetadata;
	friend class Rating;
	friend class Static;

	Image();
public:

	enum 
	{


		ImageTagsRelation = 131072, 
		ImageTagsBridgeRelation = 1073872896,

		CommentsRelation = 50331652,
		RatingsRelation = 50331653,
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
	void setFilehash(const std::string &value);
	void setCleanFilehash(const std::string &value); 
	const std::string& getFilehash() const;
	void setMime(const std::string &value);
	void setCleanMime(const std::string &value); 
	const std::string& getMime() const;
	void setUploadedDate(const boost::int64_t &value);
	void setCleanUploadedDate(const boost::int64_t &value); 
	const boost::int64_t& getUploadedDate() const;
	void addComments(Comment& value);
	void removeComments(const Comment& value);
	const std::vector< Comment >& getComments() const;
	void addRatings(Rating& value);
	void removeRatings(const Rating& value);
	const std::vector< Rating >& getRatings() const;
	void setRatingsAverage(const float &value);
	void setCleanRatingsAverage(const float &value); 
	const float& getRatingsAverage() const;

	template<typename Archive>
	void serialize(Archive& arch) 
	{
		arch.start(*this);

		arch.template registerBridgeRelationship<Image, TagBridge, Image, Tag>(*this, 131072);

		ORawrM::Constraints constraint_for_pid = ORawrM::Constraints::none();
		constraint_for_pid.combine(ORawrM::Constraints::key());
		constraint_for_pid.combine(ORawrM::Constraints::customIndex("ImageUploadedAndPid", 1));
		arch & ::ORawrM::makeEntry("pid", pid, constraint_for_pid);
		ORawrM::Constraints constraint_for_filehash = ORawrM::Constraints::none();
		constraint_for_filehash.combine(ORawrM::Constraints::index());
		arch & ::ORawrM::makeEntry("filehash", filehash, constraint_for_filehash);
		ORawrM::Constraints constraint_for_mime = ORawrM::Constraints::none();
		arch & ::ORawrM::makeEntry("mime", mime, constraint_for_mime);
		ORawrM::Constraints constraint_for_uploadedDate = ORawrM::Constraints::none();
		constraint_for_uploadedDate.combine(ORawrM::Constraints::index());
		constraint_for_uploadedDate.combine(ORawrM::Constraints::customIndex("ImageUploadedAndPid", 0));
		arch & ::ORawrM::makeEntry("uploadedDate", uploadedDate, constraint_for_uploadedDate);
		ORawrM::Constraints constraint_for_comments = ORawrM::Constraints::none();
		constraint_for_comments.combine(ORawrM::Constraints::ownsChildren());
		constraint_for_comments.combine(ORawrM::Constraints::relation(50331652));
		arch & ::ORawrM::makeEntry("comments", comments, constraint_for_comments);
		ORawrM::Constraints constraint_for_ratings = ORawrM::Constraints::none();
		constraint_for_ratings.combine(ORawrM::Constraints::ownsChildren());
		constraint_for_ratings.combine(ORawrM::Constraints::relation(50331653));
		arch & ::ORawrM::makeEntry("ratings", ratings, constraint_for_ratings);
		ORawrM::Constraints constraint_for_ratingsAverage = ORawrM::Constraints::none();
		arch & ::ORawrM::makeEntry("ratingsAverage", ratingsAverage, constraint_for_ratingsAverage);
		
		arch.end(*this);
	}

	const ORawrM::GUID& getPrimaryKey() const;
	void setPrimaryKey(const ORawrM::GUID& value); 
	void setCleanPrimaryKey(const ORawrM::GUID& value);

	void regenerateKey(); 
};

REGISTER_MODEL(Image);

#endif
