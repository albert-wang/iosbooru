

#import <Foundation/Foundation.h>
#include "orm.hpp"
#include "Mutation.hpp"

@class NTVTag;
@class NTVComment;
@class NTVTagBridge;
@class NTVImage;
@class NTVUploadMetadata;
@class NTVRating;
@class NTVStatic;
@class NTVMutation;

@class NTVDatastore;
@interface NTVMutation : NSObject 
{
	Mutation * backing;
	NTVDatastore * datastore;
	std::map<ORawrM::GUID, id> *objectCache;

}



+ (NTVMutation *) createWithValue:(const Mutation&)value andDatastore:(NTVDatastore *)datastore;
- (NTVMutation *) initWithValue:(const Mutation&)value andDatastore:(NTVDatastore *)datastore;
+ (NTVMutation *) createWithFreshValue:(const Mutation&)value andDatastore:(NTVDatastore *)datastore;
- (NTVMutation *) initWithFreshValue:(const Mutation&)value andDatastore:(NTVDatastore *)datastore;
- (void) dealloc;

- (void) update;
- (void) remove;

- (Mutation&) getBacking;



@property (nonatomic, assign) ORawrM::GUID pid; 
- (void) setCleanPid:(ORawrM::GUID)value;


@property (nonatomic, assign) NSString * tableName; 
- (void) setCleanTableName:(NSString *)value;


@property (nonatomic, assign) NSString * field; 
- (void) setCleanField:(NSString *)value;


@property (nonatomic, copy) NSString * data;
- (void) setCleanData:(NSString *)value;


@property (nonatomic, assign) NSString * operation; 
- (void) setCleanOperation:(NSString *)value;


@property (nonatomic, assign) ORawrM::GUID target; 
- (void) setCleanTarget:(ORawrM::GUID)value;

@property (nonatomic, assign) ORawrM::GUID primaryKey;
- (void) setCleanPrimaryKey:(ORawrM::GUID)guid;

@end

