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

#include <SFConfig.h>
#include <SFFont.h>
#include <SFText.h>

#import "SSFont.h"
#import "SSText.h"
#import "SSTextPrivate.h"
#import "SSRenderOperation.h"
#import "SSLabel.h"

#define PADDING                         3

@implementation SSLabel {
    NSOperationQueue *_opQueue;
    dispatch_queue_t _renderQueue;
    UIImageView *_imgView;
    
    SSText *_ssText;
    int _measuredHeight;
}

@synthesize autoFitToSize=_autoFitToSize;
@synthesize drawingMode=_drawingMode;

- (void)initialSetupWithDefaultValues:(BOOL)defaultValues drawingMode:(SSDrawingMode)drawingMode {
    _ssText = [[SSText alloc] init];
    
    _autoFitToSize = NO;
    _measuredHeight = 0;
    _drawingMode = drawingMode;

    if (_drawingMode == SSDrawingModeBackgroundThread) {
        _opQueue = [[NSOperationQueue alloc] init];
        _renderQueue = dispatch_queue_create("com.sheenfigure.LabelRenderBarrierQueue", DISPATCH_QUEUE_CONCURRENT);
        
        _imgView = [[UIImageView alloc] initWithFrame:self.bounds];
        _imgView.autoresizingMask = UIViewAutoresizingFlexibleWidth | UIViewAutoresizingFlexibleHeight;
        [self addSubview:_imgView];
    }

    if (defaultValues) {
        self.backgroundColor = [UIColor clearColor];
        self.clipsToBounds = YES;
    }
}

- (id)initWithFrame:(CGRect)frame drawingMode:(SSDrawingMode)drawingMode {
    self = [super initWithFrame:frame];
    if (self) {
        [self initialSetupWithDefaultValues:YES drawingMode:drawingMode];
    }
    
    return self;
}

- (id)initWithFrame:(CGRect)frame {
    self = [super initWithFrame:frame];
    if (self) {
        [self initialSetupWithDefaultValues:YES drawingMode:SSDrawingModeMainThread];
    }
    
    return self;
}

- (id)initWithCoder:(NSCoder *)aDecoder {
    self = [super initWithCoder:aDecoder];
    if (self) {
        [self initialSetupWithDefaultValues:NO drawingMode:SSDrawingModeMainThread];
    }
    
    return self;
}

- (id)init {
    return [self initWithFrame:CGRectZero];
}

- (CGSize)sizeThatFits:(CGSize)size {
    if (_measuredHeight == 0) {
        _measuredHeight = [_ssText measureHeightForFrameWidth:self.frame.size.width - (PADDING * 2)];
    }
    
    return CGSizeMake(self.frame.size.width, _measuredHeight);
}

- (void)setImage:(UIImage *)image {
    dispatch_async(dispatch_get_main_queue(), ^{
        _imgView.image = image;
    });
}

- (void)drawRect:(CGRect)rect {
    [super drawRect:rect];
    
    if (_drawingMode != SSDrawingModeBackgroundThread) {
        BOOL cancel = NO;
        [self renderOnContext:UIGraphicsGetCurrentContext() cancel:&cancel];
    }
}

- (void)renderOnContext:(CGContextRef)context cancel:(BOOL *)cancel {
    int totalLines = -1;
    CGPoint pos = CGPointMake(PADDING, 0);
    
    [_ssText showStringOnContext:UIGraphicsGetCurrentContext() forFrameWidth:self.frame.size.width - (PADDING * 2) atPosition:pos fromIndex:0 havingLines:&totalLines cancel:cancel];
        
    _measuredHeight = roundf(totalLines * self.font.leading);
}

- (void)render {
    if (_drawingMode != SSDrawingModeBackgroundThread) {
        [self setNeedsDisplay];
        return;
    }

    [self setImage:nil];
    
    [_opQueue cancelAllOperations];
    
    SSRenderOperation *operation = [SSRenderOperation operationWithExecutionBlock:^(SSRenderOperation *operation) {
        dispatch_barrier_async(_renderQueue, ^{
            UIGraphicsBeginImageContextWithOptions(self.frame.size, NO, 0.0);
            
            [self renderOnContext:UIGraphicsGetCurrentContext() cancel:&operation->_cancel];
            UIImage *image = UIGraphicsGetImageFromCurrentImageContext();
            
            UIGraphicsEndImageContext();
            
            if (!operation.isCancelled) {
                [self setImage:image];
            }
        });
    }];
    
    [_opQueue addOperation:operation];
}

- (SSFont *)font {
    return _ssText.font;
}

- (void)setFont:(SSFont *)font {
    if (font != _ssText.font) {
        _measuredHeight = 0;
        _ssText.font = font;
        
        [self render];
    }
}

- (NSString *)text {
    return _ssText.string;
}

- (void)setText:(NSString *)text {
    if (text != _ssText.string) {
        _measuredHeight = 0;
        _ssText.string = text;
        
        [self render];
    }
}

- (UIColor *)textColor {
    return _ssText.textColor;
}

- (void)setTextColor:(UIColor *)textColor {
    if (!textColor) {
        textColor = [UIColor blackColor];
    }
    
    if (textColor != _ssText.textColor) {
        _ssText.textColor = textColor;
        [self render];
    }
}

- (SSTextAlignment)textAlignment {
    return _ssText.textAlignment;
}

- (void)setTextAlignment:(SSTextAlignment)textAlign {
    if (textAlign != _ssText.textAlignment) {
        _ssText.textAlignment = textAlign;
        [self render];
    }
}

- (SSWritingDirection)writingDirection {
    return _ssText.writingDirection;
}

- (void)setWritingDirection:(SSWritingDirection)writingDirection {
    if (writingDirection != _ssText.writingDirection) {
        _measuredHeight = 0;
        _ssText.writingDirection = writingDirection;
        
        [self render];
    }
}

- (void)dealloc {
    if (_opQueue) {
        SS_RELEASE(_opQueue);
    }
    
    if (_renderQueue) {
        dispatch_release(_renderQueue);
    }
    
    if (_imgView) {
        SS_RELEASE(_imgView);
    }
    
    SS_RELEASE(_ssText);
    
#ifndef SS_ARC_ENABLED
    [super dealloc];
#endif
}

@end
