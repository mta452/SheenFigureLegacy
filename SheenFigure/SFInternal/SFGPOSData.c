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

#include <stdio.h>
#include <stdlib.h>

#include "SFGSUBGPOSData.h"
#include "SFGPOSData.h"

void **gposTable;

static DeviceTable nullDevice() {
    DeviceTable device;
    device.startSize = 0;
    device.endSize = 0;
    device.deltaFormat = 0;
    device.deltaValue = 0;
    
    return device;
}


#if defined(GPOS_SINGLE) || defined(GPOS_PAIR)

static void SFReadValueRecord(const SFUByte * const table, SFUShort *offset, ValueRecord *tablePtr, ValueFormat format) {
    DeviceTable nullDev;
    
    if (format & vfXPlacement) {
        tablePtr->xPlacement = SFReadUShort(table, *offset);
        *offset += 2;

#ifdef LOOKUP_TEST
        printf("\n          X Placement: %d", tablePtr->xPlacement);
#endif
    } else
        tablePtr->xPlacement = 0;
    
    if (format & vfYPlacement) {
        tablePtr->yPlacement = SFReadUShort(table, *offset);
        *offset += 2;
        
#ifdef LOOKUP_TEST
        printf("\n          Y Placement: %d", tablePtr->yPlacement);
#endif
    } else
        tablePtr->yPlacement = 0;
    
    if (format & vfXAdvance) {
        tablePtr->xAdvance = SFReadUShort(table, *offset);
        *offset += 2;
        
#ifdef LOOKUP_TEST
        printf("\n          X Advance: %d", tablePtr->xAdvance);
#endif
    } else
        tablePtr->xAdvance = 0;
    
    if (format & vfYAdvance) {
        tablePtr->yAdvance = SFReadUShort(table, *offset);
        *offset += 2;
        
#ifdef LOOKUP_TEST
        printf("\n          Y Advance: %d", tablePtr->yAdvance);
#endif
    } else
        tablePtr->yAdvance = 0;
    
    nullDev = nullDevice();
    tablePtr->xPlaDevice = nullDev;
    tablePtr->yPlaDevice = nullDev;
    tablePtr->xAdvDevice = nullDev;
    tablePtr->yAdvDevice = nullDev;
    
    if (format & vfXPlaDevice) {
        SFUShort xPlaDeviceOffset = SFReadUShort(table, *offset);
        
#ifdef LOOKUP_TEST
        printf("\n          X Placement Device Table:");
        printf("\n           Offset: %d", xPlaDeviceOffset);
#endif
        
        if (xPlaDeviceOffset)
            SFReadDeviceTable(gposTable[xPlaDeviceOffset], &tablePtr->xPlaDevice);
        
        *offset += 2;
    }
    
    if (format & vfYPlaDevice) {
        SFUShort yPlaDeviceOffset = SFReadUShort(table, *offset);
        
#ifdef LOOKUP_TEST
        printf("\n          Y Placement Device Table:");
        printf("\n           Offset: %d", yPlaDeviceOffset);
#endif
        
        if (yPlaDeviceOffset)
            SFReadDeviceTable(gposTable[yPlaDeviceOffset], &tablePtr->yPlaDevice);
        
        *offset += 2;
    }
    
    if (format & vfXAdvDevice) {
        SFUShort xAdvDeviceOffset = SFReadUShort(table, *offset);
        
#ifdef LOOKUP_TEST
        printf("\n          X Advance Device Table:");
        printf("\n           Offset: %d", xAdvDeviceOffset);
#endif
        
        if (xAdvDeviceOffset)
            SFReadDeviceTable(gposTable[xAdvDeviceOffset], &tablePtr->xAdvDevice);
        
        *offset += 2;
    }
    
    if (format & vfYAdvDevice) {
        SFUShort yAdvDeviceOffset = SFReadUShort(table, *offset);
        
#ifdef LOOKUP_TEST
        printf("\n          Y Advance Device Table:");
        printf("\n           Offset: %d", yAdvDeviceOffset);
#endif
        
        if (yAdvDeviceOffset)
            SFReadDeviceTable(gposTable[yAdvDeviceOffset], &tablePtr->yAdvDevice);
        
        *offset += 2;
    }
}


#ifdef GPOS_SINGLE

