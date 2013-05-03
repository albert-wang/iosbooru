//
//  TagViewController.m
//  iosbooru
//
//  Created by Albert Wang on 1/24/13.
//  Copyright (c) 2013 Albert Wang. All rights reserved.
//

#import "TagViewController.h"

@interface TagViewController ()

@end

@implementation TagViewController

- (id) initWithImageReference:(NTVImage *)img datastore:(NTVDatastore *)ds
{
    self = [super initWithNibName:nil bundle:nil];
    if (self)
    {
        self->image = [img retain];
        self->datastore = [ds retain];
    }
    
    return self;
}

- (void) dealloc
{
    [super dealloc];
    [self->image release];
    [self->datastore release];
}

- (id)initWithNibName:(NSString *)nibNameOrNil bundle:(NSBundle *)nibBundleOrNil
{
    self = [super initWithNibName:nibNameOrNil bundle:nibBundleOrNil];
    if (self) {
        // Custom initialization
    }
    return self;
}

- (void)loadView
{
    
}

- (void)viewDidLoad
{
    [super viewDidLoad];
}

- (void)didReceiveMemoryWarning
{
    [super didReceiveMemoryWarning];
}

@end
