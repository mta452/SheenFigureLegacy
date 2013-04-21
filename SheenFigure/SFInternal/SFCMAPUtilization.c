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

#include "SFCMAPUtilization.h"

SFGlyph SFCharToGlyph(SFTableCMAP *cmap, SFUnichar ch) {
    SFUShort segCount = cmap->encodingSubtable.encodingFormat.format.format4.segCountX2 / 2;
    
    SFGlyph charGlyph;
    SFUShort i;
    
    for (i = 0; i < segCount; i++) {
        if (cmap->encodingSubtable.encodingFormat.format.format4.endCount[i] >= ch) {
            break;
        }
    }
    
    if (i < segCount) {
        SFUShort startCount = cmap->encodingSubtable.encodingFormat.format.format4.startCount[i];
        if (startCount > ch) {
            charGlyph = 0;
        } else {
            SFUShort idRangeOffset = cmap->encodingSubtable.encodingFormat.format.format4.idRangeOffset[i];
            
            if (idRangeOffset == 0)
                charGlyph = (cmap->encodingSubtable.encodingFormat.format.format4.idDelta[i] + ch) % 0x10000;
            else {
                SFUShort idIndex = (idRangeOffset / 2) - segCount + i + (ch - startCount);
                SFGlyph glyph = cmap->encodingSubtable.encodingFormat.format.format4.glyphIdArray[idIndex];
                if (glyph) {
                    glyph += cmap->encodingSubtable.encodingFormat.format.format4.idDelta[i];
                }
                
                charGlyph = glyph;
            }
        }
    } else {
        charGlyph = 0;
    }
    
    return charGlyph;
}

void SFApplyCMAP(SFInternal *internal) {
    SFGlyphIndex index = SFMakeGlyphIndex(0, 0);
    for (; index.record < SFGetCharCount(internal); index.record++) {
        SFGetGlyph(internal, index) = SFCharToGlyph(internal->cmap, SFGetChar(internal, index.record));
    }
}
