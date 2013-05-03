

#import <Foundation/Foundation.h>
#include "orm.hpp"
#include "Image.hpp"

@class NTVTag;
@class NTVComment;
@class NTVTagBridge;
@class NTVImage;
@class NTVUploadMetadata;
@class NTVRating;
@class NTVStatic;
@class NTVMutation;

@class NTVDatastore;
@interface NTVImage : NSObject 
{
	Image * backing;
	NTVDatastore * datastore;
	std::map<ORawrM::GUID, id> *objectCache;

			NSMutableArray *_commentsCache;
			NSMutableArray *_ratingsCache;
}

+ (NTVImage *) createWithDatastore:(NTVDatastore *)datastore;
- (NTVImage *) initWithDatastore:(NTVDatastore *)datastore;


+ (NTVImage *) createWithValue:(const Image&)value andDatastore:(NTVDatastore *)datastore;
- (NTVImage *) initWithValue:(const Image&)value andDatastore:(NTVDatastore *)datastore;
+ (NTVImage *) createWithFreshValue:(const Image&)value andDatastore:(NTVDatastore *)datastore;
- (NTVImage *) initWithFreshValue:(const Image&)value andDatastore:(NTVDatastore *)datastore;
- (void) dealloc;

- (void) update;
- (void) remove;

- (Image&) getBacking;



@property (nonatomic, assign) ORawrM::GUID pid; 
- (void) setCleanPid:(ORawrM::GUID)value;


@property (nonatomic, assign) NSString * filehash; 
- (void) setCleanFilehash:(NSString *)value;


@property (nonatomic, assign) NSString * mime; 
- (void) setCleanMime:(NSString *)value;


@property (nonatomic, assign) NSDate * uploadedDate; 
- (void) setCleanUploadedDate:(NSDate *)value;



@property (readonly) NSArray * comments;
- (void) addToComments:(NTVComment *)value;
- (void) removeFromComments:(const NTVComment *)value;
- (void) invalidateComments;



@property (readonly) NSArray * ratings;
- (void) addToRatings:(NTVRating *)value;
- (void) removeFromRatings:(const NTVRating *)value;
- (void) invalidateRatings;


@property (nonatomic, assign) float ratingsAverage; 
- (void) setCleanRatingsAverage:(float)value;

@property (nonatomic, assign) ORawrM::GUID primaryKey;
- (void) setCleanPrimaryKey:(ORawrM::GUID)guid;

@end

