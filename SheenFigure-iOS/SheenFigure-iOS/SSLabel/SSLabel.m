/*
 * Copyright (C) 2013 SheenFigure
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#import <objc/runtime.h>

#include <ft2build.h>
#include FT_FREETYPE_H

#include <SFConfig.h>
#include <SFFont.h>
#include <SFText.h>

#import "SSFont.h"
#import "SSText.h"
#import "SSLabel.h"

@interface SSCachedPage : NSObject  {
	int _startIndex;
	UIImage *_bitmap;
    UIImageView *_bitmapView;
}

- (id)initWithStartIndex:(int)startIndex bitmap:(UIImage *)bitmap;
+ (SSCachedPage *)cachedPageWithStartIndex:(int)startIndex bitmap:(UIImage *)bitmap;

@property (nonatomic, readonly) int startIndex;
@property (nonatomic, retain) UIImage *bitmap;
@property (nonatomic, retain, readonly) UIImageView *bitmapView;

@end

@implementation SSCachedPage

@synthesize startIndex=_startIndex;
@synthesize bitmap=_bitmap;
@synthesize bitmapView=_bitmapView;

- (id)initWithStartIndex:(int)startIndex bitmap:(UIImage *)bitmap {
    self = [super init];
    if (self) {
        _startIndex = startIndex;
        _bitmap = SF_RETAIN(bitmap);
        _bitmapView = [[UIImageView alloc] initWithImage:_bitmap];
        _bitmapView.autoresizingMask = UIViewAutoresizingFlexibleLeftMargin | UIViewAutoresizingFlexibleBottomMargin;
    }
    
    return self;
}

+ (SSCachedPage *)cachedPageWithStartIndex:(int)startIndex bitmap:(UIImage *)bitmap {
    return SF_AUTORELEASE([[SSCachedPage alloc] initWithStartIndex:startIndex bitmap:bitmap]);
}

- (void)setBitmap:(UIImage *)bitmap {
    if (bitmap != _bitmap) {
        SF_RELEASE(_bitmap);
        _bitmap = SF_RETAIN(bitmap);
        _bitmapView.image = _bitmap;
    }
}

- (void)dealloc {
    [_bitmapView removeFromSuperview];
    
    SF_RELEASE(_bitmap);
    SF_RELEASE(_bitmapView);
    
#ifndef SF_ARC_ENABLED
    [super dealloc];
#endif
}

@end


@interface SSLabel ()

- (int)maxAllowedPixelsInAPage;

- (UIImage *)renderTextOnImageOfSize:(CGSize)size forFrameWidth:(SFFloat)frameWidth atPosition:(CGPoint)pos fromIndex:(int)index havingLines:(int)lines;

- (void)generateCachedPages;
- (void)updateCachedPages;

@end

@implementation SSLabel {
    SSText *_ssText;
    NSMutableArray *_cachedPages;
    
    int _linesInPage;
    int _measuredHeight;
    
    BOOL _drawn;
}

#define PADDING                         3

@synthesize autoFitToSize=_autoFitToSize;
@synthesize autoAdjustSuperViewContentSize=_autoAdjustSuperViewContentSize;

- (void)initialSetupWithDefaultValues:(BOOL)defaultValues {
    _ssText = [[SSText alloc] init];
    
    _autoFitToSize = YES;
    _autoAdjustSuperViewContentSize = YES;

    _cachedPages = [[NSMutableArray alloc] init];

    _linesInPage = 0;
    _measuredHeight = 0;
    
    _drawn = NO;
    
    if (defaultValues) {
        self.backgroundColor = [UIColor clearColor];
        self.clipsToBounds = YES;
    }
}

- (id)init {
    return [self initWithFrame:CGRectZero];
}

- (id)initWithFrame:(CGRect)frame {
    self = [super initWithFrame:frame];
    if (self) {
        [self initialSetupWithDefaultValues:YES];
    }
    
    return self;
}

- (id)initWithCoder:(NSCoder *)aDecoder {
    self = [super initWithCoder:aDecoder];
    if (self) {
        [self initialSetupWithDefaultValues:NO];
    }
    
    return self;
}

- (id)initWithFrame:(CGRect)frame useDefaultValues:(BOOL)defaultValues {
    self = [super initWithFrame:frame];
    if (self) {
        [self initialSetupWithDefaultValues:YES];
    }
    
    return self;
}

- (void)setFrame:(CGRect)frame {
    if (frame.size.width != self.frame.size.width) {
        _drawn = NO;
        _measuredHeight = 0;
    }
    
    [super setFrame:frame];
}

- (void)setBounds:(CGRect)bounds {
    if (bounds.size.width != self.bounds.size.width) {
        _drawn = NO;
        _measuredHeight = 0;
    }
    
    [super setBounds:bounds];
}

- (CGSize)sizeThatFits:(CGSize)size {
    return CGSizeMake(self.frame.size.width, _measuredHeight);
}

- (void)layoutSubviews {
    [super layoutSubviews];

    SSFont *font = self.font;
    NSString *text = self.text;
    
    if (font && text && text.length > 0) {
        if (!_drawn) {
            if (_measuredHeight == 0) {
                [self generateCachedPages];
            } else {
                [self updateCachedPages];
            }
        }
    } else {
        [_cachedPages removeAllObjects];
        _drawn = YES;
    }
}

- (int)maxAllowedPixelsInAPage {
    CGRect screenBounds = [UIScreen mainScreen].bounds;
    int maxPixels = screenBounds.size.width * screenBounds.size.height;
    
    return maxPixels;
}

- (UIImage *)renderTextOnImageOfSize:(CGSize)size forFrameWidth:(SFFloat)frameWidth atPosition:(CGPoint)pos fromIndex:(int)index havingLines:(int)lines {
    CGFloat scale = [[UIScreen mainScreen] scale];
    CGColorSpaceRef colorSpace = CGColorSpaceCreateDeviceRGB();
    CGContextRef context = CGBitmapContextCreate(NULL, size.width * scale, size.height * scale, 8, 0, colorSpace, kCGImageAlphaPremultipliedLast);
    
    CGContextTranslateCTM(context, 0, size.height * scale);
    CGContextScaleCTM(context, scale, -scale);
    
    [_ssText showStringOnContext:context forFrameWidth:frameWidth atPosition:pos fromIndex:index havingLines:&lines];
    
    CGImageRef cgBmp = CGBitmapContextCreateImage(context);
    UIImage *bmp = [UIImage imageWithCGImage:cgBmp scale:scale orientation:UIImageOrientationUp];
    
    CGImageRelease(cgBmp);
    CGContextRelease(context);
    CGColorSpaceRelease(colorSpace);
    
    return bmp;
}

- (void)generateCachedPages {
    [_cachedPages removeAllObjects];
    
    float lineHeight = self.font.leading;
    
    CGPoint pos = CGPointMake(PADDING, lineHeight / 2.0f);
    
    CGSize imageSize;
    imageSize.width = self.frame.size.width;
    imageSize.height = [self maxAllowedPixelsInAPage] / imageSize.width;
    
    SFFloat frameWidth = imageSize.width - (PADDING * 2);
    _linesInPage = floorf((imageSize.height - lineHeight) / lineHeight);
    float pageHeight = lineHeight * _linesInPage;

    int countLines = 0;
    int totalLines = 0;
    
    int index = 0;
    int prevIndex = 0;
    
    for (int i = 0; index > -1; i++) {
        prevIndex = index;
        countLines = _linesInPage;
        
        index = [_ssText nextLineCharIndexForFrameWidth:frameWidth fromIndex:index countLines:&countLines];
        imageSize.height = (int)(lineHeight * (countLines + 1));
        
        UIImage *bmp = [self renderTextOnImageOfSize:imageSize forFrameWidth:frameWidth atPosition:pos fromIndex:prevIndex havingLines:countLines];
        
        SSCachedPage *page = [SSCachedPage cachedPageWithStartIndex:prevIndex bitmap:bmp];
        [_cachedPages addObject:page];

        UIImageView *bmpView = page.bitmapView;
        bmpView.frame = CGRectMake(0, (i * pageHeight) - pos.y, bmp.size.width, bmp.size.height);

        [self addSubview:bmpView];
        
        totalLines += countLines;
    }

    _drawn = YES;
    _measuredHeight = roundf(totalLines * lineHeight);
    
    if (_autoFitToSize) {
        CGRect frame = self.frame;
        frame.size.height = _measuredHeight;
        super.frame = frame;
    }
    
    if (_autoAdjustSuperViewContentSize) {
        if ([self.superview isKindOfClass:[UIScrollView class]]) {
            ((UIScrollView *)self.superview).contentSize = CGSizeMake(self.frame.size.width, _measuredHeight);
        }
    }
}

- (void)updateCachedPages {
    CGPoint pos = CGPointMake(PADDING, self.font.leading / 2.0f);

    for (int i = 0; i < _cachedPages.count; i++) {
        SSCachedPage *prevPage = [_cachedPages objectAtIndex:i];
        
        CGSize imageSize = prevPage.bitmap.size;
        SFFloat frameWidth = imageSize.width - (PADDING * 2);
        
        prevPage.bitmap = nil;
        UIImage *bmp = [self renderTextOnImageOfSize:imageSize forFrameWidth:frameWidth atPosition:pos fromIndex:prevPage.startIndex havingLines:_linesInPage];
        prevPage.bitmap = bmp;
    }
    
    _drawn = YES;
}

- (SSFont *)font {
    return _ssText.font;
}

- (void)setFont:(SSFont *)font {
    if (font != _ssText.font) {
        _ssText.font = font;
        
        _drawn = NO;
        _measuredHeight = 0;
        [self setNeedsLayout];
    }
}

- (NSString *)text {
    return _ssText.string;
}

- (void)setText:(NSString *)text {
    if (text != _ssText.string) {
        _ssText.string = text;
        
        _drawn = NO;
        _measuredHeight = 0;
        [self setNeedsLayout];
    }
}

- (UIColor *)textColor {
    return _ssText.textColor;
}

- (void)setTextColor:(UIColor *)textColor {
    if (!textColor) {
        textColor = [UIColor blackColor];
    }
    
    if (textColor != _ssText.textColor) {
        _ssText.textColor = textColor;
        
        _drawn = NO;
        [self setNeedsLayout];
    }
}

- (SSTextAlignment)textAlignment {
    return _ssText.textAlignment;
}

- (void)setTextAlignment:(SSTextAlignment)textAlign {
    if (textAlign != _ssText.textAlignment) {
        _ssText.textAlignment = textAlign;

        _drawn = NO;
        [self setNeedsLayout];
    }
}

- (SSWritingDirection)writingDirection {
    return _ssText.writingDirection;
}

- (void)setWritingDirection:(SSWritingDirection)writingDirection {
    if (writingDirection != _ssText.writingDirection) {
        _ssText.writingDirection = writingDirection;

        _drawn = NO;
        _measuredHeight = 0;
        [self setNeedsLayout];
    }
}

- (void)dealloc {
    SF_RELEASE(_ssText);
    SF_RELEASE(_cachedPages);
    
#ifndef SF_ARC_ENABLED
    [super dealloc];
#endif
}

@end
