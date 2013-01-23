#import "NTVMultipartRequest.h"
#import "NTVStreamMux.h"

@implementation MultipartRequest
@synthesize boundary;
@synthesize size;

- (MultipartRequest*) initWithURL:(NSURL *)URL {
	NSString *path = [URL path];
	NSFileManager *fileManager = [NSFileManager defaultManager];
	NSString *fileType = [[fileManager attributesOfItemAtPath:[URL path] error:nil] fileType];

	if ([fileType isEqualToString:NSFileTypeSymbolicLink]) {
		NSString *destinationPath = [fileManager destinationOfSymbolicLinkAtPath:path error:nil];
		self = [super initWithURL:[NSURL URLWithString:destinationPath]];
	} else {
		self = [super initWithURL:URL];
	}
	
	parts = [[NSMutableDictionary alloc] init];
    types = [[NSMutableDictionary alloc] init];
	boundary = [[NSString stringWithFormat:@"XXX---BOUNDARY---%@", [[NSProcessInfo processInfo] globallyUniqueString]] retain];
	
	[self setHTTPMethod:@"POST"];
	[self setValue:[NSString stringWithFormat:@"multipart/mixed; boundary=%@", boundary] forHTTPHeaderField:@"Content-Type"];
	
	size = 0;
	
	return self;
}

- (void) dealloc {
	[boundary release];
	[parts release];
    [types release];
    [super dealloc];
}

- (void) addPart:(NSData*)data withName:(NSString*)name andContentType:(NSString *)type {
	if (![parts valueForKey:name]) {
		[parts setValue:data forKey:name];
		size += [data length];
        
        if (type)
        {
            [types setValue:type forKey:name];
        }
	}
}

- (void) addPartFromPath:(NSString*)path withName:(NSString*)name andContentType:(NSString *)type {
	if (![parts valueForKey:name]) {
		NSString *destinationPath = path;
		NSFileManager *fileManager = [NSFileManager defaultManager];
		NSDictionary *attributes = [fileManager attributesOfItemAtPath:path error:nil];
		
		if ([[attributes fileType] isEqualToString:NSFileTypeSymbolicLink]) {
			destinationPath = [fileManager destinationOfSymbolicLinkAtPath:path error:nil];
		}
	
		[parts setValue:destinationPath forKey:name];
		NSInteger filesize = [[fileManager attributesOfItemAtPath:destinationPath error:nil] fileSize];
		size += filesize;
        
        if (type)
        {
            [types setValue:type forKey:name];
        }
	}
}

- (void) prepareToSend {
	[super setHTTPBodyStream:[self HTTPBodyStream]];
	[super setValue:[NSString stringWithFormat:@"%d", size] forHTTPHeaderField:@"Content-Length"];
}

#pragma mark NSMutableURLRequest Overrides

- (NSData*) HTTPBody {
	NSInputStream *mux = [self HTTPBodyStream];
	NSMutableData *data = [[[NSMutableData alloc] initWithLength:size] autorelease];
	
	const NSInteger blockSize = 4096;
	uint8_t *buffer = (uint8_t *)malloc(blockSize * sizeof(uint8_t));
	NSInteger read = 0;
	
	while (0 < (read = [mux read:buffer maxLength:blockSize])) {
		[data appendBytes:buffer length:read];
	}
	
	free(buffer);
	return data;
}

- (NSInputStream*) HTTPBodyStream {
	NTVStreamMux *mux = [[[NTVStreamMux alloc] init] autorelease];
	__block BOOL first = YES;
	
	[parts enumerateKeysAndObjectsUsingBlock:^(id key, id obj, BOOL *stop) {			
		NSString *headers = @"";
		if (first) {
			first = NO;
		}
		else {
			headers = [headers stringByAppendingFormat:@"\r\n"];
		}
		
		if ([obj isKindOfClass:[NSData class]] && [obj length] > 0) {
			headers = [headers stringByAppendingFormat:@"%@", [NSString stringWithFormat:@"--%@\r\n", boundary]];
			headers = [headers stringByAppendingFormat:@"Content-Disposition: form-data; name=\"%@\";\r\n", key];
			headers = [headers stringByAppendingFormat:@"Content-Length: %d\r\n", [obj length]];
            
            NSString * optionalType = [types objectForKey:key];
            if (!optionalType)
            {
                optionalType = @"application/json";
            }
            
			headers = [headers stringByAppendingFormat:@"Content-Type: %@\r\n", optionalType];
			headers = [headers stringByAppendingFormat:@"\r\n"];
			
			[mux appendData:[headers dataUsingEncoding:NSUTF8StringEncoding]];
			[mux appendData:obj];
		}
		else if ([obj isKindOfClass:[NSString class]] && [obj length] > 0) {
			NSInteger filesize = [[[NSFileManager defaultManager] attributesOfItemAtPath:obj error:nil] fileSize];
			
			headers = [headers stringByAppendingFormat:@"%@", [NSString stringWithFormat:@"--%@\r\n", boundary]];
			headers = [headers stringByAppendingFormat:@"Content-Disposition: form-data; name=\"%@\"; filename=\"%@\"\r\n", key, key];
			headers = [headers stringByAppendingFormat:@"Content-Length: %d\r\n", filesize];
            
            NSString * optionalType = [types objectForKey:key];
            if (!optionalType)
            {
                optionalType = @"application/json";
            }
            
			headers = [headers stringByAppendingFormat:@"Content-Type: %@\r\n", optionalType];
			headers = [headers stringByAppendingFormat:@"\r\n"];
			
			[mux appendData:[headers dataUsingEncoding:NSUTF8StringEncoding]];
			[mux appendFile:obj];
		}
	}];
	
	/* Need to postpend the final boundary! */
	NSString *postfix = [NSString stringWithFormat:@"\r\n--%@--\r\n\r\n", boundary];
	[mux appendData:[postfix dataUsingEncoding:NSUTF8StringEncoding]];
	
	size = [mux size];
	
	return mux;
}

- (void) setHTTPMethod:(NSString*)method {
	return [super setHTTPMethod:method];
}

#pragma mark Banned Functions ;_;

- (void) setHTTPBody:(NSData*)data {
	[NSException raise:@"invalid_method" format:@"You're not allowed to call this function ;_;"];
}

- (void) setHTTPBodyStream:(NSInputStream*)stream {
	[NSException raise:@"invalid_method" format:@"You're not allowed to call this function ;_;"];
}

@end