static void SFReadSingleAdjustment(const SFUByte * const saTable, SingleAdjustmentPosSubtable *tablePtr) {
    SFUShort posFormat;
    SFUShort coverageOffset;
    
    posFormat = SFReadUShort(saTable, 0);
    tablePtr->posFormat = posFormat;
    
    coverageOffset = SFReadUShort(saTable, 2);
    
#ifdef LOOKUP_TEST
    printf("\n      Single Adjustment Positioning Table:");
    printf("\n       Positioning Format: %d", posFormat);
    printf("\n       Coverage Table:");
    printf("\n        Offset: %d", coverageOffset);
#endif
    
    SFReadCoverageTable(&saTable[coverageOffset], &tablePtr->coverage);
    tablePtr->valueFormat = SFReadUShort(saTable, 4);
    
#ifdef LOOKUP_TEST
    printf("\n       Value Format: %d", tablePtr->valueFormat);
#endif
    
    switch (posFormat) {
            
#ifdef GPOS_SINGLE_FORMAT1
        case 1:
        {
            SFUShort valueOffset = 6;
            
#ifdef LOOKUP_TEST
            printf("\n       Value Record:");
#endif
            
            SFReadValueRecord(saTable, &valueOffset, &tablePtr->format.format1.value, tablePtr->valueFormat);
        }
            break;
#endif
            
#ifdef GPOS_SINGLE_FORMAT2
        case 2:
        {
            SFUShort valueCount;
            ValueRecord *values;
            
            SFUShort nextValueOffset;
            SFUShort i;
            
            valueCount = SFReadUShort(saTable, 6);
            tablePtr->format.format2.valueCount = valueCount;
            
#ifdef LOOKUP_TEST
            printf("\n       Total Values: %d", valueCount);
#endif
            
            values = malloc(sizeof(ValueRecord) * valueCount);
            
            nextValueOffset = 8;
            for (i = 0; i < valueCount; i++) {
#ifdef LOOKUP_TEST
                printf("\n       Value Record At Index %d:", i);
#endif
                
                SFReadValueRecord(saTable, &nextValueOffset, &values[i], tablePtr->valueFormat);
            }
            
            tablePtr->format.format2.value = values;
        }
            break;
#endif
    }
}

static void SFFreeSingleAdjustment(SingleAdjustmentPosSubtable *tablePtr) {
    SFFreeCoverageTable(&tablePtr->coverage);
    
#ifdef GPOS_SINGLE_FORMAT2
    if (tablePtr->posFormat == 2)
        free(tablePtr->format.format2.value);
#endif
}

#endif


#ifdef GPOS_PAIR

static void SFReadPairAdjustment(const SFUByte * const paTable, PairAdjustmentPosSubtable *tablePtr) {
    SFUShort posFormat;
    SFUShort coverageOffset;
    
    posFormat = SFReadUShort(paTable, 0);
    tablePtr->posFormat = posFormat;
    
    coverageOffset = SFReadUShort(paTable, 2);
    
#ifdef LOOKUP_TEST
    printf("\n      Pair Adjustment Positioning Table:");
    printf("\n       Positioning Format: %d", posFormat);
    printf("\n       Coverage Table:");
    printf("\n        Offset: %d", coverageOffset);
#endif
    
    SFReadCoverageTable(&paTable[coverageOffset], &tablePtr->coverage);
    
    tablePtr->valueFormat1 = SFReadUShort(paTable, 4);
    tablePtr->valueFormat2 = SFReadUShort(paTable, 6);
    
#ifdef LOOKUP_TEST
    printf("\n       Value Format 1: %d", tablePtr->valueFormat1);
    printf("\n       Value Format 2: %d", tablePtr->valueFormat2);
#endif
    
    switch (posFormat) {
            
#ifdef GPOS_PAIR_FORMAT1
        case 1:
        {
            SFUShort pairSetCount;
            PairSetTable *pairSetTables;
            
			SFUShort i;

            pairSetCount = SFReadUShort(paTable, 8);
            tablePtr->format.format1.pairSetCount = pairSetCount;
            
#ifdef LOOKUP_TEST
            printf("\n       Total Pair Sets: %d", pairSetCount);
#endif
            
            pairSetTables = malloc(sizeof(PairSetTable) * pairSetCount);
            
            for (i = 0; i < pairSetCount; i++) {
                SFUShort pairSetOffset;
                const SFUByte *psTable;
                
                SFUShort pairValueCount;
                PairValueRecord *pairValueRecords;
                
                SFUShort beginOffset;
                SFUShort j;
                
                pairSetOffset = SFReadUShort(paTable, 10);
                psTable = &paTable[pairSetOffset];
                
#ifdef LOOKUP_TEST
                printf("\n       Pair Set At Index %d:", i);
                printf("\n        Offset: %d", pairSetOffset);
#endif
                
                pairValueCount = SFReadUShort(psTable, 0);
                pairSetTables[i].pairValueCount = pairValueCount;
                
#ifdef LOOKUP_TEST
                printf("\n        Total Pair Values: %d", pairValueCount);
#endif
                
                pairValueRecords = malloc(sizeof(PairValueRecord) * pairValueCount);
                
                beginOffset = 2;
                for (j = 0; j < pairValueCount; j++) {
                    pairValueRecords[j].secondGlyph = SFReadUShort(psTable, beginOffset);
                    beginOffset += 2;
                    
#ifdef LOOKUP_TEST
                    printf("\n        Pair Value At Index: %d", j);
                    printf("\n         Second Glyph: %d", pairValueRecords[j].secondGlyph);
                    printf("\n         Value 1 Record:");
#endif
                    
                    SFReadValueRecord(psTable, &beginOffset, &pairValueRecords[j].value1, tablePtr->valueFormat1);
                    
#ifdef LOOKUP_TEST
                    printf("\n         Value 2 Record:");
#endif
                    
                    SFReadValueRecord(psTable, &beginOffset, &pairValueRecords[j].value2, tablePtr->valueFormat2);
                }
                
                pairSetTables[i].pairValueRecord = pairValueRecords;
            }
            
            tablePtr->format.format1.pairSetTable = pairSetTables;
        }
            break;
#endif
            
#ifdef GPOS_PAIR_FORMAT2
        case 2:
        {
            SFUShort classDef1Offset;
            SFUShort classDef2Offset;
            
            SFUShort class1Count;
            SFUShort class2Count;
            
            Class1Record *class1Records;
            SFUShort beginOffset;
            SFUShort i = 0;
            
            classDef1Offset = SFReadUShort(paTable, 8);
            
#ifdef LOOKUP_TEST
            printf("\n       Class Definition 1:");
            printf("\n        Offset: %d", classDef1Offset);
#endif
            
            SFReadClassDefTable(&paTable[classDef1Offset], &tablePtr->format.format2.classDef1);
            
            classDef2Offset = SFReadUShort(paTable, 10);
            
#ifdef LOOKUP_TEST
            printf("\n       Class Definition 2:");
            printf("\n        Offset: %d", classDef2Offset);
#endif
            
            SFReadClassDefTable(&paTable[classDef2Offset], &tablePtr->format.format2.classDef2);
            
            class1Count = SFReadUShort(paTable, 12);
            class2Count = SFReadUShort(paTable, 14);
            
#ifdef LOOKUP_TEST
            printf("\n       Total Classes in Class Definition 1: %d", class1Count);
            printf("\n       Total Classes in Class Definition 2: %d", class2Count);
#endif
            
            tablePtr->format.format2.class1Count = class1Count;
            tablePtr->format.format2.class2Count = class2Count;
            
            class1Records = malloc(sizeof(Class1Record) * class1Count);
            
            beginOffset = 16;
            for (i = 0; i < class1Count; i++) {
                Class2Record *class2Records;
                SFUShort j;

#ifdef LOOKUP_TEST
                printf("\n       Class 1 Record At Index %d:", i);
#endif
                
                class2Records = malloc(sizeof(Class2Record) * class2Count);
                
                for (j = 0; j < class2Count; j++) {
#ifdef LOOKUP_TEST
                    printf("\n        Class 2 Record At Index %d:", j);
                    printf("\n         Value 1 Record:");
#endif
                    
                    SFReadValueRecord(paTable, &beginOffset, &class2Records[j].value1, tablePtr->valueFormat1);
                    
#ifdef LOOKUP_TEST
                    printf("\n         Value 2 Record:");
#endif
                    
                    SFReadValueRecord(paTable, &beginOffset, &class2Records[j].value2, tablePtr->valueFormat2);
                }
                
                class1Records[i].class2Record = class2Records;
            }
            
            tablePtr->format.format2.class1Record = class1Records;
        }
            break;
#endif
    }
}

