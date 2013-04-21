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

#ifdef GSUB_GPOS_CONTEXT

void SFReadContextSubPos(const SFUByte * const csTable, ContextSubPosSubtable *tablePtr) {
    SFUShort subPosFormat = SFReadUShort(csTable, 0);
    tablePtr->subPosFormat = subPosFormat;
    
#ifdef LOOKUP_TEST
    printf("\n      Context SubPos Table:");
    printf("\n       SubPos Format: %d", subPosFormat);
#endif
    
    switch (subPosFormat) {
            
#ifdef GSUB_GPOS_CONTEXT_FORMAT1
        case 1:
        {
            SFUShort coverageOffset;
            SFUShort subPosRuleSetCount;
            SubPosRuleSetTable *subPosRuleSetTables;
            
            SFUShort i;
            
            coverageOffset = SFReadUShort(csTable, 2);
            
#ifdef LOOKUP_TEST
            printf("\n       Coverage Table:");
            printf("\n        Offset: %d", coverageOffset);
#endif
            
            SFReadCoverageTable(&csTable[coverageOffset], &tablePtr->format.format1.coverage);
            
            subPosRuleSetCount = SFReadUShort(csTable, 4);
            tablePtr->format.format1.subPosRuleSetCount = subPosRuleSetCount;
            
#ifdef LOOKUP_TEST
            printf("\n       Total SubPos Rule Sets: %d", subPosRuleSetCount);
#endif
            
            subPosRuleSetTables = malloc(sizeof(SubPosRuleSetTable) * subPosRuleSetCount);
            
            for (i = 0; i < subPosRuleSetCount; i++) {
                SFUShort subPosRuleSetOffset;
                const SFUByte *srsTable;
                
                SFUShort subPosRuleCount;
                SubPosRuleTable *subPosRuleTables;
                SFUShort j;
                
                subPosRuleSetOffset = SFReadUShort(csTable, 6 + (i * 2));
                srsTable = &csTable[subPosRuleSetOffset];
                
#ifdef LOOKUP_TEST
                printf("\n       SubPos Rule Set %d:", i + 1);
                printf("\n        Offset: %d", subPosRuleSetOffset);
#endif
                
                subPosRuleCount = SFReadUShort(srsTable, 0);
                subPosRuleSetTables[i].subPosRuleCount = subPosRuleCount;
                
#ifdef LOOKUP_TEST
                printf("\n        Total SubPos Rules: %d", subPosRuleCount);
#endif
                
                subPosRuleTables = malloc(sizeof(SubPosRuleTable) * subPosRuleCount);
                
                for (j = 0; j < subPosRuleCount; j++) {
                    SFUShort subPosRuleOffset;
                    const SFUByte *srTable;
                    
                    SFUShort glyphCount;
                    SFUShort subPosCount;
                    
                    SFGlyph *glyphs;
                    SFUShort k, l;
                    
                    SubPosLookupRecord *subPosLookupRecords;
                    
                    subPosRuleOffset = SFReadUShort(srsTable, 2 + (j * 2));
                    srTable = &srsTable[subPosRuleOffset];
                    
#ifdef LOOKUP_TEST
                    printf("\n        SubPos Rule %d:", j + 1);
                    printf("\n         Offset: %d", subPosRuleOffset);
#endif
                    
                    glyphCount = SFReadUShort(srTable, 0);
                    subPosRuleTables[j].glyphCount = glyphCount;
                    
                    subPosCount = SFReadUShort(srTable, 2);
                    subPosRuleTables[j].subPosCount = subPosCount;
                    
#ifdef LOOKUP_TEST
                    printf("\n         Total Input Glyphs: %d", glyphCount);
                    printf("\n         Total Lookup Records: %d", subPosCount);
#endif
                    
                    glyphs = malloc(sizeof(SFGlyph) * glyphCount);
                    glyphs[0] = 0;
                    
                    for (k = 0; k < glyphCount; k++) {
                        glyphs[k + 1] = SFReadUShort(srTable, 4 + (k * 2));
                        
#ifdef LOOKUP_TEST
                        printf("\n         Input Glyph At Index %d: %d", k, glyphs[k + 1]);
#endif
                    }
                    
                    subPosRuleTables[j].input = glyphs;
                    
                    k += 4;
                    
                    subPosLookupRecords = malloc(sizeof(SubPosLookupRecord) * subPosCount);
                    for (l = 0; l < subPosCount; l++) {
                        SFUShort beginOffset = k + (l * 4);
                        
                        subPosLookupRecords[l].sequenceIndex = SFReadUShort(srTable, beginOffset);
                        subPosLookupRecords[l].lookupListIndex = SFReadUShort(srTable, beginOffset + 2);
                        
#ifdef LOOKUP_TEST
                        printf("\n         Lookup Record At Index %d:", l);
                        printf("\n          Sequence Index: %d", subPosLookupRecords[l].sequenceIndex);
                        printf("\n          Lookup List Index: %d", subPosLookupRecords[l].lookupListIndex);
#endif
                    }
                    
                    subPosRuleTables[j].subPosLookupRecord = subPosLookupRecords;
                }
                
                subPosRuleSetTables[i].subPosRule = subPosRuleTables;
            }
            
            tablePtr->format.format1.subPosRuleSet = subPosRuleSetTables;
        }
            break;
#endif
            
#ifdef GSUB_GPOS_CONTEXT_FORMAT2
        case 2:
        {
            SFUShort coverageOffset;
            SFUShort classDefOffset;
            SFUShort subPosClassSetCount;
            SubPosClassSetSubtable *subPosRuleSetTables;
            
            SFUShort i;
            
            coverageOffset = SFReadUShort(csTable, 2);
            
#ifdef LOOKUP_TEST
            printf("\n       Coverage Table:");
            printf("\n        Offset: %d", coverageOffset);
#endif
            
            SFReadCoverageTable(&csTable[coverageOffset], &tablePtr->format.format2.coverage);
            
            classDefOffset = SFReadUShort(csTable, 4);
            
#ifdef LOOKUP_TEST
            printf("\n       Class Definition Table:");
            printf("\n        Offset: %d", classDefOffset);
#endif
            
            SFReadClassDefTable(&csTable[classDefOffset], &tablePtr->format.format2.classDef);
            
            subPosClassSetCount = SFReadUShort(csTable, 6);
            
#ifdef LOOKUP_TEST
            printf("\n       Total SubPos Class Sets: %d", subPosClassSetCount);
#endif
            
            subPosRuleSetTables = malloc(sizeof(SubPosClassSetSubtable) * subPosClassSetCount);
            
            for (i = 0; i < subPosClassSetCount; i++) {
                SFUShort subPosClassSetOffset;
                const SFUByte *scsTable;
                
                SFUShort subPosClassRuleCount;
                SubPosClassRuleTable *subPosRuleTables;
                
                SubPosLookupRecord *subPosLookupRecords;
                
                SFUShort j, l;
                
                subPosClassSetOffset = SFReadUShort(csTable, 8 + (i * 2));
                scsTable = &csTable[subPosClassSetOffset];
                
#ifdef LOOKUP_TEST
                printf("\n       SubPos Class Set %d:", i + 1);
                printf("\n        Offset: %d", subPosClassSetOffset);
#endif
                
                subPosClassRuleCount = SFReadUShort(scsTable, 0);
                subPosRuleSetTables[i].subPosClassRuleCount = subPosClassRuleCount;
                
#ifdef LOOKUP_TEST
                printf("\n        Total SubPos Class Rules: %d", subPosClassRuleCount);
#endif
                
                subPosRuleTables = malloc(sizeof(SubPosClassRuleTable) * subPosClassRuleCount);
                
                for (j = 0; j < subPosClassRuleCount; j++) {
                    SFUShort subPosClassRuleOffset;
                    const SFUByte *scrTable;
                    
                    SFUShort glyphCount;
                    SFUShort subPosCount;
                    
                    SFUShort *classes;
                    SFUShort k;
                    
                    subPosClassRuleOffset = SFReadUShort(scsTable, 2 + (j * 2));
                    scrTable = &scsTable[subPosClassRuleOffset];
                    
#ifdef LOOKUP_TEST
                    printf("\n        SubPos Class Rule %d:", j + 1);
                    printf("\n         Offset: %d", subPosClassRuleOffset);
#endif
                    
                    glyphCount = SFReadUShort(scrTable, 0);
                    subPosRuleTables[j].glyphCount = glyphCount;
                    
                    subPosCount = SFReadUShort(scrTable, 2);
                    subPosRuleTables[j].subPosCount = subPosCount;
                    
#ifdef LOOKUP_TEST
                    printf("\n         Total Input Glyphs: %d", glyphCount);
                    printf("\n         Total Lookup Records: %d", subPosCount);
#endif
                    
                    classes = malloc(sizeof(SFUShort) * glyphCount);
                    classes[0] = 0;
                    
                    for (k = 0; k < glyphCount; k++) {
                        classes[k + 1] = SFReadUShort(scrTable, 4 + (k * 2));
                        
#ifdef LOOKUP_TEST
                        printf("\n         Class At Index %d: %d", k + 1, classes[k + 1]);
#endif
                    }
                    
                    subPosRuleTables[j].cls = classes;
                    
                    k += 4;
                    
                    subPosLookupRecords = malloc(sizeof(SubPosLookupRecord) * subPosCount);
                    for (l = 0; l < subPosCount; l++) {
                        SFUShort beginOffset = k + (l * 4);
                        
                        subPosLookupRecords[l].sequenceIndex = SFReadUShort(scrTable, beginOffset);
                        subPosLookupRecords[l].lookupListIndex = SFReadUShort(scrTable, beginOffset + 2);
                        
#ifdef LOOKUP_TEST
                        printf("\n         Lookup Record At Index %d:", l);
                        printf("\n          Sequence Index: %d", subPosLookupRecords[l].sequenceIndex);
                        printf("\n          Lookup List Index: %d", subPosLookupRecords[l].lookupListIndex);
#endif
                    }
                    
                    subPosRuleTables[j].subPosLookupRecord = subPosLookupRecords;
                }
                
                subPosRuleSetTables[i].subPosClassRule = subPosRuleTables;
            }
            
            tablePtr->format.format2.subPosClassSet = subPosRuleSetTables;
        }
            break;
#endif
            
#ifdef GSUB_GPOS_CONTEXT_FORMAT3
        case 3:
        {
            SFUShort glyphCount;
            SFUShort subPosCount;
            
            SubPosLookupRecord *subPosLookupRecords;
            
            CoverageTable *coverageTables;
            SFUShort i, j;
            
            glyphCount = SFReadUShort(csTable, 2);
            subPosCount = SFReadUShort(csTable, 4);
            
#ifdef LOOKUP_TEST
            printf("\n       Total Coverage Tables: %d", glyphCount);
            printf("\n       Total Lookup Records: %d", subPosCount);
#endif
            
            tablePtr->format.format3.glyphCount = glyphCount;
            tablePtr->format.format3.subPosCount = subPosCount;
            
            coverageTables = malloc(sizeof(CoverageTable) * glyphCount);
            
            for (i = 0; i < glyphCount; i++) {
                SFUShort coverageOffset = SFReadUShort(csTable, 6 + (i * 2));
                
#ifdef LOOKUP_TEST
                printf("\n       Coverage Table %d:", i + 1);
                printf("\n        Offset: %d", coverageOffset);
#endif
                
                SFReadCoverageTable(&csTable[coverageOffset], &coverageTables[i]);
            }
            
            tablePtr->format.format3.coverage = coverageTables;
            
            subPosLookupRecords = malloc(sizeof(SubPosLookupRecord) * subPosCount);
            for (j = 0; j < subPosCount; j++) {
                SFUShort beginOffset = 8 + (j * 4);
                
                subPosLookupRecords[j].sequenceIndex = SFReadUShort(csTable, beginOffset);
                subPosLookupRecords[j].lookupListIndex = SFReadUShort(csTable, beginOffset + 2);
                
#ifdef LOOKUP_TEST
                printf("\n       Lookup Record At Index %d:", j);
                printf("\n        Sequence Index: %d", subPosLookupRecords[j].sequenceIndex);
                printf("\n        Lookup List Index: %d", subPosLookupRecords[j].lookupListIndex);
#endif
            }
            
            tablePtr->format.format3.subPosLookupRecord = subPosLookupRecords;
        }
            break;
#endif
    }
}

