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
#include "string.h"

#include "ssunistr.h"
#include "SFTypes.h"
#include "SFInternal.h"
#include "SFCommonData.h"

const SFPositionRecord SFPositionRecordZero = {{0, 0}, {0, 0}, 0, {0, 0}};

void SFReadLangSysTable(const SFUByte * const lsTable, LangSysTable *tablePtr) {
    SFUShort *featureIndexes;
    SFUShort i;
    
#ifdef SCRIPT_TEST
    SFUShort lookupOrderOffset = SFReadUShort(lsTable, 0);
#endif
    
    tablePtr->reqFeatureIndex = SFReadUShort(lsTable, 2);
    tablePtr->featureCount = SFReadUShort(lsTable, 4);
    
#ifdef SCRIPT_TEST
    printf("\n     Lookup Order Offset: %d", lookupOrderOffset);
    printf("\n     Required Feature Index: %d", tablePtr->reqFeatureIndex);
    printf("\n     Total Features: %d", tablePtr->featureCount);
    printf("\n     Feature Indexes:");
#endif
    
    featureIndexes = malloc(2 * tablePtr->featureCount);
    
    for (i = 0; i < tablePtr->featureCount; i++) {
        featureIndexes[i] = SFReadUShort(lsTable, 6 + (i * 2));
        
#ifdef SCRIPT_TEST
        printf("\n      Index %d: %d", i, featureIndexes[i]);
#endif
    }
    
    tablePtr->featureIndex = featureIndexes;
}

void SFFreeLangSysTable(LangSysTable *tablePtr) {
    free(tablePtr->featureIndex);
}


void SFReadScriptTable(const SFUByte * const sTable, ScriptTable *tablePtr) {
    SFUShort langSysOffset = SFReadUShort(sTable, 0);
    
    SFUShort langSysCount;
    LangSysRecord *langSysRecords;
    
	SFUByte i;

#ifdef SCRIPT_TEST
    printf("\n    Default Lang Sys:");
    printf("\n     Offset: %d", langSysOffset);
#endif
    
    SFReadLangSysTable(&sTable[langSysOffset], &tablePtr->defaultLangSys);
    
    langSysCount = SFReadUShort(sTable, 2);
    tablePtr->langSysCount = langSysCount;
    
#ifdef SCRIPT_TEST
    printf("\n    Total Lang Sys: %d", langSysCount);
#endif
    
    langSysRecords = malloc(sizeof(LangSysRecord) * langSysCount);
    
    for (i = 0; i < langSysCount; i++) {
        SFUByte offset = 4 + (i * 4);
        
        langSysRecords[i].LangSysTag[0] = sTable[0 + offset];
        langSysRecords[i].LangSysTag[1] = sTable[1 + offset];
        langSysRecords[i].LangSysTag[2] = sTable[2 + offset];
        langSysRecords[i].LangSysTag[3] = sTable[3 + offset];
        langSysRecords[i].LangSysTag[4] = 0;
        
        langSysOffset = SFReadUShort(sTable, 4 + offset);
        
#ifdef SCRIPT_TEST
        printf("\n    Lang Sys Record At Index %d:", i);
        printf("\n     Lang Sys Tag: %s", langSysRecords[i].LangSysTag);
        printf("\n     Lang Sys:");
        printf("\n      Offset: %d", langSysOffset);
#endif
        
        SFReadLangSysTable(&sTable[langSysOffset], &langSysRecords[i].LangSys);
    }
    
    tablePtr->langSysRecord = langSysRecords;
}

void SFFreeScriptTable(ScriptTable *tablePtr) {
	int i;

    SFFreeLangSysTable(&tablePtr->defaultLangSys);
    
    for (i = 0; i < tablePtr->langSysCount; i++)
        SFFreeLangSysTable(&tablePtr->langSysRecord[i].LangSys);
    
    free(tablePtr->langSysRecord);
}


