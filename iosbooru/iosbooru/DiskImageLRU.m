//
//  DiskImageLRU.m
//  iosbooru
//
//  Created by Albert Wang on 1/17/13.
//  Copyright (c) 2013 Albert Wang. All rights reserved.
//

#import "DiskImageLRU.h"

@implementation FileDownloadDelegate

+ (FileDownloadDelegate *) downloadDelegateTo:(NSString *)path named:(NSString *)name cache:(DiskImageLRU *)cc;
{
    FileDownloadDelegate * result = [[FileDownloadDelegate alloc] init];
    result->name = [name retain];
    result->path = [path retain];
    result->lruCache = [cc retain];
    result->failed = false;
    
    NSFileManager * manager = [NSFileManager defaultManager];
    if (![manager createFileAtPath:path contents:nil attributes:nil])
    {
        NSLog(@"Failed to create a file at path: %@", path);
        return result;
    }
    
    result->handle = [[NSFileHandle fileHandleForWritingAtPath:path] retain];
    return result;
}

- (void) dealloc
{
    [super dealloc];
    
    [self->name release];
    [self->lruCache release];
    [self->handle release];
    [self->path release];
}

- (void) connection:(NSURLConnection *)connection didFailWithError:(NSError *)error
{
    NSLog(@"Connection Error: %@", error);

    self->failed = true;
    [self->lruCache failure:self->name error:error];
}

- (void) connection:(NSURLConnection *)connection didReceiveResponse:(NSURLResponse *)response
{
    //NSLog(@"Connection got response.");
    if ([response respondsToSelector:@selector(statusCode)])
    {
        int statusCode = [(NSHTTPURLResponse *)response statusCode];
        if (statusCode != 200)
        {
            self->failed = true;
            [connection cancel];
            return [self->lruCache failure:self->name error:[NSError errorWithDomain:@"something" code:404 userInfo:nil]];
        }
    }
}

- (void) connection:(NSURLConnection *)connection didReceiveData:(NSData *)data
{
    [self->handle writeData:data];
}

- (void) connectionDidFinishLoading:(NSURLConnection *)connection
{
    [self->handle closeFile];

    if (!self->failed)
    {
        [self->lruCache success:self->name path:self->path];
    }
}

@end


@implementation DiskImageLRU
+ (DiskImageLRU *) cacheWithCapacity:(NSUInteger)bytes
{
    //Haha, ignore the bytes capacity value for now.
    DiskImageLRU * result = [[[DiskImageLRU alloc] init] autorelease];
    return result;
}

- (id) init
{
    if (self = [super init])
    {
        //Make a /cache/ path in the documents directory.
        NSString * documentPath = [NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, true) objectAtIndex:0];
        NSFileManager * manager = [NSFileManager defaultManager];

        NSError * err = nil;
        
        self->cachePath = [[documentPath stringByAppendingPathComponent:@"cache"] retain];
        [manager createDirectoryAtPath:self->cachePath withIntermediateDirectories:FALSE attributes:nil error:&err];
        
        if (err)
        {
            NSLog(@"Error creating directory: %@", err);
        }
    }
    
    return self;
}

- (void) dealloc
{
    [super dealloc];
    [self->cachePath release];
}

- (void) clearCurrentCallbacks
{
    dispatch_async(dispatch_get_main_queue(), ^{
        for (auto it = self->callbacks.begin(); it != self->callbacks.end(); ++it)
        {
            for (size_t i = 0; i < it->second.size(); ++i)
            {
                [it->second[i] release];
                it->second[i] = nil;
            }
        }
        
        self->callbacks.clear();
    });
}

- (void) loadImage:(NTVImage *)image finished:(CallbackType)cb
{
    //Images are found at http://img.uncod.in/img/<primary-key>.png
    
    NSLog(@"%@", image.mime);
    NSString * imageURLFormat = @"http://img.uncod.in/img/%@";
    NSString * guidString = [NSString stringWithUTF8String:image.primaryKey.toString().c_str()];
    NSString * url = [NSString stringWithFormat:imageURLFormat, guidString];
    
    if ([image.mime isEqualToString:@"image/jpeg"])
    {
        url = [url stringByAppendingString:@".jpeg"];
    }
    
    if ([image.mime isEqualToString:@"image/png"])
    {
        url = [url stringByAppendingString:@".png"];
    }
    
    if ([image.mime isEqualToString:@"image/gif"])
    {
        url = [url stringByAppendingString:@".gif"];
    }
    

    [self downloadFileAt:url as:guidString withCallback:^(NSError *e, NSString * path)
     {
         if (e)
         {
             NSLog(@"Image not found, error: %@", e);
         }
         
        cb(e, path);
     }];
}

