#import <Foundation/Foundation.h>
#import "NTVStreamMux.h"

@interface MultipartRequest : NSMutableURLRequest {
	NSMutableDictionary *parts;
    NSMutableDictionary *types;
	NSString *boundary;
	
	NSUInteger size;
}

@property (nonatomic, retain, readonly) NSString *boundary;
@property (nonatomic, assign, readonly) NSUInteger size;

- (MultipartRequest *) initWithURL:(NSURL *)url;
- (void) addPart:(NSData*)data withName:(NSString*)name andContentType:(NSString *)type;
- (void) addPartFromPath:(NSString*)path withName:(NSString*)name andContentType:(NSString *)type;

- (void) prepareToSend;

- (NSData*) HTTPBody;
- (NSInputStream*) HTTPBodyStream;

- (void) setHTTPMethod:(NSString*)method;
- (void) setHTTPBody:(NSData*)data;
- (void) setHTTPBodyStream:(NSInputStream*)stream;

@end
