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
#import "SSTextView.h"

#define PADDING                         3


////////////////////////////////TEXT LINE VIEW (PRIVATE)////////////////////////////////

@interface SSTextLineView : UIView {
    NSOperationQueue *_opQueue;
    dispatch_queue_t _renderQueue;
    UIImageView *_imgView;
}

@property (nonatomic, retain) SSText *text;
@property (nonatomic, assign) SSDrawingMode drawingMode;
@property (nonatomic, assign) CGRect drawingArea;
@property (nonatomic, assign) NSInteger lineIndex;

@end

@implementation SSTextLineView

@synthesize text=_text;
@synthesize drawingMode=_drawingMode;
@synthesize drawingArea=_drawingArea;
@synthesize lineIndex=_lineIndex;

- (id)initWithFrame:(CGRect)frame drawingArea:(CGRect)drawingArea drawingMode:(SSDrawingMode)drawingMode {
    self = [super initWithFrame:frame];
    if (self) {
        _drawingMode = drawingMode;
        _drawingArea = drawingArea;
        _lineIndex = -1;
        
        if (_drawingMode == SSDrawingModeBackgroundThread) {
            _opQueue = [[NSOperationQueue alloc] init];
            _renderQueue = dispatch_queue_create("com.sheenfigure.LineRenderBarrierQueue", DISPATCH_QUEUE_CONCURRENT);
            
            _imgView = [[UIImageView alloc] initWithFrame:self.bounds];
            _imgView.autoresizingMask = UIViewAutoresizingFlexibleWidth | UIViewAutoresizingFlexibleHeight;
            [self addSubview:_imgView];
        }
        
        self.backgroundColor = [UIColor clearColor];
        self.clipsToBounds = YES;
    }
    
    return self;
}

- (void)setLineIndex:(NSInteger)lineIndex {
    [self setLineIndex:lineIndex forceRender:NO];
}

- (void)setLineIndex:(NSInteger)lineIndex forceRender:(BOOL)forceRender {
    if (lineIndex != _lineIndex) {
        _lineIndex = lineIndex;
        [self render];
    } else if (forceRender) {
        [self render];
    }
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
    int totalLines = 1;

    [_text showStringOnContext:context forFrameWidth:_drawingArea.size.width atPosition:_drawingArea.origin fromIndex:_lineIndex havingLines:&totalLines cancel:cancel];
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
    
#ifndef SS_ARC_ENABLED
    [super dealloc];
#endif
}

@end

////////////////////////////////////////////////////////////////////////////////////////


@implementation SSTextView {
    NSOperationQueue *_opQueue;
    dispatch_queue_t _measureQueue;
    
    SSText *_ssText;
    
    BOOL _settingMap;
    BOOL _forceRender;
    
    NSMutableArray *_linesMap;
    NSMutableArray *_linesView;
    
    float _lineHeight;
}

@synthesize drawingMode=_drawingMode;

- (void)initialSetupWithDefaultValues:(BOOL)defaultValues {
    _opQueue = [[NSOperationQueue alloc] init];
    _measureQueue = dispatch_queue_create("com.sheenfigure.LinesMeasureBarrierQueue", DISPATCH_QUEUE_CONCURRENT);
    
    _ssText = [[SSText alloc] init];

    _linesMap = [[NSMutableArray alloc] init];
    _linesView = [[NSMutableArray alloc] init];
    _drawingMode = SSDrawingModeMainThread;

    if (defaultValues) {
        self.backgroundColor = [UIColor clearColor];
        self.clipsToBounds = YES;
    }
}

- (id)init {
    return [self initWithFrame:CGRectZero];
}

- (id)initWithFrame:(CGRect)frame {
    self = [super initWithFrame:frame];
    if (self) {
        [self initialSetupWithDefaultValues:YES];
    }
    
    return self;
}

- (id)initWithCoder:(NSCoder *)aDecoder {
    self = [super initWithCoder:aDecoder];
    if (self) {
        [self initialSetupWithDefaultValues:NO];
    }
    
    return self;
}

- (id)initWithFrame:(CGRect)frame drawingMode:(SSDrawingMode)drawingMode {
    self = [super initWithFrame:frame];
    if (self) {
        [self initialSetupWithDefaultValues:YES];
        _drawingMode = drawingMode;
    }
    
    return self;
}

- (void)setFrame:(CGRect)frame {
    [super setFrame:frame];
    [self setLinesMap];
}

- (void)layoutSubviews {
    [self layoutTextLines];
    [super layoutSubviews];
}

