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

#include <stdio.h>
#include <stdlib.h>

#include <SFTypes.h>
#include "SFInternal.h"
#include "SFCommonData.h"
#include "SFCMAPData.h"

static SFBool SFReadEncodingFormat(const SFUByte * const efTable, long tableLength, EncodingFormat *tablePtr) {
    SFUShort formatNumber = SFReadUShort(efTable, 0);
    
    if (formatNumber == 4) {
        SFUShort segCount;
        SFUShort *startCount;
        SFUShort *endCount;
        
        SFUShort *idDelta;
        SFUShort *idRangeOffset;
        
        SFUShort glyphArrLength;
        SFUShort *glyphIdArray;
        
        SFUShort beginOffset;
        SFUShort i;
        
        tablePtr->formatNumber = formatNumber;
        tablePtr->format.format4.length = SFReadUShort(efTable, 2);
        tablePtr->format.format4.language = SFReadUShort(efTable, 4);
        tablePtr->format.format4.segCountX2 = SFReadUShort(efTable, 6);
        tablePtr->format.format4.searchRange = SFReadUShort(efTable, 8);
        tablePtr->format.format4.entrySelector = SFReadUShort(efTable, 10);
        tablePtr->format.format4.rangeShift = SFReadUShort(efTable, 12);
        
#ifdef CMAP_TEST
        printf("\n   Format Number: %d", formatNumber);
        printf("\n   Length: %d", tablePtr->format.format4.length);
        printf("\n   Language: %d", tablePtr->format.format4.language);
        printf("\n   Segement Count * 2: %d", tablePtr->format.format4.segCountX2);
        printf("\n   Search Range: %d", tablePtr->format.format4.searchRange);
        printf("\n   Entry Selector: %d", tablePtr->format.format4.entrySelector);
        printf("\n   Range Shift: %d", tablePtr->format.format4.rangeShift);
#endif
        
        segCount = tablePtr->format.format4.segCountX2 / 2;
        endCount = malloc(sizeof(SFUShort) * segCount);
        
        beginOffset = 14;
        for (i = 0; i < segCount; i++) {
            endCount[i] = SFReadUShort(efTable, beginOffset);
            
#ifdef CMAP_TEST
            printf("\n   End Code At Index %d: %d", i, endCount[i]);
#endif
            
            beginOffset += 2;
        }
        
        tablePtr->format.format4.reservedPad = SFReadUShort(efTable, beginOffset);
        beginOffset += 2;
        
        tablePtr->format.format4.endCount = endCount;
        
        startCount = malloc(sizeof(SFUShort) * segCount);
        for (i = 0; i < segCount; i++) {
            startCount[i] = SFReadUShort(efTable, beginOffset);
            
#ifdef CMAP_TEST
            printf("\n   Start Code At Index %d: %d", i, startCount[i]);
#endif
            
            beginOffset += 2;
        }
        
        tablePtr->format.format4.startCount = startCount;
        
        idDelta = malloc(sizeof(SFUShort) * segCount);
        for (i = 0; i < segCount; i++) {
            idDelta[i] = SFReadUShort(efTable, beginOffset);
            
#ifdef CMAP_TEST
            printf("\n   ID Delta At Index %d: %d", i, idDelta[i]);
#endif
            
            beginOffset += 2;
        }
        
        tablePtr->format.format4.idDelta = idDelta;
        
        idRangeOffset = malloc(sizeof(SFUShort) * segCount);
        for (i = 0; i < segCount; i++) {
            idRangeOffset[i] = SFReadUShort(efTable, beginOffset);
            
#ifdef CMAP_TEST
            printf("\n   ID Range Offset At Index %d: %d", i, idRangeOffset[i]);
#endif
            
            beginOffset += 2;
        }
        
        tablePtr->format.format4.idRangeOffset = idRangeOffset;
        
        if (tablePtr->format.format4.length > tableLength)
            glyphArrLength = (tableLength - beginOffset);
        else
            glyphArrLength = (tablePtr->format.format4.length - beginOffset);
        
        glyphIdArray = malloc(glyphArrLength);
        
        glyphArrLength /= sizeof(SFUShort);
        for (i = 0; i < glyphArrLength; i++) {
            glyphIdArray[i] = SFReadUShort(efTable, beginOffset);
            
#ifdef CMAP_TEST
            printf("\n   Glyph ID At Index %d: %d", i, glyphIdArray[i]);
#endif
            
            beginOffset += 2;
        }
        
        tablePtr->format.format4.glyphIdArray = glyphIdArray;
        
        return SFTrue;
    }
    
    return SFFalse;
}

static void SFFreeEncodingFormat(EncodingFormat *tablePtr) {
    free(tablePtr->format.format4.endCount);
    free(tablePtr->format.format4.startCount);
    free(tablePtr->format.format4.idDelta);
    free(tablePtr->format.format4.idRangeOffset);
    free(tablePtr->format.format4.glyphIdArray);
}


void SFReadCMAP(const SFUByte * const table, SFTableCMAP *tablePtr, long cmapLength) {
	SFUShort i;
    int beginOffset = 0;
    
    tablePtr->version = SFReadUShort(table, beginOffset);
    beginOffset += 2;
    
    tablePtr->numTables = SFReadUShort(table, beginOffset);
    beginOffset += 2;
    
#ifdef CMAP_TEST
    printf("\nCMAP:");
    printf("\n Version: %d", tablePtr->version);
    printf("\n Number of Encoding Subtables: %d", tablePtr->numTables);
    printf("\n%lu", sizeof(table));
#endif
    
    for (i = 0; i < tablePtr->numTables; i++) {
        SFUShort platformID = SFReadUShort(table, beginOffset + 0);
        SFUShort encodingID = SFReadUShort(table, beginOffset + 2);
        SFUInt formatOffset = SFReadUInt(table, beginOffset + 4);
        
        SFUShort previousEncodingID = 0;
        
        beginOffset += 8;
        
#ifdef CMAP_TEST
        printf("\n Encoding Subtable At Index: %d", i);
        printf("\n  Platform ID: %d", platformID);
        printf("\n  Encoding ID: %d", encodingID);
#endif
        
        if ((platformID == piUnicode && encodingID >= previousEncodingID)
            || (platformID == piMicrosoft && (encodingID == msSymbol || encodingID == msUnicodeBMP_UCS_2))) {
            EncodingFormat eFormat;
            if (SFReadEncodingFormat(&table[formatOffset], cmapLength - formatOffset, &eFormat)) {
                
#ifdef CMAP_TEST
                printf("\n  Format Table:");
                printf("\n   Offset: %u", formatOffset);
#endif
                
                tablePtr->encodingSubtable.platformID = platformID;
                tablePtr->encodingSubtable.encodingID.unicodeEncodingID = encodingID;
                tablePtr->encodingSubtable.encodingFormat = eFormat;
                
                break;
            }
        }
    }
}

void SFFreeCMAP(SFTableCMAP *tablePtr) {
    SFFreeEncodingFormat(&tablePtr->encodingSubtable.encodingFormat);
}
