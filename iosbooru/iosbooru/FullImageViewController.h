//
//  FullImageView.h
//  iosbooru
//
//  Created by Albert Wang on 1/18/13.
//  Copyright (c) 2013 Albert Wang. All rights reserved.
//

#import <UIKit/UIKit.h>

@interface FullImageViewController : UIViewController<UIWebViewDelegate>
{
    UIView * parentView;
    UIView * backButton;
}

+ (FullImageViewController *) createWithParentView:(UIView *)view path:(NSString *)path;
- (id) initWithParentView:(UIView *)view path:(NSString *)path;

@end