void SFReadScriptListTable(const SFUByte * const slTable, ScriptListTable *tablePtr) {
    ScriptRecord *scriptRecords;
    SFUShort scriptOffset;
    
	SFUShort i;

    tablePtr->scriptCount = SFReadUShort(slTable, 0);
    
#ifdef SCRIPT_TEST
    printf("\n  Total Scripts: %d", tablePtr->scriptCount);
#endif
    
    scriptRecords = malloc(sizeof(ScriptRecord) * tablePtr->scriptCount);
    
    for (i = 0; i < tablePtr->scriptCount; i++) {
        SFUShort offset = 2 + (i * 6);
        
        scriptRecords[i].scriptTag[0] = slTable[0 + offset];
        scriptRecords[i].scriptTag[1] = slTable[1 + offset];
        scriptRecords[i].scriptTag[2] = slTable[2 + offset];
        scriptRecords[i].scriptTag[3] = slTable[3 + offset];
        scriptRecords[i].scriptTag[4] = 0;
        
        scriptOffset = SFReadUShort(slTable, 4 + offset);
        
#ifdef SCRIPT_TEST
        printf("\n  Script Record At Index %d:", i);
        printf("\n   Script Tag: %s", scriptRecords[i].scriptTag);
        printf("\n   Script Table:");
        printf("\n    Offset: %d", scriptOffset);
#endif
        
        SFReadScriptTable(&slTable[scriptOffset], &scriptRecords[i].script);
    }
    
    tablePtr->scriptRecord = scriptRecords;
}

void SFFreeScriptListTable(ScriptListTable *tablePtr) {
	int i;
    for (i = 0; i < tablePtr->scriptCount; i++)
        SFFreeScriptTable(&tablePtr->scriptRecord[i].script);
    
    free(tablePtr->scriptRecord);
}



void SFReadFeatureTable(const SFUByte * const fTable, FeatureTable *tablePtr) {
#ifdef FEATURE_TEST
    SFUShort featureParams;
#endif
    
    SFUShort *lookupListIndexes;
    SFUShort i;
    
#ifdef FEATURE_TEST
    featureParams = SFReadUShort(fTable, 0);
#endif
    
    tablePtr->lookupCount = SFReadUShort(fTable, 2);
    
#ifdef FEATURE_TEST
    printf("\n    Feature Params: %d", featureParams);
    printf("\n    Total Lookups: %d", tablePtr->lookupCount);
    printf("\n    Lookup Indexes:");
#endif
    
    lookupListIndexes = malloc(sizeof(SFUShort) * tablePtr->lookupCount);
    
    for (i = 0; i < tablePtr->lookupCount; i++) {
        lookupListIndexes[i] = SFReadUShort(fTable, 4 + (i * 2));
        
#ifdef FEATURE_TEST
        printf("\n     Index %d: %d", i, lookupListIndexes[i]);
#endif
    }
    
    tablePtr->lookupListIndex = lookupListIndexes;
}

void SFFreeFeatureTable(FeatureTable *tablePtr) {
    free(tablePtr->lookupListIndex);
}


void SFReadFeatureListTable(const SFUByte * const flTable, FeatureListTable *tablePtr) {
    FeatureRecord *featureRecords;
    SFUShort i;
    
    tablePtr->featureCount = SFReadUShort(flTable, 0);
    
#ifdef FEATURE_TEST
    printf("\n  Total Feature: %d", tablePtr->featureCount);
#endif
    
    featureRecords = malloc(sizeof(FeatureRecord) * tablePtr->featureCount);
    
    for (i = 0; i < tablePtr->featureCount; i++) {
        SFUShort currentOffset;
        SFUShort featureOffset;
        
        currentOffset = 2 + (i * 6);
        
        featureRecords[i].featureTag[0] = flTable[0 + currentOffset];
        featureRecords[i].featureTag[1] = flTable[1 + currentOffset];
        featureRecords[i].featureTag[2] = flTable[2 + currentOffset];
        featureRecords[i].featureTag[3] = flTable[3 + currentOffset];
        featureRecords[i].featureTag[4] = 0;
        
        featureOffset = SFReadUShort(flTable, 4 + currentOffset);
        
#ifdef FEATURE_TEST
        printf("\n  Feature Record At Index %d:", i);
        printf("\n   Feature Tag: %s", featureRecords[i].featureTag);
        printf("\n   Feature Table:");
        printf("\n    Offset: %d", featureOffset);
#endif
        
        SFReadFeatureTable(&flTable[featureOffset], &featureRecords[i].feature);
    }
    
    tablePtr->featureRecord = featureRecords;
}

