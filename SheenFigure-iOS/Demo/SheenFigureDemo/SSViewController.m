//
//  SSViewController.m
//  SheenFigureDemo
//

#import "SSViewController.h"

@interface SSViewController ()

@end

@implementation SSViewController

- (void)viewDidLoad
{
    [super viewDidLoad];
	
    SSFont *font = [SSFont fontWithPath:[[NSBundle mainBundle] pathForResource:@"NafeesWeb" ofType:@"ttf"] size:UI_USER_INTERFACE_IDIOM() == UIUserInterfaceIdiomPhone ? 40 : 95];
    
    _txtDesc.font = font;
    _txtDesc.text = @"محمول (جمع: محمولات / mobile) کو ہاتفِ خلوی (cell phone) بھی کہا جاتا ہے اور یہ جدید طرزیات کی مدد سے تیار کی جانے والی ایک ایسی برقی اختراع (electronic device) ہوتی ہے کہ جسکے زریعے ہاتف (telephone) کا استعمال آزادانہ اور دوران حرکت و سفر کسی بھی جگہ بلا کسی قابل دید رابطے (یعنی تار وغیرہ کے بغیر) کیا جاسکتا ہے۔ آج کل جو جدید محمولات تیار کیے جارہے ہیں ان میں ناصرف یہ کہ ہاتف اور جال محیط عالم سے روابط (برقی خط اور رزمی بدیل (packet switching) وغیرہ) کی سہولیات میسر ہیں بلکہ اسکے ساتھ ساتھ ان میں تصاویر بھیجنے اور موصول کرنے کیلئے کثیرالوسیط پیغامی خدمت (multimedia messaging service) ، عکاسہ (camera) اور منظرہ (video) بنانے کی خصوصیات بھی موجود ہوتی ہیں۔";
}

- (void)didReceiveMemoryWarning
{
    [super didReceiveMemoryWarning];
    // Dispose of any resources that can be recreated.
}

@end
