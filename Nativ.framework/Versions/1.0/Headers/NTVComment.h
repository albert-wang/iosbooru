

#import <Foundation/Foundation.h>
#include "orm.hpp"
#include "Comment.hpp"

@class NTVTag;
@class NTVComment;
@class NTVTagBridge;
@class NTVImage;
@class NTVUploadMetadata;
@class NTVRating;
@class NTVStatic;
@class NTVMutation;

@class NTVDatastore;
@interface NTVComment : NSObject 
{
	Comment * backing;
	NTVDatastore * datastore;
	std::map<ORawrM::GUID, id> *objectCache;

}

+ (NTVComment *) createWithDatastore:(NTVDatastore *)datastore;
- (NTVComment *) initWithDatastore:(NTVDatastore *)datastore;


+ (NTVComment *) createWithValue:(const Comment&)value andDatastore:(NTVDatastore *)datastore;
- (NTVComment *) initWithValue:(const Comment&)value andDatastore:(NTVDatastore *)datastore;
+ (NTVComment *) createWithFreshValue:(const Comment&)value andDatastore:(NTVDatastore *)datastore;
- (NTVComment *) initWithFreshValue:(const Comment&)value andDatastore:(NTVDatastore *)datastore;
- (void) dealloc;

- (void) update;
- (void) remove;

- (Comment&) getBacking;



@property (nonatomic, assign) ORawrM::GUID pid; 
- (void) setCleanPid:(ORawrM::GUID)value;


@property (nonatomic, assign) ORawrM::GUID parent_id; 
- (void) setCleanParent_id:(ORawrM::GUID)value;


@property (nonatomic, assign) NSDate * dateCreated; 
- (void) setCleanDateCreated:(NSDate *)value;


@property (nonatomic, assign) NSString * contents; 
- (void) setCleanContents:(NSString *)value;

@property (nonatomic, assign) ORawrM::GUID primaryKey;
- (void) setCleanPrimaryKey:(ORawrM::GUID)guid;

@end

