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

#ifdef SF_IOS_CG
#import <CoreGraphics/CoreGraphics.h>
#else
#include <ft2build.h>
#include <freetype/freetype.h>
#endif

#include <SFConfig.h>
#include <SFFont.h>
#include <SFText.h>

#import "SSFont.h"
#import "SSText.h"
#import "SSFontPrivate.h"
#import "SSTextPrivate.h"

typedef struct SSReservedObjects {
    dispatch_queue_t _renderQueue;
    CGContextRef _context;
    BOOL *_cancel;
    
#ifndef SF_IOS_CG
    FT_Face _ftFont;
#endif
} SSReservedObjects;

@implementation SSText {
    SFTextRef _sfText;
}

@synthesize string=_string;
@synthesize font=_font;
@synthesize textAlignment=_textAlign;
@synthesize writingDirection=_writingDirection;

static const CGFloat _glyphDecode[] = { 1, 0 };

- (id)init {
    return [self initWithString:nil font:nil];
}

- (id)initWithString:(NSString *)str font:(SSFont *)font {
    self = [super init];
    if (self) {
        _sfText = SFTextCreateWithString(NULL, 0, NULL);

        self.font = font;
        self.textColor = [UIColor blackColor];
        self.textAlignment = SSTextAlignmentRight;
        self.writingDirection = SSWritingDirectionRTL;
        self.string = str;
    }
    
    return self;
}

- (SFTextRef)sfText {
    return _sfText;
}

- (int)nextLineCharIndexForFrameWidth:(float)frameWidth fromIndex:(int)index countLines:(int *)countLines {
    return SFTextGetNextLineCharIndex(_sfText, frameWidth, index, countLines);
}

- (int)measureLinesForFrameWidth:(float)frameWidth {
    return SFTextMeasureLines(_sfText, frameWidth);
}

- (int)measureHeightForFrameWidth:(float)frameWidth {
    return SFTextMeasureHeight(_sfText, frameWidth);
}

static void renderGlyph(SFGlyph glyph, SFFloat x, SFFloat y, void *resObj) {
    SSReservedObjects *resObjects = resObj;
    if (*(resObjects->_cancel)) {
        return;
    }
    
    dispatch_sync(resObjects->_renderQueue, ^{
#ifdef SF_IOS_CG
        CGContextShowGlyphsAtPoint(resObjects->_context, floorf(x), floorf(y), &glyph, 1);
#else
        FT_Error error;
        
        FT_Face face = resObjects->_ftFont;
        FT_GlyphSlot slot = face->glyph;
        
        error = FT_Load_Glyph(face, glyph, FT_LOAD_RENDER);
        if (error) {
            return;
        }
        
        FT_Bitmap *bmp = &slot->bitmap;
        int length = bmp->width * bmp->rows;
        if (!length) {
            return;
        }

        CGContextSaveGState(resObjects->_context);
        
        CGDataProviderRef data = CGDataProviderCreateWithData(NULL, bmp->buffer, bmp->width * bmp->rows, NULL);
        CGImageRef glyphImage = CGImageMaskCreate(bmp->width, bmp->rows, 8, 8, bmp->width, data, _glyphDecode, false);
        
        CGFloat scale = [[UIScreen mainScreen] scale];
        
        CGRect bmpRect;
        bmpRect.origin.x = floorf((x * scale) + slot->bitmap_left);
        bmpRect.origin.y = floorf((y * scale) - slot->bitmap_top);
        bmpRect.size = CGSizeMake(bmp->width, bmp->rows);
        
        CGContextTranslateCTM(resObjects->_context, bmpRect.origin.x, bmpRect.origin.y + bmpRect.size.height);
        CGContextScaleCTM(resObjects->_context, 1, -1);
        
        bmpRect.origin = CGPointZero;

        CGContextClipToMask(resObjects->_context, bmpRect, glyphImage);
        CGContextAddRect(resObjects->_context, bmpRect);
        CGContextDrawPath(resObjects->_context, kCGPathFill);
        
        CGImageRelease(glyphImage);
        CGDataProviderRelease(data);
        
        CGContextRestoreGState(resObjects->_context);
#endif
    });
}

