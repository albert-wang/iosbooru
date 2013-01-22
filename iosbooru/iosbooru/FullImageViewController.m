//
//  FullImageViewController.m
//  iosbooru
//
//  Created by Albert Wang on 1/18/13.
//  Copyright (c) 2013 Albert Wang. All rights reserved.
//

#import "FullImageViewController.h"
#import "ImageboardView.h"

@implementation FullImageViewController

+ (FullImageViewController *) createWithParentView:(UIView *)view path:(NSString *)path
{
    return [[[FullImageViewController alloc] initWithParentView:view path:path] autorelease];
}

- (id) initWithParentView:(UIView *)parent path:(NSString *)path
{
    self = [super initWithNibName:nil bundle:nil];
    if (self)
    {
        self->parentView = [parent retain];
        
        UIWebView * view = [[[UIWebView alloc] initWithFrame:CGRectMake(0, 0, parent.frame.size.width, parent.frame.size.height)] autorelease];
        view.backgroundColor = [UIColor blackColor];
        view.delegate = self;
        
        NSString * imageFormat = [NSString stringWithContentsOfFile:[[NSBundle mainBundle] pathForResource:@"ImageHtmlPage" ofType:nil] encoding:NSUTF8StringEncoding error:nil];
        NSString * fullData = [NSString stringWithFormat:imageFormat, parent.frame.size.height, [NSURL fileURLWithPath:path]];
        
        [view loadHTMLString:fullData baseURL:nil];
        view.scalesPageToFit = YES;
        view.opaque = NO;
        view.hidden = true;
        
        //Add in a back button.
        backButton = [[[UIImageView alloc] initWithImage:[UIImage imageNamed:@"Left"]] autorelease];
        backButton.frame = CGRectMake(0, view.frame.size.height - 40, 40, 40);
        backButton.userInteractionEnabled = YES;
        [backButton retain];
        
        UITapGestureRecognizer * tapRecog = [[UITapGestureRecognizer alloc] initWithTarget:self action:@selector(backTapped:)];
        [backButton addGestureRecognizer:tapRecog];
        [tapRecog release];
        
        [view addSubview:backButton];
        [parent addSubview:view];
        self.view = view;
    }
    return self;
}

- (void) dealloc
{
    [super dealloc];
    [self->parentView release];
    [self->backButton release];
}

- (void) webViewDidFinishLoad:(UIWebView *)webView
{
    [(ImageboardView *)self->parentView displayWebview:webView];
}

- (void) backTapped:(UITapGestureRecognizer *)sender
{
    if (sender.state == UIGestureRecognizerStateEnded)
    {
        backButton.userInteractionEnabled = NO;
        [(UIWebView *)self.view setDelegate:nil];
        
        [self.view removeFromSuperview];
        [(ImageboardView *)parentView reappearImages];
    }
}

@end