static void SFFreePairAdjustment(PairAdjustmentPosSubtable *tablePtr) {
    SFUShort i = 0;
    
    SFFreeCoverageTable(&tablePtr->coverage);
    
    switch (tablePtr->posFormat) {
            
#ifdef GPOS_PAIR_FORMAT1
        case 1:
        {
            for (; i < tablePtr->format.format1.pairSetCount; i++)
                free(tablePtr->format.format1.pairSetTable[i].pairValueRecord);
            
            free(tablePtr->format.format1.pairSetTable);
        }
            break;
#endif
            
#ifdef GPOS_PAIR_FORMAT2
        case 2:
        {
            SFFreeClassDefTable(&tablePtr->format.format2.classDef1);
            SFFreeClassDefTable(&tablePtr->format.format2.classDef2);
            
            for (; i < tablePtr->format.format2.class1Count; i++)
                free(tablePtr->format.format2.class1Record[i].class2Record);
            
            free(tablePtr->format.format2.class1Record);
        }
            break;
#endif
    }
}

#endif

#endif


#ifdef GPOS_ANCHOR

static void SFReadAnchorTable(const SFUByte * const aTable, AnchorTable *tablePtr) {
    tablePtr->anchorFormat = SFReadUShort(aTable, 0);
    tablePtr->xCoordinate = SFReadUShort(aTable, 2);
    tablePtr->yCoordinate = SFReadUShort(aTable, 4);
    
#ifdef LOOKUP_TEST
    printf("\n         Anchor Format: %d", tablePtr->anchorFormat);
    printf("\n         X Coordinate: %d", tablePtr->xCoordinate);
    printf("\n         Y Coordinate: %d", tablePtr->yCoordinate);
#endif
    
    switch (tablePtr->anchorFormat) {
        case 2:
            tablePtr->format.format2.anchorPoint = SFReadUShort(aTable, 6);
            
#ifdef LOOKUP_TEST
            printf("\n         Anchor Point: %d", tablePtr->format.format2.anchorPoint);
#endif
            break;
            
        case 3:
        {
            SFUShort xDeviceOffset;
            SFUShort yDeviceOffset;
            
            xDeviceOffset = SFReadUShort(aTable, 6);
            
#ifdef LOOKUP_TEST
            printf("\n         X Device Table:");
            printf("\n          Offset: %d", xDeviceOffset);
#endif
            SFReadDeviceTable(&aTable[xDeviceOffset], &tablePtr->format.format3.xDeviceTable);
            
            yDeviceOffset = SFReadUShort(aTable, 8);
            
#ifdef LOOKUP_TEST
            printf("\n         Y Device Table:");
            printf("\n          Offset: %d", yDeviceOffset);
#endif
            SFReadDeviceTable(&aTable[yDeviceOffset], &tablePtr->format.format3.yDeviceTable);
        }
            break;
    }
}