- (int)showStringOnContext:(CGContextRef)context forFrameWidth:(float)frameWidth atPosition:(CGPoint)pos fromIndex:(int)index havingLines:(int *)lines cancel:(BOOL *)cancel {
    CGContextSaveGState(context);
    
#ifdef SF_IOS_CG
    CGAffineTransform flip = CGAffineTransformMake(1.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f);
    CGContextSetTextMatrix(context, flip);
    CGContextSetTextDrawingMode(context, kCGTextFill);
    
    SFFontRef sfFont = [_font sfFont];
    CGFontRef cgFont = CGFontRetain(SFFontGetCGFont(sfFont));
    CGContextSetFont(context, cgFont);
    CGContextSetFontSize(context, SFFontGetSize(sfFont));
#else
    CGFloat scale = [[UIScreen mainScreen] scale];
    CGContextScaleCTM(context, 1 / scale, 1 / scale);
#endif
    CGContextSetFillColorWithColor(context, _textColor.CGColor);
    
    SSReservedObjects resObj;
    resObj._context = context;
    resObj._cancel = cancel;
    resObj._renderQueue = _font.renderQueue;
#ifndef SF_IOS_CG
    SFFontRef sfFont = [_font sfFont];
    FT_Face ftFace = SFFontGetFTFace(sfFont);
    FT_Reference_Face(ftFace);
    
    resObj._ftFont = ftFace;
#endif
    
    SFPoint point;
    point.x = pos.x;
    point.y = pos.y;
    
	int result = SFTextShowString(_sfText, frameWidth, point, index, lines, &resObj, &renderGlyph);
    
#ifdef SF_IOS_CG
    CGFontRelease(cgFont);
#else
    FT_Done_Face(ftFace);
#endif
    
    CGContextRestoreGState(context);

    return result;
}

- (int)showStringOnContext:(CGContextRef)context forFrameWidth:(float)frameWidth atPosition:(CGPoint)pos fromIndex:(int)index havingLines:(int *)lines {
    BOOL cancel = NO;
    
    return [self showStringOnContext:context forFrameWidth:frameWidth atPosition:pos fromIndex:index havingLines:lines cancel:&cancel];
}

- (int)showStringOnContext:(CGContextRef)context forFrameWidth:(float)frameWidth atPosition:(CGPoint)pos fromIndex:(int)index {
    return [self showStringOnContext:context forFrameWidth:frameWidth atPosition:pos fromIndex:index havingLines:NULL];
}

- (void)setString:(NSString *)string {
    if (string != _string) {
        SS_RELEASE(_string);
        _string = SS_RETAIN(string);

        SFUnichar *unistr = NULL;
        
        if (_string && _string.length > 0) {
            unistr = malloc(_string.length * sizeof(SFUnichar));
            [_string getCharacters:unistr range:NSMakeRange(0, _string.length)];
        }
        
        SFTextSetString(_sfText, unistr, _string.length);
    }
}

- (void)setFont:(SSFont *)font {
    if (font != _font) {
        SS_RELEASE(_font);
        _font = SS_RETAIN(font);
        
        SFFontRef sfFont = NULL;
        if (_font) {
            sfFont = [_font sfFont];
        }
        
        SFTextSetFont(_sfText, sfFont);
    }
}

- (void)setTextColor:(UIColor *)textColor {
    if (!textColor) {
        textColor = [UIColor blackColor];
    }
    
    if (textColor != _textColor) {
        SS_RELEASE(_textColor);
        _textColor = SS_RETAIN(textColor);
    }
}

- (void)setTextAlignment:(SSTextAlignment)textAlign {
    if (textAlign != _textAlign) {
        _textAlign = textAlign;
        
        SFTextSetAlignment(_sfText, (SFTextAlignment)_textAlign);
    }
}

- (void)setWritingDirection:(SSWritingDirection)writingDirection {
    if (writingDirection != _writingDirection) {
        _writingDirection = writingDirection;
        
        SFTextSetWritingDirection(_sfText, (SFWritingDirection)_writingDirection);
    }
}

- (void)dealloc {
    SS_RELEASE(_string);
    SS_RELEASE(_font);
    
    SFTextRelease(_sfText);
    
#ifndef SS_ARC_ENABLED
    [super dealloc];
#endif
}

@end
