#import <Foundation/Foundation.h>
#import <CoreGraphics/CoreGraphics.h>
#include "ormnetworking.hpp"

#include "json/json.h"

#import "NTVTag.h"
#import "NTVComment.h"
#import "NTVTagBridge.h"
#import "NTVImage.h"
#import "NTVUploadMetadata.h"
#import "NTVRating.h"
#import "NTVStatic.h"

#ifndef NTV_OBJC_NETWORKING_H
#define NTV_OBJC_NETWORKING_H

@protocol NTVNetworking <NSObject>
	- (int) computeSHA1HashOfFile:(NSString *)localPath output:(NSString **)o;
	- (int) createDeviceIndependentResources:(ORawrM::INetworkHandler*)owningHandler;
	- (int) sendJSONData:(const Json::Value *)root withOperation:(NSString *)operation andOutputPath:(NSString *)output callbacks:(ORawrM::INetworkCallbacks *)cbs;
	- (int) uploadFileWithJSON:(const Json::Value *)root atLocalPath:(NSString *)localPath withID:(NSString *)unique callbacks:(ORawrM::INetworkCallbacks *)cb;
@end


// The Obj-C -> C++ bridge class.
class ObjCNetworkHandler : public ORawrM::INetworkHandler
{
public:
	typedef ORawrM::INetworkHandler::ErrorHandle ErrorHandle;

	ObjCNetworkHandler(	id<NTVNetworking> handle );
	~ObjCNetworkHandler();

	ErrorHandle computeSHA1HashOfFile(const std::string& lp, std::string& o);
	ErrorHandle createDeviceIndependentResources();
	ErrorHandle sendJSONData(const Json::Value& root, const std::string& operation, boost::optional<std::string> outputPath, ORawrM::INetworkCallbacks * cbs);
	ErrorHandle uploadFileWithJSON(const Json::Value& root, const std::string& localPath, const std::string& uid, ORawrM::INetworkCallbacks * cbs);

	id<NTVNetworking> handle;
};

@interface NTVDownloadParameters : NSObject
	@property(nonatomic) CGSize size;
	@property(nonatomic, retain) NSString * connectionQuality;

	+(NTVDownloadParameters *) parametersWithSize:(CGSize)size andConnectionQuality:(NSString *)quality;
	-(NTVDownloadParameters *) initWithSize:(CGSize)size andConnectionQuality:(NSString *)quality;

	+(NTVDownloadParameters *) parametersForOriginal;
	+(NTVDownloadParameters *) parametersForThumbnail;
	+(NTVDownloadParameters *) parametersFor270x200;
	+(NTVDownloadParameters *) parametersFor900x300;
	+(NTVDownloadParameters *) parametersFor750x550;
	+(NTVDownloadParameters *) parametersFor1280x720;
	+(NTVDownloadParameters *) parametersFor1920x1080;
	+(NTVDownloadParameters *) parametersForLarge;

@end

// Callbacks, and the Obj-C -> C++ bridge class.
@interface NTVDeniedMutation : NSObject
	@property(nonatomic, assign) ORawrM::GUID guid;
	@property(nonatomic, retain) NSString* model;
	@property(nonatomic, retain) NSString* reason;
@end

@interface NTVNetworkFailure : NSObject
	@property(nonatomic, retain) NSArray* deniedObjects;
@end

@class NTVDatastore;
@protocol NTVNetworkCallbacks <NSObject>
	- (void) networkHandler:(id<NTVNetworking>)handle receivedData:(const Json::Value&)value;
	- (void) networkHandler:(id<NTVNetworking>)handle gotError:(int)errorCode withMessage:(NSString *)msg withFailures:(NTVNetworkFailure*)failures;
	- (void) networkFinished:(NSString *)extras;

	@optional
	- (void) receivedTag:(NSArray *)models named:(NSString *)name withCount:(NSUInteger)count;
	@optional
	- (void) receivedComment:(NSArray *)models named:(NSString *)name withCount:(NSUInteger)count;
	@optional
	- (void) receivedTagBridge:(NSArray *)models named:(NSString *)name withCount:(NSUInteger)count;
	@optional
	- (void) receivedImage:(NSArray *)models named:(NSString *)name withCount:(NSUInteger)count;
	@optional
	- (void) receivedUploadMetadata:(NSArray *)models named:(NSString *)name withCount:(NSUInteger)count;
	@optional
	- (void) receivedRating:(NSArray *)models named:(NSString *)name withCount:(NSUInteger)count;
	@optional
	- (void) receivedStatic:(NSArray *)models named:(NSString *)name withCount:(NSUInteger)count;
@end

class ObjCNetworkCallbacks : public ORawrM::INetworkCallbacks
{
public:
	ObjCNetworkCallbacks(id<NTVNetworkCallbacks> handle, NTVDatastore * datastore);
	ObjCNetworkCallbacks(const ObjCNetworkCallbacks& other);
	~ObjCNetworkCallbacks();
	ObjCNetworkCallbacks& operator=(const ObjCNetworkCallbacks& other);


