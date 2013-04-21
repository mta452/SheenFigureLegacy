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
#include "SFGSUBData.h"

#ifdef GSUB_SINGLE

static void SFReadSingleSubst(const SFUByte * const ssTable, SingleSubstSubtable *tablePtr) {
    SFUShort substFormat;
    SFUShort coverageOffset;
    
    SFUShort glyphCount;
    
    substFormat = SFReadUShort(ssTable, 0);
    tablePtr->substFormat = substFormat;
    
    coverageOffset = SFReadUShort(ssTable, 2);
    
#ifdef LOOKUP_TEST
    printf("\n      Single Substitution Table:");
    printf("\n       Substitution Format: %d", substFormat);
    printf("\n       Coverage Table Offset: %d", coverageOffset);
    printf("\n       Coverage Table:");
#endif
    
    SFReadCoverageTable(&ssTable[coverageOffset], &tablePtr->coverage);
    
    switch (substFormat) {
            
#ifdef GSUB_SINGLE_FORMAT1
        case 1:
        {
            tablePtr->format.format1.deltaGlyphID = SFReadUShort(ssTable, 4);
            
#ifdef LOOKUP_TEST
            printf("\n       Delta Glyph ID: %d", tablePtr->format.format1.deltaGlyphID);
#endif
        }
            break;
            
#endif
            
#ifdef GSUB_SINGLE_FORMAT2
        case 2:
        {
            SFGlyph *substitutes;
            SFUShort i;
            
            glyphCount = SFReadUShort(ssTable, 4);
            tablePtr->format.format2.glyphCount = glyphCount;
            
#ifdef LOOKUP_TEST
            printf("\n       Total Substitutes: %d", glyphCount);
#endif
            
            substitutes = malloc(sizeof(SFGlyph) * glyphCount);
            for (i = 0; i < glyphCount; i++) {
                substitutes[i] = SFReadUShort(ssTable, 6 + (i * 2));
                
#ifdef LOOKUP_TEST
                printf("\n        Substitute At Index %d: %d", i + 1, substitutes[i]);
#endif
            }
            
            tablePtr->format.format2.substitute = substitutes;
        }
            break;
#endif
    }
}

static void SFFreeSingleSubst(SingleSubstSubtable *tablePtr) {
    SFFreeCoverageTable(&tablePtr->coverage);
    
#ifdef GSUB_SINGLE_FORMAT2
    if (tablePtr->substFormat == 2)
        free(tablePtr->format.format2.substitute);
#endif
}

#endif


#ifdef GSUB_MULTIPLE

static void SFReadMultipleSubst(const SFUByte * const msTable, MultipleSubstSubtable *tablePtr) {
    SFUShort substFormat;
    SFUShort coverageOffset;
    SFUShort sequenceCount;
    SequenceTable *sequenceTables;
    
    SFUShort i;
    
    substFormat = SFReadUShort(msTable, 0);
    tablePtr->substFormat = substFormat;
    
    coverageOffset = SFReadUShort(msTable, 2);
    
#ifdef LOOKUP_TEST
    printf("\n      Multiple Substitution Table:");
    printf("\n       Substitution Format: %d", substFormat);
    printf("\n       Coverage Table:");
    printf("\n        Offset: %d", coverageOffset);
#endif
    
    SFReadCoverageTable(&msTable[coverageOffset], &tablePtr->coverage);
    
    sequenceCount = SFReadUShort(msTable, 4);
    tablePtr->sequenceCount = sequenceCount;
    
#ifdef LOOKUP_TEST
    printf("\n       Total Sequences: %d", sequenceCount);
#endif
    
    sequenceTables = malloc(sizeof(SequenceTable) * sequenceCount);
    
    for (i = 0; i < sequenceCount; i++) {
        SFUShort offset;
        const SFUByte *sqTable;
        
        SFUShort glyphCount;
        SFGlyph *substitutes;
        
		SFUShort j;

        offset = SFReadUShort(msTable, 6 + (i * 2));
        
        sqTable = &msTable[offset];
        glyphCount = SFReadUShort(sqTable, 0);
        
#ifdef LOOKUP_TEST
        printf("\n       Sequence %d:", i + 1);
        printf("\n        Offset: %d", offset);
#endif
        
        sequenceTables[i].glyphCount = glyphCount;
        
        substitutes = malloc(sizeof(SFGlyph) * glyphCount);
        
#ifdef LOOKUP_TEST
        printf("\n        Total Substitutes: %d", glyphCount);
#endif
        
        for (j = 0; j < glyphCount; j++) {
            substitutes[j] = SFReadUShort(sqTable, 2 + (j * 2));
            
#ifdef LOOKUP_TEST
            printf("\n        Substitute At Index %d: %d", j, substitutes[j]);
#endif
        }
        
        sequenceTables[i].substitute = substitutes;
    }
    
    tablePtr->sequence = sequenceTables;
}

