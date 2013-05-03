

#import <Foundation/Foundation.h>
#include "orm.hpp"
#include "UploadMetadata.hpp"

@class NTVTag;
@class NTVComment;
@class NTVTagBridge;
@class NTVImage;
@class NTVUploadMetadata;
@class NTVRating;
@class NTVStatic;
@class NTVMutation;

@class NTVDatastore;
@interface NTVUploadMetadata : NSObject 
{
	UploadMetadata * backing;
	NTVDatastore * datastore;
	std::map<ORawrM::GUID, id> *objectCache;

}

+ (NTVUploadMetadata *) createWithDatastore:(NTVDatastore *)datastore;
- (NTVUploadMetadata *) initWithDatastore:(NTVDatastore *)datastore;


+ (NTVUploadMetadata *) createWithValue:(const UploadMetadata&)value andDatastore:(NTVDatastore *)datastore;
- (NTVUploadMetadata *) initWithValue:(const UploadMetadata&)value andDatastore:(NTVDatastore *)datastore;
+ (NTVUploadMetadata *) createWithFreshValue:(const UploadMetadata&)value andDatastore:(NTVDatastore *)datastore;
- (NTVUploadMetadata *) initWithFreshValue:(const UploadMetadata&)value andDatastore:(NTVDatastore *)datastore;
- (void) dealloc;

- (void) update;
- (void) remove;

- (UploadMetadata&) getBacking;



@property (nonatomic, assign) ORawrM::GUID pid; 
- (void) setCleanPid:(ORawrM::GUID)value;


@property (nonatomic, assign) ORawrM::GUID imageGUID; 
- (void) setCleanImageGUID:(ORawrM::GUID)value;


@property (nonatomic, assign) NSString * uploadedBy; 
- (void) setCleanUploadedBy:(NSString *)value;


@property (nonatomic, assign) NSString * originalExtension; 
- (void) setCleanOriginalExtension:(NSString *)value;

@property (nonatomic, assign) ORawrM::GUID primaryKey;
- (void) setCleanPrimaryKey:(ORawrM::GUID)guid;

@end

