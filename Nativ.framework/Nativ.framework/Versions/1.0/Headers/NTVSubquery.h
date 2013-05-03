#import <Foundation/Foundation.h>

@interface NTVSubquery : NSObject
{
	NSString * subgraph;
	NSString * relationshipName;
	NSString * subqueryName;
	size_t limit;
	size_t offset;
};

@property (retain) NSString * subgraph;
@property (retain) NSString * relationshipName;
@property (retain) NSString * subqueryName;
@property (nonatomic, assign) size_t limit;
@property (nonatomic, assign) size_t offset;

+ (NTVSubquery *) subqueryNamed:(NSString *)s onSubgraph:(NSString *)graph onRelation:(NSString *)relation;
- (NTVSubquery *) initWithName:(NSString *)s onSubgraph:(NSString *)graph onRelation:(NSString *)relation;
@end