static void SFFreeMultipleSubst(MultipleSubstSubtable *tablePtr) {
	int i;

    SFFreeCoverageTable(&tablePtr->coverage);
    
    for (i = 0; i < tablePtr->sequenceCount; i++)
        free(tablePtr->sequence[i].substitute);
    
    free(tablePtr->sequence);
}

#endif


#ifdef GSUB_ALTERNATE

static void SFReadAlternateSubst(const SFUByte * const asTable, AlternateSubstSubtable *tablePtr) {
    SFUShort substFormat;
    SFUShort coverageOffset;
    
    AlternateSetTable *alternateSetTables;
    
    SFUShort i;
    
    substFormat = SFReadUShort(asTable, 0);
    tablePtr->substFormat = substFormat;
    
    coverageOffset = SFReadUShort(asTable, 2);
    
#ifdef LOOKUP_TEST
    printf("\n      Alternate Substitution Table:");
    printf("\n       Substitution Format: %d", substFormat);
    printf("\n       Coverage Table Offset: %d", coverageOffset);
    printf("\n       Coverage Table:");
#endif
    
    SFReadCoverageTable(&asTable[coverageOffset], &tablePtr->coverage);
    
    tablePtr->alternateSetCount = SFReadUShort(asTable, 4);
    
#ifdef LOOKUP_TEST
    printf("\n       Total Alternate Sets: %d", tablePtr->alternateSetCount);
#endif
    
    alternateSetTables = malloc(sizeof(AlternateSetTable) * tablePtr->alternateSetCount);
    
    for (i = 0; i < tablePtr->alternateSetCount; i++) {
        SFUShort offset;
        const SFUByte *aTable;
        
        SFUShort glyphCount;
        SFGlyph *substitutes;
        
        SFUShort j;
        
        offset = SFReadUShort(asTable, 6 + (i * 2));
        
#ifdef LOOKUP_TEST
        printf("\n       Alternate Set %d:", i + 1);
        printf("\n        Offset: %d", offset);
#endif
        
        aTable = &asTable[offset];
        glyphCount = SFReadUShort(aTable, 0);
        
#ifdef LOOKUP_TEST
        printf("\n        Total Alternates: %d", glyphCount);
#endif
        
        substitutes = malloc(sizeof(SFGlyph) * glyphCount);
        
        for (j = 0; j < glyphCount; j++) {
            substitutes[j] = SFReadUShort(aTable, 2 + (j * 2));
            
#ifdef LOOKUP_TEST
            printf("\n        Alternate At Index %d: %d", j, substitutes[j]);
#endif
        }
        
        alternateSetTables[i].substitute = substitutes;
    }
    
    tablePtr->alternateSet = alternateSetTables;
}

static void SFFreeAlternateSubst(AlternateSubstSubtable *tablePtr) {
	int i;

    SFFreeCoverageTable(&tablePtr->coverage);
    
    for (i = 0; i < tablePtr->alternateSetCount; i++)
        free(tablePtr->alternateSet[i].substitute);
    
    free(tablePtr->alternateSet);
}

#endif


#ifdef GSUB_LIGATURE

