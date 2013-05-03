//This header defines a set of protocols that the NTVDatastore will use to 
//Return the results of queries.
#import <Foundation/Foundation.h>
#include "NTVTraits.hpp"

@protocol NTVQueryDelegate
	- (void) queryDidReceiveRow:(id)value ofType:(Class)cls;
	- (void) queryDidFinish;
@end

