#import <Foundation/Foundation.h>

@interface NTVStreamMux : NSInputStream {
	NSMutableArray *streams;
	NSUInteger size;
	
	NSStreamStatus status;
	id <NSStreamDelegate> delegate;
	NSRunLoop *runLoop;
	NSString *runLoopMode;
	NSDictionary *properties;
}

@property (nonatomic, readonly, assign) NSUInteger size;

- (NTVStreamMux*) init;
- (NTVStreamMux*) initWithStreams:(NSArray*)streams;

- (void) appendData:(NSData*)data;
- (void) appendFile:(NSString*)path;

- (NSUInteger) size;

@end