static void SFReadLigatureSubst(const SFUByte * const lsTable, LigatureSubstSubtable *tablePtr) {
    SFUShort substFormat;
    SFUShort coverageOffset;
    
    SFUShort ligSetCount;
    LigatureSetTable *ligSetTables;
    
    SFUShort i;
    
    substFormat = SFReadUShort(lsTable, 0);
    tablePtr->substFormat = substFormat;
    
    coverageOffset = SFReadUShort(lsTable, 2);
    
#ifdef LOOKUP_TEST
    printf("\n      Ligature Substitution Table:");
    printf("\n       Substitution Format: %d", substFormat);
    printf("\n       Coverage Table:");
    printf("\n        Offset: %d", coverageOffset);
#endif
    
    SFReadCoverageTable(&lsTable[coverageOffset], &tablePtr->coverage);
    
    ligSetCount = SFReadUShort(lsTable, 4);
    tablePtr->ligSetCount = ligSetCount;
    
#ifdef LOOKUP_TEST
    printf("\n       Total Ligature Sets: %d", ligSetCount);
#endif
    
    ligSetTables = malloc(sizeof(LigatureSetTable) * ligSetCount);
    
    for (i = 0; i < ligSetCount; i++) {
        SFUShort offset;
        const SFUByte *ligsTable;
        
        SFUShort ligCount;
        LigatureTable *ligatureTables;
        
        SFUShort j;
        
        offset = SFReadUShort(lsTable, 6 + (i * 2));
        
        ligsTable = &lsTable[offset];
        ligCount = SFReadUShort(ligsTable, 0);
        ligSetTables[i].ligatureCount = ligCount;
        
#ifdef LOOKUP_TEST
        printf("\n       Ligature Set %d:", i + 1);
        printf("\n        Offset: %d", offset);
        printf("\n        Total Ligature Tables: %d", ligCount);
#endif
        
        ligatureTables = malloc(sizeof(LigatureTable) * ligCount);
        
        for (j = 0; j < ligCount; j++) {
            SFUShort ligOffset;
            const SFUByte *lTable;
            
            SFUShort compCount;
            SFGlyph *components;
            
			SFUShort k;

            ligOffset = SFReadUShort(ligsTable, 2 + (j * 2));
            
            lTable = &ligsTable[ligOffset];
            ligatureTables[j].ligGlyph = SFReadUShort(lTable, 0);
            
            compCount = SFReadUShort(lTable, 2);
            ligatureTables[j].compCount = compCount;
            
#ifdef LOOKUP_TEST
            printf("\n        Ligature Table %d:", j + 1);
            printf("\n         Offset: %d", ligOffset);
            printf("\n         Ligature Glyph: %d", ligatureTables[j].ligGlyph);
            printf("\n         Total Components: %d", compCount);
#endif
            
            components = malloc(sizeof(SFGlyph) * compCount);
            components[0] = 0;
            
            for (k = 0; k < compCount - 1; k++) {
                components[k + 1] = SFReadUShort(lTable, 4 + (k * 2));
                
#ifdef LOOKUP_TEST
                printf("\n         Component At Index %d: %d", k + 1, components[k + 1]);
#endif
            }
            
            ligatureTables[j].component = components;
        }
        
        ligSetTables[i].ligature = ligatureTables;
    }
    
    tablePtr->ligatureSet = ligSetTables;
}

static void SFFreeLigatureSubst(LigatureSubstSubtable *tablePtr) {
	int i, j;

    SFFreeCoverageTable(&tablePtr->coverage);
    
    for (i = 0; i < tablePtr->ligSetCount; i++) {
        for (j = 0; j < tablePtr->ligatureSet[i].ligatureCount; j++)
            free(tablePtr->ligatureSet[i].ligature[j].component);
        
        free(tablePtr->ligatureSet[i].ligature);
    }
    
    free(tablePtr->ligatureSet);
}

#endif


#ifdef GSUB_REVERSE_CHAINING_CONTEXT

