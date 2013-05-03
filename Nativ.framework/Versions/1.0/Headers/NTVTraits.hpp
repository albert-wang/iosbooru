
#include "orm.hpp"

#ifndef OBJC_TRAITS_HPP
#define OBJC_TRAITS_HPP


template<typename T>
struct ModelToObjCModel;

template<typename T>
struct ObjCModelToModel;

class Tag;
@class NTVTag;

template<>
struct ModelToObjCModel<Tag> 
{
	typedef NTVTag result;
};

template<>
struct ObjCModelToModel<NTVTag>
{
	typedef Tag result;
};

class Comment;
@class NTVComment;

template<>
struct ModelToObjCModel<Comment> 
{
	typedef NTVComment result;
};

template<>
struct ObjCModelToModel<NTVComment>
{
	typedef Comment result;
};

class TagBridge;
@class NTVTagBridge;

template<>
struct ModelToObjCModel<TagBridge> 
{
	typedef NTVTagBridge result;
};

template<>
struct ObjCModelToModel<NTVTagBridge>
{
	typedef TagBridge result;
};

class Image;
@class NTVImage;

template<>
struct ModelToObjCModel<Image> 
{
	typedef NTVImage result;
};

template<>
struct ObjCModelToModel<NTVImage>
{
	typedef Image result;
};

class UploadMetadata;
@class NTVUploadMetadata;

template<>
struct ModelToObjCModel<UploadMetadata> 
{
	typedef NTVUploadMetadata result;
};

template<>
struct ObjCModelToModel<NTVUploadMetadata>
{
	typedef UploadMetadata result;
};

class Rating;
@class NTVRating;

template<>
struct ModelToObjCModel<Rating> 
{
	typedef NTVRating result;
};

template<>
struct ObjCModelToModel<NTVRating>
{
	typedef Rating result;
};

class Static;
@class NTVStatic;

template<>
struct ModelToObjCModel<Static> 
{
	typedef NTVStatic result;
};

template<>
struct ObjCModelToModel<NTVStatic>
{
	typedef Static result;
};

class Mutation;
@class NTVMutation;

template<>
struct ModelToObjCModel<Mutation> 
{
	typedef NTVMutation result;
};

template<>
struct ObjCModelToModel<NTVMutation>
{
	typedef Mutation result;
};


@class NTVDatastore;

template<typename T>
NSMutableArray * createModeledNSArray(const std::vector<T>& in, NTVDatastore * datastore)
{
	typedef typename ModelToObjCModel<T>::result result_type;
	NSMutableArray * result = [NSMutableArray arrayWithCapacity:in.size()];

	for (size_t i = 0; i < in.size(); ++i)
	{
		[result insertObject:[result_type createWithValue:in[i] andDatastore:datastore] atIndex:i];
	}

	return result;
}

#endif