void SFFreeContextSubPos(ContextSubPosSubtable *tablePtr) {
	int i;

    switch (tablePtr->subPosFormat) {
#ifdef GSUB_GPOS_CONTEXT_FORMAT1
        case 1:
        {
            SFFreeCoverageTable(&tablePtr->format.format1.coverage);
            
            for (i = 0; i < tablePtr->format.format1.subPosRuleSetCount; i++) {
                for (int j = 0; j < tablePtr->format.format1.subPosRuleSet[i].subPosRuleCount; j++) {
                    free(tablePtr->format.format1.subPosRuleSet[i].subPosRule[j].input);
                    free(tablePtr->format.format1.subPosRuleSet[i].subPosRule[j].subPosLookupRecord);
                }
                
                free(tablePtr->format.format1.subPosRuleSet[i].subPosRule);
            }
            
            free(tablePtr->format.format1.subPosRuleSet);
        }
            break;
#endif
            
#ifdef GSUB_GPOS_CONTEXT_FORMAT2
        case 2:
        {
            SFFreeCoverageTable(&tablePtr->format.format2.coverage);
            
            for (i = 0; i < tablePtr->format.format2.subPosClassSetCount; i++) {
                for (int j = 0; j < tablePtr->format.format2.subPosClassSet[i].subPosClassRuleCount; j++) {
                    free(tablePtr->format.format2.subPosClassSet[i].subPosClassRule[j].cls);
                    free(tablePtr->format.format2.subPosClassSet[i].subPosClassRule[j].subPosLookupRecord);
                }
                
                free(tablePtr->format.format2.subPosClassSet[i].subPosClassRule);
            }
            
            SFFreeClassDefTable(&tablePtr->format.format2.classDef);
            free(tablePtr->format.format2.subPosClassSet);
        }
            break;
#endif
            
#ifdef GSUB_GPOS_CONTEXT_FORMAT3
        case 3:
        {
            for (i = 0; i < tablePtr->format.format3.glyphCount; i++)
                SFFreeCoverageTable(&tablePtr->format.format3.coverage[i]);
            
            free(tablePtr->format.format3.coverage);
            free(tablePtr->format.format3.subPosLookupRecord);
        }
            break;
#endif
    }
}