- (void)setLinesMap {
    if (!_ssText) {
        return;
    }
    
    _settingMap = YES;
    
    [_opQueue cancelAllOperations];
    
    SSRenderOperation *operation = [SSRenderOperation operationWithExecutionBlock:^(SSRenderOperation *operation) {
        dispatch_barrier_async(_measureQueue, ^{
            [_linesMap removeAllObjects];
            
            float lineHeight = self.font.leading;
            
            int index = 0;
            int countLines = 1;
            int totalLines = 0;
            
            SFFloat frameWidth = self.frame.size.width - (PADDING * 2);
            
            for (int i = 0; index > -1; i++) {
                if (operation.isCancelled) {
                    break;
                }
                
                [_linesMap addObject:[NSNumber numberWithInteger:index]];
                
                index = [_ssText nextLineCharIndexForFrameWidth:frameWidth fromIndex:index countLines:&countLines];
                totalLines += countLines;
            }
            
            if (!operation.isCancelled) {
                dispatch_sync(dispatch_get_main_queue(), ^{
                    int measuredHeight = roundf(totalLines * lineHeight);
                    self.contentSize = CGSizeMake(self.frame.size.width, measuredHeight);
                    
                    _forceRender = YES;
                    _settingMap = NO;
                    
                    [self setNeedsLayout];
                });
            }
        });
    }];
    
    [_opQueue addOperation:operation];
}

- (void)layoutTextLines {
    if (_settingMap) {
        return;
    }
    
    const CGSize boundsSize = self.bounds.size;
    const CGFloat contentOffset = self.contentOffset.y;
    const CGRect visibleBounds = CGRectMake(0, contentOffset, boundsSize.width, boundsSize.height);

    NSMutableArray *linesMap = SS_AUTORELEASE([_linesMap copy]);
    
    const NSInteger numLines = linesMap.count;
    const NSInteger numViews = _linesView.count;
    
    const NSInteger visibleLines = floorf(boundsSize.height / _lineHeight) + 2;
    NSInteger lineIndex = contentOffset / _lineHeight;
    
    if (lineIndex < 0) {
        return;
    }
    
    const NSInteger gotoLines = visibleLines + lineIndex;
    
    for (; lineIndex < gotoLines && lineIndex < numLines; lineIndex++) {
        CGRect rowRect = CGRectMake(0, lineIndex * _lineHeight - (_lineHeight / 2), boundsSize.width, _lineHeight * 2);
        if (CGRectIntersectsRect(rowRect, visibleBounds)) {
            CGRect drawingArea = CGRectMake(PADDING, _lineHeight / 2, boundsSize.width - (PADDING * 2), _lineHeight);
            
            NSInteger modIndex = lineIndex % visibleLines;
            if (modIndex < numViews) {
                SSTextLineView *lineView = [_linesView objectAtIndex:modIndex];
                lineView.frame = rowRect;
                lineView.drawingArea = drawingArea;
                [lineView setLineIndex:[[linesMap objectAtIndex:lineIndex] integerValue] forceRender:_forceRender];
            } else {
                SSTextLineView *lineView = [[SSTextLineView alloc] initWithFrame:rowRect drawingArea:drawingArea drawingMode:_drawingMode];
                lineView.text = _ssText;
                [lineView setLineIndex:[[linesMap objectAtIndex:lineIndex] integerValue] forceRender:NO];
                [_linesView addObject:lineView];
                [self addSubview:lineView];
                
                SS_RELEASE(lineView);
            }
        }
    }
    
    _forceRender = NO;
    
    for (; lineIndex < _linesView.count;) {
        SSTextLineView *lineView = [_linesView lastObject];
        [lineView removeFromSuperview];
        [_linesView removeLastObject];
    }
}

- (SSFont *)font {
    return _ssText.font;
}

- (void)setFont:(SSFont *)font {
    if (font != _ssText.font) {
        _ssText.font = font;
        _lineHeight = font.leading;

        [self setLinesMap];
    }
}

- (NSString *)text {
    return _ssText.string;
}

- (void)setText:(NSString *)text {
    if (text != _ssText.string) {
        _ssText.string = text;

        [self setLinesMap];
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

        [self setNeedsLayout];
    }
}

- (SSTextAlignment)textAlignment {
    return _ssText.textAlignment;
}

- (void)setTextAlignment:(SSTextAlignment)textAlign {
    if (textAlign != _ssText.textAlignment) {
        _ssText.textAlignment = textAlign;
        
        [self setNeedsLayout];
    }
}

- (SSWritingDirection)writingDirection {
    return _ssText.writingDirection;
}

- (void)setWritingDirection:(SSWritingDirection)writingDirection {
    if (writingDirection != _ssText.writingDirection) {
        _ssText.writingDirection = writingDirection;
        
        [self setLinesMap];
    }
}

- (void)dealloc {
    SS_RELEASE(_opQueue);
    SS_RELEASE(_ssText);
    
    dispatch_release(_measureQueue);
    
#ifndef SS_ARC_ENABLED
    [super dealloc];
#endif
}

@end
