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
    <UIImagePickerControllerDelegate
    , UINavigationControllerDelegate
    , UIActionSheetDelegate
    , NSURLConnectionDelegate
    >
{
    NTVDatastore * datastore;
    FullImageViewController * fullImageController;
    UIPopoverController * possiblePopover;
    
    NSArray * currentImages;
    size_t offset;
    
    size_t networkRequestsOutstanding;
}

+ (ImageboardViewController *) createWithDatastore:(NTVDatastore *)ds;

- (void) cameraClicked:(UITapGestureRecognizer *)recog;
- (void) frameClicked:(UITapGestureRecognizer *)recog;

- (void) swipeLeft:(UISwipeGestureRecognizer *)recog;
- (void) swipeRight:(UISwipeGestureRecognizer *)recog;
@end