#endif


#ifdef GSUB_GPOS_CHAINING_CONTEXT

void SFReadChainingContextSubPos(const SFUByte * const ccsTable, ChainingContextualSubPosSubtable *tablePtr) {
    SFUShort subPosFormat = SFReadUShort(ccsTable, 0);
    tablePtr->subPosFormat = subPosFormat;
    
#ifdef LOOKUP_TEST
    printf("\n      Chaining Context SubPos Table:");
    printf("\n       SubPos Format: %d", subPosFormat);
#endif
    
    switch (subPosFormat) {
            
#ifdef GSUB_GPOS_CHAINING_CONTEXT_FORMAT1
        case 1:
        {
            SFUShort coverageOffset;
            SFUShort chainSubPosRuleSetCount;
            
            ChainSubPosRuleSetTable *chainSubPosRuleSetTables;
            
            SFUShort i;
            
            coverageOffset = SFReadUShort(ccsTable, 2);
            
#ifdef LOOKUP_TEST
            printf("\n       Coverage Table:");
            printf("\n        Offset: %d", coverageOffset);
#endif
            SFReadCoverageTable(&ccsTable[coverageOffset], &tablePtr->format.format1.coverage);
            
            chainSubPosRuleSetCount = SFReadUShort(ccsTable, 4);
            
#ifdef LOOKUP_TEST
            printf("\n       Total Chain Sub Rule Sets: %d", chainSubPosRuleSetCount);
#endif
            tablePtr->format.format1.chainSubPosRuleSetCount = chainSubPosRuleSetCount;
            
            chainSubPosRuleSetTables = malloc(sizeof(ChainSubPosRuleSetTable) * chainSubPosRuleSetCount);
            
            for (i = 0; i < chainSubPosRuleSetCount; i++) {
                SFUShort chainSubPosRuleSetOffset;
                const SFUByte *csrsTable;
                
                SFUShort chainSubPosRuleCount;
                ChainSubPosRuleSubtable *chainSubPosRuleTables;
                
                SFUShort j;
                
                chainSubPosRuleSetOffset = SFReadUShort(ccsTable, 6 + (i * 2));
                csrsTable = &ccsTable[chainSubPosRuleSetOffset];
                
#ifdef LOOKUP_TEST
                printf("\n       Chain Sub Rule Set %d:", i + 1);
                printf("\n        Offset: %d", chainSubPosRuleSetOffset);
#endif
                
                chainSubPosRuleCount = SFReadUShort(csrsTable, 0);
                chainSubPosRuleSetTables[i].chainSubPosRuleCount = chainSubPosRuleCount;
                
#ifdef LOOKUP_TEST
                printf("\n        Total Chain Sub Rule Tables: %d", chainSubPosRuleCount);
#endif
                
                chainSubPosRuleTables = malloc(sizeof(ChainSubPosRuleSubtable) * chainSubPosRuleCount);
                
                for (j = 0; j < chainSubPosRuleCount; j++) {
                    SFUShort chainSubPosRuleOffset;
                    const SFUByte *csrTable;
                    
                    SFUShort backtrackGlyphCount;
                    SFGlyph *backtrackGlyphs;
                    
                    SFUShort inputGlyphCount;
                    SFGlyph *inputGlyphs;
                    
                    SFUShort lookaheadGlyphCount;
                    SFGlyph *lookaheadGlyphs;
                    
                    SFUShort subPosCount;
                    SubPosLookupRecord *subPosLookupRecords;
                    
                    SFUShort k, l, m, n;
                    
                    chainSubPosRuleOffset = SFReadUShort(csrsTable, 2 + (j * 2));
                    csrTable = &csrsTable[chainSubPosRuleOffset];
                    
#ifdef LOOKUP_TEST
                    printf("\n        Chain Sub Rule Table %d:", j + 1);
                    printf("\n         Offset: %d", chainSubPosRuleOffset);
#endif
                    
                    backtrackGlyphCount = SFReadUShort(csrTable, 0);
                    chainSubPosRuleTables[j].backtrackGlyphCount = backtrackGlyphCount;
                    
#ifdef LOOKUP_TEST
                    printf("\n         Total Backtrack Glyphs: %d", backtrackGlyphCount);
#endif
                    
                    backtrackGlyphs = malloc(sizeof(SFGlyph) * backtrackGlyphCount);
                    
                    for (k = 0; k < backtrackGlyphCount; k++) {
                        backtrackGlyphs[k] = SFReadUShort(csrTable, 2 + (k * 2));
                        
#ifdef LOOKUP_TEST
                        printf("\n         Backtrack Glyph At Index %d: %d", k, backtrackGlyphs[k]);
#endif
                    }
                    k *= 2;
                    
                    chainSubPosRuleTables[j].backtrack = backtrackGlyphs;
                    
                    inputGlyphCount = SFReadUShort(csrTable, k);
                    chainSubPosRuleTables[j].inputGlyphCount = inputGlyphCount;
                    
#ifdef LOOKUP_TEST
                    printf("\n         Total Input Glyphs: %d", inputGlyphCount);
#endif
                    
                    inputGlyphs = malloc(sizeof(SFGlyph) * inputGlyphCount);
                    inputGlyphs[0] = 0;
                    
                    k += 2;
                    for (l = 0; l < inputGlyphCount - 1; l++) {
                        inputGlyphs[l + 1] = SFReadUShort(csrTable, k + (l * 2));
                        
#ifdef LOOKUP_TEST
                        printf("\n         Input Glyph At Index %d: %d", l + 1, inputGlyphs[l + 1]);
#endif
                    }
                    k += l * 2;
                    
                    chainSubPosRuleTables[j].input = inputGlyphs;
                    
                    lookaheadGlyphCount = SFReadUShort(csrTable, k);
                    chainSubPosRuleTables[j].lookaheadGlyphCount = lookaheadGlyphCount;
                    
#ifdef LOOKUP_TEST
                    printf("\n         Total Lookahead Glyphs: %d", lookaheadGlyphCount);
#endif
                    
                    lookaheadGlyphs = malloc(sizeof(SFGlyph) * lookaheadGlyphCount);
                    
                    k += 2;
                    for (m = 0; m < lookaheadGlyphCount; m++) {
                        lookaheadGlyphs[m] = SFReadUShort(csrTable, k + (m * 2));
                        
#ifdef LOOKUP_TEST
                        printf("\n         Lookahead Glyph At Index %d: %d", m, inputGlyphs[m]);
#endif
                    }
                    k += m * 2;
                    
                    chainSubPosRuleTables[j].lookAhead = lookaheadGlyphs;
                    
                    subPosCount = SFReadUShort(csrTable, k);
                    
#ifdef LOOKUP_TEST
                    printf("\n         Total SubPositute Lookup Records: %d", subPosCount);
#endif
                    
                    subPosLookupRecords = malloc(sizeof(SubPosLookupRecord) * subPosCount);
                    
                    for (n = 0; n < subPosCount; l++) {
                        SFUShort beginOffset = k + (n * 4);
                        
                        subPosLookupRecords[n].sequenceIndex = SFReadUShort(csrTable, beginOffset);
                        subPosLookupRecords[n].lookupListIndex = SFReadUShort(csrTable, beginOffset + 2);
                        
#ifdef LOOKUP_TEST
                        printf("\n         Lookup Record At Index %d:", n);
                        printf("\n          Sequence Index: %d", subPosLookupRecords[n].sequenceIndex);
                        printf("\n          Lookup List Index: %d", subPosLookupRecords[n].lookupListIndex);
#endif
                    }
                    
                    chainSubPosRuleTables[j].subPosLookupRecord = subPosLookupRecords;
                }
                
                chainSubPosRuleSetTables[i].chainSubPosRule = chainSubPosRuleTables;
            }
            
            tablePtr->format.format1.chainSubPosRuleSet = chainSubPosRuleSetTables;
        }
            break;
#endif
            
#ifdef GSUB_GPOS_CHAINING_CONTEXT_FORMAT2
        case 2:
        {
            SFUShort coverageOffset;
            SFUShort backtrackClassDefOffset;
            SFUShort inputClassDefOffset;
            SFUShort lookaheadClassDefOffset;
            
            SFUShort chainSubPosClassSetCount;
            ChainSubPosClassSetSubtable *chainSubPosClassSets;
            
            SFUShort i;
            
            coverageOffset = SFReadUShort(ccsTable, 2);
            
#ifdef LOOKUP_TEST
            printf("\n       Coverage Table:");
            printf("\n        Offset: %d", coverageOffset);
#endif
            
            SFReadCoverageTable(&ccsTable[coverageOffset], &tablePtr->format.format2.coverage);
            
            backtrackClassDefOffset = SFReadUShort(ccsTable, 4);
            
#ifdef LOOKUP_TEST
            printf("\n       Backtrack Class Definition:");
            printf("\n        Offset: %d", backtrackClassDefOffset);
#endif
            
            SFReadClassDefTable(&ccsTable[backtrackClassDefOffset], &tablePtr->format.format2.backtrackClassDef);
            
            inputClassDefOffset = SFReadUShort(ccsTable, 6);
            
#ifdef LOOKUP_TEST
            printf("\n       Input Class Definition:");
            printf("\n        Offset: %d", inputClassDefOffset);
#endif
            
            SFReadClassDefTable(&ccsTable[inputClassDefOffset], &tablePtr->format.format2.inputClassDef);
            
            lookaheadClassDefOffset = SFReadUShort(ccsTable, 8);
            
#ifdef LOOKUP_TEST
            printf("\n       Lookahead Class Definition:");
            printf("\n        Offset: %d", lookaheadClassDefOffset);
#endif
            
            SFReadClassDefTable(&ccsTable[lookaheadClassDefOffset], &tablePtr->format.format2.lookaheadClassDef);
            
            chainSubPosClassSetCount = SFReadUShort(ccsTable, 10);
            
#ifdef LOOKUP_TEST
            printf("\n       Total Chain Sub Class Sets: %d", chainSubPosClassSetCount);
#endif
            
            chainSubPosClassSets = malloc(sizeof(ChainSubPosClassSetSubtable) * chainSubPosClassSetCount);
            
            for (i = 0; i < chainSubPosClassSetCount; i++) {
                SFUShort chainSubPosClassSetOffset;
                const SFUByte *cscsTable;
                
                SFUShort chainSubPosClassRuleCount;
                ChainSubPosClassRuleTable *chainSubPosClassRuleTables;
                
                SFUShort j;
                
                chainSubPosClassSetOffset = SFReadUShort(ccsTable, 12 + (i * 2));
                cscsTable = &ccsTable[chainSubPosClassSetOffset];
                
#ifdef LOOKUP_TEST
                printf("\n       Chain Sub Class Set %d:", i + 1);
                printf("\n        Offset: %d", chainSubPosClassSetOffset);
#endif
                
                chainSubPosClassRuleCount = SFReadUShort(cscsTable, 0);
                chainSubPosClassSets[i].chainSubPosClassRuleCount = chainSubPosClassRuleCount;
                
#ifdef LOOKUP_TEST
                printf("\n        Total Chain Sub Class Rules: %d", chainSubPosClassRuleCount);
#endif
                
                chainSubPosClassRuleTables = malloc(sizeof(ChainSubPosClassRuleTable) * chainSubPosClassRuleCount);
                
                for (j = 0; j < chainSubPosClassRuleCount; j++) {
                    SFUShort chainSubPosClassRuleOffset;
                    const SFUByte *cscrTable;
                    
                    SFUShort backtrackGlyphCount;
                    SFGlyph *backtrackGlyphs;
                    
                    SFUShort inputGlyphCount;
                    SFGlyph *inputGlyphs;
                    
                    SFUShort lookaheadGlyphCount;
                    SFGlyph *lookaheadGlyphs;
                    
                    SFUShort subPosCount;
                    SubPosLookupRecord *subPosLookupRecords;
                    
                    SFUShort k, l, m, n;
                    
                    chainSubPosClassRuleOffset = SFReadUShort(cscsTable, 2 + (j * 2));
                    cscrTable = &cscsTable[chainSubPosClassRuleOffset];
                    
#ifdef LOOKUP_TEST
                    printf("\n        Chain Sub Class Rule %d:", j + 1);
                    printf("\n         Offset: %d", chainSubPosClassRuleOffset);
#endif
                    
                    backtrackGlyphCount = SFReadUShort(cscrTable, 0);
                    chainSubPosClassRuleTables[j].backtrackGlyphCount = backtrackGlyphCount;
                    
#ifdef LOOKUP_TEST
                    printf("\n         Total Backtrack Glyphs: %d", backtrackGlyphCount);
#endif
                    
                    backtrackGlyphs = malloc(sizeof(SFGlyph) * backtrackGlyphCount);
                    
                    for (k = 0; k < backtrackGlyphCount; k++) {
                        backtrackGlyphs[k] = SFReadUShort(cscrTable, 2 + (k * 2));
                        
#ifdef LOOKUP_TEST
                        printf("\n         Backtrack Glyph At Index %d: %d", k, backtrackGlyphs[k]);
#endif
                    }
                    k *= 2;
                    
                    chainSubPosClassRuleTables[j].backtrack = backtrackGlyphs;
                    
                    inputGlyphCount = SFReadUShort(cscrTable, k);
                    chainSubPosClassRuleTables[j].inputGlyphCount = inputGlyphCount;
                    
#ifdef LOOKUP_TEST
                    printf("\n         Total Input Glyphs: %d", inputGlyphCount);
#endif
                    
                    inputGlyphs = malloc(sizeof(SFGlyph) * inputGlyphCount);
                    inputGlyphs[0] = 0;
                    
                    k += 2;
                    for (l = 0; l < inputGlyphCount - 1; l++) {
                        inputGlyphs[l + 1] = SFReadUShort(cscrTable, k + (l * 2));
                        
#ifdef LOOKUP_TEST
                        printf("\n         Input Glyph At Index %d: %d", l + 1, inputGlyphs[l + 1]);
#endif
                    }
                    k += l * 2;
                    
                    chainSubPosClassRuleTables[j].input = inputGlyphs;
                    
                    lookaheadGlyphCount = SFReadUShort(cscrTable, k);
                    chainSubPosClassRuleTables[j].lookaheadGlyphCount = lookaheadGlyphCount;
                    
#ifdef LOOKUP_TEST
                    printf("\n         Total Lookahead Glyphs: %d", lookaheadGlyphCount);
#endif
                    
                    lookaheadGlyphs = malloc(sizeof(SFGlyph) * lookaheadGlyphCount);
                    
                    k += 2;
                    for (m = 0; m < lookaheadGlyphCount; m++) {
                        lookaheadGlyphs[m] = SFReadUShort(cscrTable, k + (m * 2));
                        
#ifdef LOOKUP_TEST
                        printf("\n         Lookahead Glyph At Index %d: %d", m, lookaheadGlyphs[m]);
#endif
                    }
                    k += m * 2;
                    
                    chainSubPosClassRuleTables[j].lookAhead = lookaheadGlyphs;
                    
                    subPosCount = SFReadUShort(cscrTable, k);
                    
#ifdef LOOKUP_TEST
                    printf("\n         Total SubPositute Lookup Records: %d", subPosCount);
#endif
                    
                    subPosLookupRecords = malloc(sizeof(SubPosLookupRecord) * subPosCount);
                    
                    for (n = 0; n < subPosCount; l++) {
                        SFUShort beginOffset = k + (n * 4);
                        
                        subPosLookupRecords[n].sequenceIndex = SFReadUShort(cscrTable, beginOffset);
                        subPosLookupRecords[n].lookupListIndex = SFReadUShort(cscrTable, beginOffset + 2);
                        
#ifdef LOOKUP_TEST
                        printf("\n         Lookup Record At Index %d:", n);
                        printf("\n          Sequence Index: %d", subPosLookupRecords[n].sequenceIndex);
                        printf("\n          Lookup List Index: %d", subPosLookupRecords[n].lookupListIndex);
#endif
                    }
                    
                    chainSubPosClassRuleTables[j].subPosLookupRecord = subPosLookupRecords;
                }
                
                chainSubPosClassSets[i].chainSubPosClassRule = chainSubPosClassRuleTables;
            }
            
            tablePtr->format.format2.chainSubPosClassSet = chainSubPosClassSets;
        }
            break;
#endif
            
#ifdef GSUB_GPOS_CHAINING_CONTEXT_FORMAT3
        case 3:
        {
            SFUShort backtrackGlyphCount;
            CoverageTable *backtrackCoverageTables;
            
            SFUShort inputGlyphCount;
            CoverageTable *inputCoverageTables;
            
            SFUShort lookaheadGlyphCount;
            CoverageTable *lookaheadCoverageTables;
            
            SFUShort subPosCount;
            SubPosLookupRecord *subPosLookupRecords;
            
            SFUShort i, j, k, l;
            
            backtrackGlyphCount = SFReadUShort(ccsTable, 2);
            tablePtr->format.format3.backtrackGlyphCount = backtrackGlyphCount;
            
#ifdef LOOKUP_TEST
            printf("\n       Total Backtrack Glyphs: %d", backtrackGlyphCount);
#endif
            
            backtrackCoverageTables = malloc(sizeof(CoverageTable) * backtrackGlyphCount);
            
            for (i = 0; i < backtrackGlyphCount; i++) {
                SFUShort coverageOffset = SFReadUShort(ccsTable, 4 + (i * 2));
                
#ifdef LOOKUP_TEST
                printf("\n        Backtrack Coverage %d:", i + 1);
                printf("\n         Offset: %d", coverageOffset);
                
#endif
                SFReadCoverageTable(&ccsTable[coverageOffset], &backtrackCoverageTables[i]);
            }
            i = (4 + (i * 2));
            
            tablePtr->format.format3.backtrackGlyphCoverage = backtrackCoverageTables;
            
            inputGlyphCount = SFReadUShort(ccsTable, i);
            tablePtr->format.format3.inputGlyphCount = inputGlyphCount;
            
#ifdef LOOKUP_TEST
            printf("\n       Total Input Glyphs: %d", inputGlyphCount);
#endif
            
            inputCoverageTables = malloc(sizeof(CoverageTable) * inputGlyphCount);
            
            i += 2;
            
            for (j = 0; j < inputGlyphCount; j++) {
                SFUShort coverageOffset = SFReadUShort(ccsTable, i + (j * 2));
                
#ifdef LOOKUP_TEST
                printf("\n        Input Coverage %d:", j);
                printf("\n         Offset: %d", coverageOffset);
#endif
                
                SFReadCoverageTable(&ccsTable[coverageOffset], &inputCoverageTables[j]);
            }
            i += (j * 2);
            
            tablePtr->format.format3.inputGlyphCoverage = inputCoverageTables;
            
            lookaheadGlyphCount = SFReadUShort(ccsTable, i);
            tablePtr->format.format3.lookaheadGlyphCount = lookaheadGlyphCount;
            
#ifdef LOOKUP_TEST
            printf("\n       Total Lookahead Glyphs: %d", lookaheadGlyphCount);
#endif
            
            lookaheadCoverageTables = malloc(sizeof(CoverageTable) * lookaheadGlyphCount);
            
            i += 2;
            
            for (k = 0; k < lookaheadGlyphCount; k++) {
                SFUShort coverageOffset = SFReadUShort(ccsTable, i + (k * 2));
                
#ifdef LOOKUP_TEST
                printf("\n        Lookahead Coverage %d:", k);
                printf("\n         Offset: %d", coverageOffset);
                
#endif
                SFReadCoverageTable(&ccsTable[coverageOffset], &lookaheadCoverageTables[k]);
            }
            i += (k * 2);
            
            tablePtr->format.format3.lookaheadGlyphCoverage = lookaheadCoverageTables;
            
            subPosCount = SFReadUShort(ccsTable, i);
            tablePtr->format.format3.subPosCount = subPosCount;
            
            subPosLookupRecords = malloc(sizeof(SubPosLookupRecord) * subPosCount);
            
#ifdef LOOKUP_TEST
            printf("\n         Total SubPositute Lookup Records: %d", subPosCount);
#endif
            
            i += 2;
            for (l = 0; l < subPosCount; l++) {
                SFUShort beginOffset = i + (l * 4);
                
                subPosLookupRecords[l].sequenceIndex = SFReadUShort(ccsTable, beginOffset);
                subPosLookupRecords[l].lookupListIndex = SFReadUShort(ccsTable, beginOffset + 2);
                
#ifdef LOOKUP_TEST
                printf("\n         Lookup Record At Index %d:", l);
                printf("\n          Sequence Index: %d", subPosLookupRecords[l].sequenceIndex);
                printf("\n          Lookup List Index: %d", subPosLookupRecords[l].lookupListIndex);
#endif
            }
            
            tablePtr->format.format3.subPosLookupRecord = subPosLookupRecords;
        }
            break;
#endif
    }
}

