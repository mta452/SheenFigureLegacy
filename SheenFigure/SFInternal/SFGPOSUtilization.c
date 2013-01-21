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
#include <string.h>
#include <stdint.h>

#include "SFTypes.h"
#include "SFGlobal.h"
#include "SFCommonMethods.h"
#include "SFGPOSUtilization.h"

#define GPOS_FEATURE_TAGS 4
static unsigned char gposFeaturesTagOrder[GPOS_FEATURE_TAGS][5] = {
    "curs",                         //Cursive positioning
    "kern",                         //Pair kerning
    "mark",                         //Mark to base positioning
    "mkmk"                          //Mark to mark positioning
};

static void SFApplyGPOSLookup(LookupTable lookup, SFGlyphIndex sindex, SFGlyphIndex eindex);

static void createPositioningValues(ValueRecord record, ValueFormat format, SFPoint *point, SFPoint *advance) {
    if (format & vfXPlacement)
        point->x = record.xPlacement;
    
    if (format & vfYPlacement)
        point->y = record.yPlacement;
    
    if (format & vfXAdvance)
        advance->x = record.xAdvance;
    
    if (format & vfYAdvance)
        advance->y = record.yAdvance;
}

static void SFApplySingleAdjustment(SingleAdjustmentPosSubtable *stable, LookupFlag lookupFlag, SFGlyphIndex sindex, SFGlyphIndex eindex) {
    int endRecordIndex = eindex.recordIndex + 1;

	int i = sindex.recordIndex;
    int j = sindex.glyphIndex;
    
    int endGlyphIndex;
    
    for (; i < endRecordIndex; i++) {
        if (i == (endRecordIndex - 1))
            endGlyphIndex = eindex.glyphIndex + 1;
        else
            endGlyphIndex = record->charRecord[i].glyphCount;
        
        for (; j < endGlyphIndex; j++) {
            int coverageIndex;
            
            if (SFIsIgnoredGlyph(i, j, lookupFlag))
                continue;
            
            coverageIndex = SFGetIndexOfGlyphInCoverage(&stable->coverage, record->charRecord[i].gRec[j].glyph);
            if (coverageIndex != UNDEFINED_INDEX) {
                if (stable->posFormat == 1)
                    createPositioningValues(stable->format.format1.value,
                                            stable->valueFormat,
                                            &record->charRecord[i].gRec[j].posRec.placement,
                                            &record->charRecord[i].gRec[j].posRec.advance);
                else if (stable->posFormat == 2)
                    createPositioningValues(stable->format.format2.value[coverageIndex],
                                            stable->valueFormat,
                                            &record->charRecord[i].gRec[j].posRec.placement,
                                            &record->charRecord[i].gRec[j].posRec.advance);
            }
        }
        
        j = 0;
    }
}

static int SFGetIndexOfGlyphInPairSet(PairSetTable *pairSet, SFGlyph glyph) {
	int i;
    for (i = 0; i < pairSet->pairValueCount; i++) {
        SFGlyph inputGlyph = pairSet->pairValueRecord[i].secondGlyph;
        
        if (inputGlyph == glyph)
            return i;
    }
    
    return UNDEFINED_INDEX;
}

static int SFGetClassOfGlyphInClassDef(ClassDefTable *cls, SFGlyph glyph) {
	SFUShort i;

    if (cls->classFormat == 1) {
        SFGlyph startGlyph = cls->format.format1.startGlyph;
        SFUShort glyphCount = cls->format.format1.glyphCount;
        
        for (i = 0; i < glyphCount; i++) {
            if ((startGlyph + i) == glyph)
                return cls->format.format1.classValueArray[i];
        }
    } else if (cls->classFormat == 2) {
        SFUShort classRangeCount = cls->format.format2.classRangeCount;
        
        for (i = 0; i < classRangeCount; i++) {
            SFGlyph startGlyph = cls->format.format2.classRangeRecord[i].start;
            SFGlyph endGlyph = cls->format.format2.classRangeRecord[i].end;
            
            if (glyph >= startGlyph && glyph <= endGlyph)
                return cls->format.format2.classRangeRecord[i].cls;
        }
    }
    
    return UNDEFINED_INDEX;
}

