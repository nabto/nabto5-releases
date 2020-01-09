//
//  DetailViewController.h
//  Nabto5 iOS Demo
//
//  Created by Ulrik Gammelby on 08/05/2019.
//  Copyright © 2019 Nabto ApS. All rights reserved.
//

#import <UIKit/UIKit.h>

@interface DetailViewController : UIViewController

@property (strong, nonatomic) NSObject *detailItem;
@property (weak, nonatomic) IBOutlet UILabel *detailDescriptionLabel;

@end

