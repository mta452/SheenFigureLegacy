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

#ifndef _SF_TEXT_H
#define _SF_TEXT_H

#include "SFConfig.h"
#include "SFTypes.h"

typedef struct SFText *SFTextRef;

typedef void (*SFGlyphRenderFunction)(void *, int, int, float, float, void *);

typedef enum {
    SFTextAlignmentRight = 0,
    SFTextAlignmentCenter = 1,
    SFTextAlignmentLeft = 2,
} SFTextAlignment;

#ifdef SF_IOS_CG

#include <CoreGraphics/CoreGraphics.h>

SFTextRef SFTextCreate(SFFontRef sfFont, CFStringRef str, SFFloat pageWidth);
void SFTextChangeString(SFTextRef sfText, CFStringRef str);
void SFTextSetColor(SFTextRef sfText, CGColorRef color);
void CGContextDrawText(CGContextRef context, SFTextRef sfText, int startIndex, int *lines);

#else

SFTextRef SFTextCreate(SFFontRef sfFont, SFUnichar *str, int length, SFFloat pageWidth);
void SFTextChangeString(SFTextRef sfText, SFUnichar *str, int length);
void SFTextSetColor(SFTextRef sfText, SFColor color);
int SFTextShowString(SFTextRef sfText, SFGlyphRenderFunction func, int startIndex, int *lines);

#endif

void SFTextSetReservedObject(SFTextRef sfText, void *obj);
void SFTextChangeFont(SFTextRef sfText, SFFontRef sfFont);
void SFTextChangePageWidth(SFTextRef sfText, SFFloat pageWidth);
void SFTextSetAlignment(SFTextRef sfText, SFTextAlignment align);
void SFTextSetInitialPosition(SFTextRef sfText, CGPoint pos);

SFFontRef SFTextGetFont(SFTextRef sfText);

SFTextRef SFTextRetain(SFTextRef sfText);
void SFTextRelease(SFTextRef sfText);

int SFTextGetNextLineCharIndex(SFTextRef sfText, int maxLines, int startIndex, int *createdLines);

#endif
