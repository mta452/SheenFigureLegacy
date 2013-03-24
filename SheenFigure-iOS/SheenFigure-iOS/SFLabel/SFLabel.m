/*
 * Copyright (C) 2012 SheenFigure
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

#import <QuartzCore/QuartzCore.h>
#import "SFLabelSegment.h"
#import "SFLabel.h"

@implementation SFLabel

#define DRAW_OFFSET             2.0
#define PAGE_HEIGHT             1024

- (CGFloat)calculatePageWidth {
    CGFloat pageWidth = self.frame.size.width - (2 * DRAW_OFFSET);
    
#ifndef SF_IOS_CG
    CGFloat scale = 1.0;
    
    if ([[UIScreen mainScreen] respondsToSelector:@selector(scale)]) {
        scale = [[UIScreen mainScreen] scale];
        pageWidth *= scale;
    }
#endif
    
    return pageWidth;
}

- (void)initSetup {
    _text = nil;
    _sfFont = NULL;
    
    CGFloat pageWidth = [self calculatePageWidth];
    
#ifdef SF_IOS_CG
    _sfText = SFTextCreate(NULL, NULL, pageWidth);
#else
    _sfText = SFTextCreate(NULL, NULL, 0, pageWidth);
#endif

    _segments = [[NSMutableArray alloc] init];
    
    self.clipsToBounds = YES;
    self.backgroundColor = [UIColor clearColor];
}

- (id)init {
    self = [super init];
    
    if (self) {
        [self initSetup];
    }
    
    return self;
}

- (id)initWithCoder:(NSCoder *)aDecoder {
    self = [super initWithCoder:aDecoder];
    
    if (self) {
       [self initSetup];
    }
    
    return self;
}

- (id)initWithFrame:(CGRect)frame {
    self = [super initWithFrame:frame];
    
    if (self) {
        [self initSetup];
    }
    
    return self;
}

- (void)clearSegments {
    for (NSInteger i = [_segments count] - 1; i >= 0; i--) {
        SFLabelSegment *label = [_segments objectAtIndex:i];
        if (label) {
            [label removeFromSuperview];
            [_segments removeObjectAtIndex:i];
        }
    }
}

- (void)refreshSegments {
    for (NSInteger i = [_segments count] - 1; i >= 0; i--) {
        SFLabelSegment *label = [_segments objectAtIndex:i];
        if (label)
            [label setNeedsDisplay];
    }
}

- (void)setFrame:(CGRect)frame {
    [super setFrame:frame];
    
    if (_sfText)
        SFTextChangePageWidth(_sfText, [self calculatePageWidth]);
    
    [self clearSegments];
    [self setNeedsDisplay];
}

- (void)drawRect:(CGRect)rect {
    [super drawRect:rect];
    
    if (!_text || _text.length == 0)
        return;

    if ([_segments count] > 0) {
        [self refreshSegments];
        return;
    }
    
    CGFloat scale = 1.0;
    
#ifndef SF_IOS_CG

    if ([[UIScreen mainScreen] respondsToSelector:@selector(scale)])
        scale = [[UIScreen mainScreen] scale];
    
#endif
    
    CGFloat ascenderNoScale = SFFontGetAscender(_sfFont);
    
    CGFloat singleLineHeightNoScale = SFFontGetLeading(_sfFont);
    CGFloat singleLineHeight = singleLineHeightNoScale / scale;

    int linesPerPage = floor(PAGE_HEIGHT / singleLineHeight);
    
    CGFloat pageHeight = (linesPerPage + 1) * singleLineHeight;
    CGPoint initialPosition = CGPointMake(0, singleLineHeightNoScale + ascenderNoScale);
    
    SFTextSetInitialPosition(_sfText, initialPosition);

    CGFloat y = -singleLineHeight;
    int startIndex = 0;
    int numLines = 0;
    
    while (startIndex > -1) {
        CGRect segmentFrame = CGRectMake(DRAW_OFFSET, y, self.frame.size.width - 2 * DRAW_OFFSET, pageHeight);
        
        SFLabelSegment *label = [[SFLabelSegment alloc] initWithFrame: segmentFrame
                                                                 text: _sfText
                                                           startIndex: startIndex
                                                       linesToBeDrawn: linesPerPage];
        
        [_segments addObject:label];
        [self addSubview:label];
        [label release];
        
        y += pageHeight - singleLineHeight;
        startIndex = label.nextIndex;
        numLines += label.lines;
    }

    self.contentSize = CGSizeMake(self.contentSize.width, (numLines * singleLineHeight));
}

- (void)setFont:(SFFontRef)font {
    SFFontRelease(_sfFont);
    
    _sfFont = SFFontRetain(font);
    SFTextChangeFont(_sfText, _sfFont);
    
    [self setNeedsDisplay];
}

- (void)setFontWithFileName:(NSString *)name size:(CGFloat)fontSize {
    SFFontRelease(_sfFont);
    
    NSString *fName = [name stringByDeletingPathExtension];
    NSString *extension = [name pathExtension];
    
#ifdef SF_IOS_CG
    _sfFont = SFFontCreateWithFileName((CFStringRef)fName, (CFStringRef)extension, fontSize);
#else
    CGFloat scale = 1.0;
    
    if ([[UIScreen mainScreen] respondsToSelector:@selector(scale)]) {
        scale = [[UIScreen mainScreen] scale];
        fontSize *= scale;
    }
    
    const char *path = [[[NSBundle mainBundle] pathForResource:fName ofType:extension] UTF8String];
    _sfFont = SFFontCreateWithFileName(path, fontSize);
#endif

    SFTextChangeFont(_sfText, _sfFont);
    [self setNeedsDisplay];
}

- (void)setText:(NSString *)text {
    self.contentOffset = CGPointZero;

    [_text release];
    _text = [text retain];

#ifdef SF_IOS_CG
    SFTextChangeString(_sfText, (CFStringRef)_text);
#else
    SFUnichar *unistr = malloc(_text.length * sizeof(SFUnichar));
    [_text getCharacters:unistr range:NSMakeRange(0, _text.length)];

    SFTextChangeString(_sfText, unistr, _text.length);
#endif

    [self clearSegments];
    [self setNeedsDisplay];
}

- (NSString *)text {
    return [[_text retain] autorelease];
}

- (void)setTextAlign:(NSTextAlignment)textAlign {
    if (_textAlign == textAlign)
        return;
    
    _textAlign = textAlign;
    SFTextAlignment align;
    
    switch (textAlign) {
        case NSTextAlignmentLeft:
            align = SFTextAlignmentLeft;
            break;
            
        case NSTextAlignmentCenter:
            align = SFTextAlignmentCenter;
            break;
            
        default:
            align = SFTextAlignmentRight;
            break;
    }
    
    SFTextSetAlignment(_sfText, align);
    
    [self setNeedsDisplay];
}

- (NSTextAlignment)textAlign {
    return _textAlign;
}

- (void)setTextColor:(UIColor *)textColor {
    [_textColor release];
    _textColor = [textColor retain];
    
#ifdef SF_IOS_CG
    SFTextSetColor(_sfText, [_textColor CGColor]);
#else
    CGFloat r, g, b, a;
    if (![_textColor getRed:&r green:&g blue:&b alpha:&a]) {
        if ([_textColor getWhite:&r alpha:&a]) {
            g = r;
            b = r;
        } else {
            // hue, saturation, brightness are not supported.
            // make default black color.
            r = g = b = 0;
        }
    }
    
    SFColor clr = ((((int)(a * 255) << 24) | ((int)(r * 255) << 16)) | ((int)(g * 255) << 8)) | (int)(b * 255);
    SFTextSetColor(_sfText, clr);
#endif
    
    [self setNeedsDisplay];
}

- (UIColor *)textColor {
    return [[_textColor retain] autorelease];
}

- (void)dealloc {
    SFFontRelease(_sfFont);
    SFTextRelease(_sfText);
    
    [_segments release];
    [_text release];
    [_textColor release];

    [super dealloc];
}

@end