- (void) loadThumbnail:(NTVImage *)image finished:(CallbackType)cb
{
    NSString * thumbURLFormat = @"http://img.uncod.in/thumb/%@_thumb.jpg";
    //Thumbnails are found at http://img.uncod.in/thumb/<primary-key_thumb.jpg
    NSString * guidString = [NSString stringWithUTF8String:image.primaryKey.toString().c_str()];
    NSString * url = [NSString stringWithFormat:thumbURLFormat, guidString];
    
    CallbackType callbackCopy = [cb copy];
    
    [self downloadFileAt:url as:[guidString stringByAppendingString:@"-thumb"] withCallback:^(NSError *e, NSString * path)
     {
         if (e)
         {
             NSLog(@"Image not found, trying to get alternate file.");
             NSString * altThumbURLFormat = @"http://img.uncod.in/thumb/%@_thumb-0.jpg";
             NSString * altUrl = [NSString stringWithFormat:altThumbURLFormat, guidString];
             [self downloadFileAt:altUrl as:[guidString stringByAppendingString:@"-thumb"] withCallback:callbackCopy];
             [callbackCopy release];
         }
         else
         {
             callbackCopy(e, path);
             [callbackCopy release];
         }
     }];
}

- (void) downloadFileAt:(NSString *)url as:(NSString *)name withCallback:(CallbackType)cb
{
    dispatch_async(dispatch_get_main_queue(), ^{
        std::string n = [name UTF8String];
        
        //If the file is being downloaded, append the callback to the finished array.
        auto it = self->callbacks.find(n);
        if (it != self->callbacks.end())
        {
            //Its currently downloading. Append to the array.
            it->second.push_back([cb copy]);
            return;
        }
        
        NSString * targetPath = [self->cachePath stringByAppendingPathComponent:name];
        
        //If the file exists on disk, call the callback with the downloaded path.
        NSFileManager * manager = [NSFileManager defaultManager];
        if ([manager fileExistsAtPath:targetPath])
        {
            NSError * err = nil;
            NSDictionary * type = [manager attributesOfFileSystemForPath:targetPath error:&err];
            
            if (err)
            {
                cb(err, nil);
                return;
            }
            
            NSNumber * val = [type objectForKey:NSFileSize];
            
            if ([val intValue] != 0)
            {
                cb(nil, targetPath);
                return;
            }
        }
        
        //Otherwise, start a download.
        self->callbacks[n].push_back([cb copy]);
        NSURLRequest * req = [NSURLRequest requestWithURL:[NSURL URLWithString:url]];
        
        FileDownloadDelegate * del = [FileDownloadDelegate downloadDelegateTo:targetPath named:name cache:self];
        [NSURLConnection connectionWithRequest:req delegate:del];
    });
}

- (void) failure:(NSString *)name error:(NSError *)error
{
    std::string n = [name UTF8String];
    
    auto it = self->callbacks.find(n);
    if (it == self->callbacks.end())
    {
        NSLog(@"Failed %@ with error %@, but no callbacks to handle it.", name, error);
        return;
    }
    
    std::vector<CallbackType> cbs = it->second;
    self->callbacks.erase(it);
    
    for (size_t i = 0; i < cbs.size(); ++i)
    {
        cbs[i](error, nil);
        [cbs[i] release];
    }
}

- (void) success:(NSString *)name path:(NSString *)path
{
    std::string n = [name UTF8String];
    
    auto it = self->callbacks.find(n);
    if (it == self->callbacks.end())
    {
        NSLog(@"Success for %@, but no callbacks to handle it.", name);
        return;
    }

    std::vector<CallbackType> cbs = it->second;
    self->callbacks.erase(it);
    
    for (size_t i = 0; i < cbs.size(); ++i)
    {
        cbs[i](nil, path);
        [cbs[i] release];
    }
}

@end
