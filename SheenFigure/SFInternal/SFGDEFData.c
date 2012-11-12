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

#include "SFGlobal.h"
#include "SFTypes.h"
#include "SFInternal.h"
#include "SFGDEFData.h"


#ifdef GDEF_ATTACH_LIST

static void SFReadAttachListTable(const SFUByte * const alTable, AttachListTable *tablePtr) {
    SFUShort coverageOffset;
    SFUShort glyphCount;
    
    AttachPointTable *attachPoints;
    
    SFUShort i;
    
    coverageOffset = SFReadUShort(alTable, 0);
    
#ifdef GDEF_TEST
    printf("\n  Coverage Table:");
    printf("\n   Offset: %d", coverageOffset);
#endif
    
    SFReadCoverageTable(&alTable[coverageOffset], &tablePtr->coverage);
    
    glyphCount = SFReadUShort(alTable, 2);
    tablePtr->glyphCount = glyphCount;
    
#ifdef GDEF_TEST
    printf("\n   Total Glyphs: %d", glyphCount);
#endif
    
    attachPoints = malloc(sizeof(AttachPointTable) * glyphCount);
    
    for (i = 0; i < glyphCount; i++) {
        SFUShort attachPointOffset;
        const SFUByte *apTable;
        
        SFUShort pointCount;
        SFUShort *pointIndexes;
        
        SFUShort j;
        
        attachPointOffset = SFReadUShort(alTable, 4 + (i * 2));
        apTable = &alTable[attachPointOffset];
        
#ifdef GDEF_TEST
        printf("\n   Attach Point At Index %d:", i);
        printf("\n    Offset: %d", attachPointOffset);
#endif
        
        pointCount = SFReadUShort(apTable, 0);
        attachPoints[i].pointCount = pointCount;
        
#ifdef GDEF_TEST
        printf("\n    Total Points: %d", pointCount);
#endif
        
        pointIndexes = malloc(sizeof(SFUShort) * pointCount);
        
        for (j = 0; j < glyphCount; j++) {
            pointIndexes[j] = SFReadUShort(apTable, 2 + (j * 2));
            
#ifdef GDEF_TEST
            printf("\n    Point At Index %d: %d", j, pointIndexes[j]);
#endif
        }
        
        attachPoints[i].pointIndex = pointIndexes;
    }
    
    tablePtr->attachPoint = attachPoints;
}

static void SFFreeAttachListTable(AttachListTable *tablePtr) {
	SFUShort i;

    SFFreeCoverageTable(&tablePtr->coverage);
    
    for (i = 0; i < tablePtr->glyphCount; i++)
        free(tablePtr->attachPoint[i].pointIndex);
    
    free(tablePtr->attachPoint);
}

#endif


#ifdef GDEF_LIG_CARET_LIST

static void SFReadLigatureGlyphTable(const SFUByte * const lgTable, LigatureGlyphTable *tablePtr) {
    SFUShort caretCount;
    CaretValuesTable *caretValues;
    
    SFUShort i;
    
    caretCount = SFReadUShort(lgTable, 0);
    tablePtr->caretCount = caretCount;
    
#ifdef GDEF_TEST
    printf("\n   Caret Count: %d", caretCount);
#endif
    
    caretValues = malloc(sizeof(CaretValuesTable) * caretCount);
    
    for (i = 0; i < caretCount; i++) {
        SFUShort caretValueOffset;
        const SFUByte *cvTable;
        
        SFUShort caretValueFormat;
        
        caretValueOffset = SFReadUShort(lgTable, 2 + (i * 2));
        cvTable = &lgTable[caretValueOffset];
        
#ifdef GDEF_TEST
        printf("\n   Caret Value At Index %d:", i);
        printf("\n    Offset: %d", caretValueOffset);
#endif
        
        caretValueFormat = SFReadUShort(cvTable, 0);
        caretValues[i].caretValueFormat = caretValueFormat;
        
#ifdef GDEF_TEST
        printf("\n    Format: %d", caretValueFormat);
#endif
        
        switch (caretValueFormat) {
            case 1:
            {
                caretValues[i].format.format1.coordinate = SFReadUShort(cvTable, 2);
                
#ifdef GDEF_TEST
                printf("\n    Coordinate: %d", caretValues[i].format.format1.coordinate);
#endif
            }
                break;
                
            case 2:
            {
                caretValues[i].format.format2.caretValuePoint = SFReadUShort(cvTable, 2);
                
#ifdef GDEF_TEST
                printf("\n    Caret Value Point: %d", caretValues[i].format.format2.caretValuePoint);
#endif
            }
                break;
                
            case 3:
            {
                SFUShort deviceTableOffset;
                
                caretValues[i].format.format3.coordinate = SFReadUShort(cvTable, 2);
                
#ifdef GDEF_TEST
                printf("\n    Coordinate: %d", caretValues[i].format.format3.coordinate);
#endif
                
                deviceTableOffset = SFReadUShort(cvTable, 4);
                
#ifdef GDEF_TEST
                printf("\n    Device Table:");
                printf("\n     Offset: %d", deviceTableOffset);
#endif
                
                SFReadDeviceTable(&cvTable[deviceTableOffset], &caretValues[i].format.format3.deviceTable);
            }
                break;
        }
    }
    
    tablePtr->caretValue = caretValues;
}

