#import <Foundation/Foundation.h>
#include "ormpredicate.hpp"

@class NTVDatastore;

@interface NTVKeyPredicate : NSObject
{
	ORawrM::KeyPredicate backingPredicate;
};


+ (NTVKeyPredicate *) primaryKeysForTagWithBuffer:(const ORawrM::GUID *)buffer ofLength:(size_t)len;
+ (NTVKeyPredicate *) primaryKeysForTagWithGUID:(ORawrM::GUID)guid;
+ (NTVKeyPredicate *) relationKeysOnTag:(NSArray *)models withRelationID:(size_t)relationID;
+ (NTVKeyPredicate *) inverseRelationOnTag:(NSArray *)models withRelationID:(size_t)relationID;


+ (NTVKeyPredicate *) primaryKeysForCommentWithBuffer:(const ORawrM::GUID *)buffer ofLength:(size_t)len;
+ (NTVKeyPredicate *) primaryKeysForCommentWithGUID:(ORawrM::GUID)guid;
+ (NTVKeyPredicate *) relationKeysOnComment:(NSArray *)models withRelationID:(size_t)relationID;
+ (NTVKeyPredicate *) inverseRelationOnComment:(NSArray *)models withRelationID:(size_t)relationID;


+ (NTVKeyPredicate *) primaryKeysForTagBridgeWithBuffer:(const ORawrM::GUID *)buffer ofLength:(size_t)len;
+ (NTVKeyPredicate *) primaryKeysForTagBridgeWithGUID:(ORawrM::GUID)guid;
+ (NTVKeyPredicate *) relationKeysOnTagBridge:(NSArray *)models withRelationID:(size_t)relationID;
+ (NTVKeyPredicate *) inverseRelationOnTagBridge:(NSArray *)models withRelationID:(size_t)relationID;


+ (NTVKeyPredicate *) primaryKeysForImageWithBuffer:(const ORawrM::GUID *)buffer ofLength:(size_t)len;
+ (NTVKeyPredicate *) primaryKeysForImageWithGUID:(ORawrM::GUID)guid;
+ (NTVKeyPredicate *) relationKeysOnImage:(NSArray *)models withRelationID:(size_t)relationID;
+ (NTVKeyPredicate *) inverseRelationOnImage:(NSArray *)models withRelationID:(size_t)relationID;


+ (NTVKeyPredicate *) primaryKeysForUploadMetadataWithBuffer:(const ORawrM::GUID *)buffer ofLength:(size_t)len;
+ (NTVKeyPredicate *) primaryKeysForUploadMetadataWithGUID:(ORawrM::GUID)guid;
+ (NTVKeyPredicate *) relationKeysOnUploadMetadata:(NSArray *)models withRelationID:(size_t)relationID;
+ (NTVKeyPredicate *) inverseRelationOnUploadMetadata:(NSArray *)models withRelationID:(size_t)relationID;


+ (NTVKeyPredicate *) primaryKeysForRatingWithBuffer:(const ORawrM::GUID *)buffer ofLength:(size_t)len;
+ (NTVKeyPredicate *) primaryKeysForRatingWithGUID:(ORawrM::GUID)guid;
+ (NTVKeyPredicate *) relationKeysOnRating:(NSArray *)models withRelationID:(size_t)relationID;
+ (NTVKeyPredicate *) inverseRelationOnRating:(NSArray *)models withRelationID:(size_t)relationID;


+ (NTVKeyPredicate *) primaryKeysForStaticWithBuffer:(const ORawrM::GUID *)buffer ofLength:(size_t)len;
+ (NTVKeyPredicate *) primaryKeysForStaticWithGUID:(ORawrM::GUID)guid;
+ (NTVKeyPredicate *) relationKeysOnStatic:(NSArray *)models withRelationID:(size_t)relationID;
+ (NTVKeyPredicate *) inverseRelationOnStatic:(NSArray *)models withRelationID:(size_t)relationID;


+ (NTVKeyPredicate *) defaultKeyPredicate;


- (NTVKeyPredicate *) initWithKeyPredicate:(const ORawrM::KeyPredicate&) pred;
- (NTVKeyPredicate *) orderBy:(NSString *)field isDescending:(bool)desc;
- (NTVKeyPredicate *) where:(NSString *)command;
- (NTVKeyPredicate *) where:(NSString *)column GUID:(ORawrM::GUID)guid;
- (NTVKeyPredicate *) limit:(size_t)lim;
- (NTVKeyPredicate *) offset:(size_t)off;
- (NTVKeyPredicate *) bridgeRelationshipIsLogicalOr:(bool)ored;
- (NTVKeyPredicate *) externalSearch:(NSString *)search;
- (ORawrM::KeyPredicate&) getBacking;
@end
