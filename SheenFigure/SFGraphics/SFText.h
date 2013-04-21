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

#ifndef _SF_TEXT_H
#define _SF_TEXT_H

#include "SFConfig.h"
#include "SFTypes.h"

#ifndef _SF_TEXT_REF
#define _SF_TEXT_REF

typedef struct SFText *SFTextRef;

#endif

typedef void (*SFGlyphRenderFunction)(SFTextRef sfText, SFGlyph glyph, SFFloat x, SFFloat y, void *resObj);

typedef enum {
    SFTextAlignmentRight = 0,
    SFTextAlignmentCenter = 1,
    SFTextAlignmentLeft = 2,
} SFTextAlignment;

typedef enum {
	SFWritingDirectionAuto = 0,
    SFWritingDirectionRTL = 1,
    SFWritingDirectionLTR = 2,
} SFWritingDirection;

SFTextRef SFTextCreateWithString(const SFUnichar *str, int length, SFFontRef sfFont);
SFTextRef SFTextRetain(SFTextRef sfText);
void SFTextRelease(SFTextRef sfText);

void SFTextSetString(SFTextRef sfText, const SFUnichar *str, int length);
void SFTextSetFont(SFTextRef sfText, SFFontRef sfFont);
void SFTextSetAlignment(SFTextRef sfText, SFTextAlignment alignment);
void SFTextSetWritingDirection(SFTextRef sfText, SFWritingDirection writingDirection);

int SFTextGetNextLineCharIndex(SFTextRef sfText, SFFloat frameWidth, int startIndex, int *countLines);
int SFTextMeasureLines(SFTextRef sfText, SFFloat frameWidth);
SFFloat SFTextMeasureHeight(SFTextRef sfText, SFFloat frameWidth);

int SFTextShowString(SFTextRef sfText, SFFloat frameWidth, SFPoint position, int startIndex, int *lines, void *resObj, SFGlyphRenderFunction func);

#endif
