//
//  TagViewController.h
//  iosbooru
//
//  Created by Albert Wang on 1/24/13.
//  Copyright (c) 2013 Albert Wang. All rights reserved.
//

#import <UIKit/UIKit.h>

@interface TagViewController : UIViewController
{
    NTVImage * image;
    NTVDatastore * datastore;
}

- (id) initWithImageReference:(NTVImage *)img datastore:(NTVDatastore *)ds;
@end