static void SFApplyPairAdjustment(PairAdjustmentPosSubtable *stable, LookupFlag lookupFlag, SFGlyphIndex sindex, SFGlyphIndex eindex) {
    int endRecordIndex;
    int endGlyphIndex;
    
    int i, j;
    
    if (sindex.recordIndex == eindex.recordIndex && sindex.glyphIndex == eindex.glyphIndex)
        return;
    
    endRecordIndex = eindex.recordIndex + 1;
    j = sindex.glyphIndex;
    
    for (i = sindex.recordIndex; i < endRecordIndex; i++) {
        if (i == (endRecordIndex - 1))
            endGlyphIndex = eindex.glyphIndex + 1;
        else
            endGlyphIndex = record->charRecord[i].glyphCount;
        
        for (; j < endGlyphIndex; j++) {
            int coverageIndex;
            
            if (SFIsIgnoredGlyph(i, j, lookupFlag))
                continue;
            
            coverageIndex = SFGetIndexOfGlyphInCoverage(&stable->coverage, record->charRecord[i].gRec[j].glyph);
            if (coverageIndex != UNDEFINED_INDEX) {
                SFGlyphIndex tmpIndex;
                SFGlyph nextGlyph;
                
                tmpIndex = SFMakeGlyphIndex(i, j);
                if (!SFGetNextValidGlyphIndex(&tmpIndex, lookupFlag))
                    continue;
                
                nextGlyph = record->charRecord[tmpIndex.recordIndex].gRec[tmpIndex.glyphIndex].glyph;
                
                if (stable->posFormat == 1) {
                    int pairSetIndex = SFGetIndexOfGlyphInPairSet(stable->format.format1.pairSetTable, nextGlyph);
                    
                    if (pairSetIndex != UNDEFINED_INDEX) {
                        createPositioningValues(
                                                stable->format.format1.pairSetTable[coverageIndex].pairValueRecord[pairSetIndex].value1,
                                                stable->valueFormat1,
                                                &record->charRecord[i].gRec[j].posRec.placement,
                                                &record->charRecord[i].gRec[j].posRec.advance);
                        
                        createPositioningValues(
                                                stable->format.format1.pairSetTable[coverageIndex].pairValueRecord[pairSetIndex].value2,
                                                stable->valueFormat2,
                                                &record->charRecord[tmpIndex.recordIndex].gRec[tmpIndex.glyphIndex].posRec.placement,
                                                &record->charRecord[tmpIndex.recordIndex].gRec[tmpIndex.glyphIndex].posRec.advance);
                    }
                } else if (stable->posFormat == 2) {
                    int class1Index;
                    int class2Index;
                    
                    class2Index = SFGetClassOfGlyphInClassDef(&stable->format.format2.classDef2, nextGlyph);
                    if (class2Index == UNDEFINED_INDEX)
                        continue;
                    
                    class1Index = SFGetClassOfGlyphInClassDef(&stable->format.format2.classDef1, record->charRecord[i].gRec[j].glyph);
                    
                    createPositioningValues(
                                            stable->format.format2.class1Record[class1Index].class2Record[class2Index].value1,
                                            stable->valueFormat1,
                                            &record->charRecord[i].gRec[j].posRec.placement,
                                            &record->charRecord[i].gRec[j].posRec.advance);
                    
                    createPositioningValues(
                                            stable->format.format2.class1Record[class1Index].class2Record[class2Index].value2,
                                            stable->valueFormat2,
                                            &record->charRecord[tmpIndex.recordIndex].gRec[tmpIndex.glyphIndex].posRec.placement,
                                            &record->charRecord[tmpIndex.recordIndex].gRec[tmpIndex.glyphIndex].posRec.advance);
                }
                
                i = tmpIndex.recordIndex;
                j = tmpIndex.glyphIndex;
            }
        }
        
        j = 0;
    }
}

