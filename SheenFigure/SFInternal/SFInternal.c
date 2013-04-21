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

#include <stdlib.h>

#include "bidi.h"
#include "SFGDEFUtilization.h"
#include "SFInternal.h"

const SFPositionRecord SFPositionRecordZero = {{0, 0}, {0, 0}, 0, {0, 0}};

SFStringRecord *SFMakeStringRecordForBaseLevel(const SFUnichar *charsPtr, int len, int baselevel) {
    SFStringRecord *record = malloc(sizeof(SFStringRecord));

    int i;
    
    record->charCount = len;
    record->glyphCount = len;
    
    record->chars = charsPtr;
    record->types = malloc(sizeof(int) * len);
    record->levels = malloc(sizeof(int) * len);
    record->charRecord = malloc(sizeof(SFCharRecord) * len);
    
    generateBidiTypesAndLevels(baselevel, charsPtr, record->types, record->levels, len);
    
    for (i = 0; i < len; i++) {
        record->charRecord[i].glyphCount = 1;
        record->charRecord[i].gRec = malloc(sizeof(SFGlyphRecord));
        record->charRecord[i].gRec[0].glyphProp = gpNotReceived;
        record->charRecord[i].gRec[0].posRec = SFPositionRecordZero;
    }
    
    return record;
}

void SFClearCharRecord(SFStringRecord *record) {
    int i;
    
    record->glyphCount = record->glyphCount;
    
    for (i = 0; i < record->charCount; i++) {
        record->charRecord[i].glyphCount = 1;
        record->charRecord[i].gRec[0].glyphProp = gpNotReceived;
        record->charRecord[i].gRec[0].posRec = SFPositionRecordZero;
    }
}

void SFClearStringRecordForBaseLevel(SFStringRecord *record, int baselevel) {
    SFClearCharRecord(record);
    generateBidiTypesAndLevels(baselevel, record->chars, record->types, record->levels, record->charCount);
}

void SFFreeStringRecord(SFStringRecord *record) {
    if (record) {
        int i;
        for (i = 0; i < record->charCount; i++) {
            free(record->charRecord[i].gRec);
        }

        free(record->types);
        free(record->levels);
        free(record->charRecord);
        
        free(record);
    }
}

SFGlyphIndex SFMakeGlyphIndex(int recordIndex, int glyphIndex) {
    SFGlyphIndex index;
    index.record = recordIndex;
    index.glyph = glyphIndex;
    
    return index;
}

int SFCompareGlyphIndex(const SFGlyphIndex index1, const SFGlyphIndex index2) {
    if (index1.record < index2.record)
        return -1;
    
    if (index1.record > index2.record)
        return 1;
    
    if (index1.glyph < index2.glyph)
        return -1;
    
    if (index1.glyph > index2.glyph)
        return 1;
    
    return 0;
}

void SFInsertGlyphs(SFGlyphRecord **src, const SFGlyph *val, int srcsize, int valsize, int sidx, int *length) {
    if (valsize > 0 && sidx <= srcsize) {
        int i = 0;
        
        *length = sidx + valsize;
        *src = realloc(*src, sizeof(SFGlyphRecord) * *length);
        
        while (sidx < valsize) {
            (*src)[sidx].glyph = val[i++];
            (*src)[sidx].glyphProp = gpNotReceived;
            (*src)[sidx].posRec = SFPositionRecordZero;
            sidx++;
        }
    }
}

SFGlyphIndex SFGetLastGlyphIndex(SFStringRecord *record) {
    SFGlyphIndex index;
    index.record = record->charCount - 1;
    index.glyph = record->charRecord[index.record].glyphCount - 1;
    
    return index;
}

SFBool SFGetPreviousGlyphIndex(SFInternal *internal, SFGlyphIndex *index, const LookupFlag lookupFlag) {
	SFGlyphIndex pidx;
	pidx.record = index->record;
    pidx.glyph = index->glyph - 1;
    
    for (; pidx.record >= 0;) {
        for (; pidx.glyph >= 0; pidx.glyph--) {
            *index = pidx;
			return SFTrue;
        }
        
        --pidx.record;
        pidx.glyph = SFGetGlyphCount(internal, pidx.record) - 1;
    }
    
    return SFFalse;
}

SFBool SFGetPreviousValidGlyphIndex(SFInternal *internal, SFGlyphIndex *index, const LookupFlag lookupFlag) {
    SFGlyphIndex pidx;
	pidx.record = index->record;
    pidx.glyph = index->glyph - 1;
    
    for (; pidx.record >= 0;) {
        for (; pidx.glyph >= 0; pidx.glyph--) {
            if (!SFIsIgnoredGlyph(internal, pidx, lookupFlag)) {
                *index = pidx;
                
                return SFTrue;
            }
        }
        
        --pidx.record;
        pidx.glyph = SFGetGlyphCount(internal, pidx.record) - 1;
    }
    
    return SFFalse;
}

SFBool SFGetNextValidGlyphIndex(SFInternal *internal, SFGlyphIndex *index, const LookupFlag lookupFlag) {
    SFGlyphIndex nidx;
	nidx.record = index->record;
    nidx.glyph = index->glyph + 1;
    
    for (; nidx.record < internal->record->charCount;) {
        for (; nidx.glyph < SFGetGlyphCount(internal, nidx.record); nidx.glyph++) {
            if (!SFIsIgnoredGlyph(internal, nidx, lookupFlag)) {
                *index = nidx;
                
                return SFTrue;
            }
        }
        
        ++nidx.record;
        nidx.glyph = 0;
    }
    
    return SFFalse;
}

SFBool SFGetPreviousBaseGlyphIndex(SFInternal *internal, SFGlyphIndex *index, const LookupFlag lookupFlag) {
    SFGlyphIndex pidx;
    pidx.record = index->record;
    pidx.glyph = index->glyph - 1;
    
    for (; pidx.record >= 0;) {
        for (; pidx.glyph >= 0; pidx.glyph--) {
            if (!SFIsIgnoredGlyph(internal, pidx, lookupFlag)) {
                SFGlyphProperty prop = SFGetGlyphProperties(internal, pidx);
                
                if (prop & gpBase) {
                    *index = pidx;
                    return SFTrue;
                }
				
				if (!(prop & gpMark)) {
                    return SFFalse;
                }
            }
        }
        
        --pidx.record;
        pidx.glyph = internal->record->charRecord[pidx.record].glyphCount - 1;
    }
    
    return SFFalse;
}

SFBool SFGetPreviousLigatureGlyphIndex(SFInternal *internal, SFGlyphIndex *index, const LookupFlag lookupFlag, int *emptyGlyphs) {
	SFGlyphIndex pidx;
    pidx.record = index->record;
    pidx.glyph = index->glyph - 1;
    
    for (; pidx.record >= 0;) {
        for (; pidx.glyph >= 0; pidx.glyph--) {
            SFGlyph currentGlyph = SFGetGlyph(internal, pidx);
            
            if (currentGlyph == 0) {
                ++(*emptyGlyphs);
                continue;
            }
            
            if (!SFIsIgnoredGlyph(internal, pidx, lookupFlag)) {
                SFGlyphProperty prop = SFGetGlyphProperties(internal, pidx);
                
                if (prop & gpLigature) {
                    *index = pidx;
                    return SFTrue;
                }
				
				if (!(prop & gpMark))
                    return SFFalse;
            }
        }
        
        --pidx.record;
        pidx.glyph = internal->record->charRecord[pidx.record].glyphCount - 1;
    }
    
    return SFFalse;
}
