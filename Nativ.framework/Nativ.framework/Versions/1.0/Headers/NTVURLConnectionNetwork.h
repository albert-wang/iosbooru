//
//	NTVURLConnectionNetwork.h
//	Nativ
//
//	Created by Albert Wang on 4/23/12.
//	Copyright (c) 2012 Uncodin. All rights reserved.
//

#import <Foundation/Foundation.h>
#import "NTVDatastore.h"
#include <set>

@interface NTVURLConnectionNetwork : NSObject< NTVNetworking, NSURLConnectionDelegate > 
{
	ORawrM::INetworkHandler * handler;
	NSURL * target;

	std::set<id> activeConnections;
}

+ (NTVURLConnectionNetwork *) networkWithTarget:(NSURL *)target;
- (id) initWithTargetURL:(NSURL *)target;
- (int) cancelAllActiveRequests;

//Don't call this method.
- (void) connectionFinished:(id)conn;
- (void) dealloc;

// NTVNetworking
- (int) computeSHA1HashOfFile:(NSString *)localPath output:(NSString **)o;
- (int) createDeviceIndependentResources:(ORawrM::INetworkHandler *)owningHandler;
- (int) sendJSONData:(const Json::Value *)root withOperation:(NSString *)operation andOutputPath:(NSString *)output callbacks:(ORawrM::INetworkCallbacks *)cb;
- (int) uploadFileWithJSON:(const Json::Value *)root atLocalPath:(NSString *)path withID:(NSString *)unique callbacks:(ORawrM::INetworkCallbacks *)cb;

@end

@interface URLWithJSONResponseDelegate : NSObject < NSURLConnectionDelegate >
{
	NSMutableData * receivedData;
	ORawrM::INetworkCallbacks * callbacks;
	ORawrM::INetworkHandler * handler;

	NSURLConnection * ownedConnection;
	NTVURLConnectionNetwork * ownedNetwork;
	dispatch_queue_t targetQueue;
}

+ (URLWithJSONResponseDelegate *) delegateWithCallbacks:(ORawrM::INetworkCallbacks *)cbs andHandler:(ORawrM::INetworkHandler *)handler;
- (id) initWithCallbacks:(ORawrM::INetworkCallbacks *)cbs andHandler:(ORawrM::INetworkHandler *)handler;
- (void) dealloc;

//Ownership semantics
- (void) assumeOwnership:(NSURLConnection *)connection network:(NTVURLConnectionNetwork *)net callbackQueue:(dispatch_queue_t)q;
- (void) forcedCancel; 

//NSURLConnectionDelegate
- (void) connection:(NSURLConnection *)connection didReceiveResponse:(NSURLResponse *)response;
- (void) connection:(NSURLConnection *)connection didReceiveData:(NSData *)data;
- (void) connectionDidFinishLoading:(NSURLConnection *)connection;
- (void) connection:(NSURLConnection *)connection didFailWithError:(NSError *)error;
@end

@interface URLWithFileResponseDelegate : NSObject < NSURLConnectionDelegate >
{
	NSMutableData * receivedData;
	NSURL * outputPath;
	NSFileHandle * file;
	ORawrM::INetworkCallbacks * callbacks;
	ORawrM::INetworkHandler * handler;

	NSURLConnection * ownedConnection;
	NTVURLConnectionNetwork * ownedNetwork;
	dispatch_queue_t targetQueue;
}

+ (URLWithFileResponseDelegate *) delegateWithCallbacks:(ORawrM::INetworkCallbacks *)cbs outputPath:(NSURL *)path andHandler:(ORawrM::INetworkHandler *)handler;

- (id) initWithCallbacks:(ORawrM::INetworkCallbacks *)cbs outputPath:(NSURL *)path andHandler:(ORawrM::INetworkHandler *)handler;
- (void) dealloc;

//Ownership semantics
- (void) assumeOwnership:(NSURLConnection *)connection network:(NTVURLConnectionNetwork *)net callbackQueue:(dispatch_queue_t)q;

//NSURLConnectionDelegate
- (void) connection:(NSURLConnection *)connection didReceiveResponse:(NSURLResponse *)response;
- (void) connection:(NSURLConnection *)connection didReceiveData:(NSData *)data;
- (void) connectionDidFinishLoading:(NSURLConnection *)connection;
- (void) connection:(NSURLConnection *)connection didFailWithError:(NSError *)error;
@end

