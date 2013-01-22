//
//  BlockNetworkingDelegate.h
//  iosbooru
//
//  Created by Albert Wang on 1/17/13.
//  Copyright (c) 2013 Albert Wang. All rights reserved.
//

#import <Foundation/Foundation.h>

@interface BlockNetworkingDelegate : NSObject<NTVNetworkCallbacks>
{
    void(^finished)(NSString *);
    void(^failure)(int, NSString *, NTVNetworkFailure *);
    void(^results)(NSArray *, NSString *, NSUInteger);
    
    bool failed;
}

+ (BlockNetworkingDelegate *) createWithSuccess:(void(^)(NSString *))success;
+ (BlockNetworkingDelegate *) createWithResults:(void (^)(NSArray *, NSString *, NSUInteger))results;
+ (BlockNetworkingDelegate *) createWithResults:(void (^)(NSArray *, NSString *, NSUInteger))results failure:(void(^)(int, NSString *, NTVNetworkFailure *))fail;
+ (BlockNetworkingDelegate *) createWithSuccess:(void(^)(NSString *))success failure:(void(^)(int, NSString *, NTVNetworkFailure *))fail;

- (void) networkHandler:(id<NTVNetworking>)handle receivedData:(const Json::Value&)value;
- (void) networkHandler:(id<NTVNetworking>)handle gotError:(int)errorCode withMessage:(NSString *)msg withFailures:(NTVNetworkFailure *)failures;
- (void) networkFinished:(NSString *)extras;
@end