void SFFreeFeatureListTable(FeatureListTable *tablePtr) {
	int i;
    for (i = 0; i < tablePtr->featureCount; i++)
        SFFreeFeatureTable(&tablePtr->featureRecord[i].feature);
    
    free(tablePtr->featureRecord);
}

void SFReadLookupTable(const SFUByte * const llTable, LookupTable *tablePtr, SubtableFunction func) {
    LookupType lookupType;
    LookupFlag lookupFlag;
    SFUShort subTableCount;
    
    void **subtables;
    
    LookupType tmpLookup;
    SFUShort i, offset;
    
    lookupType = SFReadUShort(llTable, 0);
    lookupFlag = SFReadUShort(llTable, 2);
    subTableCount = SFReadUShort(llTable, 4);
    
    tablePtr->subTableCount = subTableCount;
    
#ifdef LOOKUP_TEST
    printf("\n    Lookup Type: %d", lookupType);
    printf("\n    Lookup Flag: %d", lookupFlag);
    printf("\n    Total Subtables: %d", subTableCount);
#endif
    
    subtables = malloc(sizeof(void *) * subTableCount);
    tmpLookup = lookupType;
    
    for (i = 0, offset = 0; i < subTableCount; i++) {
        offset = SFReadUShort(llTable, 6 + (i * 2));
        
#ifdef LOOKUP_TEST
        printf("\n    Subtable At Index %d:", i);
        printf("\n     Offset: %d", offset);
#endif
        
        tmpLookup = lookupType;
        subtables[i] = (*func)(&llTable[offset], &tmpLookup);
    }
    
    tablePtr->lookupType = tmpLookup;
    tablePtr->lookupFlag = lookupFlag;
    
    tablePtr->subtables = subtables;
    
    if ((lookupFlag >> 4) == 1)
        tablePtr->markFilteringSet = SFReadUShort(llTable, offset + 2);
    else
        tablePtr->markFilteringSet = 0;
    
#ifdef LOOKUP_TEST
    printf("\n    Mark Filtering Set: %d", tablePtr->markFilteringSet);
#endif
}

void SFFreeLookupTable(LookupTable *tablePtr, FreeSubtableFunction func) {
	int i;
    for (i = 0; i < tablePtr->subTableCount; i++)
        (*func)(tablePtr->subtables[i], tablePtr->lookupType);
    
    free(tablePtr->subtables);
}


void SFReadLookupListTable(const SFUByte * const llTable, LookupListTable *tablePtr, SubtableFunction func) {
    LookupTable *lookupTables;
    SFUShort i = 0;
    
    tablePtr->lookupCount = SFReadUShort(llTable, 0);
    
#ifdef LOOKUP_TEST
    printf("\n  Total Lookup Tables: %d", tablePtr->lookupCount);
#endif
    
    lookupTables = malloc(sizeof(LookupTable) * tablePtr->lookupCount);
    
    for (i = 0; i < tablePtr->lookupCount; i++) {
        SFUShort offset = SFReadUShort(llTable, 2 + (i * 2));
        
#ifdef LOOKUP_TEST
        printf("\n  Lookup Table At Index %d:", i);
        printf("\n   Offset: %d", offset);
#endif
        
        SFReadLookupTable(&llTable[offset], &lookupTables[i], func);
    }
    
    tablePtr->lookupTables = lookupTables;
}

void SFFreeLookupListTable(LookupListTable *tablePtr, FreeSubtableFunction func) {
	int i;
    for (i = 0; i < tablePtr->lookupCount; i++)
        SFFreeLookupTable(&tablePtr->lookupTables[i], func);
    
    free(tablePtr->lookupTables);
}

