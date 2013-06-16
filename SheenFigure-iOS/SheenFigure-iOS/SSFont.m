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

@implementation SSFont {
    SFFontRef _sfFont;
    
#ifdef SF_IOS_CG
    CGFontRef _cgFont;
#else
    FT_Library _ftLib;
    FT_Face _ftFace;
#endif
    
    dispatch_queue_t _renderQueue;
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
            _cgFont = CGFontCreateWithDataProvider(dataProvider);
            CGDataProviderRelease(dataProvider);

            _sfFont = SFFontCreateWithCGFont(_cgFont, size);
        }
#else
        const char *utf8path = [_path UTF8String];
        
        FT_Init_FreeType(&_ftLib);
        
        CGFloat scale = [[UIScreen mainScreen] scale];
        FT_New_Face(_ftLib, utf8path, 0, &_ftFace);
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

- (dispatch_queue_t)renderQueue {
    if (!_renderQueue) {
        _renderQueue = dispatch_queue_create("com.sheenfigue.GlyphRenderQueue", NULL);
    }
    
    return _renderQueue;
}

- (void)dealloc {
    SS_RELEASE(_path);
    SFFontRelease(_sfFont);
    
#ifdef SF_IOS_CG
    CGFontRelease(_cgFont);
#else
    FT_Done_Face(_ftFace);
    FT_Done_FreeType(_ftLib);
#endif
    
    if (_renderQueue) {
        dispatch_release(_renderQueue);
    }
    
#ifndef SS_ARC_ENABLED
    [super dealloc];
#endif
}

@end