	void onReceivedData(ORawrM::INetworkHandler * handler, const Json::Value& value);
	void onError(ORawrM::INetworkHandler * handler, ORawrM::INetworkHandler::ErrorHandle err, const std::string& message, const Json::Value& data);
	void finished(boost::optional<std::string> extra);

	template<typename DS>
	void operator()(DS& store, const std::string& modelname, const std::vector<Tag>& models, size_t count)
	{
		if ([handle respondsToSelector:@selector(receivedTag:named:withCount:)])
		{
			NSMutableArray * buffer = [NSMutableArray arrayWithCapacity:models.size()];
			for (size_t i = 0; i < models.size(); ++i)
			{
				NTVTag * m = [NTVTag createWithValue:models[i] andDatastore:datastore];
				[buffer addObject:m];
			}
			[handle receivedTag:buffer named:[NSString stringWithUTF8String:modelname.c_str()] withCount:count];
		}
	}
	template<typename DS>
	void operator()(DS& store, const std::string& modelname, const std::vector<Comment>& models, size_t count)
	{
		if ([handle respondsToSelector:@selector(receivedComment:named:withCount:)])
		{
			NSMutableArray * buffer = [NSMutableArray arrayWithCapacity:models.size()];
			for (size_t i = 0; i < models.size(); ++i)
			{
				NTVComment * m = [NTVComment createWithValue:models[i] andDatastore:datastore];
				[buffer addObject:m];
			}
			[handle receivedComment:buffer named:[NSString stringWithUTF8String:modelname.c_str()] withCount:count];
		}
	}
	template<typename DS>
	void operator()(DS& store, const std::string& modelname, const std::vector<TagBridge>& models, size_t count)
	{
		if ([handle respondsToSelector:@selector(receivedTagBridge:named:withCount:)])
		{
			NSMutableArray * buffer = [NSMutableArray arrayWithCapacity:models.size()];
			for (size_t i = 0; i < models.size(); ++i)
			{
				NTVTagBridge * m = [NTVTagBridge createWithValue:models[i] andDatastore:datastore];
				[buffer addObject:m];
			}
			[handle receivedTagBridge:buffer named:[NSString stringWithUTF8String:modelname.c_str()] withCount:count];
		}
	}
	template<typename DS>
	void operator()(DS& store, const std::string& modelname, const std::vector<Image>& models, size_t count)
	{
		if ([handle respondsToSelector:@selector(receivedImage:named:withCount:)])
		{
			NSMutableArray * buffer = [NSMutableArray arrayWithCapacity:models.size()];
			for (size_t i = 0; i < models.size(); ++i)
			{
				NTVImage * m = [NTVImage createWithValue:models[i] andDatastore:datastore];
				[buffer addObject:m];
			}
			[handle receivedImage:buffer named:[NSString stringWithUTF8String:modelname.c_str()] withCount:count];
		}
	}
	template<typename DS>
	void operator()(DS& store, const std::string& modelname, const std::vector<UploadMetadata>& models, size_t count)
	{
		if ([handle respondsToSelector:@selector(receivedUploadMetadata:named:withCount:)])
		{
			NSMutableArray * buffer = [NSMutableArray arrayWithCapacity:models.size()];
			for (size_t i = 0; i < models.size(); ++i)
			{
				NTVUploadMetadata * m = [NTVUploadMetadata createWithValue:models[i] andDatastore:datastore];
				[buffer addObject:m];
			}
			[handle receivedUploadMetadata:buffer named:[NSString stringWithUTF8String:modelname.c_str()] withCount:count];
		}
	}
	template<typename DS>
	void operator()(DS& store, const std::string& modelname, const std::vector<Rating>& models, size_t count)
	{
		if ([handle respondsToSelector:@selector(receivedRating:named:withCount:)])
		{
			NSMutableArray * buffer = [NSMutableArray arrayWithCapacity:models.size()];
			for (size_t i = 0; i < models.size(); ++i)
			{
				NTVRating * m = [NTVRating createWithValue:models[i] andDatastore:datastore];
				[buffer addObject:m];
			}
			[handle receivedRating:buffer named:[NSString stringWithUTF8String:modelname.c_str()] withCount:count];
		}
	}
	template<typename DS>
	void operator()(DS& store, const std::string& modelname, const std::vector<Static>& models, size_t count)
	{
		if ([handle respondsToSelector:@selector(receivedStatic:named:withCount:)])
		{
			NSMutableArray * buffer = [NSMutableArray arrayWithCapacity:models.size()];
			for (size_t i = 0; i < models.size(); ++i)
			{
				NTVStatic * m = [NTVStatic createWithValue:models[i] andDatastore:datastore];
				[buffer addObject:m];
			}
			[handle receivedStatic:buffer named:[NSString stringWithUTF8String:modelname.c_str()] withCount:count];
		}
	}
	id<NTVNetworkCallbacks> handle;
private:
	NTVDatastore * datastore;
};
#endif

