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

#include <stdint.h>

#include "SFTypes.h"
#include "SFCMAPUtilization.h"

void SFApplyCMAP(SFTableCMAP *tablePtr, SFStringRecord *strRecord) {
	int i;
    for (i = 0; i < strRecord->charCount; i++) {
        SFUnichar ch = strRecord->chars[i];
        SFUShort segCount = tablePtr->encodingSubtable.encodingFormat.format.format4.segCountX2 / 2;

		SFUShort j;
        for (j = 0; j < segCount; j++) {
            if (tablePtr->encodingSubtable.encodingFormat.format.format4.endCount[j] >= ch)
                break;
        }
        
        if (j < segCount) {
            SFUShort startCount = tablePtr->encodingSubtable.encodingFormat.format.format4.startCount[j];
            
            if (startCount > ch)
                strRecord->charRecord[i].gRec[0].glyph = 0;
            else {
                SFUShort idRangeOffset = tablePtr->encodingSubtable.encodingFormat.format.format4.idRangeOffset[j];
                
                if (idRangeOffset == 0)
                    strRecord->charRecord[i].gRec[0].glyph = (tablePtr->encodingSubtable.encodingFormat.format.format4.idDelta[j] + ch) % 0x10000;
                else {
                    SFUShort index = (idRangeOffset / 2) - segCount + j + (ch - startCount);
                    SFGlyph glyph = tablePtr->encodingSubtable.encodingFormat.format.format4.glyphIdArray[index];
                    if (glyph)
                        glyph += tablePtr->encodingSubtable.encodingFormat.format.format4.idDelta[j];
                    
                    strRecord->charRecord[i].gRec[0].glyph = glyph;
                }
            }
        } else
            strRecord->charRecord[i].gRec[0].glyph = 0;
    }
}
