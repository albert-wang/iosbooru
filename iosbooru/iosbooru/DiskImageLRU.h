//
//  DiskImageLRU.h
//  iosbooru
//
//  Created by Albert Wang on 1/17/13.
//  Copyright (c) 2013 Albert Wang. All rights reserved.
//

#import <Foundation/Foundation.h>
#include <map>
#include <vector>

typedef void(^CallbackType)(NSError *, NSString*);
@interface DiskImageLRU : NSObject
{
    std::map<std::string, std::vector<CallbackType>> callbacks;
    NSString * cachePath;
}

+ (DiskImageLRU *) cacheWithCapacity:(NSUInteger)bytes;

- (id) init;
- (void) loadImage:(NTVImage *)image finished:(CallbackType)cb;
- (void) loadThumbnail:(NTVImage *)image finished:(CallbackType)cb;
- (void) clearCurrentCallbacks;

//Methods for the delegate to hit
- (void) failure:(NSString *)name error:(NSError *)error;
- (void) success:(NSString *)name path:(NSString *)path;
@end

@interface FileDownloadDelegate : NSObject<NSURLConnectionDelegate>
{
    DiskImageLRU * lruCache;
    NSString * name;
    NSString * path;
    NSFileHandle * handle;
    bool failed;
}

+ (FileDownloadDelegate *) downloadDelegateTo:(NSString *)path named:(NSString *)name cache:(DiskImageLRU *)cc;

- (void) connection:(NSURLConnection *)connection didFailWithError:(NSError *)error;
- (void) connection:(NSURLConnection *)connection didReceiveResponse:(NSURLResponse *)response;
- (void) connection:(NSURLConnection *)connection didReceiveData:(NSData *)data;
- (void) connectionDidFinishLoading:(NSURLConnection *)connection;

@end