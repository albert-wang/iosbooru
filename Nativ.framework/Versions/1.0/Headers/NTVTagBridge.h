

#import <Foundation/Foundation.h>
#include "orm.hpp"
#include "TagBridge.hpp"

@class NTVTag;
@class NTVComment;
@class NTVTagBridge;
@class NTVImage;
@class NTVUploadMetadata;
@class NTVRating;
@class NTVStatic;
@class NTVMutation;

@class NTVDatastore;
@interface NTVTagBridge : NSObject 
{
	TagBridge * backing;
	NTVDatastore * datastore;
	std::map<ORawrM::GUID, id> *objectCache;

}

+ (NTVTagBridge *) createWithDatastore:(NTVDatastore *)datastore;
- (NTVTagBridge *) initWithDatastore:(NTVDatastore *)datastore;


+ (NTVTagBridge *) createWithValue:(const TagBridge&)value andDatastore:(NTVDatastore *)datastore;
- (NTVTagBridge *) initWithValue:(const TagBridge&)value andDatastore:(NTVDatastore *)datastore;
+ (NTVTagBridge *) createWithFreshValue:(const TagBridge&)value andDatastore:(NTVDatastore *)datastore;
- (NTVTagBridge *) initWithFreshValue:(const TagBridge&)value andDatastore:(NTVDatastore *)datastore;
- (void) dealloc;

- (void) update;
- (void) remove;

- (TagBridge&) getBacking;



@property (nonatomic, assign) ORawrM::GUID pid; 
- (void) setCleanPid:(ORawrM::GUID)value;


@property (nonatomic, assign) ORawrM::GUID image_id; 
- (void) setCleanImage_id:(ORawrM::GUID)value;


@property (nonatomic, assign) ORawrM::GUID tag_id; 
- (void) setCleanTag_id:(ORawrM::GUID)value;

@property (nonatomic, assign) ORawrM::GUID primaryKey;
- (void) setCleanPrimaryKey:(ORawrM::GUID)guid;

@end

