//
//  ImageboardViewController.h
//  iosbooru
//
//  Created by Albert Wang on 1/17/13.
//  Copyright (c) 2013 Albert Wang. All rights reserved.
//

#import <UIKit/UIKit.h>
#import "FullImageViewController.h"

@interface ImageboardViewController : UIViewController
{
    NTVDatastore * datastore;
    FullImageViewController * fullImageController;
    
    NSArray * currentImages;
    size_t offset;
}

+ (ImageboardViewController *) createWithDatastore:(NTVDatastore *)ds;

- (void) leftClicked:(UITapGestureRecognizer *)recog;
- (void) rightClicked:(UITapGestureRecognizer *)recog;
- (void) frameClicked:(UITapGestureRecognizer *)recog;

- (void) swipeLeft:(UISwipeGestureRecognizer *)recog;
- (void) swipeRight:(UISwipeGestureRecognizer *)recog;
@end
