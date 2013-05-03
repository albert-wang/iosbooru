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
    UIView * tagButton;
    
    NSString * path;
    NTVImage * image;
    NTVDatastore * datastore;
}

+ (FullImageViewController *) createWithParentView:(UIView *)view path:(NSString *)path image:(NTVImage *)image datastore:(NTVDatastore *)ds;
- (id) initWithParentView:(UIView *)view path:(NSString *)path image:(NTVImage *)image datastore:(NTVDatastore *)ds;

@end
