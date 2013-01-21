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

SFBool SFGetPreviousGlyphIndex(SFGlyphIndex *index, const LookupFlag lookupFlag) {
	int i = index->recordIndex;
    int j = index->glyphIndex - 1;
    
    for (; i >= 0;) {
        for (; j >= 0; j--) {
            index->recordIndex = i;
            index->glyphIndex = j;
                
			return SFTrue;
        }
        
        --i;
        j = record->charRecord[i].glyphCount - 1;
    }
    
    return SFFalse;
}

SFBool SFGetPreviousValidGlyphIndex(SFGlyphIndex *index, LookupFlag lookupFlag) {
	int i = index->recordIndex;
    int j = index->glyphIndex - 1;
    
    for (; i >= 0;) {
        for (; j >= 0; j--) {
            if (!SFIsIgnoredGlyph(i, j, lookupFlag)) {
                index->recordIndex = i;
                index->glyphIndex = j;
                
                return SFTrue;
            }
        }
        
        --i;
        j = record->charRecord[i].glyphCount - 1;
    }
    
    return SFFalse;
}

SFBool SFGetNextValidGlyphIndex(SFGlyphIndex *index, LookupFlag lookupFlag) {
	int i = index->recordIndex;
    int j = index->glyphIndex + 1;
    
    for (; i < record->charCount;) {
        for (; j < record->charRecord[i].glyphCount; j++) {
            if (!SFIsIgnoredGlyph(i, j, lookupFlag)) {
                index->recordIndex = i;
                index->glyphIndex = j;
                
                return SFTrue;
            }
        }
        
        ++i;
        j = 0;
    }
    
    return SFFalse;
}

SFBool SFGetPreviousBaseGlyphIndex(SFGlyphIndex *index, const LookupFlag lookupFlag) {
	int i = index->recordIndex;
    int j = index->glyphIndex - 1;
    
    for (; i >= 0;) {
        for (; j >= 0; j--) {
            if (!SFIsIgnoredGlyph(i, j, lookupFlag)) {
                SFGlyphProperty prop = record->charRecord[i].gRec[j].glyphProp;
                
                if (prop & gpBase) {
                    index->recordIndex = i;
                    index->glyphIndex = j;
                    
                    return SFTrue;
                }
				
				if (!(prop & gpMark))
                    return SFFalse;
            }
        }
        
        --i;
        j = record->charRecord[i].glyphCount - 1;
    }
    
    return SFFalse;
}

SFBool SFGetPreviousLigatureGlyphIndex(SFGlyphIndex *index, const LookupFlag lookupFlag, int *emptyGlyphs) {
	int i = index->recordIndex;
    int j = index->glyphIndex - 1;
    
    for (; i >= 0;) {
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
                    
                    return SFTrue;
                }
				
				if (!(prop & gpMark))
                    return SFFalse;
            }
        }
        
        --i;
        j = record->charRecord[i].glyphCount - 1;
    }
    
    return SFFalse;
}