static void SFApplyCursiveAttachment(CursiveAttachmentPosSubtable *stable, LookupFlag lookupFlag, SFGlyphIndex sindex, SFGlyphIndex eindex) {
    int endRecordIndex = eindex.recordIndex + 1;

	int i = sindex.recordIndex;
    int j = sindex.glyphIndex;
    
    int endGlyphIndex;
    
    SFBool hasExitAnchor = SFFalse;
    AnchorTable exitAnchor;
    
    for (; i < endRecordIndex; i++) {
        if (i == (endRecordIndex - 1))
            endGlyphIndex = eindex.glyphIndex + 1;
        else
            endGlyphIndex = record->charRecord[i].glyphCount;
        
        for (; j < endGlyphIndex; j++) {
            int coverageIndex;
            
            if (SFIsIgnoredGlyph(i, j, lookupFlag) && hasExitAnchor) {
                record->charRecord[i].gRec[j].posRec.anchorType |= atCursiveIgnored;
                continue;
            }
            
            coverageIndex = SFGetIndexOfGlyphInCoverage(&stable->coverage, record->charRecord[i].gRec[j].glyph);
            if (coverageIndex != UNDEFINED_INDEX) {
                if (stable->entryExitRecord[coverageIndex].hasEntryAnchor && hasExitAnchor) {
                    record->charRecord[i].gRec[j].posRec.anchorType |= atEntry;
                    record->charRecord[i].gRec[j].posRec.anchor.x = exitAnchor.xCoordinate - stable->entryExitRecord[coverageIndex].entryAnchor.xCoordinate;
                    record->charRecord[i].gRec[j].posRec.anchor.y = exitAnchor.yCoordinate - stable->entryExitRecord[coverageIndex].entryAnchor.yCoordinate;
                    
                    hasExitAnchor = SFFalse;
                }
                
                if (stable->entryExitRecord[coverageIndex].hasExitAnchor) {
                    record->charRecord[i].gRec[j].posRec.anchorType |= atExit;
                    
                    hasExitAnchor = SFTrue;
                    exitAnchor = stable->entryExitRecord[coverageIndex].exitAnchor;
                }
            } else
                hasExitAnchor = SFFalse;
        }
        
        j = 0;
    }
}

static void SFApplyMarkToBaseAttachment(MarkToBaseAttachmentPosSubtable *stable, LookupFlag lookupFlag, SFGlyphIndex sindex, SFGlyphIndex eindex) {
    int endRecordIndex = eindex.recordIndex + 1;

	int i = sindex.recordIndex;
    int j = sindex.glyphIndex;
    
    int endGlyphIndex;
    
    for (; i < endRecordIndex; i++) {
        if (odd(record->levels[i]))
            goto continue_loop;
        
        if (i == (endRecordIndex - 1))
            endGlyphIndex = eindex.glyphIndex + 1;
        else
            endGlyphIndex = record->charRecord[i].glyphCount;
        
        for (; j < endGlyphIndex; j++) {
            int markIndex = SFGetIndexOfGlyphInCoverage(&stable->markCoverage, record->charRecord[i].gRec[j].glyph);
            
            if (markIndex != UNDEFINED_INDEX) {
                SFGlyphIndex tmpIndex;
                SFUShort classValue;
                SFGlyph previousGlyph;
                
                int baseIndex;
                int x, y;
                
                tmpIndex = SFMakeGlyphIndex(i, j);
                
                // Class Index to be applied on base glyph.
                classValue = stable->markArray.markRecord[markIndex].cls;
                
                if (!SFGetPreviousBaseGlyphIndex(&tmpIndex, lookupFlag))
                    continue;
                
                previousGlyph = record->charRecord[tmpIndex.recordIndex].gRec[tmpIndex.glyphIndex].glyph;
                
                baseIndex = SFGetIndexOfGlyphInCoverage(&stable->baseCoverage, previousGlyph);
                
                if (baseIndex == UNDEFINED_INDEX)
                    continue;   // Previous glyph did not match any of the base glyphs
								// listed in base coverage table, so try matching sequence
								// from next input glyph.
                
                record->charRecord[i].gRec[j].posRec.anchorType |= atMark;
                
                x = stable->markArray.markRecord[markIndex].markAnchor.xCoordinate - stable->baseArray.baseRecord[baseIndex].baseAnchor[classValue].xCoordinate;
                if (x)
                    record->charRecord[i].gRec[j].posRec.anchor.x = x;
                
                y = stable->markArray.markRecord[markIndex].markAnchor.yCoordinate - stable->baseArray.baseRecord[baseIndex].baseAnchor[classValue].yCoordinate;
                record->charRecord[i].gRec[j].posRec.anchor.y = y;
            }
        }
        
    continue_loop:
        j = 0;
    }
}

