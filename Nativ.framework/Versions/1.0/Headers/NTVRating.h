

#import <Foundation/Foundation.h>
#include "orm.hpp"
#include "Rating.hpp"

@class NTVTag;
@class NTVComment;
@class NTVTagBridge;
@class NTVImage;
@class NTVUploadMetadata;
@class NTVRating;
@class NTVStatic;
@class NTVMutation;

@class NTVDatastore;
@interface NTVRating : NSObject 
{
	Rating * backing;
	NTVDatastore * datastore;
	std::map<ORawrM::GUID, id> *objectCache;

}

+ (NTVRating *) createWithDatastore:(NTVDatastore *)datastore;
- (NTVRating *) initWithDatastore:(NTVDatastore *)datastore;


+ (NTVRating *) createWithValue:(const Rating&)value andDatastore:(NTVDatastore *)datastore;
- (NTVRating *) initWithValue:(const Rating&)value andDatastore:(NTVDatastore *)datastore;
+ (NTVRating *) createWithFreshValue:(const Rating&)value andDatastore:(NTVDatastore *)datastore;
- (NTVRating *) initWithFreshValue:(const Rating&)value andDatastore:(NTVDatastore *)datastore;
- (void) dealloc;

- (void) update;
- (void) remove;

- (Rating&) getBacking;



@property (nonatomic, assign) ORawrM::GUID pid; 
- (void) setCleanPid:(ORawrM::GUID)value;


@property (nonatomic, assign) ORawrM::GUID image_id; 
- (void) setCleanImage_id:(ORawrM::GUID)value;


@property (nonatomic, assign) NSInteger rating; 
- (void) setCleanRating:(NSInteger)value;


@property (nonatomic, assign) NSString * raterEmail; 
- (void) setCleanRaterEmail:(NSString *)value;

@property (nonatomic, assign) ORawrM::GUID primaryKey;
- (void) setCleanPrimaryKey:(ORawrM::GUID)guid;

@end

