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
#import "NTVMultipartRequest.h"

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

- (void) cameraClicked:(UITapGestureRecognizer *)sender
{
    if (sender.state == UIGestureRecognizerStateEnded)
    {
        //Choose between photo album and camera.
        UIActionSheet * albumCameraSheet = [[UIActionSheet alloc] initWithTitle:@"Photo Source" delegate:self cancelButtonTitle:@"Cancel" destructiveButtonTitle:nil otherButtonTitles:@"Photos", @"Camera", nil];
        [albumCameraSheet showInView:self.view];
    }
}

//Handle the action sheet actions to show a camera or photo album
- (void) actionSheet:(UIActionSheet *)actionSheet clickedButtonAtIndex:(NSInteger)buttonIndex
{
    if (buttonIndex == actionSheet.cancelButtonIndex)
    {
        return;
    }
    
    //Assumes english =/
    NSString * title = [actionSheet buttonTitleAtIndex:buttonIndex];
    
    UIImagePickerController * picker = [[UIImagePickerController alloc] init];
    picker.delegate = self;
    picker.allowsEditing = NO;
    
    if ([title isEqualToString:@"Photos"])
    {
        picker.sourceType = UIImagePickerControllerSourceTypePhotoLibrary;
        
        if (UI_USER_INTERFACE_IDIOM() == UIUserInterfaceIdiomPad)
        {
            UIPopoverController * popover = [[UIPopoverController alloc] initWithContentViewController:picker];
            [popover presentPopoverFromRect:CGRectMake(0, 0, 40, 40) inView:self.view permittedArrowDirections:UIPopoverArrowDirectionAny animated:YES];
            
            possiblePopover = popover;
        }
        else
        {
            [self presentViewController:picker animated:YES completion:nil];
        }
    }
    else
    {
        picker.sourceType = UIImagePickerControllerSourceTypeCamera;
        [self presentViewController:picker animated:YES completion:nil];
    }
}

//Handle image picking
- (void) imagePickerController:(UIImagePickerController *)picker didFinishPickingMediaWithInfo:(NSDictionary *)info
{
    UIImage * img = [info objectForKey:UIImagePickerControllerOriginalImage];
    
    [picker dismissViewControllerAnimated:YES completion:nil];
    [possiblePopover dismissPopoverAnimated:YES];
    [possiblePopover release];
    possiblePopover = nil;
    
    dispatch_async(dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0), ^{
        //Write the file to /uploads/<timestamp>
        NSString * documentPath = [NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, true) objectAtIndex:0];
        NSFileManager * manager = [NSFileManager defaultManager];
        
        NSError * err = nil;
        
        NSString * uploadPath = [[documentPath stringByAppendingPathComponent:@"uploads"] retain];
        [manager createDirectoryAtPath:uploadPath withIntermediateDirectories:FALSE attributes:nil error:&err];
        
        NSString * uniquing = [NSString stringWithFormat:@"upload-%d-%f", rand(), [[NSDate date] timeIntervalSince1970]];
        NSString * target = [uploadPath stringByAppendingPathComponent:uniquing];
        [UIImageJPEGRepresentation(img, 1.0) writeToFile:target atomically:YES];
        
        dispatch_async(dispatch_get_main_queue(), ^{
            MultipartRequest * req = [[[MultipartRequest alloc] initWithURL:[NSURL URLWithString:@"http://img.uncod.in/upload/curl"]] autorelease];
            
            [req addPart: [@"iosbooru@ironclad.mobi" dataUsingEncoding:NSUTF8StringEncoding] withName:@"email" andContentType:@"text/plain"];
            [req addPartFromPath:target withName:@"file" andContentType:@"image/jpeg"];
            [req prepareToSend];
            
            NSLog(@"Sending request...");
            
            [self displayNetworkBusy];
            [NSURLConnection connectionWithRequest:req delegate:self];
        });
    });
}

//Handle error and finished.
- (void) connection:(NSURLConnection *)connection didFailWithError:(NSError *)error
{
    NSLog(@"Upload failed with error %@", error);
    [self networkFinished];
}

- (void) connectionDidFinishLoading:(NSURLConnection *)connection
{
    [self networkFinished];
    
    //Let thumbnail generation happen
    [self performSelector:@selector(reloadImages) withObject:self afterDelay:1.0];
}

- (void) displayNetworkBusy
{
    if (networkRequestsOutstanding == 0)
    {
        [[UIApplication sharedApplication] setNetworkActivityIndicatorVisible:YES];
    }
    
    networkRequestsOutstanding++;
}

- (void) networkFinished
{
    if (networkRequestsOutstanding > 0)
    {
        networkRequestsOutstanding--;
    }
    
    if (networkRequestsOutstanding == 0)
    {
        [[UIApplication sharedApplication] setNetworkActivityIndicatorVisible:NO];
    }
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
                
                [self addChildViewController:fullImageController];
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
