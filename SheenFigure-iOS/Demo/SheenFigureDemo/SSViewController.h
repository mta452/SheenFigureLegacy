//
//  SSViewController.h
//  SheenFigureDemo
//

#import <UIKit/UIKit.h>

#import "SSTextView.h"

@interface SSViewController : UIViewController {
    IBOutlet UILabel *_lblTitle;
    IBOutlet UILabel *_lblReference;
    IBOutlet SSTextView *_txtDesc;
}

@end