void SFFreeChainingContextSubPos(ChainingContextualSubPosSubtable *tablePtr) {
	int i;

    switch (tablePtr->subPosFormat) {
            
#ifdef GSUB_GPOS_CHAINING_CONTEXT_FORMAT1
        case 1:
        {
            SFFreeCoverageTable(&tablePtr->format.format1.coverage);
            
            for (i = 0; i < tablePtr->format.format1.chainSubPosRuleSetCount; i++) {
                for (int j = 0; j < tablePtr->format.format1.chainSubPosRuleSet[i].chainSubPosRuleCount; j++) {
                    free(tablePtr->format.format1.chainSubPosRuleSet[i].chainSubPosRule[j].backtrack);
                    free(tablePtr->format.format1.chainSubPosRuleSet[i].chainSubPosRule[j].input);
                    free(tablePtr->format.format1.chainSubPosRuleSet[i].chainSubPosRule[j].lookAhead);
                    free(tablePtr->format.format1.chainSubPosRuleSet[i].chainSubPosRule[j].subPosLookupRecord);
                }
                
                free(tablePtr->format.format1.chainSubPosRuleSet[i].chainSubPosRule);
            }
            
            free(tablePtr->format.format1.chainSubPosRuleSet);
        }
            break;
#endif
            
#ifdef GSUB_GPOS_CHAINING_CONTEXT_FORMAT2
        case 2:
        {
            SFFreeCoverageTable(&tablePtr->format.format2.coverage);
            
            SFFreeClassDefTable(&tablePtr->format.format2.backtrackClassDef);
            SFFreeClassDefTable(&tablePtr->format.format2.inputClassDef);
            SFFreeClassDefTable(&tablePtr->format.format2.lookaheadClassDef);
            
            for (i = 0; i < tablePtr->format.format2.chainSubPosClassSetCount; i++) {
                for (int j = 0; j < tablePtr->format.format2.chainSubPosClassSet[i].chainSubPosClassRuleCount; j++) {
                    free(tablePtr->format.format2.chainSubPosClassSet[i].chainSubPosClassRule[j].backtrack);
                    free(tablePtr->format.format2.chainSubPosClassSet[i].chainSubPosClassRule[j].input);
                    free(tablePtr->format.format2.chainSubPosClassSet[i].chainSubPosClassRule[j].lookAhead);
                    free(tablePtr->format.format2.chainSubPosClassSet[i].chainSubPosClassRule[j].subPosLookupRecord);
                }
                
                free(tablePtr->format.format2.chainSubPosClassSet[i].chainSubPosClassRule);
            }
            
            free(tablePtr->format.format2.chainSubPosClassSet);
        }
            break;
#endif
            
#ifdef GSUB_GPOS_CHAINING_CONTEXT_FORMAT3
        case 3:
        {
            for (i = 0; i < tablePtr->format.format3.backtrackGlyphCount; i++)
                SFFreeCoverageTable(&tablePtr->format.format3.backtrackGlyphCoverage[i]);
            
            free(tablePtr->format.format3.backtrackGlyphCoverage);
            
            for (i = 0; i < tablePtr->format.format3.inputGlyphCount; i++)
                SFFreeCoverageTable(&tablePtr->format.format3.inputGlyphCoverage[i]);
            
            free(tablePtr->format.format3.inputGlyphCoverage);
            
            for (i = 0; i < tablePtr->format.format3.lookaheadGlyphCount; i++)
                SFFreeCoverageTable(&tablePtr->format.format3.lookaheadGlyphCoverage[i]);
            
            free(tablePtr->format.format3.lookaheadGlyphCoverage);
            free(tablePtr->format.format3.subPosLookupRecord);
        }
            break;
#endif
    }
}

#endif