void SFReadClassDefTable(const SFUByte * const cdTable, ClassDefTable *tablePtr) {
    SFUShort format = SFReadUShort(cdTable, 0);
    tablePtr->classFormat = format;
    
#ifdef LOOKUP_TEST
    printf("\n         Class Definition Format: %d", format);
#endif
    
    switch (format) {
        case 1:
        {
            SFUShort glyphCount;
            SFUShort *classValueArray;
            
			SFUShort i;

            tablePtr->format.format1.startGlyph = SFReadUShort(cdTable, 2);
            
#ifdef LOOKUP_TEST
            printf("\n         Start Glyph: %d", tablePtr->format.format1.startGlyph);
#endif
            
            glyphCount = SFReadUShort(cdTable, 4);
            tablePtr->format.format1.glyphCount = glyphCount;
            
#ifdef LOOKUP_TEST
            printf("\n         Total Class Values: %d", glyphCount);
#endif
            
            classValueArray = malloc(sizeof(SFUShort) * glyphCount);
            for (i = 0; i < glyphCount; i++) {
                classValueArray[i] = SFReadUShort(cdTable, 6 + (i * 2));
                
#ifdef LOOKUP_TEST
                printf("\n         Class Value At Index %d: %d",i , glyphCount);
#endif
            }
            
            tablePtr->format.format1.classValueArray = classValueArray;
        }
            break;
        case 2:
        {
            SFUShort classRangeCount;
            ClassRangeRecord *classRangeRecords;
            
            SFUShort i;
            
            classRangeCount = SFReadUShort(cdTable, 2);
            tablePtr->format.format2.classRangeCount = classRangeCount;
            
#ifdef LOOKUP_TEST
            printf("\n         Total Class Ranges: %d", classRangeCount);
#endif
            
            classRangeRecords = malloc(sizeof(ClassRangeRecord) * classRangeCount);
            
            for (i = 0; i < classRangeCount; i++) {
                SFUShort offset = 4 + (i * 6);
                
                classRangeRecords[i].start = SFReadUShort(cdTable, offset);
                classRangeRecords[i].end = SFReadUShort(cdTable, offset + 2);
                classRangeRecords[i].cls = SFReadUShort(cdTable, offset + 4);
                
#ifdef LOOKUP_TEST
                printf("\n         Class At Index %d:", i);
                printf("\n          Start Glyph: %d", classRangeRecords[i].start);
                printf("\n          End Glyph: %d", classRangeRecords[i].end);
                printf("\n          Class: %d", classRangeRecords[i].cls);
#endif
            }
            
            tablePtr->format.format2.classRangeRecord = classRangeRecords;
        }
            break;
    }
}

void SFFreeClassDefTable(ClassDefTable *tablePtr) {
    switch (tablePtr->classFormat) {
        case 1:
            free(tablePtr->format.format1.classValueArray);
            break;
            
        case 2:
            free(tablePtr->format.format2.classRangeRecord);
            break;
    }
}


void SFReadCoverageTable(const SFUByte * const cTable, CoverageTable *tablePtr) {
    SFUShort coverageFormat;
    SFUShort count;
    
    coverageFormat = SFReadUShort(cTable, 0);
    count = SFReadUShort(cTable, 2);
    
    tablePtr->coverageFormat = coverageFormat;
    
#ifdef LOOKUP_TEST
    printf("\n        Coverage Format: %d", coverageFormat);
#endif
    
    switch (coverageFormat) {
        case 1:
        {
            SFGlyph *glyphs;
            SFUShort glyphIndex;
            
            tablePtr->format.format1.glyphCount = count;
            
#ifdef LOOKUP_TEST
            printf("\n        Total Glyphs: %d", count);
#endif
            
            glyphs = malloc(2 * count);
            for (glyphIndex = 0; glyphIndex < count; glyphIndex++) {
                glyphs[glyphIndex] = SFReadUShort(cTable, 4 + (glyphIndex * 2));
                
#ifdef LOOKUP_TEST
                printf("\n        Glyph At Index %d: %d", glyphIndex, glyphs[glyphIndex]);
#endif
            }
            
            tablePtr->format.format1.glyphArray = glyphs;
        }
            break;
            
        case 2:
        {
            RangeRecord *rangeRecords;
            SFUShort recordIndex;
            
            tablePtr->format.format2.rangeCount = count;
            
#ifdef LOOKUP_TEST
            printf("\n        Total Ranges: %d", count);
#endif
            
            rangeRecords = malloc(sizeof(RangeRecord) * count);
            
            for (recordIndex = 0; recordIndex < count; recordIndex++) {
                SFUShort offset = 4 + (recordIndex * 6);
                
                rangeRecords[recordIndex].start = SFReadUShort(cTable, 0 + offset);
                rangeRecords[recordIndex].end = SFReadUShort(cTable, 2 + offset);
                rangeRecords[recordIndex].startCoverageIndex = SFReadUShort(cTable, 4 + offset);
                
#ifdef LOOKUP_TEST
                printf("\n         Range Record At Index %d:", recordIndex);
                printf("\n          Start: %d", rangeRecords[recordIndex].start);
                printf("\n          End: %d", rangeRecords[recordIndex].end);
                printf("\n          Start Coverage Index: %d", rangeRecords[recordIndex].startCoverageIndex);
#endif
            }
            
            tablePtr->format.format2.rangeRecord = rangeRecords;
        }
            break;
    }
}

