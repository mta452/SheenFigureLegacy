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

#include <stdbool.h>
#include <stdlib.h>

#include "SFGlobal.h"
#include "SFCommonMethods.h"

SFGlyphIndex SFMakeGlyphIndex(int recordIndex, int glyphIndex) {
    SFGlyphIndex index;
    index.recordIndex = recordIndex;
    index.glyphIndex = glyphIndex;
    
    return index;
}

SFGlyphIndex SFGetLastGlyphIndex(SFStringRecord *record) {
    SFGlyphIndex index;
    index.recordIndex = record->charCount - 1;
    index.glyphIndex = record->charRecord[index.recordIndex].glyphCount - 1;
    
    return index;
}

int SFCompareGlyphIndex(const SFGlyphIndex index1, const SFGlyphIndex index2) {
    if (index1.recordIndex < index2.recordIndex)
        return -1;
    
    if (index1.recordIndex > index2.recordIndex)
        return 1;
    
    if (index1.glyphIndex < index2.glyphIndex)
        return -1;
    
    if (index1.glyphIndex > index2.glyphIndex)
        return 1;
    
    return 0;
}

void SFInsertGlyphs(SFGlyphRecord **src, const SFGlyph *val, int srcsize, int valsize, int sidx, int *length) {
    if (valsize == 0 || sidx > srcsize)
        return;

    *length = sidx + valsize;
    *src = realloc(*src, sizeof(SFGlyphRecord) * *length);
    
    int i = 0;
    while (sidx < valsize) {
        (*src)[sidx].glyph = val[i++];
        (*src)[sidx].glyphProp = gpNotReceived;
        (*src)[sidx].posRec = SFPositionRecordZero;
        sidx++;
    }
}

bool SFGetPreviousValidGlyphIndex(SFGlyphIndex *index, LookupFlag lookupFlag) {
    int j = index->glyphIndex - 1;
    
    for (int i = index->recordIndex; i >= 0;) {
        for (; j >= 0; j--) {
            if (!SFIsIgnoredGlyph(i, j, lookupFlag)) {
                index->recordIndex = i;
                index->glyphIndex = j;
                
                return true;
            }
        }
        
        --i;
        j = record->charRecord[i].glyphCount - 1;
    }
    
    return false;
}

bool SFGetNextValidGlyphIndex(SFGlyphIndex *index, LookupFlag lookupFlag) {
    int j = index->glyphIndex + 1;
    
    for (int i = index->recordIndex; i < record->charCount;) {
        for (; j < record->charRecord[i].glyphCount; j++) {
            if (!SFIsIgnoredGlyph(i, j, lookupFlag)) {
                index->recordIndex = i;
                index->glyphIndex = j;
                
                return true;
            }
        }
        
        ++i;
        j = 0;
    }
    
    return false;
}

bool SFGetPreviousBaseGlyphIndex(SFGlyphIndex *index, const LookupFlag lookupFlag) {
    int j = index->glyphIndex - 1;
    
    for (int i = index->recordIndex; i >= 0;) {
        for (; j >= 0; j--) {
            if (!SFIsIgnoredGlyph(i, j, lookupFlag)) {
                SFGlyphProperty prop = record->charRecord[i].gRec[j].glyphProp;
                
                if (prop & gpBase) {
                    index->recordIndex = i;
                    index->glyphIndex = j;
                    
                    return true;
                } else if (!(prop & gpMark))
                    return false;
            }
        }
        
        --i;
        j = record->charRecord[i].glyphCount - 1;
    }
    
    return false;
}

bool SFGetPreviousLigatureGlyphIndex(SFGlyphIndex *index, const LookupFlag lookupFlag, int *emptyGlyphs) {
    int j = index->glyphIndex - 1;
    
    for (int i = index->recordIndex; i >= 0;) {
        for (; j >= 0; j--) {
            SFGlyph currentGlyph = record->charRecord[i].gRec[j].glyph;

            if (currentGlyph == 0) {
                ++*emptyGlyphs;
                continue;
            }
                
            if (!SFIsIgnoredGlyph(i, j, lookupFlag)) {
                SFGlyphProperty prop = record->charRecord[i].gRec[j].glyphProp;
                
                if (prop & gpLigature) {
                    index->recordIndex = i;
                    index->glyphIndex = j;
                
                    return true;
                } else if (!(prop & gpMark))
                    return false;
            }
        }
        
        --i;
        j = record->charRecord[i].glyphCount - 1;
    }
    
    return false;
}

