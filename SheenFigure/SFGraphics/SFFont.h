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

#ifndef _SF_FONT_H
#define _SF_FONT_H

#include "SFConfig.h"
#include "SFTypes.h"

#ifndef _SF_FONT_REF
#define _SF_FONT_REF

typedef void *SFFontRef;

#endif

#ifndef _SF_STRING_RECORD_REF
#define _SF_STRING_RECORD_REF

typedef void *SFStringRecordRef;

#endif

#ifdef SF_IOS_CG

#include <CoreGraphics/CoreGraphics.h>

SFFontRef SFFontCreate(CGFontRef cgFont, SFFloat size);
SFFontRef SFFontCreateWithFileName(CFStringRef name, CFStringRef extension, SFFloat size);
CGFontRef SFFontGetCGFont(SFFontRef sfFont);

#else

#include <ft2build.h>
#include <freetype/freetype.h>

SFFontRef SFFontCreateWithFileName(const char *name, SFFloat size);
FT_Face SFFontGetFTFace(SFFontRef sfFont);

#endif

SFStringRecordRef SFFontAllocateStringRecordForString(SFFontRef sfFont, SFUnichar *inputString, int length);
void SFFontDeallocateStringRecord(SFFontRef sfFont, SFStringRecordRef strRecord);

SFFloat SFFontGetSize(SFFontRef sfFont);
SFFloat SFFontGetSizeByEm(SFFontRef sfFont);

SFFloat SFFontGetAscender(SFFontRef sfFont);
SFFloat SFFontGetDescender(SFFontRef sfFont);
SFFloat SFFontGetLeading(SFFontRef sfFont);

SFFontRef SFFontMakeClone(SFFontRef sfFont, SFFloat size);
SFFontRef SFFontRetain(SFFontRef sfFont);
void SFFontRelease(SFFontRef sfFont);

#endif
