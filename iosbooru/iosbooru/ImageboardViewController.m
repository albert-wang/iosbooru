//
//  ImageboardViewController.m
//  iosbooru
//
//  Created by Albert Wang on 1/17/13.
//  Copyright (c) 2013 Albert Wang. All rights reserved.
//

#import "ImageboardViewController.h"
#import "ImageboardView.h"
#import "BlockNetworkingDelegate.h"

@interface ImageboardViewController ()

@end

@implementation ImageboardViewController

- (id)initWithDatastore:(NTVDatastore *)ds
{
    self = [super initWithNibName:nil bundle:nil];
    if (self) {
        self->datastore = [ds retain];
        self->currentImages = nil;
        self->fullImageController = nil;
        self->offset = 0;
    }
    return self;
}

+ (ImageboardViewController *) createWithDatastore:(NTVDatastore *)ds
{
    ImageboardViewController * result = [[[ImageboardViewController alloc] initWithDatastore:ds] autorelease];
    result.view = [[ImageboardView alloc] initWithFrame: [[UIScreen mainScreen] bounds] controller:result];
    
    //Log the user in as the test client.
    [ds authenticateUser:@"test" withPassword:@"test" delegate: [BlockNetworkingDelegate createWithSuccess:^(NSString *) {
        [result reloadImages];
        NSLog(@"Finished authenticating user");
    }]];
    
    return result;
}

- (void) dealloc
{
    [super dealloc];
    [datastore release];
    [currentImages release];
    [fullImageController release];
}

- (void)reloadImages
{
    if (![self->datastore authenticationToken])
    {
        NSLog(@"Attempted to reload images without a valid authentication token? No possible response.");
        return;
    }

    //Get some images.
    NTVKeyPredicate * predicate = [NTVKeyPredicate defaultKeyPredicate];
    [predicate limit:12];
    [predicate offset:self->offset];
    [predicate orderBy:@"uploadedDate" isDescending:YES];
    
    [self->datastore externalQueryForImage:predicate withSubqueries:nil delegate: [BlockNetworkingDelegate createWithResults:^(NSArray * values, NSString *, NSUInteger count) {
        self->currentImages = [values retain];
        dispatch_async(dispatch_get_main_queue(), ^{
            [self loadImages:values];
        });
    }]];
}

- (void)next:(int)pages
{
    self->offset += pages * 12;
    [self reloadImages];
}

- (void)prev:(int)pages
{
    if (self->offset == 0)
    {
        return;
    }
    
    if (self->offset <= 12 * pages)
    {
        self->offset = 0;
    }
    else
    {
        self->offset -= 12 * pages;
    }
    
    [self reloadImages];
}

- (void) frameClicked:(UITapGestureRecognizer *)sender
{
    ImageboardView * view = (ImageboardView *)self.view;
    
    if (sender.state == UIGestureRecognizerStateEnded && view.displayStatus == DISPLAY_BOARD)
    {
        CGPoint clicked = [sender locationInView:nil];
        size_t index = [view indexFromClickedPoint:clicked];
        if ([view imageHasThumbnail:index])
        {
            [view hideAll];
            
            NTVImage * blowup = [currentImages objectAtIndex:index];
            assert(blowup);
            
            [view.lruCache loadImage:blowup finished:^(NSError *err, NSString *path) {
                NSLog(@"Finished downloading, displaying: %@", path);
                
                FullImageViewController * nextController = [FullImageViewController createWithParentView:view path:path];
                
                [fullImageController release];
                fullImageController = [nextController retain];
            }];
        }
    }
}

- (void) swipeLeft:(UISwipeGestureRecognizer *)recog
{
    if (recog.state == UIGestureRecognizerStateEnded)
    {
        [self next: 1];
    }
}

- (void) swipeRight:(UISwipeGestureRecognizer *)recog
{
    if (recog.state == UIGestureRecognizerStateEnded)
    {
        [self prev: 1];
    }
}

- (void)loadImages:(NSArray *)images
{
    [self.view loadImages:images];
}

- (void)viewDidLoad
{
    [super viewDidLoad];
	// Do any additional setup after loading the view.
}

- (void)viewDidDisappear:(BOOL)animated
{
    [super viewDidDisappear:animated];
}

- (void)didReceiveMemoryWarning
{
    [super didReceiveMemoryWarning];
    // Dispose of any resources that can be recreated.
}

@end
