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

#import <UIKit/UIKit.h>

#import "SSFont.h"
#import "SSText.h"
#import "SSLabel.h"

@interface SSTextView : UIScrollView

- (id)initWithFrame:(CGRect)frame drawingMode:(SSDrawingMode)drawingMode;

@property (nonatomic, retain) SSFont *font;
@property (nonatomic, retain) NSString *text;
@property (nonatomic, retain) UIColor *textColor;

@property (nonatomic, readonly) SSDrawingMode drawingMode;
@property (nonatomic, assign) SSTextAlignment textAlignment;
@property (nonatomic, assign) SSWritingDirection writingDirection;

@end
