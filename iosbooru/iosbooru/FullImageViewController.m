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

+ (FullImageViewController *) createWithParentView:(UIView *)view path:(NSString *)path image:(NTVImage *)image datastore:(NTVDatastore *)ds
{
    return [[[FullImageViewController alloc] initWithParentView:view path:path image:image datastore:ds] autorelease];
}

- (id) initWithParentView:(UIView *)parent path:(NSString *)inputPath image:(NTVImage *)img datastore:(NTVDatastore *)ds
{
    self = [super initWithNibName:nil bundle:nil];
    if (self)
    {
        self->parentView = [parent retain];
        self->path = [inputPath retain];
        self->image = [img retain];
        self->datatore = [ds retain];
        
        [self loadView];
    }
    return self;
}

- (void) loadView
{
    UIWebView * view = [[[UIWebView alloc] initWithFrame:CGRectMake(0, 0, parentView.frame.size.width, parentView.frame.size.height)] autorelease];
    view.backgroundColor = [UIColor blackColor];
    view.delegate = self;
    
    NSString * imageFormat = [NSString stringWithContentsOfFile:[[NSBundle mainBundle] pathForResource:@"ImageHtmlPage" ofType:nil] encoding:NSUTF8StringEncoding error:nil];
    NSString * fullData = [NSString stringWithFormat:imageFormat, parentView.frame.size.height, [NSURL fileURLWithPath:path]];
    
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
    
    //Tag button.
    tagButton = [[[UIImageView alloc] initWithImage:[UIImage imageNamed:@"Tag"]] autorelease];
    tagButton.frame = CGRectMake(view.frame.size.width - 40, view.frame.size.height - 40, 40, 40);
    tagButton.userInteractionEnabled = YES;
    [tagButton retain];
    
    UITapGestureRecognizer * tagRecog = [[UITapGestureRecognizer alloc] initWithTarget:self action:@selector(tagTapped:)];
    [tagButton addGestureRecognizer:tagRecog];
    [tagRecog release];
    [view addSubview:tagButton];
    
    [parentView addSubview:view];
    self.view = view;
}

- (void) dealloc
{
    [super dealloc];
    [self->parentView release];
    [self->backButton release];
    [self->tagButton release];
    [self->path release];
    [self->image release];
    [self->datastore release];
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
        
        [(ImageboardView *)parentView reappearImages];
        [self.view removeFromSuperview];
    }
}

- (void) tagTapped:(UITapGestureRecognizer *)sender
{
    if (sender.state == UIGestureRecognizerStateEnded)
    {
        TagViewController * controller = [[[TagViewController alloc] initWithImageReference:image datastore:datastore] autorelease];
    }
}

@end
