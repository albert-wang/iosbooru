//
//  ImageboardView.m
//  iosbooru
//
//  Created by Albert Wang on 1/17/13.
//  Copyright (c) 2013 Albert Wang. All rights reserved.
//

#import "ImageboardView.h"
#import "FullImageViewController.h"

@implementation ImageboardView
@synthesize lruCache, displayStatus;

static const size_t MARGIN = 1;

- (id)initWithFrame:(CGRect)frame controller:(UIViewController *)controller
{
    self = [super initWithFrame:frame];
    if (self) {
        self.backgroundColor = [UIColor blackColor];        
        //80mb image cache.
        self.lruCache = [DiskImageLRU cacheWithCapacity:1000 * 1000 * 80];
        [self createImageSlotsWithController:controller];
        [self createUIElementsWithController:controller];
    }
    return self;
}

- (void) dealloc
{
    [super dealloc];
    
    [self->placeholder release];
    [self->lruCache release];
}

- (size_t) indexFromClickedPoint:(CGPoint)point
{
    float squareSize = self.frame.size.width / 3;
    size_t result = static_cast<size_t>(floor(point.y / squareSize) * 3 + floor(point.x / squareSize));
    if (result >= 12)
    {
        return 11;
    }
    return result;
}

- (void) createImageSlotsWithController:(UIViewController *)controller
{
    self->placeholder = [[UIImage imageNamed:@"Placeholder"] retain];
    float squareSize = self.frame.size.width / 3;

    for (size_t i = 0; i < 12; ++i)
    {
        thumbnailDisplays[i] = [[UIImageView alloc] initWithImage:self->placeholder];
        thumbnailDisplays[i].contentMode = UIViewContentModeScaleAspectFill;

        thumbnailDisplays[i].frame = CGRectMake(0, 0, squareSize - MARGIN * 2, squareSize - MARGIN * 2);
        thumbnailDisplays[i].transform = CGAffineTransformMakeTranslation(i % 3 * squareSize + MARGIN, i / 3 * squareSize + MARGIN);
        thumbnailDisplays[i].clipsToBounds = YES;
        
        [self addSubview:thumbnailDisplays[i]];
    }
    
    self.userInteractionEnabled = YES;
    [self addGestureRecognizer:[[UITapGestureRecognizer alloc] initWithTarget:controller action:@selector(frameClicked:)]];
}

- (void) createUIElementsWithController:(UIViewController *)controller;
{
    UISwipeGestureRecognizer * rightSwipe = [[UISwipeGestureRecognizer alloc] initWithTarget:controller action:@selector(swipeRight:)];
    rightSwipe.direction = UISwipeGestureRecognizerDirectionRight;
    [self addGestureRecognizer:rightSwipe];
    [rightSwipe release];
    
    UISwipeGestureRecognizer * leftSwipe = [[UISwipeGestureRecognizer alloc] initWithTarget:controller action:@selector(swipeLeft:)];
    leftSwipe.direction = UISwipeGestureRecognizerDirectionLeft;
    [self addGestureRecognizer:leftSwipe];
    [leftSwipe release];
}

- (void) loadImages:(NSArray *)images
{
    //Cancel the previous set of animations.
    [self->lruCache clearCurrentCallbacks];
    
    for (size_t i = 0; i < [images count]; ++i)
    {
        [self->lruCache loadThumbnail:[images objectAtIndex:i] finished:^(NSError * err, NSString * path) {
            if (i < 12)
            {
                self->thumbnailDisplays[i].image = [UIImage imageWithContentsOfFile:path];
                if (!self->thumbnailDisplays[i].image)
                {
                    self->thumbnailDisplays[i].image = self->placeholder;
                }
                
                [self animate:self->thumbnailDisplays[i]];
            }
        }];
    }
}

- (bool) imageHasThumbnail:(size_t)ind
{
    return self->thumbnailDisplays[ind].image != placeholder;
}

- (void) reappearImages
{
    self->displayStatus = DISPLAY_BOARD;
    for (size_t i = 0; i < 12; ++i)
    {
        [self flip:self->thumbnailDisplays[i] hidden:NO];
    }
}

- (void) hideAll
{
    if (self->displayStatus == DISPLAY_BOARD)
    {
        for (size_t i = 0; i < 12; ++i)
        {
            [self flip:self->thumbnailDisplays[i] hidden:YES];
        }
        
        self->displayStatus = ANIMATING;
    }
}

- (void) displayWebview:(UIWebView *)target
{
    [self flip:target hidden:NO];
    self->displayStatus = DISPLAY_SINGLE_IMAGE;
}

- (void) flip:(UIView *)view hidden:(BOOL)hiddenStatus
{
    float duration = 0.25f + (static_cast<float>(rand()) / RAND_MAX) * 0.2f - 0.1f;
    
    [UIView beginAnimations:@"Flip" context:nil];
    [UIView setAnimationDuration:duration];
    [UIView setAnimationCurve:UIViewAnimationCurveEaseOut];
    
    view.hidden = hiddenStatus;
    [UIView setAnimationTransition:UIViewAnimationTransitionFlipFromRight forView:view cache:NO];
    
    [UIView commitAnimations];
}

- (void) animate:(UIView *)view
{
    float duration = 0.25f + (static_cast<float>(rand()) / RAND_MAX) * 0.2f - 0.1f;
    
    [UIView beginAnimations:@"Flip" context:nil];
    [UIView setAnimationDuration:duration];
    [UIView setAnimationCurve:UIViewAnimationCurveEaseOut];
    
    [UIView setAnimationTransition:UIViewAnimationTransitionFlipFromRight forView:view cache:NO];
    [UIView commitAnimations];
}

/*
// Only override drawRect: if you perform custom drawing.
// An empty implementation adversely affects performance during animation.
- (void)drawRect:(CGRect)rect
{
    // Drawing code
}
*/

@end
