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

#import <Foundation/Foundation.h>

#import "SFFont.h"
#import "SFText.h"

@interface SFLabel : UIScrollView <UIScrollViewDelegate> {
    NSMutableArray *_segments;
    
    SFFontRef _sfFont;
    SFTextRef _sfText;
    
    NSString *_text;
    UIColor *_textColor;
    NSTextAlignment _textAlign;
}

- (void)setFont:(SFFontRef)font;
- (void)setFontWithFileName:(NSString *)name size:(CGFloat)fontSize;

@property (nonatomic, retain) NSString *text;
@property (nonatomic, retain) UIColor *textColor;
@property (nonatomic) NSTextAlignment textAlign;

@end
