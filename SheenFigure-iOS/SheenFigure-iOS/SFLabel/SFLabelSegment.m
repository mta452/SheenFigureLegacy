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

#import "SFLabel.h"
#import "SFLabelSegment.h"
#import "SFText.h"

@implementation SFLabelSegment

@synthesize lines;
@synthesize nextIndex;

static CGColorSpaceRef _colorSpace = NULL;

- (id)initWithFrame:(CGRect)frame text:(SFTextRef)sfText startIndex:(int)stIndex linesToBeDrawn:(int)count {
    self = [super initWithFrame:frame];
    
    if (self) {
        _sfText = SFTextRetain(sfText);
        _startIndex = stIndex;
        _linesToBeDrawn = count;
        
        nextIndex = SFTextGetNextLineCharIndex(_sfText, count, _startIndex, &lines);
        
        if (!_colorSpace)
            _colorSpace = CGColorSpaceCreateDeviceRGB();
        else
            CGColorSpaceRetain(_colorSpace);
        
        self.backgroundColor = [UIColor clearColor];
        self.opaque = NO;
    }
    
    return self;
}

void renderGlyph(void *pixels, int width, int rows, float x, float y, void *reserved) {
    CGContextRef imgCtx = CGBitmapContextCreate(pixels, width, rows, 8, 4 * width, _colorSpace, kCGImageAlphaPremultipliedFirst);

    CGFloat scale = 1.0;
    
    x = floorf(x);
    y = floorf(y);
    
    if ([[UIScreen mainScreen] respondsToSelector:@selector(scale)]) {
        scale = [[UIScreen mainScreen] scale];
        
        x /= scale;
        y /= scale;
    }
    
    CGImageRef img = CGBitmapContextCreateImage(imgCtx);

    UIImage *uiImage = [[UIImage alloc] initWithCGImage:img scale:scale orientation:UIImageOrientationUp];
    
    CGImageRelease(img);
    CGContextRelease(imgCtx);

    [uiImage drawAtPoint:CGPointMake(x, y) blendMode:kCGBlendModeOverlay alpha:1.0];
    [uiImage release];
}

- (void)drawRect:(CGRect)rect {
    int lns = _linesToBeDrawn;
    
    CGContextRef ctx = UIGraphicsGetCurrentContext();
    CGContextClearRect(ctx, rect);

#ifdef SF_IOS_CG
    CGContextDrawText(ctx, _sfText, _startIndex, &lns);
#else
    CGContextSetInterpolationQuality(ctx, kCGInterpolationNone);
    
    SFTextShowString(_sfText, &renderGlyph, _startIndex, &lns);
#endif
}

- (void)dealloc {
    CGColorSpaceRelease(_colorSpace);
    SFTextRelease(_sfText);
    
    [super dealloc];
}

@end