#endif


#ifdef GPOS_CURSIVE

static void SFReadCursiveAttachment(const SFUByte * const caTable, CursiveAttachmentPosSubtable *tablePtr) {
    SFUShort posFormat;
    SFUShort coverageOffset;
    
    SFUShort entryExitCount;
    EntryExitRecord *entryExitRecords;
    
    SFUShort beginOffset;
    SFUShort i;
    
    posFormat = SFReadUShort(caTable, 0);
    tablePtr->posFormat = posFormat;
    
    coverageOffset = SFReadUShort(caTable, 2);
    
#ifdef LOOKUP_TEST
    printf("\n      Cursive Attachment Positioning Table:");
    printf("\n       Positioning Format: %d", posFormat);
    printf("\n       Coverage Table:");
    printf("\n        Offset: %d", coverageOffset);
#endif
    
    SFReadCoverageTable(&caTable[coverageOffset], &tablePtr->coverage);
    
    entryExitCount = SFReadUShort(caTable, 4);
    
#ifdef LOOKUP_TEST
    printf("\n       Total Entry Exits: %d", entryExitCount);
#endif
    
    entryExitRecords = malloc(sizeof(EntryExitRecord) * entryExitCount);
    
    beginOffset = 6;
    for (i = 0; i < entryExitCount; i++) {
        SFUShort entryAnchorOffset;
        SFUShort exitAnchorOffset;
        
#ifdef LOOKUP_TEST
        printf("\n       Entry Exit At Index: %d", i);
#endif
        
        entryAnchorOffset = SFReadUShort(caTable, beginOffset);
        beginOffset += 2;
        
#ifdef LOOKUP_TEST
        printf("\n        Entry Anchor:");
        printf("\n         Offset: %d", entryAnchorOffset);
#endif
        
        entryExitRecords[i].hasEntryAnchor = entryAnchorOffset;
        SFReadAnchorTable(&caTable[entryAnchorOffset], &entryExitRecords[i].entryAnchor);
        
        exitAnchorOffset = SFReadUShort(caTable, beginOffset);
        beginOffset += 2;
        
#ifdef LOOKUP_TEST
        printf("\n        Exit Anchor:");
        printf("\n         Offset: %d", exitAnchorOffset);
#endif
        
        entryExitRecords[i].hasExitAnchor = exitAnchorOffset;
        SFReadAnchorTable(&caTable[exitAnchorOffset], &entryExitRecords[i].exitAnchor);
    }
    
    tablePtr->entryExitRecord = entryExitRecords;
}

static void SFFreeCursiveAttachment(CursiveAttachmentPosSubtable *tablePtr) {
    SFFreeCoverageTable(&tablePtr->coverage);
    free(tablePtr->entryExitRecord);
}

#endif


#ifdef GPOS_MARK

static void SFReadMarkArray(const SFUByte * const maTable, MarkArrayTable *tablePtr) {
    SFUShort markCount;
    MarkRecord *markRecords;
    SFUShort beginOffset;
    
	SFUShort i;

    markCount = SFReadUShort(maTable, 0);
    tablePtr->markCount = markCount;
    
#ifdef LOOKUP_TEST
    printf("\n        Total Mark Records: %d", markCount);
#endif
    
    markRecords = malloc(sizeof(MarkRecord) * markCount);
    
    beginOffset = 2;
    for (i = 0; i < markCount; i++) {
        SFUShort markAnchorOffset;
        
        markRecords[i].cls = SFReadUShort(maTable, beginOffset);
        
#ifdef LOOKUP_TEST
        printf("\n        Mark Record At Index %d:", i);
        printf("\n         Class: %d", markRecords[i].cls);
#endif
        
        beginOffset += 2;
        markAnchorOffset = SFReadUShort(maTable, beginOffset);
        
#ifdef LOOKUP_TEST
        printf("\n         Mark Anchor Table:");
        printf("\n          Offset: %d", markAnchorOffset);
#endif
        
        SFReadAnchorTable(&maTable[markAnchorOffset], &markRecords[i].markAnchor);
        
        beginOffset += 2;
    }
    
    tablePtr->markRecord = markRecords;
}

static void SFFreeMarkArray(MarkArrayTable *tablePtr) {
    free(tablePtr->markRecord);
}

#endif


#ifdef GPOS_MARK_TO_BASE