static void SFFreeLigatureGlyphTable(LigatureGlyphTable *tablePtr) {
    free(tablePtr->caretValue);
}


static void SFReadLigatureCaretListTable(const SFUByte * const lclTable, LigatureCaretListTable *tablePtr) {
    SFUShort coverageOffset;
    SFUShort ligatureGlyphCount;
    LigatureGlyphTable *ligatureGlyphTables;
    
    SFUShort i;
    
    coverageOffset = SFReadUShort(lclTable, 0);
    
#ifdef GDEF_TEST
    printf("\n  Coverage Table:");
    printf("\n   Offset: %d", coverageOffset);
#endif
    
    SFReadCoverageTable(&lclTable[coverageOffset], &tablePtr->coverage);
    
    ligatureGlyphCount = SFReadUShort(lclTable, 2);
    tablePtr->ligGlyphCount = ligatureGlyphCount;
    
#ifdef GDEF_TEST
    printf("\n  Total Ligature Glyph Tables: %d", ligatureGlyphCount);
#endif
    
    ligatureGlyphTables = malloc(sizeof(LigatureGlyphTable) * ligatureGlyphCount);
    
    for (i = 0; i < ligatureGlyphCount; i++) {
        SFUShort ligatureGlyphOffset = SFReadUShort(lclTable, 4 + (i * 2));
        
#ifdef GDEF_TEST
        printf("\n  Ligature Glyph Table At Index %d:", i);
        printf("\n   Offset: %d", coverageOffset);
#endif
        
        SFReadLigatureGlyphTable(&lclTable[ligatureGlyphOffset], &ligatureGlyphTables[i]);
    }
    
    tablePtr->LigGlyph = ligatureGlyphTables;
}

static void SFFreeLigatureCaretListTable(LigatureCaretListTable *tablePtr) {
	SFUShort i;

    SFFreeCoverageTable(&tablePtr->coverage);
    
    for (i = 0; i < tablePtr->ligGlyphCount; i++)
        SFFreeLigatureGlyphTable(&tablePtr->LigGlyph[i]);
    
    free(tablePtr->LigGlyph);
}

#endif


#ifdef GDEF_MARK_GLYPH_SETS_DEF

static void SFReadMarkGlyphSetsDefTable(const SFUByte * const mgsTable, MarkGlyphSetsDefTable *tablePtr) {
    SFUShort markSetCount;
    CoverageTable *coverages;

	SFUShort i;
    
    tablePtr->markSetTableFormat = SFReadUShort(mgsTable, 0);
    
#ifdef GDEF_TEST
    printf("\n  Format: %d", tablePtr->markSetTableFormat);
#endif
    
    markSetCount = SFReadUShort(mgsTable, 2);
    tablePtr->markSetCount = markSetCount;
    
#ifdef GDEF_TEST
    printf("\n  Mark Set Count: %d", markSetCount);
#endif
    
    coverages = malloc(sizeof(CoverageTable) * markSetCount);
    
    for (i = 0; i < markSetCount; i++) {
        SFUShort coverageOffset = SFReadUShort(mgsTable, 4 + (i * 2));
        
#ifdef GDEF_TEST
        printf("\n  Mark Set Coverage At Index %d:", i);
        printf("\n   Offset: %d", coverageOffset);
#endif
        
        SFReadCoverageTable(&mgsTable[coverageOffset], &coverages[i]);
    }
    
    tablePtr->coverage = coverages;
}

static void SFFreeMarkGlyphSetsDefTable(MarkGlyphSetsDefTable *tablePtr) {
	SFUShort i;
    for (i = 0; i < tablePtr->markSetCount; i++)
        SFFreeCoverageTable(&tablePtr->coverage[i]);
    
    free(tablePtr->coverage);
}

#endif