static void SFApplyMarkToLigatureAttachment(MarkToLigatureAttachmentPosSubtable *stable, LookupFlag lookupFlag, SFGlyphIndex sindex, SFGlyphIndex eindex) {
    int endRecordIndex = eindex.recordIndex + 1;

	int i = sindex.recordIndex;
    int j = sindex.glyphIndex;
    
    int endGlyphIndex;
    
    for (; i < endRecordIndex; i++) {
        if (odd(record->levels[i]))
            goto continue_loop;
        
        if (i == (endRecordIndex - 1))
            endGlyphIndex = eindex.glyphIndex + 1;
        else
            endGlyphIndex = record->charRecord[i].glyphCount;
        
        for (; j < endGlyphIndex; j++) {
            int markIndex = SFGetIndexOfGlyphInCoverage(&stable->markCoverage, record->charRecord[i].gRec[j].glyph);
            
            if (markIndex != UNDEFINED_INDEX) {
                SFGlyphIndex tmpIndex;
                SFUShort classValue;
                int component;
                
                SFGlyph previousGlyph;
                LigatureAttachTable ligAttach;
                
                int ligatureIndex;
                int x, y;
                
                tmpIndex = SFMakeGlyphIndex(i, j);
                
                // Class Index to be applied on base glyph.
                classValue = stable->markArray.markRecord[markIndex].cls;
                component = 0;
                
                if (!SFGetPreviousLigatureGlyphIndex(&tmpIndex, lookupFlag, &component))
                    continue;
                
                previousGlyph = record->charRecord[tmpIndex.recordIndex].gRec[tmpIndex.glyphIndex].glyph;
                
                ligatureIndex = SFGetIndexOfGlyphInCoverage(&stable->ligatureCoverage, previousGlyph);
                ligAttach = stable->ligatureArray.ligatureAttach[ligatureIndex];
                
                if (ligatureIndex == UNDEFINED_INDEX || component >= ligAttach.componentCount)
                    continue;   // Previous glyph did not match any of the ligature glyphs
								// listed in base coverage table, so try matching sequence
								// from next input glyph.
                
                record->charRecord[i].gRec[j].posRec.anchorType = atMark;
                
                x = stable->markArray.markRecord[markIndex].markAnchor.xCoordinate - ligAttach.componentRecord[component].ligatureAnchor[classValue].xCoordinate;
                if (x)
                    record->charRecord[i].gRec[j].posRec.anchor.x = x;
                
                y = stable->markArray.markRecord[markIndex].markAnchor.yCoordinate - ligAttach.componentRecord[component].ligatureAnchor[classValue].yCoordinate;
                record->charRecord[i].gRec[j].posRec.anchor.y = y;
            }
        }
        
    continue_loop:
        j = 0;
    }
}

