
#import <Foundation/Foundation.h>
#include "orm.hpp"

#import "NTVTag.h"
#import "NTVComment.h"
#import "NTVTagBridge.h"
#import "NTVImage.h"
#import "NTVUploadMetadata.h"
#import "NTVRating.h"
#import "NTVStatic.h"
#import "NTVMutation.h"
#import "NTVResults.h"
#import "NTVNetworking.h"
#import "NTVKeyPredicate.h"

@protocol NTVLogger <NSObject>
	- (void) logAtLevel:(size_t)level message:(NSString *)msg inFile:(NSString *)fileName atLine:(size_t)line;
@end

@class NTVDatastore;
@class NTVKeyPredicate;
@class NTVDownloadParameters;

@interface NTVDatastore : NSObject  {
	@public
	ORawrM::Datastore<ORawrM::SQLiteAdapter> * datastore;

	@public
	dispatch_queue_t dispatchQueue;

	std::map<ORawrM::GUID, id> *objectCache;
}

@property (retain) id<NTVLogger> logger;
@property (retain) id<NTVNetworking> network;
@property (nonatomic, copy) NSURL * downloadPathPrefix;

//Lifetime management
- (NTVDatastore *) initWithDatabasePath:(NSString *)path; // Designated Initializer
+ (NTVDatastore *) datastoreWithDatabasePath:(NSString *)path;
- (NTVDatastore *) initWithInMemoryDatabase;
+ (NTVDatastore *) datastoreWithInMemoryDatabase;
- (void) dealloc;

//Basic operations
- (void) setDeviceID:(NSString *)deviceID;

//Model Creation
- (NTVTag *) createTag;
- (NTVComment *) createComment;
- (NTVTagBridge *) createTagBridge;
- (NTVImage *) createImage;
- (NTVUploadMetadata *) createUploadMetadata;
- (NTVRating *) createRating;
- (NTVStatic *) createStatic;
- (NTVMutation *) createMutation;

//Model management
- (void) update:(id)model;
- (void) remove:(id)model;

- (void) removeTagByGUID:(ORawrM::GUID)guid;
- (void) removeTagWhereColumn:(NSString *)col is:(ORawrM::GUID)guid;
- (void) removeCommentByGUID:(ORawrM::GUID)guid;
- (void) removeCommentWhereColumn:(NSString *)col is:(ORawrM::GUID)guid;
- (void) removeTagBridgeByGUID:(ORawrM::GUID)guid;
- (void) removeTagBridgeWhereColumn:(NSString *)col is:(ORawrM::GUID)guid;
- (void) removeImageByGUID:(ORawrM::GUID)guid;
- (void) removeImageWhereColumn:(NSString *)col is:(ORawrM::GUID)guid;
- (void) removeUploadMetadataByGUID:(ORawrM::GUID)guid;
- (void) removeUploadMetadataWhereColumn:(NSString *)col is:(ORawrM::GUID)guid;
- (void) removeRatingByGUID:(ORawrM::GUID)guid;
- (void) removeRatingWhereColumn:(NSString *)col is:(ORawrM::GUID)guid;
- (void) removeStaticByGUID:(ORawrM::GUID)guid;
- (void) removeStaticWhereColumn:(NSString *)col is:(ORawrM::GUID)guid;
- (void) removeMutationByGUID:(ORawrM::GUID)guid;
- (void) removeMutationWhereColumn:(NSString *)col is:(ORawrM::GUID)guid;

//Select, count and exists
- (NTVTag *) selectTagWithGUID:(ORawrM::GUID)guid;
- (size_t) countTag:(NTVKeyPredicate *)pred;
- (void) selectTag:(NTVKeyPredicate *)pred
  delegate:(id<NTVQueryDelegate>)del;
- (bool) existsTag:(ORawrM::GUID)guid;


- (void) linkTag:(NTVTag *)f andImage:(NTVImage *)s;
- (void) unlinkTag:(NTVTag *)f andImage:(NTVImage *)s;
- (NTVComment *) selectCommentWithGUID:(ORawrM::GUID)guid;
- (size_t) countComment:(NTVKeyPredicate *)pred;
- (void) selectComment:(NTVKeyPredicate *)pred
  delegate:(id<NTVQueryDelegate>)del;
- (bool) existsComment:(ORawrM::GUID)guid;


- (NTVTagBridge *) selectTagBridgeWithGUID:(ORawrM::GUID)guid;
- (size_t) countTagBridge:(NTVKeyPredicate *)pred;
- (void) selectTagBridge:(NTVKeyPredicate *)pred
  delegate:(id<NTVQueryDelegate>)del;
- (bool) existsTagBridge:(ORawrM::GUID)guid;


- (NTVImage *) selectImageWithGUID:(ORawrM::GUID)guid;
- (size_t) countImage:(NTVKeyPredicate *)pred;
- (void) selectImage:(NTVKeyPredicate *)pred
  delegate:(id<NTVQueryDelegate>)del;
- (bool) existsImage:(ORawrM::GUID)guid;


- (void) linkImage:(NTVImage *)f andTag:(NTVTag *)s;
- (void) unlinkImage:(NTVImage *)f andTag:(NTVTag *)s;
- (NTVUploadMetadata *) selectUploadMetadataWithGUID:(ORawrM::GUID)guid;
- (size_t) countUploadMetadata:(NTVKeyPredicate *)pred;
- (void) selectUploadMetadata:(NTVKeyPredicate *)pred
  delegate:(id<NTVQueryDelegate>)del;