void SFReadGDEF(const SFUByte * const table, SFTableGDEF *tablePtr) {
    SFUShort glyphClassDefOffset;
    SFUShort attachListOffset;
    SFUShort ligCaretListOffset;
    SFUShort markAttachClassDefOffset;
    
    tablePtr->version = SFReadUInt(table, 0);
    
#ifdef GDEF_TEST
    printf("\nGDEF Header:");
    printf("\n Version: %u", tablePtr->version);
#endif
    
#ifdef GDEF_GLYPH_CLASS_DEF
    
    glyphClassDefOffset = SFReadUShort(table, 4);
    
#ifdef GDEF_TEST
    printf("\n Glyph Class Definition:");
    printf("\n  Offset: %d", glyphClassDefOffset);
#endif
    
    tablePtr->hasGlyphClassDef = (glyphClassDefOffset > 0);
    if (tablePtr->hasGlyphClassDef)
        SFReadClassDefTable(&table[glyphClassDefOffset], &tablePtr->glyphClassDef);
    
#endif
    
    
#ifdef GDEF_ATTACH_LIST
    
    attachListOffset = SFReadUShort(table, 6);
    
#ifdef GDEF_TEST
    printf("\n Attach List:");
    printf("\n  Offset: %d", attachListOffset);
#endif
    
    tablePtr->hasAttachList = (attachListOffset > 0);
    if (tablePtr->hasAttachList)
        SFReadAttachListTable(&table[attachListOffset], &tablePtr->attachList);
    
#endif
    
    
#ifdef GDEF_LIG_CARET_LIST
    
    ligCaretListOffset = SFReadUShort(table, 8);
    
#ifdef GDEF_TEST
    printf("\n Ligature Caret List:");
    printf("\n  Offset: %d", ligCaretListOffset);
#endif
    
    tablePtr->hasLigCaretList = (ligCaretListOffset > 0);
    if (tablePtr->hasLigCaretList)
        SFReadLigatureCaretListTable(&table[ligCaretListOffset], &tablePtr->ligCaretList);
    
#endif
    
    
#ifdef GDEF_MARK_ATTACH_CLASS_DEF
    markAttachClassDefOffset = SFReadUShort(table, 10);
    
#ifdef GDEF_TEST
    printf("\n Mark Attach Class Definition:");
    printf("\n  Offset: %d", markAttachClassDefOffset);
#endif
    
    tablePtr->hasMarkAttachClassDef = (markAttachClassDefOffset > 0);
    if (tablePtr->hasMarkAttachClassDef)
        SFReadClassDefTable(&table[markAttachClassDefOffset], &tablePtr->markAttachClassDef);
    
#endif
    
    
#ifdef GDEF_MARK_GLYPH_SETS_DEF
    
    tablePtr->hasMarkGlyphSetsDef = (tablePtr->version == 0x00010002);
    if (tablePtr->hasMarkGlyphSetsDef) {
        SFUShort markGlyphSetsDefOffset = SFReadUShort(table, 12);
        
#ifdef GDEF_TEST
        printf("\n Mark Glyph Sets Definition:");
        printf("\n  Offset: %d", markGlyphSetsDefOffset);
#endif
        
        tablePtr->hasMarkGlyphSetsDef = (markGlyphSetsDefOffset > 0);
        if (tablePtr->hasMarkGlyphSetsDef)
            SFReadMarkGlyphSetsDefTable(&table[markGlyphSetsDefOffset], &tablePtr->markGlyphSetsDef);
    }
    
#endif
}

void SFFreeGDEF(SFTableGDEF *tablePtr) {
    
#ifdef GDEF_GLYPH_CLASS_DEF
    
    if (tablePtr->hasGlyphClassDef)
        SFFreeClassDefTable(&tablePtr->glyphClassDef);
    
#endif
    
    
#ifdef GDEF_ATTACH_LIST
    
    if (tablePtr->hasAttachList)
        SFFreeAttachListTable(&tablePtr->attachList);
    
#endif
    
    
#ifdef GDEF_LIG_CARET_LIST
    
    if (tablePtr->hasLigCaretList)
        SFFreeLigatureCaretListTable(&tablePtr->ligCaretList);
    
#endif
    
    
#ifdef GDEF_MARK_ATTACH_CLASS_DEF
    
    if (tablePtr->hasMarkAttachClassDef)
        SFFreeClassDefTable(&tablePtr->markAttachClassDef);
    
#endif
    
    
#ifdef GDEF_MARK_GLYPH_SETS_DEF
    
    if (tablePtr->hasMarkGlyphSetsDef)
        SFFreeMarkGlyphSetsDefTable(&tablePtr->markGlyphSetsDef);
    
#endif
    
}