static void SFApplyMarkToMarkAttachment(MarkToMarkAttachmentPosSubtable *stable, LookupFlag lookupFlag, SFGlyphIndex sindex, SFGlyphIndex eindex) {
    int endRecordIndex = eindex.recordIndex + 1;

	int i = sindex.recordIndex;
    int j = sindex.glyphIndex;
    
    int endGlyphIndex;
    
    for (; i < endRecordIndex; i++) {
        if (odd(record->levels[i]))
            goto continue_loop;
        
        if (i == (endRecordIndex - 1))
            endGlyphIndex = eindex.glyphIndex + 1;
        else
            endGlyphIndex = record->charRecord[i].glyphCount;
        
        for (; j < endGlyphIndex; j++) {
            int mark1Index = SFGetIndexOfGlyphInCoverage(&stable->mark1Coverage, record->charRecord[i].gRec[j].glyph);
            
            if (mark1Index != UNDEFINED_INDEX) {
                SFGlyphIndex tmpIndex;
                SFUShort classValue;
                SFGlyph previousGlyph;
                
                int mark2Index;
                int x, y;
                
                tmpIndex = SFMakeGlyphIndex(i, j);
                
                // Class Index to be applied on base glyph.
                classValue = stable->mark1Array.markRecord[mark1Index].cls;

                if (!SFGetPreviousGlyphIndex(&tmpIndex, lookupFlag))
                    continue;
                
                previousGlyph = record->charRecord[tmpIndex.recordIndex].gRec[tmpIndex.glyphIndex].glyph;
                if (previousGlyph == 0)
					continue;

                mark2Index = SFGetIndexOfGlyphInCoverage(&stable->mark2Coverage, previousGlyph);
                if (mark2Index == UNDEFINED_INDEX)
                    continue;   // Previous glyph did not match any of the base glyphs
								// listed in base coverage table, so try matching sequence
								// from next input glyph.
                
                record->charRecord[i].gRec[j].posRec.anchorType = atMark;
                
                x = stable->mark1Array.markRecord[mark1Index].markAnchor.xCoordinate - stable->mark2Array.mark2Record[mark2Index].mark2Anchor[classValue].xCoordinate;
                if (x)
                    record->charRecord[i].gRec[j].posRec.anchor.x = x - record->charRecord[tmpIndex.recordIndex].gRec[tmpIndex.glyphIndex].posRec.anchor.x;
                
                y = stable->mark1Array.markRecord[mark1Index].markAnchor.yCoordinate - stable->mark2Array.mark2Record[mark2Index].mark2Anchor[classValue].yCoordinate;
                record->charRecord[i].gRec[j].posRec.anchor.y = y + record->charRecord[tmpIndex.recordIndex].gRec[tmpIndex.glyphIndex].posRec.anchor.y;
            }
        }
        
    continue_loop:
        j = 0;
    }
}

