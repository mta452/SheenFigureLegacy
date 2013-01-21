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

#ifndef _SF_COMMON_METHODS_H
#define _SF_COMMON_METHODS_H

#include "SFCommonData.h"
#include "SFGDEFUtilization.h"

SFGlyphIndex SFMakeGlyphIndex(int recordIndex, int glyphIndex);
SFGlyphIndex SFGetLastGlyphIndex(SFStringRecord *record);

int SFCompareGlyphIndex(const SFGlyphIndex index1, const SFGlyphIndex index2);

void SFInsertGlyphs(SFGlyphRecord **src, const SFGlyph *val, int srcsize, int valsize, int sidx, int *length);

SFBool SFGetPreviousGlyphIndex(SFGlyphIndex *index, const LookupFlag lookupFlag);
SFBool SFGetPreviousValidGlyphIndex(SFGlyphIndex *index, const LookupFlag lookupFlag);
SFBool SFGetNextValidGlyphIndex(SFGlyphIndex *index, LookupFlag lookupFlag);

SFBool SFGetPreviousBaseGlyphIndex(SFGlyphIndex *index, const LookupFlag lookupFlag);
SFBool SFGetPreviousLigatureGlyphIndex(SFGlyphIndex *index, const LookupFlag lookupFlag, int *emptyGlyphs);

#endif
