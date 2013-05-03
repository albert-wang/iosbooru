

#import <Foundation/Foundation.h>
#include "orm.hpp"
#include "Static.hpp"

@class NTVTag;
@class NTVComment;
@class NTVTagBridge;
@class NTVImage;
@class NTVUploadMetadata;
@class NTVRating;
@class NTVStatic;
@class NTVMutation;

@class NTVDatastore;
@interface NTVStatic : NSObject 
{
	Static * backing;
	NTVDatastore * datastore;
	std::map<ORawrM::GUID, id> *objectCache;

}

+ (NTVStatic *) createWithDatastore:(NTVDatastore *)datastore;
- (NTVStatic *) initWithDatastore:(NTVDatastore *)datastore;

+ (NTVStatic *) createWithDatastore:(NTVDatastore *)datastore andFile:(NSURL *)url;
- (void) setWithURL:(NSURL *)url;
- (void) setWithURL:(NSURL *)url andPointX:(NSInteger)x Y:(NSInteger)y;
- (void) setThumbPointX:(NSInteger)x Y:(NSInteger)y;
- (NSURL *) toURL;

+ (NTVStatic *) createWithValue:(const Static&)value andDatastore:(NTVDatastore *)datastore;
- (NTVStatic *) initWithValue:(const Static&)value andDatastore:(NTVDatastore *)datastore;
+ (NTVStatic *) createWithFreshValue:(const Static&)value andDatastore:(NTVDatastore *)datastore;
- (NTVStatic *) initWithFreshValue:(const Static&)value andDatastore:(NTVDatastore *)datastore;
- (void) dealloc;

- (void) update;
- (void) remove;

- (Static&) getBacking;



@property (nonatomic, assign) ORawrM::GUID pid; 
- (void) setCleanPid:(ORawrM::GUID)value;


@property (nonatomic, copy) NSDate * lastUpdatedTimestamp;
- (void) setCleanLastUpdatedTimestamp:(NSDate *)value;


@property (nonatomic, assign) NSString * mime; 
- (void) setCleanMime:(NSString *)value;


@property (nonatomic, assign) NSString * path; 
- (void) setCleanPath:(NSString *)value;


@property (nonatomic, assign) NSString * SHA1Hash; 
- (void) setCleanSHA1Hash:(NSString *)value;


@property (nonatomic, assign) NSString * thumb; 
- (void) setCleanThumb:(NSString *)value;

@property (nonatomic, assign) ORawrM::GUID primaryKey;
- (void) setCleanPrimaryKey:(ORawrM::GUID)guid;

@end