static void SFReadMarkToBaseAttachment(const SFUByte * const mtbTable, MarkToBaseAttachmentPosSubtable *tablePtr) {
    SFUShort posFormat;
    SFUShort markCoverageOffset;
    SFUShort baseCoverageOffset;
    SFUShort markArrayOffset;
    SFUShort baseArrayOffset;
    
    SFUShort classCount;
    
    const SFUByte *baTable;
    BaseArrayTable baseArrayTable;
    SFUShort baseCount;
    
    BaseRecord *baseRecords;
    SFUShort beginOffset;
    SFUShort i;
    
    posFormat = SFReadUShort(mtbTable, 0);
    tablePtr->posFormat = posFormat;
    
    markCoverageOffset = SFReadUShort(mtbTable, 2);
    
#ifdef LOOKUP_TEST
    printf("\n      Mark To Base Attachment Positioning Table:");
    printf("\n       Positioning Format: %d", posFormat);
    printf("\n       Mark Coverage Table:");
    printf("\n        Offset: %d", markCoverageOffset);
#endif
    
    SFReadCoverageTable(&mtbTable[markCoverageOffset], &tablePtr->markCoverage);
    
    baseCoverageOffset = SFReadUShort(mtbTable, 4);
    
#ifdef LOOKUP_TEST
    printf("\n       Base Coverage Table:");
    printf("\n        Offset: %d", baseCoverageOffset);
#endif
    
    SFReadCoverageTable(&mtbTable[baseCoverageOffset], &tablePtr->baseCoverage);
    
    classCount = SFReadUShort(mtbTable, 6);
    tablePtr->classCount = classCount;
    
#ifdef LOOKUP_TEST
    printf("\n       Total Classes: %d", classCount);
#endif
    
    markArrayOffset = SFReadUShort(mtbTable, 8);
    
#ifdef LOOKUP_TEST
    printf("\n       Mark Array Table:");
    printf("\n        Offset: %d", markArrayOffset);
#endif
    
    SFReadMarkArray(&mtbTable[markArrayOffset], &tablePtr->markArray);
    
    baseArrayOffset = SFReadUShort(mtbTable, 10);
    
#ifdef LOOKUP_TEST
    printf("\n       Base Array Table:");
    printf("\n        Offset: %d", baseArrayOffset);
#endif
    
    baTable = &mtbTable[baseArrayOffset];
    
    baseCount = SFReadUShort(baTable, 0);
    baseArrayTable.baseCount = baseCount;
    
#ifdef LOOKUP_TEST
    printf("\n        Total Base Records: %d", baseCount);
#endif
    
    baseRecords = malloc(sizeof(BaseRecord) * baseCount);
    
    beginOffset = 2;
    for (i = 0; i < baseCount; i++) {
        AnchorTable *baseAnchors;
        SFUShort j;
        
#ifdef LOOKUP_TEST
        printf("\n        Base Record At Index %d:", i);
        printf("\n         Total Base Anchors: %d", classCount);
#endif
        
        baseAnchors = malloc(sizeof(AnchorTable) * classCount);
        
        for (j = 0; j < classCount; j++) {
            SFUShort baseAnchorOffset = SFReadUShort(baTable, beginOffset);
            
#ifdef LOOKUP_TEST
            printf("\n         Base Anchor At Index %d:", j);
            printf("\n          Offset: %d", baseAnchorOffset);
#endif
            
            SFReadAnchorTable(&baTable[baseAnchorOffset], &baseAnchors[j]);
            
            beginOffset += 2;
        }
        
        baseRecords[i].baseAnchor = baseAnchors;
    }
    
    baseArrayTable.baseRecord = baseRecords;
    tablePtr->baseArray = baseArrayTable;
}

static void SFFreeMarkToBaseAttachment(MarkToBaseAttachmentPosSubtable *tablePtr) {
	SFUShort i;

    SFFreeCoverageTable(&tablePtr->markCoverage);
    SFFreeCoverageTable(&tablePtr->baseCoverage);
    SFFreeMarkArray(&tablePtr->markArray);
    
    for (i = 0; i < tablePtr->baseArray.baseCount; i++)
        free(tablePtr->baseArray.baseRecord[i].baseAnchor);
    
    free(tablePtr->baseArray.baseRecord);
}

#endif


#ifdef GPOS_MARK_TO_LIGATURE

