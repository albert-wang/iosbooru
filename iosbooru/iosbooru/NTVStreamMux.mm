#import "NTVStreamMux.h"

@implementation NTVStreamMux
@synthesize size;

- (NTVStreamMux*) init {
	streams = [[NSMutableArray alloc] init];
	status = NSStreamStatusNotOpen;
	size = 0;
	return self;
}

- (NTVStreamMux*) initWithStreams:(NSArray*)x {
	streams = [[NSMutableArray alloc] initWithArray:x];
	status = NSStreamStatusNotOpen;
	size = 0;
	return self;
}

- (void) dealloc {
	[runLoop release];
	[runLoopMode release];
	[streams release];
	[super dealloc];
}

- (void) appendStream:(NSInputStream*)stream withSize:(NSUInteger)s {
	[streams addObject:stream];
	size += s;
}

- (void) appendData:(NSData *)data {
	[self appendStream:[NSInputStream inputStreamWithData:data] withSize:[data length]];
}

- (void) appendFile:(NSString *)path {
    NSError * err = nil;
	NSInteger filesize = [[[NSFileManager defaultManager] attributesOfItemAtPath:path error:&err] fileSize];
    
    if (err)
    {
        NSLog(@"%@", err);
    }
	[self appendStream:[NSInputStream inputStreamWithFileAtPath:path] withSize:filesize];
}

- (NSUInteger) size {
    return size;
}

#pragma mark NSInputStream Overrides

- (NSInteger) read:(uint8_t*)buffer maxLength:(NSUInteger)len {
	status = NSStreamStatusReading;
	
	if ([streams count] == 0) {
		status = NSStreamStatusAtEnd;
		return 0;
	}
	
	NSInteger count = 0;
	
	while (count < len && [streams count]) {
		NSInputStream *is = [streams objectAtIndex:0];
        
        /* Pre-emptively open streams */
        if ([is streamStatus] == NSStreamStatusNotOpen) {
            [is open];
        }
        
		NSInteger read = [is read:(buffer + count) maxLength:(len - count)];
		count += read;
		
		if (read == 0 || [is streamStatus] == NSStreamStatusAtEnd) {
			/* Stream is finished, pop it and go to the next one */
			[is setDelegate:nil];
			[is close];
			[streams removeObjectAtIndex:0];
			
			if ([streams count]) {
				is = [streams objectAtIndex:0];
				[is open];
				
				if (delegate) {
					[is setDelegate:delegate];
				}
				
				if (runLoop) {
					[is scheduleInRunLoop:runLoop forMode:runLoopMode];
				}
			}
			
			continue;
		}
		else if (read == -1) {
			/* ...something is broken */
			status = NSStreamStatusError;
            NSLog(@"Mux'd NSInputStream returned an error: %@", [is streamError]);
            [NSException raise:@"stream_error" format:@"Mux'd NSInputStream returned an error: %@", [is streamError]];
		}
		else {
			/* Stream returned non-zero; it may not be len bytes but that means
			 * the stream still potentially has data waiting */
			break;
		}
	}
	
	if ([streams count] == 0) {
		status = NSStreamStatusAtEnd;
	}
	
	return count;
}

- (BOOL) getBuffer:(uint8_t**)buffer length:(NSUInteger *)len {
	if ([streams count] == 0)
		return NO;
	
	return [[streams objectAtIndex:0] getBuffer:buffer length:len];
}

- (BOOL) hasBytesAvailable {
	if ([streams count] == 1) {
		return [[streams objectAtIndex:0] hasBytesAvailable];
	}
	else if ([streams count] == 0) {
		return NO;
	}
	else {
		return YES;
	}
}

#pragma mark NSStream Overrides

- (void) open {		
	status = NSStreamStatusOpen;
	
	if ([streams count]) {
		[[streams objectAtIndex:0] open];
	}
}

- (void) close {
	status = NSStreamStatusClosed;
	[streams removeAllObjects];
}

- (id <NSStreamDelegate>) delegate {
	return delegate;
}

- (void) setDelegate:(id <NSStreamDelegate>)d {
	delegate = d;
	
	if ([streams count]) {
		[[streams objectAtIndex:0] setDelegate:d];
	}
}

- (NSStreamStatus) streamStatus {
	return status;
}

- (NSError*) streamError {
	return nil;
}

- (void)scheduleInRunLoop:(NSRunLoop*)aRunLoop forMode:(NSString*)mode {
	runLoop = [aRunLoop retain];
	runLoopMode = [mode retain];
	
	if ([streams count]) {
		[[streams objectAtIndex:0] scheduleInRunLoop:aRunLoop forMode:mode];
	}
}

- (void)removeFromRunLoop:(NSRunLoop*)aRunLoop forMode:(NSString*)mode {
	[runLoop release];
	[runLoopMode release];
	runLoop = nil;
	runLoopMode = nil;
	
	if ([streams count]) {
		[[streams objectAtIndex:0] removeFromRunLoop:aRunLoop forMode:mode];
	}
}

#pragma mark Undocumented but necessary NSStream Overrides (fuck you Apple)

- (void) _scheduleInCFRunLoop:(NSRunLoop*) inRunLoop forMode:(id)inMode {
	/* FUCK YOU APPLE */
}

- (void) _setCFClientFlags:(CFOptionFlags)inFlags 
				  callback:(CFReadStreamClientCallBack)inCallback 
				   context:(CFStreamClientContext)inContext
{
	/* NO SERIOUSLY, FUCK YOU */
}


@end