static void SFReadReverseChainingContextSubst(const SFUByte * const rccssTable, ReverseChainingContextSubstSubtable *tablePtr) {
    SFUShort coverageOffset;
    SFUShort backtrackGlyphCount;
    CoverageTable *backtrackCoverageTables;
    
    SFUShort lookaheadGlyphCount;
    CoverageTable *lookaheadCoverageTables;
    
    SFUShort glyphCount;
    SFGlyph *substitutes;
    
    SFUShort i, j, k;
    
    coverageOffset = SFReadUShort(rccssTable, 2);
    
#ifdef LOOKUP_TEST
    printf("\n       Coverage Table:");
    printf("\n        Offset: %d", coverageOffset);
#endif
    
    SFReadCoverageTable(&rccssTable[coverageOffset], &tablePtr->Coverage);
    
    backtrackGlyphCount = SFReadUShort(rccssTable, 4);
    tablePtr->backtrackGlyphCount = backtrackGlyphCount;
    
#ifdef LOOKUP_TEST
    printf("\n       Total Backtrack Glyphs: %d", backtrackGlyphCount);
#endif
    
    backtrackCoverageTables = malloc(sizeof(CoverageTable) * backtrackGlyphCount);
    
    for (i = 0; i < backtrackGlyphCount; i++) {
        SFUShort coverageOffset = SFReadUShort(rccssTable, 6 + (i * 2));
        
#ifdef LOOKUP_TEST
        printf("\n       Backtrack Coverage %d:", i + 1);
        printf("\n        Offset: %d", coverageOffset);
#endif
        
        SFReadCoverageTable(&rccssTable[coverageOffset], &backtrackCoverageTables[i]);
    }
    i = i + (6 + (i * 2));
    
    tablePtr->backtrackGlyphCoverage = backtrackCoverageTables;
    
    lookaheadGlyphCount = SFReadUShort(rccssTable, i);
    tablePtr->lookaheadGlyphCount = lookaheadGlyphCount;
    
#ifdef LOOKUP_TEST
    printf("\n       Total Lookahead Glyphs: %d", backtrackGlyphCount);
#endif
    
    lookaheadCoverageTables = malloc(sizeof(CoverageTable) * lookaheadGlyphCount);
    
    i += 2;
    for (j = 0; j < lookaheadGlyphCount; j++) {
        SFUShort coverageOffset = SFReadUShort(rccssTable, i + (j * 2));
        
#ifdef LOOKUP_TEST
        printf("\n       Lookahead Coverage %d:", i + 1);
        printf("\n        Offset: %d", coverageOffset);
#endif
        
        SFReadCoverageTable(&rccssTable[coverageOffset], &lookaheadCoverageTables[j]);
    }
    i = i + (j * 2) + 2;
    
    tablePtr->lookaheadGlyphCoverage = lookaheadCoverageTables;
    
    glyphCount = SFReadUShort(rccssTable, i);
    tablePtr->glyphCount = glyphCount;
    
#ifdef LOOKUP_TEST
    printf("\n       Total Substitute Glyphs: %d", glyphCount);
#endif
    
    substitutes = malloc(sizeof(SFGlyph) * glyphCount);
    
    i += 2;
    for (k = 0; k < glyphCount; k++) {
        substitutes[k] = SFReadUShort(rccssTable, i + (k * 2));
        
#ifdef LOOKUP_TEST
        printf("\n       Substitute At Index %d: %d", k, substitutes[k]);
#endif
    }
    
    tablePtr->substitute = substitutes;
}

static void SFFreeReverseChainingContextSubst(ReverseChainingContextSubstSubtable *tablePtr) {
    int i;
    
    SFFreeCoverageTable(&tablePtr->Coverage);
    
    for (i = 0; i < tablePtr->backtrackGlyphCount; i++)
        SFFreeCoverageTable(&tablePtr->backtrackGlyphCoverage[i]);
    
    free(tablePtr->backtrackGlyphCoverage);
    
    for (i = 0; i < tablePtr->lookaheadGlyphCount; i++)
        SFFreeCoverageTable(&tablePtr->lookaheadGlyphCoverage[i]);
    
    free(tablePtr->lookaheadGlyphCoverage);
    free(tablePtr->substitute);
}

#endif