static void SFReadMarkToLigatureAttachment(const SFUByte * const mtlTable, MarkToLigatureAttachmentPosSubtable *tablePtr) {
    SFUShort posFormat;
    SFUShort markCoverageOffset;
    SFUShort ligatureCoverageOffset;
    SFUShort markArrayOffset;
    SFUShort ligatureArrayOffset;
    
    SFUShort classCount;
    
    const SFUByte *laTable;
    LigatureArrayTable ligatureArrayTable;
    SFUShort ligatureCount;
    
    LigatureAttachTable *ligatureAttachTables;
    SFUShort i;
    
    posFormat = SFReadUShort(mtlTable, 0);
    tablePtr->posFormat = posFormat;
    
    markCoverageOffset = SFReadUShort(mtlTable, 2);
    
#ifdef LOOKUP_TEST
    printf("\n      Mark To Ligature Attachment Positioning Table:");
    printf("\n       Positioning Format: %d", posFormat);
    printf("\n       Mark Coverage Table:");
    printf("\n        Offset: %d", markCoverageOffset);
#endif
    
    SFReadCoverageTable(&mtlTable[markCoverageOffset], &tablePtr->markCoverage);
    
    ligatureCoverageOffset = SFReadUShort(mtlTable, 4);
    
#ifdef LOOKUP_TEST
    printf("\n       Ligature Coverage Table:");
    printf("\n        Offset: %d", ligatureCoverageOffset);
#endif
    
    SFReadCoverageTable(&mtlTable[ligatureCoverageOffset], &tablePtr->ligatureCoverage);
    
    classCount = SFReadUShort(mtlTable, 6);
    tablePtr->classCount = classCount;
    
#ifdef LOOKUP_TEST
    printf("\n       Total Classes: %d", classCount);
#endif
    
    markArrayOffset = SFReadUShort(mtlTable, 8);
    
#ifdef LOOKUP_TEST
    printf("\n       Mark Array Table:");
    printf("\n        Offset: %d", markArrayOffset);
#endif
    
    SFReadMarkArray(&mtlTable[markArrayOffset], &tablePtr->markArray);
    
    ligatureArrayOffset = SFReadUShort(mtlTable, 10);
    
#ifdef LOOKUP_TEST
    printf("\n       Ligature Array Table:");
    printf("\n        Offset: %d", ligatureArrayOffset);
#endif
    
    laTable = &mtlTable[ligatureArrayOffset];
    
    ligatureCount = SFReadUShort(laTable, 0);
    ligatureArrayTable.ligatureCount = ligatureCount;
    
#ifdef LOOKUP_TEST
    printf("\n        Total Ligature Attach Tables: %d", ligatureCount);
#endif
    
    ligatureAttachTables = malloc(sizeof(LigatureAttachTable) * ligatureCount);
    
    for (i = 0; i < ligatureCount; i++) {
        SFUShort ligatureAttachOffset;
        const SFUByte *latTable;
        
        SFUShort componentCount;
        ComponentRecord *componentRecords;
        
        SFUShort beginOffset;
        SFUShort j;
        
        ligatureAttachOffset = SFReadUShort(laTable, 2 + (i * 2));
        
#ifdef LOOKUP_TEST
        printf("\n        Ligature Attach Table At Index %d:", i);
        printf("\n         Offset: %d", ligatureAttachOffset);
#endif
        
        latTable = &laTable[ligatureAttachOffset];
        
        componentCount = SFReadUShort(latTable, 0);
        ligatureAttachTables[i].componentCount = componentCount;
        
#ifdef LOOKUP_TEST
        printf("\n         Total Component Records: %d", componentCount);
#endif
        
        componentRecords = malloc(sizeof(ComponentRecord) * componentCount);
        
        beginOffset = 2;
        for (j = 0; j < componentCount; j++) {
            AnchorTable *ligatureAnchors;
            SFUShort k;
            
#ifdef LOOKUP_TEST
            printf("\n         Component Record At Index %d:", j);
            printf("\n          Total Ligature Anchor Tables: %d", classCount);
#endif
            
            ligatureAnchors = malloc(sizeof(AnchorTable) * classCount);
            
            for (k = 0; k < classCount; k++) {
                SFUShort ligatureAnchorOffset = SFReadUShort(latTable, beginOffset);
                beginOffset += 2;
                
#ifdef LOOKUP_TEST
                printf("\n          Ligature Anchor At Index %d:", k);
                printf("\n           Offset: %d", ligatureAnchorOffset);
#endif
                
                SFReadAnchorTable(&latTable[ligatureAnchorOffset], &ligatureAnchors[k]);
            }
            
            componentRecords[j].ligatureAnchor = ligatureAnchors;
        }
        
        ligatureAttachTables[i].componentRecord = componentRecords;
    }
    
    ligatureArrayTable.ligatureAttach = ligatureAttachTables;
    tablePtr->ligatureArray = ligatureArrayTable;
}

static void SFFreeMarkToLigatureAttachment(MarkToLigatureAttachmentPosSubtable *tablePtr) {
	SFUShort i, j;

    SFFreeCoverageTable(&tablePtr->markCoverage);
    SFFreeCoverageTable(&tablePtr->ligatureCoverage);
    SFFreeMarkArray(&tablePtr->markArray);
    
    for (i = 0; i < tablePtr->ligatureArray.ligatureCount; i++) {
        for (j = 0; j < tablePtr->ligatureArray.ligatureAttach[i].componentCount; j++)
            free(tablePtr->ligatureArray.ligatureAttach[i].componentRecord[j].ligatureAnchor);
        
        free(tablePtr->ligatureArray.ligatureAttach[i].componentRecord);
    }
    
    free(tablePtr->ligatureArray.ligatureAttach);
}

#endif


#ifdef GPOS_MARK_TO_MARK

