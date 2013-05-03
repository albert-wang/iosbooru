

#import <Foundation/Foundation.h>
#include "orm.hpp"
#include "Tag.hpp"

@class NTVTag;
@class NTVComment;
@class NTVTagBridge;
@class NTVImage;
@class NTVUploadMetadata;
@class NTVRating;
@class NTVStatic;
@class NTVMutation;

@class NTVDatastore;
@interface NTVTag : NSObject 
{
	Tag * backing;
	NTVDatastore * datastore;
	std::map<ORawrM::GUID, id> *objectCache;

}

+ (NTVTag *) createWithDatastore:(NTVDatastore *)datastore;
- (NTVTag *) initWithDatastore:(NTVDatastore *)datastore;


+ (NTVTag *) createWithValue:(const Tag&)value andDatastore:(NTVDatastore *)datastore;
- (NTVTag *) initWithValue:(const Tag&)value andDatastore:(NTVDatastore *)datastore;
+ (NTVTag *) createWithFreshValue:(const Tag&)value andDatastore:(NTVDatastore *)datastore;
- (NTVTag *) initWithFreshValue:(const Tag&)value andDatastore:(NTVDatastore *)datastore;
- (void) dealloc;

- (void) update;
- (void) remove;

- (Tag&) getBacking;



@property (nonatomic, assign) ORawrM::GUID pid; 
- (void) setCleanPid:(ORawrM::GUID)value;


@property (nonatomic, assign) NSString * name; 
- (void) setCleanName:(NSString *)value;

@property (nonatomic, assign) ORawrM::GUID primaryKey;
- (void) setCleanPrimaryKey:(ORawrM::GUID)guid;

@end

