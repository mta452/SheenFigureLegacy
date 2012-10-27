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

#import <UIKit/UIKit.h>

#import "SFFont.h"
#import "SFText.h"

@interface SFLabelSegment : UIView {
    SFTextRef _sfText;
    int _startIndex;
    int _linesToBeDrawn;
}

- (id)initWithFrame:(CGRect)frame text:(SFTextRef)sfText startIndex:(int)stIndex linesToBeDrawn:(int)count;

@property (nonatomic, readonly) int lines;
@property (nonatomic, readonly) int nextIndex;

@end