static void SFReadMarkToMarkAttachment(const SFUByte * const mtmTable, MarkToMarkAttachmentPosSubtable *tablePtr) {
    SFUShort posFormat;
    SFUShort mark1CoverageOffset;
    SFUShort mark2CoverageOffset;
    SFUShort mark1ArrayOffset;
    SFUShort mark2ArrayOffset;
    
    SFUShort classCount;
    
    const SFUByte *m2aTable;
    Mark2ArrayTable mark2ArrayTable;
    SFUShort mark2Count;
    
    Mark2Record *mark2Records;
    SFUShort beginOffset;
    SFUShort i;
    
    posFormat = SFReadUShort(mtmTable, 0);
    tablePtr->posFormat = posFormat;
    
    mark1CoverageOffset = SFReadUShort(mtmTable, 2);
    
#ifdef LOOKUP_TEST
    printf("\n      Mark To Mark Attachment Positioning Table:");
    printf("\n       Positioning Format: %d", posFormat);
    printf("\n       Mark 1 Coverage Table:");
    printf("\n        Offset: %d", mark1CoverageOffset);
#endif
    
    SFReadCoverageTable(&mtmTable[mark1CoverageOffset], &tablePtr->mark1Coverage);
    
    mark2CoverageOffset = SFReadUShort(mtmTable, 4);
    
#ifdef LOOKUP_TEST
    printf("\n       Mark 2 Coverage Table:");
    printf("\n        Offset: %d", mark2CoverageOffset);
#endif
    
    SFReadCoverageTable(&mtmTable[mark2CoverageOffset], &tablePtr->mark2Coverage);
    
    classCount = SFReadUShort(mtmTable, 6);
    tablePtr->classCount = classCount;
    
#ifdef LOOKUP_TEST
    printf("\n       Total Classes: %d", classCount);
#endif
    
    mark1ArrayOffset = SFReadUShort(mtmTable, 8);
    
#ifdef LOOKUP_TEST
    printf("\n       Mark 1 Array Table:");
    printf("\n        Offset: %d", mark1ArrayOffset);
#endif
    
    SFReadMarkArray(&mtmTable[mark1ArrayOffset], &tablePtr->mark1Array);
    
    mark2ArrayOffset = SFReadUShort(mtmTable, 10);
    
#ifdef LOOKUP_TEST
    printf("\n       Mark 2 Array Table:");
    printf("\n        Offset: %d", mark2ArrayOffset);
#endif
    
    m2aTable = &mtmTable[mark2ArrayOffset];
    
    mark2Count = SFReadUShort(m2aTable, 0);
    mark2ArrayTable.mark2Count = mark2Count;
    
#ifdef LOOKUP_TEST
    printf("\n        Total Mark 2 Records: %d", mark2Count);
#endif
    
    mark2Records = malloc(sizeof(Mark2Record) * mark2Count);
    
    beginOffset = 2;
    for (i = 0; i < mark2Count; i++) {
        AnchorTable *mark2Anchors;
        SFUShort j;
        
#ifdef LOOKUP_TEST
        printf("\n        Mark 2 Record At Index %d:", i);
        printf("\n         Total Mark 2 Anchor Tables: %d", classCount);
#endif
        
        mark2Anchors = malloc(sizeof(AnchorTable) * classCount);
        
        for (j = 0; j < classCount; j++) {
            SFUShort mark2AnchorOffset = SFReadUShort(m2aTable, beginOffset);
            
#ifdef LOOKUP_TEST
            printf("\n         Mark 2 Anchor At Index: %d", j);
            printf("\n          Offset: %d", mark2AnchorOffset);
#endif
            
            SFReadAnchorTable(&m2aTable[mark2AnchorOffset], &mark2Anchors[j]);
            
            beginOffset += 2;
        }
        
        mark2Records[i].mark2Anchor = mark2Anchors;
    }
    
    mark2ArrayTable.mark2Record = mark2Records;
    tablePtr->mark2Array = mark2ArrayTable;
}

static void SFFreeMarkToMarkAttachment(MarkToMarkAttachmentPosSubtable *tablePtr) {
	SFUShort i;

    SFFreeCoverageTable(&tablePtr->mark1Coverage);
    SFFreeCoverageTable(&tablePtr->mark2Coverage);
    SFFreeMarkArray(&tablePtr->mark1Array);
    
    for (i = 0; i < tablePtr->mark2Array.mark2Count; i++)
        free(tablePtr->mark2Array.mark2Record[i].mark2Anchor);
    
    free(tablePtr->mark2Array.mark2Record);
}

#endif