static void *SFReadSubstitution(const SFUByte * const sTable, LookupType *type) {
    void *subtablePtr = NULL;
    
    if (*type == ltsExtensionSubstitution) {
        SFUInt extensionOffset;
        
        *type = SFReadUShort(sTable, 2);
        extensionOffset = SFReadUInt(sTable, 4);
        
        return SFReadSubstitution(&sTable[extensionOffset], type);
    }
    
    switch (*type) {
            
#ifdef GSUB_SINGLE
        case ltsSingle:
        {
            SingleSubstSubtable *subtable = malloc(sizeof(SingleSubstSubtable));
            SFReadSingleSubst(sTable, subtable);
            subtablePtr = subtable;
            break;
        }
#endif
            
#ifdef GSUB_MULTIPLE
        case ltsMultiple:
        {
            MultipleSubstSubtable *subtable = malloc(sizeof(MultipleSubstSubtable));
            SFReadMultipleSubst(sTable, subtable);
            subtablePtr = subtable;
            break;
        }
#endif
            
#ifdef GSUB_ALTERNATE
        case ltsAlternate:
        {
            AlternateSubstSubtable *subtable = malloc(sizeof(AlternateSubstSubtable));
            SFReadAlternateSubst(sTable, subtable);
            subtablePtr = subtable;
            break;
        }
#endif
            
#ifdef GSUB_LIGATURE
        case ltsLigature:
        {
            LigatureSubstSubtable *subtable = malloc(sizeof(LigatureSubstSubtable));
            SFReadLigatureSubst(sTable, subtable);
            subtablePtr = subtable;
            break;
        }
#endif
            
#ifdef GSUB_GPOS_CONTEXT
        case ltsContext:
        {
            ContextSubPosSubtable *subtable = malloc(sizeof(ContextSubPosSubtable));
            SFReadContextSubPos(sTable, subtable);
            subtablePtr = subtable;
            break;
        }
#endif
            
#ifdef GSUB_GPOS_CHAINING_CONTEXT
        case ltsChainingContext:
        {
            ChainingContextualSubPosSubtable *subtable = malloc(sizeof(ChainingContextualSubPosSubtable));
            SFReadChainingContextSubPos(sTable, subtable);
            subtablePtr = subtable;
            break;
        }
#endif
            
#ifdef GSUB_REVERSE_CHAINING_CONTEXT
        case ltsReverseChainingContextSingle:
        {
            ReverseChainingContextSubstSubtable *subtable = malloc(sizeof(ReverseChainingContextSubstSubtable));
            SFReadReverseChainingContextSubst(sTable, subtable);
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

static void SFFreeSubstitution(void *tablePtr, LookupType type) {
    switch (type) {
            
#ifdef GSUB_SINGLE
        case ltsSingle:
            SFFreeSingleSubst(tablePtr);
            break;
#endif
            
#ifdef GSUB_MULTIPLE
        case ltsMultiple:
            SFFreeMultipleSubst(tablePtr);
            break;
#endif
            
#ifdef GSUB_ALTERNATE
        case ltsAlternate:
            SFFreeAlternateSubst(tablePtr);
            break;
#endif
            
#ifdef GSUB_LIGATURE
        case ltsLigature:
            SFFreeLigatureSubst(tablePtr);
            break;
#endif
            
#ifdef GSUB_GPOS_CONTEXT
        case ltsContext:
            SFFreeContextSubPos(tablePtr);
            break;
#endif
            
#ifdef GSUB_GPOS_CHAINING_CONTEXT
        case ltsChainingContext:
            SFFreeChainingContextSubPos(tablePtr);
            break;
#endif
            
#ifdef GSUB_REVERSE_CHAINING_CONTEXT
        case ltsReverseChainingContextSingle:
            SFFreeReverseChainingContextSubst(tablePtr);
            break;
#endif
        default:
            break;
    }
    
    free(tablePtr);
}


void SFReadGSUB(const SFUByte * const table, SFTableGSUB *tablePtr) {
    SFUShort scriptListOffset;
    SFUShort featureListOffset;
    SFUShort lookupListOffset;
    
    tablePtr->version = SFReadUInt(table, 0);
    
    scriptListOffset = SFReadUShort(table, 4);
    
#ifdef SCRIPT_TEST
    printf("\nGSUB Header:");
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
    
    SFReadLookupListTable(&table[lookupListOffset], &tablePtr->lookupList, &SFReadSubstitution);
}

void SFFreeGSUB(SFTableGSUB *tablePtr) {
    SFFreeScriptListTable(&tablePtr->scriptList);
    SFFreeFeatureListTable(&tablePtr->featureList);
    SFFreeLookupListTable(&tablePtr->lookupList, &SFFreeSubstitution);
}