static void SFApplyChainingContextual(ChainingContextualPosSubtable *stable, LookupFlag lookupFlag) {
    SFGlyphIndex index;
	SFGlyphIndex *inputIndexes = malloc(sizeof(SFGlyphIndex) * stable->format.format3.inputGlyphCount);
    
    if (stable->posFormat != 3)
        return;
    
    index = SFMakeGlyphIndex(0, 0);
    if (SFIsIgnoredGlyph(0, 0, lookupFlag)) {
        if (!SFGetNextValidGlyphIndex(&index, lookupFlag))
            return;
    }
    
    do {
        int coverageIndex;
        
        SFGlyphIndex tmpNextIndex;
        SFGlyphIndex tmpPreviousIndex;
        
		int j = 1, k = 0, l = 0, m = 0, n = 0;

        coverageIndex = SFGetIndexOfGlyphInCoverage(&stable->format.format3.inputGlyphCoverage[0], record->charRecord[index.recordIndex].gRec[index.glyphIndex].glyph);
        
        if (coverageIndex == UNDEFINED_INDEX)
            continue;
        
        inputIndexes[0] = index;
        
        // Loop to check input glyphs.
        for (; j < stable->format.format3.inputGlyphCount; j++) {
            SFGlyphIndex tmpIndex;
            SFGlyph currentGlyph;
            
            tmpIndex = inputIndexes[k];
            
            // Checking if we have another glyph (except ignored glyphs) to match
            if (!SFGetNextValidGlyphIndex(&tmpIndex, lookupFlag))
                return;
            
            currentGlyph = record->charRecord[tmpIndex.recordIndex].gRec[tmpIndex.glyphIndex].glyph;
            coverageIndex = SFGetIndexOfGlyphInCoverage(&stable->format.format3.inputGlyphCoverage[j], currentGlyph);
            
            inputIndexes[++k] = tmpIndex;
            
            // Current glyph does not match the input glyph,
            // so try checking from next glyph.
            if (coverageIndex == UNDEFINED_INDEX)
                goto continue_parent_loop;
        }
        
        tmpNextIndex = index;
        
        // Loop to check lookahead glyphs.
        for (; m < stable->format.format3.lookaheadGlyphCount; m++) {
            SFGlyph currentGlyph;
            
            if (!SFGetNextValidGlyphIndex(&tmpNextIndex, lookupFlag))
                return;
            
            currentGlyph = record->charRecord[tmpNextIndex.recordIndex].gRec[tmpNextIndex.glyphIndex].glyph;
            coverageIndex = SFGetIndexOfGlyphInCoverage(&stable->format.format3.lookaheadGlyphCoverage[m], currentGlyph);
            
            // Current glyph does not match the lookahead glyph,
            // so try checking from next glyph.
            if (coverageIndex == UNDEFINED_INDEX)
                goto continue_parent_loop;
        }
        
        tmpPreviousIndex = index;
        
        // Loop to check backtrack glyphs.
        for (; l < stable->format.format3.backtrackGlyphCount; l++) {
            SFGlyph currentGlyph;
            
            if (!SFGetPreviousValidGlyphIndex(&tmpPreviousIndex, lookupFlag))
                goto continue_parent_loop;
            
            currentGlyph = record->charRecord[tmpPreviousIndex.recordIndex].gRec[tmpPreviousIndex.glyphIndex].glyph;
            coverageIndex = SFGetIndexOfGlyphInCoverage(&stable->format.format3.backtrackGlyphCoverage[l], currentGlyph);
            
            // Current glyph does not match the backtrack glyph,
            // so try checking from next glyph.
            if (coverageIndex == UNDEFINED_INDEX)
                goto continue_parent_loop;
        }
        
        // Finally all conditions are satisfied, so we apply substitution here.
        for (; n < stable->format.format3.posCount; n++) {
            PosLookupRecord currentRecord = stable->format.format3.PosLookupRecord[n];
            
            SFApplyGPOSLookup(gpos->lookupList.lookupTables[currentRecord.lookupListIndex], inputIndexes[currentRecord.sequenceIndex], inputIndexes[stable->format.format3.inputGlyphCount - 1]);
        }
        
        index = inputIndexes[stable->format.format3.inputGlyphCount - 1];
        
    continue_parent_loop:
        {
            //continue;
        }
    } while (SFGetNextValidGlyphIndex(&index, lookupFlag));

	free(inputIndexes);
}