- (bool) existsUploadMetadata:(ORawrM::GUID)guid;


- (NTVRating *) selectRatingWithGUID:(ORawrM::GUID)guid;
- (size_t) countRating:(NTVKeyPredicate *)pred;
- (void) selectRating:(NTVKeyPredicate *)pred
  delegate:(id<NTVQueryDelegate>)del;
- (bool) existsRating:(ORawrM::GUID)guid;


- (NTVStatic *) selectStaticWithGUID:(ORawrM::GUID)guid;
- (size_t) countStatic:(NTVKeyPredicate *)pred;
- (void) selectStatic:(NTVKeyPredicate *)pred
  delegate:(id<NTVQueryDelegate>)del;
- (bool) existsStatic:(ORawrM::GUID)guid;



//Static operations.
- (NTVStatic *) createStaticFrom:(NSURL *)url;
- (NSURL *) pathForStatic:(NTVStatic *)s atResolution:(NTVDownloadParameters *)params;

//Network operations
- (void) externalQueryForTag:(NTVKeyPredicate *)pred withSubqueries:(NSArray *)subs delegate:(id<NTVNetworkCallbacks>) cbs;
- (void) hybridQueryForTag:(NTVKeyPredicate *)pred withSubqueries:(NSArray *)subs localDelegate:(id<NTVNetworkCallbacks>)local remoteDelegate:(id<NTVNetworkCallbacks>)remote;
- (void) externalQueryForComment:(NTVKeyPredicate *)pred withSubqueries:(NSArray *)subs delegate:(id<NTVNetworkCallbacks>) cbs;
- (void) hybridQueryForComment:(NTVKeyPredicate *)pred withSubqueries:(NSArray *)subs localDelegate:(id<NTVNetworkCallbacks>)local remoteDelegate:(id<NTVNetworkCallbacks>)remote;
- (void) externalQueryForTagBridge:(NTVKeyPredicate *)pred withSubqueries:(NSArray *)subs delegate:(id<NTVNetworkCallbacks>) cbs;
- (void) hybridQueryForTagBridge:(NTVKeyPredicate *)pred withSubqueries:(NSArray *)subs localDelegate:(id<NTVNetworkCallbacks>)local remoteDelegate:(id<NTVNetworkCallbacks>)remote;
- (void) externalQueryForImage:(NTVKeyPredicate *)pred withSubqueries:(NSArray *)subs delegate:(id<NTVNetworkCallbacks>) cbs;
- (void) hybridQueryForImage:(NTVKeyPredicate *)pred withSubqueries:(NSArray *)subs localDelegate:(id<NTVNetworkCallbacks>)local remoteDelegate:(id<NTVNetworkCallbacks>)remote;
- (void) externalQueryForUploadMetadata:(NTVKeyPredicate *)pred withSubqueries:(NSArray *)subs delegate:(id<NTVNetworkCallbacks>) cbs;
- (void) hybridQueryForUploadMetadata:(NTVKeyPredicate *)pred withSubqueries:(NSArray *)subs localDelegate:(id<NTVNetworkCallbacks>)local remoteDelegate:(id<NTVNetworkCallbacks>)remote;
- (void) externalQueryForRating:(NTVKeyPredicate *)pred withSubqueries:(NSArray *)subs delegate:(id<NTVNetworkCallbacks>) cbs;
- (void) hybridQueryForRating:(NTVKeyPredicate *)pred withSubqueries:(NSArray *)subs localDelegate:(id<NTVNetworkCallbacks>)local remoteDelegate:(id<NTVNetworkCallbacks>)remote;
- (void) externalQueryForStatic:(NTVKeyPredicate *)pred withSubqueries:(NSArray *)subs delegate:(id<NTVNetworkCallbacks>) cbs;
- (void) hybridQueryForStatic:(NTVKeyPredicate *)pred withSubqueries:(NSArray *)subs localDelegate:(id<NTVNetworkCallbacks>)local remoteDelegate:(id<NTVNetworkCallbacks>)remote;

- (void) authenticateUser:(NSString *)user withPassword:(NSString *)password delegate:(id<NTVNetworkCallbacks>)cbs;
- (bool) hasAuthenticationToken;
- (void) setAuthenticationToken:(NSString *)token;
- (NSString *)authenticationToken;
- (void) uploadMutationsWithDelegate:(id<NTVNetworkCallbacks>) delegate;
- (void) downloadFileWithGUID:(ORawrM::GUID)guid withMime:(NSString *)mime toLiteralPath:(NSString *)path downloadParameters:(NTVDownloadParameters *)params delegate:(id<NTVNetworkCallbacks>)cbs;
- (void) downloadFileWithGUID:(ORawrM::GUID)guid withMime:(NSString *)mime toPath:(NSString *)path downloadParameters:(NTVDownloadParameters *)params delegate:(id<NTVNetworkCallbacks>)cbs;
- (void) downloadFile:(NTVStatic *)f downloadParameters:(NTVDownloadParameters *)params delegate:(id<NTVNetworkCallbacks>)cbs;
- (void) uploadFile:(NSString *)localPath withMime:(NSString *)mime delegate:(id<NTVNetworkCallbacks>)cbs;
- (void) sendCustomOperation:(NSString *)operation withBody:(NSData *)data delegate:(id<NTVNetworkCallbacks>)cbs;

@end
