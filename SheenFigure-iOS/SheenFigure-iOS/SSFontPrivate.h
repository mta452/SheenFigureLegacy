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

#include <SFFont.h>

#import "SSFont.h"

#ifndef SF_IOS_CG

@interface SSGlyph : NSObject {
@private
    int _top;
    int _left;
    void *_pixels;
    CGImageRef _image;
}

- (id)initWithTop:(int)top left:(int)left bitmap:(FT_Bitmap *)bitmap;
+ (SSGlyph *)glyphWithTop:(int)top left:(int)left bitmap:(FT_Bitmap *)bitmap;

@property (nonatomic, readonly) int top;
@property (nonatomic, readonly) int left;
@property (nonatomic, readonly) int width;
@property (nonatomic, readonly) int height;
@property (nonatomic, readonly) CGImageRef image;

@end

#endif

@interface SSFont ()

- (id)initWithPath:(NSString *)path size:(float)size refPtr:(SFFontRef)refPtr;

#ifdef SF_IOS_CG
- (CGFontRef)cgFont;
#else
- (FT_Face)ftFace;
- (SSGlyph *)getGlyph:(SFGlyph)glyph;
#endif

- (SFFontRef)sfFont;

@end