static void SFApplyGPOSLookup(LookupTable lookup, SFGlyphIndex sindex, SFGlyphIndex eindex) {
	int i = 0;

    if (lookup.lookupType == ltpSingleAdjustment) {
        for (; i < lookup.subTableCount; i++)
            SFApplySingleAdjustment(lookup.subtables[i], lookup.lookupFlag, sindex, eindex);
    } else if (lookup.lookupType == ltpPairAdjustment) {
        for (; i < lookup.subTableCount; i++)
            SFApplyPairAdjustment(lookup.subtables[i], lookup.lookupFlag, sindex, eindex);
    } else if (lookup.lookupType == ltpCursiveAttachment) {
        for (; i < lookup.subTableCount; i++)
            SFApplyCursiveAttachment(lookup.subtables[i], lookup.lookupFlag, sindex, eindex);
    } else if (lookup.lookupType == ltpMarkToBaseAttachment) {
        for (; i < lookup.subTableCount; i++)
            SFApplyMarkToBaseAttachment(lookup.subtables[i], lookup.lookupFlag, sindex, eindex);
    } else if (lookup.lookupType == ltpMarkToLigatureAttachment) {
        for (; i < lookup.subTableCount; i++)
            SFApplyMarkToLigatureAttachment(lookup.subtables[i], lookup.lookupFlag, sindex, eindex);
    } else if (lookup.lookupType == ltpMarkToMarkAttachment) {
        for (; i < lookup.subTableCount; i++)
            SFApplyMarkToMarkAttachment(lookup.subtables[i], lookup.lookupFlag, sindex, eindex);
    } else if (lookup.lookupType == ltpChainedContextPositioning) {
        for (; i < lookup.subTableCount; i++)
            SFApplyChainingContextual(lookup.subtables[i], lookup.lookupFlag);
    }
}

static void SFApplyGPOSFeatureList(int featureIndex) {
    FeatureTable feature = gpos->featureList.featureRecord[featureIndex].feature;
    
    SFGlyphIndex sindex;
    SFGlyphIndex eindex;
    
    int i;
    
    sindex.recordIndex = 0;
    sindex.glyphIndex = 0;
    
    eindex.recordIndex = record->charCount - 1;
    eindex.glyphIndex = record->charRecord[eindex.recordIndex].glyphCount - 1;
    
    for (i = 0; i < feature.lookupCount; i++) {
        LookupTable currentLookup = gpos->lookupList.lookupTables[feature.lookupListIndex[i]];
        
        eindex.glyphIndex = record->charRecord[record->charCount - 1].glyphCount - 1;
        
        SFApplyGPOSLookup(currentLookup, sindex, eindex);
    }
}

static int SFGetIndexOfGPOSFeatureTag(char tag[5]) {
	int i;
    for (i = 0; i < GPOS_FEATURE_TAGS; i++) {
        if (strcmp((const char *)gposFeaturesTagOrder[i], tag) == 0)
            return i;
    }
    
    return -1;
}

void SFApplyGPOS(SFTableGPOS *gposTable, SFTableGDEF *gdefTable, SFStringRecord *strRecord) {
    SFBool arabScriptFound = SFFalse;
    int arabScriptIndex;
    
	int i;

    gpos = gposTable;
    gdef = gdefTable;
    record = strRecord;
    
    for (i = 0; i < gpos->scriptList.scriptCount; i++) {
        if (strcmp((const char *)gpos->scriptList.scriptRecord[i].scriptTag, "arab") == 0) {
            arabScriptFound = SFTrue;
            arabScriptIndex = i;
        }
    }
    
    if (arabScriptFound) {
        int totalFeatures = gpos->scriptList.scriptRecord[arabScriptIndex].script.defaultLangSys.featureCount;
        
        int order1[GPOS_FEATURE_TAGS];
        
        int order2Len = 0;
		int *order2 = malloc(sizeof(int) * totalFeatures);

        for (i = 0; i < GPOS_FEATURE_TAGS; i++)
            order1[i] = -1;
        
        for (i = 0; i < totalFeatures; i++) {
            int featureIndex = gpos->scriptList.scriptRecord[arabScriptIndex].script.defaultLangSys.featureIndex[i];
            
            int val = SFGetIndexOfGPOSFeatureTag((char *)gpos->featureList.featureRecord[featureIndex].featureTag);
            if (val > -1)
                order1[val] = featureIndex;
            else
                order2[order2Len++] = featureIndex;
        }
        
        for (i = 0; i < GPOS_FEATURE_TAGS; i++) {
            if (order1[i] > -1)
                SFApplyGPOSFeatureList(order1[i]);
        }
        
        for (i = 0; i < order2Len; i++)
            SFApplyGPOSFeatureList(order2[i]);

		free(order2);
    }
}
