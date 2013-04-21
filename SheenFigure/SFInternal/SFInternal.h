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

#ifndef _SF_INTERNAL_H
#define _SF_INTERNAL_H

#include "SFConfig.h"
#include "SFTypes.h"
#include "SFCommonData.h"
#include "SFCMAPData.h"
#include "SFGDEFData.h"
#include "SFGSUBData.h"
#include "SFGPOSData.h"
#include "SFGSUBGPOSData.h"

typedef struct SFGlyphIndex {
    int record;
    int glyph;
} SFGlyphIndex;

typedef struct SFPosition {
    short x;
    short y;
} SFPosition;

typedef enum SFAnchorType {
    atNone = 0x0000,
    atCursiveIgnored = 0x001,
    atMark = 0x002,
    atExit = 0x004,
    atEntry = 0x008,
} SFAnchorType;

typedef enum SFGlyphProperty {
    gpNotReceived = 0,
    gpReceived = 1,
    gpBase = 2,
    gpMark = 4,
    gpLigature = 8,
    gpComponent = 16,
    gpAdvance = 32
} SFGlyphProperty;

typedef struct SFPositionRecord {
    SFPosition placement;
    SFPosition advance;
    
    SFAnchorType anchorType;
    SFPosition anchor;             //either mark or base anchor
} SFPositionRecord;

extern const SFPositionRecord SFPositionRecordZero;

typedef struct SFGlyphRecord {
    SFGlyph glyph;
    SFGlyphProperty glyphProp;
    SFPositionRecord posRec;
    float advance;
} SFGlyphRecord;

typedef struct SFCharRecord {
    int glyphCount;
    SFGlyphRecord *gRec;            //glyphs[glyphCount]
} SFCharRecord;

typedef struct SFStringRecord {
    int charCount;
    int glyphCount;
    
    const SFUnichar *chars;         //chars[charCount]
    int *types;                     //types[charCount]
    int *levels;                    //levels[charCount]
    SFCharRecord *charRecord;       //charRecord[charCount]
} SFStringRecord;

typedef struct SFInternal {
    SFStringRecord *record;
    SFTableCMAP *cmap;
    SFTableGDEF *gdef;
    SFTableGSUB *gsub;
    SFTableGPOS *gpos;
} SFInternal;

SFStringRecord *SFMakeStringRecordForBaseLevel(const SFUnichar *charsPtr, int len, int baselevel);
void SFClearCharRecord(SFStringRecord *record);
void SFClearStringRecordForBaseLevel(SFStringRecord *record, int baselevel);
void SFFreeStringRecord(SFStringRecord *record);

#define SFIsOddLevel(p, i)          (p->record->levels[i] & 1)

#define SFGetChar(p, i)             (p->record->chars[i])
#define SFGetCharCount(p)           (p->record->charCount)
#define SFGetTotalGlyphCount(p)     (p->record->glyphCount)
#define SFGetGlyph(p, i)            (p->record->charRecord[i.record].gRec[i.glyph].glyph)
#define SFGetGlyphCount(p, i)       (p->record->charRecord[i].glyphCount)
#define SFGetGlyphRecord(p, i)      (p->record->charRecord[i].gRec)
#define SFGetGlyphProperties(p, i)  (p->record->charRecord[i.record].gRec[i.glyph].glyphProp)
#define SFGetPositionRecord(p, i)   (p->record->charRecord[i.record].gRec[i.glyph].posRec)

SFGlyphIndex SFMakeGlyphIndex(int recordIndex, int glyphIndex);
int SFCompareGlyphIndex(const SFGlyphIndex index1, const SFGlyphIndex index2);
void SFInsertGlyphs(SFGlyphRecord **src, const SFGlyph *val, int srcsize, int valsize, int sidx, int *length);
SFGlyphIndex SFGetLastGlyphIndex(SFStringRecord *record);

SFBool SFGetPreviousGlyphIndex(SFInternal *internal, SFGlyphIndex *index, const LookupFlag lookupFlag);
SFBool SFGetPreviousValidGlyphIndex(SFInternal *internal, SFGlyphIndex *index, const LookupFlag lookupFlag);
SFBool SFGetNextValidGlyphIndex(SFInternal *internal, SFGlyphIndex *index, const LookupFlag lookupFlag);

SFBool SFGetPreviousBaseGlyphIndex(SFInternal *internal, SFGlyphIndex *index, const LookupFlag lookupFlag);
SFBool SFGetPreviousLigatureGlyphIndex(SFInternal *internal, SFGlyphIndex *index, const LookupFlag lookupFlag, int *emptyGlyphs);

#endif
