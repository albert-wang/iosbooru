//
//  BlockNetworkingDelegate.m
//  iosbooru
//
//  Created by Albert Wang on 1/17/13.
//  Copyright (c) 2013 Albert Wang. All rights reserved.
//

#import "BlockNetworkingDelegate.h"

@implementation BlockNetworkingDelegate

+ (BlockNetworkingDelegate *) createWithSuccess:(void (^)(NSString *))success
{
    BlockNetworkingDelegate * result = [[[BlockNetworkingDelegate alloc] init] autorelease];
    result->finished = [success copy];
    result->failed = false;
    
    return result;
}

+ (BlockNetworkingDelegate *)createWithResults:(void (^)(NSArray *, NSString *, NSUInteger))results
{
    BlockNetworkingDelegate * result = [[[BlockNetworkingDelegate alloc] init] autorelease];
    result->results = [results copy];
    result->finished = nil;
    result->failed = false;
    
    return result;
}

+ (BlockNetworkingDelegate *)createWithResults:(void (^)(NSArray *, NSString *, NSUInteger))results failure:(void (^)(int, NSString *, NTVNetworkFailure *))fail
{
    BlockNetworkingDelegate * result = [[[BlockNetworkingDelegate alloc] init] autorelease];
    result->results = [results copy];
    result->failure = fail;
    result->finished = nil;
    result->failed = false;
    
    return result;
}

+ (BlockNetworkingDelegate *)createWithSuccess:(void (^)(NSString *))success failure:(void (^)(int, NSString *, NTVNetworkFailure *))fail
{
    BlockNetworkingDelegate * result = [[[BlockNetworkingDelegate alloc] init] autorelease];
    result->finished = [success copy];
    result->failure = [fail copy];
    result->failed = false;
    
    return result;
}

- (void) networkHandler:(id<NTVNetworking>)handle receivedData:(const Json::Value &)value
{}

- (void) networkHandler:(id<NTVNetworking>)handle gotError:(int)errorCode withMessage:(NSString *)msg withFailures:(NTVNetworkFailure *)failures
{
    NSLog(@"Network request has failed with error code %d and message %@", errorCode, msg);
    self->failed = true;
    if (self->failure)
    {
        self->failure(errorCode, msg, failures);
    }
}

- (void) networkFinished:(NSString *)extras
{
    if (!self->failed)
    {
        if (self->finished)
        {
            self->finished(extras);
        } else
        {
            NSLog(@"Network finished, without a finished callback");
        }
    }
    
    [self->finished release];
    [self->results release];
    [self->failure release];
}

- (void) receivedComment:(NSArray *)models named:(NSString *)name withCount:(NSUInteger)count
{
    if (self->results)
    {
        self->results(models, name, count);
    }
}

- (void) receivedTagBridge:(NSArray *)models named:(NSString *)name withCount:(NSUInteger)count
{
    if (self->results)
    {
        self->results(models, name, count);
    }
}


- (void) receivedImage:(NSArray *)models named:(NSString *)name withCount:(NSUInteger)count
{
    if (self->results)
    {
        self->results(models, name, count);
    }
}


- (void) receivedUploadMetadata:(NSArray *)models named:(NSString *)name withCount:(NSUInteger)count
{
    if (self->results)
    {
        self->results(models, name, count);
    }
}

- (void) receivedRating:(NSArray *)models named:(NSString *)name withCount:(NSUInteger)count
{
    if (self->results)
    {
        self->results(models, name, count);
    }
}

- (void) receivedStatic:(NSArray *)models named:(NSString *)name withCount:(NSUInteger)count
{
    if (self->results)
    {
        self->results(models, name, count);
    }
}

@end
