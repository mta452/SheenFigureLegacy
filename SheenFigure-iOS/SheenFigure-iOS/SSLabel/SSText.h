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

#import <Foundation/Foundation.h>

#import "SSFont.h"

typedef enum {
    SSTextAlignmentRight = 0,
    SSTextAlignmentCenter = 1,
    SSTextAlignmentLeft = 2,
} SSTextAlignment;

typedef enum {
    SSWritingDirectionAuto = 0,
    SSWritingDirectionRTL = 1,
    SSWritingDirectionLTR = 2,
} SSWritingDirection;

@interface SSText : NSObject {
    NSString *_string;
    SSFont *_font;
    SSTextAlignment _textAlignment;
    SSWritingDirection _writingDirection;
}

- (id)initWithString:(NSString *)str font:(SSFont *)font;

- (int)nextLineCharIndexForFrameWidth:(float)frameWidth fromIndex:(int)index countLines:(int *)countLines;
- (int)measureLinesForFrameWidth:(float)frameWidth;
- (int)measureHeightForFrameWidth:(float)frameWidth;

- (int)showStringOnContext:(CGContextRef)context forFrameWidth:(float)frameWidth atPosition:(CGPoint)pos fromIndex:(int)index havingLines:(int *)lines;

@property (nonatomic, retain) NSString *string;
@property (nonatomic, retain) SSFont *font;
@property (nonatomic, retain) UIColor *textColor;
@property (nonatomic) SSTextAlignment textAlignment;
@property (nonatomic) SSWritingDirection writingDirection;

@end
