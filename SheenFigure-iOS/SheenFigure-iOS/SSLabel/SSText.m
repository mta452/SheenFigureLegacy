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

#import "SSText.h"

static CGColorSpaceRef _colorSpace = NULL;

static void refColorSpace() {
    if (_colorSpace) {
        CGColorSpaceRetain(_colorSpace);
    } else {
        _colorSpace = CGColorSpaceCreateDeviceRGB();
    }
}

typedef struct SSReservedObjects {
    CGContextRef _context;
    
#ifndef SF_IOS_CG
    FT_Face _ftFont;
    uint32_t _bgrColor;
#endif
} SSReservedObjects;

@interface SSText ()

- (void *)sfText;

@end

@implementation SSText {
    SFTextRef _sfText;
    SFUnichar *_unistr;
    
#ifndef SF_IOS_CG
    uint32_t _bgrColor;
#endif
}

@synthesize string=_string;
@synthesize font=_font;
@synthesize textAlignment=_textAlign;
@synthesize writingDirection=_writingDirection;

- (id)init {
    return [self initWithString:nil font:nil];
}

- (id)initWithString:(NSString *)str font:(SSFont *)font {
    self = [super init];
    if (self) {
        refColorSpace();
        _sfText = SFTextCreateWithString(NULL, 0, NULL);

        self.font = font;
        self.textColor = [UIColor blackColor];
        self.textAlignment = SSTextAlignmentRight;
        self.writingDirection = SSWritingDirectionRTL;
        self.string = str;
    }
    
    return self;
}

- (void *)sfText {
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

static void renderGlyph(SFTextRef sfText, SFGlyph glyph, SFFloat x, SFFloat y, void *resObj) {
    SSReservedObjects *resObjects = resObj;
    
    CGContextSaveGState(resObjects->_context);
    
#ifdef SF_IOS_CG
	CGContextShowGlyphsAtPoint(resObjects->_context, floorf(x), floorf(y), &glyph, 1);
#else
    FT_Error error;
    
	FT_Face face = resObjects->_ftFont;
	FT_GlyphSlot slot = face->glyph;
    
	error = FT_Load_Glyph(face, glyph, FT_LOAD_DEFAULT);
    if (error) {
		return;
    }
    
	error = FT_Render_Glyph(face->glyph, FT_RENDER_MODE_NORMAL);
	if (error) {
		return;
    }
    
	FT_Bitmap *bmp = &slot->bitmap;
	int length = bmp->width * bmp->rows;
    if (!length) {
        return;
    }
    
	uint32_t pixels[length];
    
    //Here we will treat grayscale pixels as alpha
	for (int i = 0; i < length; i++) {
		//For iOS, in one uint, bytes are stored in reversed order
        //as BGRA, i.e. blue at first place and alpha at last place.
		pixels[i] = (resObjects->_bgrColor << 8) | bmp->buffer[i];
    }
    
    CGDataProviderRef data = CGDataProviderCreateWithData(NULL, pixels, bmp->width * bmp->rows * 4, NULL);
    CGImageRef glyphImage = CGImageCreate(bmp->width, bmp->rows, 8, 32, 4 * bmp->width, _colorSpace,
                                          kCGImageAlphaFirst, data, NULL, false, kCGRenderingIntentDefault);

    CGFloat scale = [[UIScreen mainScreen] scale];
    
    CGRect bmpRect;
    bmpRect.origin.x = floorf((x * scale) + slot->bitmap_left);
    bmpRect.origin.y = floorf((y * scale) - slot->bitmap_top);
    bmpRect.size = CGSizeMake(bmp->width, bmp->rows);

    CGContextTranslateCTM(resObjects->_context, bmpRect.origin.x, bmpRect.origin.y + bmpRect.size.height);
    CGContextScaleCTM(resObjects->_context, 1, -1);
    
    bmpRect.origin = CGPointZero;
    
    CGContextDrawImage(resObjects->_context, bmpRect, glyphImage);
    
    CGImageRelease(glyphImage);
    CGDataProviderRelease(data);
#endif

    CGContextRestoreGState(resObjects->_context);
}

- (int)showStringOnContext:(CGContextRef)context forFrameWidth:(float)frameWidth atPosition:(CGPoint)pos fromIndex:(int)index havingLines:(int *)lines {
    CGContextSaveGState(context);
    
#ifdef SF_IOS_CG
    CGAffineTransform flip = CGAffineTransformMake(1.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f);
    CGContextSetTextMatrix(context, flip);
    CGContextSetTextDrawingMode(context, kCGTextFill);
    
    SFFontRef sfFont = (SF_BRIDGE void *)([_font performSelector:@selector(sfFont)]);
    CGFontRef cgFont = CGFontRetain(SFFontGetCGFont(sfFont));
    CGContextSetFont(context, cgFont);
    CGContextSetFontSize(context, SFFontGetSize(sfFont));
    CGContextSetFillColorWithColor(context, _textColor.CGColor);
#else
    CGFloat scale = [[UIScreen mainScreen] scale];
    CGContextScaleCTM(context, 1 / scale, 1 / scale);
#endif
    
    SSReservedObjects resObj;
    resObj._context = context;
#ifndef SF_IOS_CG
    SFFontRef sfFont = (SF_BRIDGE void *)([_font performSelector:@selector(sfFont)]);
    FT_Face ftFace = SFFontGetFTFace(sfFont);
    FT_Reference_Face(ftFace);
    
    resObj._ftFont = ftFace;
    resObj._bgrColor = _bgrColor;
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

- (void)setString:(NSString *)string {
    if (string != _string) {
        SF_RELEASE(_string);
        _string = SF_RETAIN(string);
        
        free(_unistr);
        _unistr = NULL;
        
        if (_string && _string.length > 0) {
            _unistr = malloc(_string.length * sizeof(SFUnichar));
            [_string getCharacters:_unistr range:NSMakeRange(0, _string.length)];
        }
        
        SFTextSetString(_sfText, _unistr, _string.length);
    }
}

- (void)setFont:(SSFont *)font {
    if (font != _font) {
        SF_RELEASE(_font);
        _font = SF_RETAIN(font);
        
        void *fontPtr = NULL;
        if (_font) {
            fontPtr = (SF_BRIDGE void *)([_font performSelector:@selector(sfFont)]);
        }
        
        SFTextSetFont(_sfText, fontPtr);
    }
}

- (void)setTextColor:(UIColor *)textColor {
    if (!textColor) {
        textColor = [UIColor blackColor];
    }
    
    if (textColor != _textColor) {
        SF_RELEASE(_textColor);
        _textColor = SF_RETAIN(textColor);
        
#ifndef SF_IOS_CG
        CGFloat r, g, b, a;
        if (![_textColor getRed:&r green:&g blue:&b alpha:&a]) {
            if ([_textColor getWhite:&r alpha:&a]) {
                // it is a grayscale color.
                g = r;
                b = r;
            } else {
                // hue, saturation, brightness are not supported.
                // make default black color.
                r = g = b = 0;
            }
        }
        
        _bgrColor = (((int)(b * 255) << 16) | ((int)(g * 255) << 8)) | (int)(r * 255);
#endif
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
    SF_RELEASE(_string);
    SF_RELEASE(_font);
    
    SFTextRelease(_sfText);
    free(_unistr);
    
#ifndef SF_ARC_ENABLED
    [super dealloc];
#endif
}

@end