void SFFreeCoverageTable(CoverageTable *tablePtr) {
    switch (tablePtr->coverageFormat) {
        case 1:
            free(tablePtr->format.format1.glyphArray);
            break;
            
        case 2:
            free(tablePtr->format.format2.rangeRecord);
            break;
    }
}


void SFReadDeviceTable(const SFUByte * const dTable, DeviceTable *tablePtr) {
    tablePtr->startSize = SFReadUShort(dTable, 0);
    tablePtr->endSize = SFReadUShort(dTable, 2);
    tablePtr->deltaFormat = SFReadUShort(dTable, 4);
    tablePtr->deltaValue = SFReadUShort(dTable, 6);
    
#ifdef LOOKUP_TEST
    printf("\n          Start Size: %d", tablePtr->startSize);
    printf("\n          End Size: %d", tablePtr->endSize);
    printf("\n          Delta Format: %d", tablePtr->deltaFormat);
    printf("\n          Delta Value: %d", tablePtr->deltaValue);
#endif
}


void SFAllocateStringRecord(SFStringRecord *record, SFUnichar *charsPtr, int *levelsPtr, int *lOrderPtr, int len) {
    SFCharRecord *charRecord;
    int recordIndex;
    
    record->charCount = len;
    record->glyphCount = len;
    
    charRecord = malloc(sizeof(SFCharRecord) * len);
    
    for (recordIndex = 0; recordIndex < len; recordIndex++) {
        charRecord[recordIndex].glyphCount = 1;
        charRecord[recordIndex].gRec = malloc(sizeof(SFGlyphRecord));
        charRecord[recordIndex].gRec[0].glyphProp = gpNotReceived;
        charRecord[recordIndex].gRec[0].posRec = SFPositionRecordZero;
    }
    
    record->chars = charsPtr;
    record->levels = levelsPtr;
    record->lOrder = lOrderPtr;
    
    record->charRecord = charRecord;
}

void SFFreeStringRecord(SFStringRecord *record) {
	int i;
    for (i = 0; i < record->charCount; i++)
        free(record->charRecord[i].gRec);
    
    free(record->chars);
    free(record->levels);
    free(record->lOrder);
    free(record->charRecord);
}

int SFGetIndexOfGlyphInCoverage(CoverageTable *tablePtr, SFGlyph glyph) {
    int index = UNDEFINED_INDEX;
	int j;
    
    if (tablePtr->coverageFormat == 1) {
        for (j = 0; j < tablePtr->format.format1.glyphCount; j++) {
            SFGlyph inputGlyph = tablePtr->format.format1.glyphArray[j];
            
            if (inputGlyph == glyph) {
                index = j;
                break;
            }
        }
    } else if (tablePtr->coverageFormat == 2) {
        for (j = 0; j < tablePtr->format.format2.rangeCount; j++) {
            RangeRecord record = tablePtr->format.format2.rangeRecord[j];
            
            if (glyph >= record.start && glyph <= record.end) {
                index = record.startCoverageIndex + glyph - record.start;
                break;
            }
        }
    }
    
    return index;
}
