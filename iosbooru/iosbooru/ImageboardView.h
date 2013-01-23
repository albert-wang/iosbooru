//
//  ImageboardView.h
//  iosbooru
//
//  Created by Albert Wang on 1/17/13.
//  Copyright (c) 2013 Albert Wang. All rights reserved.
//

#import <UIKit/UIKit.h>
#import "DiskImageLRU.h"

enum ImageboardDisplayStatus
{
    DISPLAY_BOARD,
    ANIMATING,
    DISPLAY_SINGLE_IMAGE
};

@interface ImageboardView : UIView<UIWebViewDelegate>
{
    DiskImageLRU * lruCache;
    UIImage * placeholder;
    size_t targetIndex;
    
    //The entire scene is just 12 image views.
    UIImageView * thumbnailDisplays[12];
    UIImageView * cameraButton;
    UIViewController *  delegate;
}

@property (strong, retain, nonatomic) DiskImageLRU * lruCache;
@property (assign, nonatomic) ImageboardDisplayStatus displayStatus;

- (id) initWithFrame:(CGRect)frame controller:(UIViewController *)controller;
- (void) loadImages:(NSArray *)images;
- (void) hideAll;
- (void) reappearImages;

- (bool) imageHasThumbnail:(size_t)ind;
- (size_t) indexFromClickedPoint:(CGPoint)point;

- (void) displayWebview:(UIWebView *)target;
@end