static void *SFReadPositioning(const SFUByte * const sTable, LookupType *type) {
    void *subtablePtr = NULL;
    
    if (*type == ltpExtensionPositioning) {
        SFUInt extensionOffset;
        
        *type = SFReadUShort(sTable, 2);
        extensionOffset = SFReadUInt(sTable, 4);

        return SFReadPositioning(&sTable[extensionOffset], type);
    }
    
    switch (*type) {
            
#ifdef GPOS_SINGLE
        case ltpSingleAdjustment:
        {
            SingleAdjustmentPosSubtable *subtable = malloc(sizeof(SingleAdjustmentPosSubtable));
            SFReadSingleAdjustment(sTable, subtable);
            subtablePtr = subtable;
            break;
        }
#endif
            
#ifdef GPOS_PAIR
        case ltpPairAdjustment:
        {
            PairAdjustmentPosSubtable *subtable = malloc(sizeof(PairAdjustmentPosSubtable));
            SFReadPairAdjustment(sTable, subtable);
            subtablePtr = subtable;
            break;
        }
#endif
            
#ifdef GPOS_CURSIVE
        case ltpCursiveAttachment:
        {
            CursiveAttachmentPosSubtable *subtable = malloc(sizeof(CursiveAttachmentPosSubtable));
            SFReadCursiveAttachment(sTable, subtable);
            subtablePtr = subtable;
            break;
        }
#endif
            
#ifdef GPOS_MARK_TO_BASE
        case ltpMarkToBaseAttachment:
        {
            MarkToBaseAttachmentPosSubtable *subtable = malloc(sizeof(MarkToBaseAttachmentPosSubtable));
            SFReadMarkToBaseAttachment(sTable, subtable);
            subtablePtr = subtable;
            break;
        }
#endif
            
#ifdef GPOS_MARK_TO_LIGATURE
        case ltpMarkToLigatureAttachment:
        {
            MarkToLigatureAttachmentPosSubtable *subtable = malloc(sizeof(MarkToLigatureAttachmentPosSubtable));
            SFReadMarkToLigatureAttachment(sTable, subtable);
            subtablePtr = subtable;
            break;
        }
#endif
            
#ifdef GPOS_MARK_TO_MARK
        case ltpMarkToMarkAttachment:
        {
            MarkToMarkAttachmentPosSubtable *subtable = malloc(sizeof(MarkToMarkAttachmentPosSubtable));
            SFReadMarkToMarkAttachment(sTable, subtable);
            subtablePtr = subtable;
            break;
        }
#endif
            
#ifdef GSUB_GPOS_CONTEXT
        case ltpContextPositioning:
        {
            ContextSubPosSubtable *subtable = malloc(sizeof(ContextSubPosSubtable));
            SFReadContextSubPos(sTable, subtable);
            subtablePtr = subtable;
            break;
        }
#endif
            
#ifdef GSUB_GPOS_CHAINING_CONTEXT
        case ltpChainedContextPositioning:
        {
            ChainingContextualSubPosSubtable *subtable = malloc(sizeof(ChainingContextualSubPosSubtable));
            SFReadChainingContextSubPos(sTable, subtable);
            subtablePtr = subtable;
            break;
        }
#endif
            
        default:
            //Reserved for future use
            break;
    }
    
    return subtablePtr;
}

static void SFFreePositioning(void *tablePtr, LookupType type) {
    switch (type) {
            
#ifdef GPOS_SINGLE
        case ltpSingleAdjustment:
            SFFreeSingleAdjustment(tablePtr);
            break;
#endif
            
#ifdef GPOS_PAIR
        case ltpPairAdjustment:
            SFFreePairAdjustment(tablePtr);
            break;
#endif
            
#ifdef GPOS_CURSIVE
        case ltpCursiveAttachment:
            SFFreeCursiveAttachment(tablePtr);
            break;
#endif
            
#ifdef GPOS_MARK_TO_BASE
        case ltpMarkToBaseAttachment:
            SFFreeMarkToBaseAttachment(tablePtr);
            break;
#endif
            
#ifdef GPOS_MARK_TO_LIGATURE
        case ltpMarkToLigatureAttachment:
            SFFreeMarkToLigatureAttachment(tablePtr);
            break;
#endif
            
#ifdef GPOS_MARK_TO_MARK
        case ltpMarkToMarkAttachment:
            SFFreeMarkToMarkAttachment(tablePtr);
            break;
#endif
            
#ifdef GSUB_GPOS_CONTEXT
        case ltpContextPositioning:
            SFFreeContextSubPos(tablePtr);
            break;
#endif
            
#ifdef GSUB_GPOS_CHAINING_CONTEXT
        case ltpChainedContextPositioning:
            SFFreeChainingContextSubPos(tablePtr);
            break;
#endif
            
        default:
            break;
    }
    
    free(tablePtr);
}


void SFReadGPOS(const SFUByte * const table, SFTableGPOS *tablePtr) {
    SFUShort scriptListOffset;
    SFUShort featureListOffset;
    SFUShort lookupListOffset;
    
    gposTable = (void **)&table;
    
    tablePtr->version = SFReadUInt(table, 0);
    
    scriptListOffset = SFReadUShort(table, 4);
    
#ifdef SCRIPT_TEST
    printf("\nGPOS Header:");
    printf("\n Header Version: %u", tablePtr->version);
    printf("\n Script List:");
    printf("\n  Offset: %d", scriptListOffset);
#endif
    
    SFReadScriptListTable(&table[scriptListOffset], &tablePtr->scriptList);
    
    featureListOffset = SFReadUShort(table, 6);
    
#ifdef FEATURE_TEST
    printf("\n Feature List:");
    printf("\n  Offset: %d", featureListOffset);
#endif
    
    SFReadFeatureListTable(&table[featureListOffset], &tablePtr->featureList);
    
    lookupListOffset = SFReadUShort(table, 8);
    
#ifdef LOOKUP_TEST
    printf("\n Lookup List:");
    printf("\n  Offset: %d", lookupListOffset);
#endif
    
    SFReadLookupListTable(&table[lookupListOffset], &tablePtr->lookupList, &SFReadPositioning);
}

void SFFreeGPOS(SFTableGPOS *tablePtr) {
    SFFreeScriptListTable(&tablePtr->scriptList);
    SFFreeFeatureListTable(&tablePtr->featureList);
    SFFreeLookupListTable(&tablePtr->lookupList, &SFFreePositioning);
}
