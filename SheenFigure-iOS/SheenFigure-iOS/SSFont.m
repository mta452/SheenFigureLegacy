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

#import "SSFont.h"
#import "SSFontPrivate.h"

#ifndef SF_IOS_CG

static const CGFloat _glyphDecode[] = { 1, 0 };

@implementation SSGlyph

@synthesize top=_top;
@synthesize left=_left;
@synthesize image=_image;

- (id)initWithTop:(int)top left:(int)left bitmap:(FT_Bitmap *)bitmap {
    self = [super init];
    if (self) {
        _top = top;
        _left = left;
        _image = NULL;
        
        if (bitmap) {
            int length = bitmap->width * bitmap->rows;
            if (length) {
                _pixels = malloc(length);
                memcpy(_pixels, bitmap->buffer, length);
                
                CGDataProviderRef data = CGDataProviderCreateWithData(NULL, _pixels, length, NULL);
                _image = CGImageMaskCreate(bitmap->width, bitmap->rows, 8, 8, bitmap->width, data, _glyphDecode, false);
                CGDataProviderRelease(data);
            }
        }
    }
    
    return self;
}

+ (SSGlyph *)glyphWithTop:(int)top left:(int)left bitmap:(FT_Bitmap *)bitmap {
    return SS_AUTORELEASE([[SSGlyph alloc] initWithTop:top left:left bitmap:bitmap]);
}

- (BOOL)isEmpty {
    return !_image;
}

- (int)width {
    return CGImageGetWidth(_image);
}

- (int)height {
    return CGImageGetHeight(_image);
}

- (void)dealloc {
    if (_image) {
        CGImageRelease(_image);
        free(_pixels);
    }
    
#ifndef SS_ARC_ENABLED
    [super dealloc];
#endif
}

@end

#endif


@implementation SSFont {
    SFFontRef _sfFont;
    
#ifdef SF_IOS_CG
    CGFontRef _cgFont;
#else
    FT_Library _ftLib;
    FT_Face _ftFace;
    NSLock *_lock;
    NSMutableDictionary *_cache;
#endif
}

- (id)initWithPath:(NSString *)path size:(float)size refPtr:(SFFontRef)refPtr {
    self = [super init];
    if (self) {
        _path = SS_RETAIN(path);
        
#ifdef SF_IOS_CG
        if (refPtr) {
            _cgFont = CGFontRetain(SFFontGetCGFont(refPtr));
            _sfFont = SFFontMakeCloneForCGFont(refPtr, _cgFont, size);
        } else {
            CGDataProviderRef dataProvider = CGDataProviderCreateWithFilename([_path fileSystemRepresentation]);
            if (dataProvider) {
                _cgFont = CGFontCreateWithDataProvider(dataProvider);
                CGDataProviderRelease(dataProvider);
                
                _sfFont = SFFontCreateWithCGFont(_cgFont, size);
            } else {
                SS_RELEASE(self);
                return nil;
            }
        }
#else
        _lock = [[NSLock alloc] init];
        _cache = [[NSMutableDictionary alloc] init];
        
        FT_Init_FreeType(&_ftLib);
        
        const char *utf8path = [_path UTF8String];
        FT_Error error = FT_New_Face(_ftLib, utf8path, 0, &_ftFace);
        if (error) {
            FT_Done_FreeType(_ftLib);
            SS_RELEASE(self);
            return nil;
        }
        
        CGFloat scale = [[UIScreen mainScreen] scale];
        FT_Set_Char_Size(_ftFace, 0, size * scale * 64, 72, 72);
        
        if (refPtr) {
            _sfFont = SFFontMakeCloneForFTFace(refPtr, _ftFace, size);
        } else {
            _sfFont = SFFontCreateWithFTFace(_ftFace, size);
        }
#endif
    }
    
    return self;
}

- (id)initWithPath:(NSString *)path size:(float)size {
    return [self initWithPath:path size:size refPtr:NULL];
}

+ (SSFont *)fontWithPath:(NSString *)path size:(float)size {
    return SS_AUTORELEASE([[SSFont alloc] initWithPath:path size:size]);
}

#ifdef SF_IOS_CG

- (CGFontRef)cgFont {
    return _cgFont;
}

#else

- (FT_Face)ftFace {
    return _ftFace;
}

#endif

- (SFFontRef)sfFont {
    return _sfFont;
}

- (SSFont *)makeCloneForSize:(float)size {
    return SS_AUTORELEASE([[SSFont alloc] initWithPath:_path size:size refPtr:_sfFont]);
}

- (float)size {
    return SFFontGetSize(_sfFont);
}

- (float)sizeByEm {
    return SFFontGetSizeByEm(_sfFont);
}

- (float)ascender {
    return SFFontGetAscender(_sfFont);
}

- (float)descender {
    return SFFontGetDescender(_sfFont);
}

- (float)leading {
    return SFFontGetLeading(_sfFont);
}

#ifndef SF_IOS_CG

- (SSGlyph *)generateGlyph:(SFGlyph)glyph {
    [_lock lock];
    
    SSGlyph *g;
    
    FT_Error error;
    error = FT_Load_Glyph(_ftFace, glyph, FT_LOAD_RENDER);
    if (error) {
        g = [SSGlyph glyphWithTop:_ftFace->glyph->bitmap_top left:_ftFace->glyph->bitmap_left bitmap:NULL];
    } else {
        g = [SSGlyph glyphWithTop:_ftFace->glyph->bitmap_top left:_ftFace->glyph->bitmap_left bitmap:&_ftFace->glyph->bitmap];
    }
    
    [_lock unlock];
    
    return g;
}

- (SSGlyph *)getGlyph:(SFGlyph)glyph {
    NSNumber *cacheKey = [NSNumber numberWithUnsignedShort:glyph];
    SSGlyph *cacheGlyph;
    @synchronized(_cache) {
        cacheGlyph = [_cache objectForKey:cacheKey];
        if (!cacheGlyph) {
            cacheGlyph = [self generateGlyph:glyph];
            [_cache setObject:cacheGlyph forKey:cacheKey];
        }
    }
    
    return cacheGlyph;
}

- (void)clearCache {
    @synchronized(_cache) {
        [_cache removeAllObjects];
    }
}

#endif

- (void)dealloc {
    SS_RELEASE(_path);
    SFFontRelease(_sfFont);
    
#ifdef SF_IOS_CG
    CGFontRelease(_cgFont);
#else
    SS_RELEASE(_lock);
    SS_RELEASE(_cache);
    
    FT_Done_Face(_ftFace);
    FT_Done_FreeType(_ftLib);
#endif

#ifndef SS_ARC_ENABLED
    [super dealloc];
#endif
}

@end
