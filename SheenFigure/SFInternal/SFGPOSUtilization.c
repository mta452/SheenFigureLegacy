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

#include <stdlib.h>
#include <string.h>

#include "SFGDEFUtilization.h"
#include "SFGSUBGPOSUtilization.h"
#include "SFGPOSUtilization.h"

#define GPOS_FEATURE_TAGS 4
static unsigned char gposFeaturesTagOrder[GPOS_FEATURE_TAGS][5] = {
    "curs",                         //Cursive positioning
    "kern",                         //Pair kerning
    "mark",                         //Mark to base positioning
    "mkmk"                          //Mark to mark positioning
};

static void SFApplyGPOSLookup(SFInternal *internal, LookupTable lookup, SFGlyphIndex sidx, SFGlyphIndex eidx);

#if defined(GPOS_SINGLE) || defined(GPOS_PAIR)

static void createPositioningValues(ValueRecord record, ValueFormat format, SFPosition *pos, SFPosition *advance) {
    if (format & vfXPlacement)
        pos->x = record.xPlacement;
    
    if (format & vfYPlacement)
        pos->y = record.yPlacement;
    
    if (format & vfXAdvance)
        advance->x = record.xAdvance;
    
    if (format & vfYAdvance)
        advance->y = record.yAdvance;
}


#ifdef GPOS_SINGLE

static void SFApplySingleAdjustment(SFInternal *internal, SingleAdjustmentPosSubtable *stable, LookupFlag lookupFlag, SFGlyphIndex sidx, SFGlyphIndex eidx) {
    int gidx;                   // end glyph index
    int cidx;                   // coverage index
    
    for (; ;) {
        if (sidx.record < eidx.record) {
            gidx = SFGetGlyphCount(internal, sidx.record);
        } else if (sidx.record == eidx.record) {
            gidx = eidx.glyph + 1;
        } else {
            break;
        }
        
        for (; sidx.glyph < gidx; sidx.glyph++) {
            if (SFIsIgnoredGlyph(internal, sidx, lookupFlag)) {
                continue;
            }
            
            cidx = SFGetIndexOfGlyphInCoverage(&stable->coverage, SFGetGlyph(internal, sidx));
            if (cidx != UNDEFINED_INDEX) {
                if (stable->posFormat == 1) {
                    createPositioningValues(stable->format.format1.value,
                                            stable->valueFormat,
                                            &SFGetPositionRecord(internal, sidx).placement,
                                            &SFGetPositionRecord(internal, sidx).advance);
                } else if (stable->posFormat == 2) {
                    createPositioningValues(stable->format.format2.value[cidx],
                                            stable->valueFormat,
                                            &SFGetPositionRecord(internal, sidx).placement,
                                            &SFGetPositionRecord(internal, sidx).advance);
                }
            }
        }
        
        sidx.record++;
        sidx.glyph = 0;
    }
}

#endif

#ifdef GPOS_PAIR

static int SFGetIndexOfGlyphInPairSet(PairSetTable *pairSet, SFGlyph glyph) {
	int i;
    for (i = 0; i < pairSet->pairValueCount; i++) {
        SFGlyph inputGlyph = pairSet->pairValueRecord[i].secondGlyph;
        
        if (inputGlyph == glyph) {
            return i;
        }
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

static void SFApplyPairAdjustment(SFInternal *internal, PairAdjustmentPosSubtable *stable, LookupFlag lookupFlag, SFGlyphIndex sidx, SFGlyphIndex eidx) {
    int gidx;                   // end glyph index
    int cidx;                   // coverage index
    
    for (; ;) {
        if (sidx.record < eidx.record) {
            gidx = SFGetGlyphCount(internal, sidx.record);
        } else if (sidx.record == eidx.record) {
            gidx = eidx.glyph + 1;
        } else {
            break;
        }
        
        for (; sidx.glyph < gidx; sidx.glyph++) {
            if (SFIsIgnoredGlyph(internal, sidx, lookupFlag)) {
                continue;
            }
            
            cidx = SFGetIndexOfGlyphInCoverage(&stable->coverage, SFGetGlyph(internal, sidx));
            if (cidx != UNDEFINED_INDEX) {
                SFGlyphIndex nidx = sidx;
                SFGlyph nglyph;

                if (!SFGetNextValidGlyphIndex(internal, &nidx, lookupFlag)
                    || SFCompareGlyphIndex(nidx, eidx) == 1) {
                    return;     // if there is no next glyph or if index of next glyph is greater
                                // than end index then there is no need to perform any action
                }

                nglyph = SFGetGlyph(internal, nidx);
                
                if (stable->posFormat == 1) {
                    int pairSetIndex = SFGetIndexOfGlyphInPairSet(stable->format.format1.pairSetTable, nglyph);
                    
                    if (pairSetIndex != UNDEFINED_INDEX) {
                        createPositioningValues(
                                                stable->format.format1.pairSetTable[cidx].pairValueRecord[pairSetIndex].value1,
                                                stable->valueFormat1,
                                                &SFGetPositionRecord(internal, sidx).placement,
                                                &SFGetPositionRecord(internal, sidx).advance);
                        
                        createPositioningValues(
                                                stable->format.format1.pairSetTable[cidx].pairValueRecord[pairSetIndex].value2,
                                                stable->valueFormat2,
                                                &SFGetPositionRecord(internal, nidx).placement,
                                                &SFGetPositionRecord(internal, nidx).advance);
                    }
                } else if (stable->posFormat == 2) {
                    int class1Index;
                    int class2Index;
                    
                    class2Index = SFGetClassOfGlyphInClassDef(&stable->format.format2.classDef2, nglyph);
                    if (class2Index == UNDEFINED_INDEX)
                        continue;
                    
                    class1Index = SFGetClassOfGlyphInClassDef(&stable->format.format2.classDef1, SFGetGlyph(internal, sidx));
                    
                    createPositioningValues(
                                            stable->format.format2.class1Record[class1Index].class2Record[class2Index].value1,
                                            stable->valueFormat1,
                                            &SFGetPositionRecord(internal, sidx).placement,
                                            &SFGetPositionRecord(internal, sidx).advance);
                    
                    createPositioningValues(
                                            stable->format.format2.class1Record[class1Index].class2Record[class2Index].value2,
                                            stable->valueFormat2,
                                            &SFGetPositionRecord(internal, nidx).placement,
                                            &SFGetPositionRecord(internal, nidx).advance);
                }
                
                sidx = nidx;
            }
        }
        
        sidx.record++;
        sidx.glyph = 0;
    }
}

#endif

#endif


#ifdef GPOS_CURSIVE

static void SFApplyCursiveAttachment(SFInternal *internal, CursiveAttachmentPosSubtable *stable, LookupFlag lookupFlag, SFGlyphIndex sidx, SFGlyphIndex eidx) {
    int gidx;                   // end glyph index
    int cidx;                   // coverage index
    
    SFBool hasExitAnchor = SFFalse;
    AnchorTable exitAnchor;
    
    for (; ;) {
        if (sidx.record < eidx.record) {
            gidx = SFGetGlyphCount(internal, sidx.record);
        } else if (sidx.record == eidx.record) {
            gidx = eidx.glyph + 1;
        } else {
            break;
        }
        
        for (; sidx.glyph < gidx; sidx.glyph++) {
            if (SFIsIgnoredGlyph(internal, sidx, lookupFlag) && hasExitAnchor) {
                SFGetPositionRecord(internal, sidx).anchorType |= atCursiveIgnored;
                continue;
            }
            
            cidx = SFGetIndexOfGlyphInCoverage(&stable->coverage, SFGetGlyph(internal, sidx));
            if (cidx != UNDEFINED_INDEX) {
                if (stable->entryExitRecord[cidx].hasEntryAnchor && hasExitAnchor) {
                    SFGetPositionRecord(internal, sidx).anchorType |= atEntry;
                    SFGetPositionRecord(internal, sidx).anchor.x = exitAnchor.xCoordinate - stable->entryExitRecord[cidx].entryAnchor.xCoordinate;
                    SFGetPositionRecord(internal, sidx).anchor.y = exitAnchor.yCoordinate - stable->entryExitRecord[cidx].entryAnchor.yCoordinate;
                    
                    hasExitAnchor = SFFalse;
                }
                
                if (stable->entryExitRecord[cidx].hasExitAnchor) {
                    SFGetPositionRecord(internal, sidx).anchorType |= atExit;
                    
                    hasExitAnchor = SFTrue;
                    exitAnchor = stable->entryExitRecord[cidx].exitAnchor;
                }
            } else {
                hasExitAnchor = SFFalse;
            }
        }
        
        sidx.record++;
        sidx.glyph = 0;
    }
}

#endif

#ifdef GPOS_MARK_TO_BASE

static void SFApplyMarkToBaseAttachment(SFInternal *internal, MarkToBaseAttachmentPosSubtable *stable, LookupFlag lookupFlag, SFGlyphIndex sidx, SFGlyphIndex eidx) {
    int gidx;                   // end glyph index

    for (; ;) {
        if (sidx.record < eidx.record) {
            if (SFIsOddLevel(internal, sidx.record)) {
                goto continue_loop;
            }
            
            gidx = SFGetGlyphCount(internal, sidx.record);
        } else if (sidx.record == eidx.record) {
            if (SFIsOddLevel(internal, sidx.record)) {
                break;
            }
            
            gidx = eidx.glyph + 1;
        } else {
            break;
        }

        for (; sidx.glyph < gidx; sidx.glyph++) {
            int midx = SFGetIndexOfGlyphInCoverage(&stable->markCoverage, SFGetGlyph(internal, sidx));
            if (midx != UNDEFINED_INDEX) {
                SFGlyphIndex pidx;  // previous index
                SFUShort cls;
                
                int bidx;           // base index
                int x, y;
                
                pidx = sidx;
                if (!SFGetPreviousBaseGlyphIndex(internal, &pidx, lookupFlag)) {
                    continue;
                }

                bidx = SFGetIndexOfGlyphInCoverage(&stable->baseCoverage, SFGetGlyph(internal, pidx));
                if (bidx == UNDEFINED_INDEX) {
                    continue;   // previous glyph did not match any of the base glyphs
								// listed in base coverage table, so try matching sequence
								// from next input glyph.
                }
                
                SFGetPositionRecord(internal, sidx).anchorType |= atMark;
                
                // class Index to be applied on base glyph.
                cls = stable->markArray.markRecord[midx].cls;
                
                x = stable->markArray.markRecord[midx].markAnchor.xCoordinate - stable->baseArray.baseRecord[bidx].baseAnchor[cls].xCoordinate;
                if (x) {
                    SFGetPositionRecord(internal, sidx).anchor.x = x;
                }
                
                y = stable->markArray.markRecord[midx].markAnchor.yCoordinate - stable->baseArray.baseRecord[bidx].baseAnchor[cls].yCoordinate;
                SFGetPositionRecord(internal, sidx).anchor.y = y;
            }
        }
        
    continue_loop:
        sidx.record++;
        sidx.glyph = 0;
    }
}

#endif

#ifdef GPOS_MARK_TO_LIGATURE

static void SFApplyMarkToLigatureAttachment(SFInternal *internal, MarkToLigatureAttachmentPosSubtable *stable, LookupFlag lookupFlag, SFGlyphIndex sidx, SFGlyphIndex eidx) {
    int gidx;                   // end glyph index
    
    for (; ;) {
        if (sidx.record < eidx.record) {
            if (SFIsOddLevel(internal, sidx.record)) {
                goto continue_loop;
            }
            
            gidx = SFGetGlyphCount(internal, sidx.record);
        } else if (sidx.record == eidx.record) {
            if (SFIsOddLevel(internal, sidx.record)) {
                break;
            }
            
            gidx = eidx.glyph + 1;
        } else {
            break;
        }
        
        for (; sidx.glyph < gidx; sidx.glyph++) {
            int markIndex = SFGetIndexOfGlyphInCoverage(&stable->markCoverage, SFGetGlyph(internal, sidx));
            
            if (markIndex != UNDEFINED_INDEX) {
                SFGlyphIndex pidx;  // previous index
                SFUShort cls;
                int component;

                LigatureAttachTable *ligAttach;
                
                int lidx;           // ligature index
                int x, y;
                
                pidx = sidx;
                component = 0;
                
                if (!SFGetPreviousLigatureGlyphIndex(internal, &pidx, lookupFlag, &component)) {
                    continue;
                }

                lidx = SFGetIndexOfGlyphInCoverage(&stable->ligatureCoverage, SFGetGlyph(internal, pidx));
                ligAttach = &stable->ligatureArray.ligatureAttach[lidx];
                
                if (lidx == UNDEFINED_INDEX || component >= ligAttach->componentCount) {
                    continue;   // Previous glyph did not match any of the ligature glyphs
								// listed in base coverage table, so try matching sequence
								// from next input glyph.
                }
                
                SFGetPositionRecord(internal, sidx).anchorType = atMark;
                
                // class Index to be applied on base glyph.
                cls = stable->markArray.markRecord[markIndex].cls;
                
                x = stable->markArray.markRecord[markIndex].markAnchor.xCoordinate - ligAttach->componentRecord[component].ligatureAnchor[cls].xCoordinate;
                if (x) {
                    SFGetPositionRecord(internal, sidx).anchor.x = x;
                }
                
                y = stable->markArray.markRecord[markIndex].markAnchor.yCoordinate - ligAttach->componentRecord[component].ligatureAnchor[cls].yCoordinate;
                SFGetPositionRecord(internal, sidx).anchor.y = y;
            }
        }
        
    continue_loop:
        sidx.record++;
        sidx.glyph = 0;
    }
}

#endif

#ifdef GPOS_MARK_TO_MARK

static void SFApplyMarkToMarkAttachment(SFInternal *internal, MarkToMarkAttachmentPosSubtable *stable, LookupFlag lookupFlag, SFGlyphIndex sidx, SFGlyphIndex eidx) {
    int gidx;                   // end glyph index
    
    for (; ;) {
        if (sidx.record < eidx.record) {
            if (SFIsOddLevel(internal, sidx.record)) {
                goto continue_loop;
            }
            
            gidx = SFGetGlyphCount(internal, sidx.record);
        } else if (sidx.record == eidx.record) {
            if (SFIsOddLevel(internal, sidx.record)) {
                break;
            }
            
            gidx = eidx.glyph + 1;
        } else {
            break;
        }
        
        for (; sidx.glyph < gidx; sidx.glyph++) {
            int mark1Index = SFGetIndexOfGlyphInCoverage(&stable->mark1Coverage, SFGetGlyph(internal, sidx));
            
            if (mark1Index != UNDEFINED_INDEX) {
                SFGlyphIndex pidx;
                SFUShort cls;
                
                int mark2Index;
                int x, y;
                
                pidx = sidx;
                if (!SFGetPreviousGlyphIndex(internal, &pidx, lookupFlag) || !SFGetGlyph(internal, pidx)) {
                    continue;
                }

                mark2Index = SFGetIndexOfGlyphInCoverage(&stable->mark2Coverage, SFGetGlyph(internal, pidx));
                if (mark2Index == UNDEFINED_INDEX) {
                    continue;   // Previous glyph did not match any of the base glyphs
								// listed in base coverage table, so try matching sequence
								// from next input glyph.
                }
                
                SFGetPositionRecord(internal, sidx).anchorType = atMark;
                
                // class Index to be applied on first mark glyph.
                cls = stable->mark1Array.markRecord[mark1Index].cls;
                
                x = stable->mark1Array.markRecord[mark1Index].markAnchor.xCoordinate - stable->mark2Array.mark2Record[mark2Index].mark2Anchor[cls].xCoordinate;
                if (x) {
                    SFGetPositionRecord(internal, sidx).anchor.x = x - SFGetPositionRecord(internal, pidx).anchor.x;
                }
                
                y = stable->mark1Array.markRecord[mark1Index].markAnchor.yCoordinate - stable->mark2Array.mark2Record[mark2Index].mark2Anchor[cls].yCoordinate;
                SFGetPositionRecord(internal, sidx).anchor.y = y + SFGetPositionRecord(internal, pidx).anchor.y;
            }
        }
        
    continue_loop:
        sidx.record++;
        sidx.glyph = 0;
    }
}

#endif

static void SFApplyGPOSLookup(SFInternal *internal, LookupTable lookup, SFGlyphIndex sidx, SFGlyphIndex eidx) {
	int i = 0;

#ifdef GPOS_SINGLE
    
    if (lookup.lookupType == ltpSingleAdjustment) {
        for (; i < lookup.subTableCount; i++)
            SFApplySingleAdjustment(internal, lookup.subtables[i], lookup.lookupFlag, sidx, eidx);
    }
    
#define GPOS_ELSE
    
#endif
    
#ifdef GPOS_PAIR
    
#ifdef GPOS_ELSE
    else
#else
#define GPOS_ELSE
#endif
        
     if (lookup.lookupType == ltpPairAdjustment) {
        for (; i < lookup.subTableCount; i++)
            SFApplyPairAdjustment(internal, lookup.subtables[i], lookup.lookupFlag, sidx, eidx);
    }
    
#endif
    
#ifdef GPOS_CURSIVE
    
#ifdef GPOS_ELSE
     else
#else
#define GPOS_ELSE
#endif
             
    if (lookup.lookupType == ltpCursiveAttachment) {
        for (; i < lookup.subTableCount; i++)
            SFApplyCursiveAttachment(internal, lookup.subtables[i], lookup.lookupFlag, sidx, eidx);
    }
    
#endif
    
#ifdef GPOS_MARK_TO_BASE
    
#ifdef GPOS_ELSE
    else
#else
#define GPOS_ELSE
#endif
        
    if (lookup.lookupType == ltpMarkToBaseAttachment) {
        for (; i < lookup.subTableCount; i++)
            SFApplyMarkToBaseAttachment(internal, lookup.subtables[i], lookup.lookupFlag, sidx, eidx);
    }
    
#endif
    
#ifdef GPOS_MARK_TO_LIGATURE
    
#ifdef GPOS_ELSE
    else
#else
#define GPOS_ELSE
#endif
    
    if (lookup.lookupType == ltpMarkToLigatureAttachment) {
        for (; i < lookup.subTableCount; i++)
            SFApplyMarkToLigatureAttachment(internal, lookup.subtables[i], lookup.lookupFlag, sidx, eidx);
    }
    
#endif

#ifdef GPOS_MARK_TO_MARK
    
#ifdef GPOS_ELSE
    else
#else
#define GPOS_ELSE
#endif
        
    if (lookup.lookupType == ltpMarkToMarkAttachment) {
        for (; i < lookup.subTableCount; i++)
            SFApplyMarkToMarkAttachment(internal, lookup.subtables[i], lookup.lookupFlag, sidx, eidx);
    }
    
#endif
    
#ifdef GSUB_GPOS_CHAINING_CONTEXT
    
#ifdef GPOS_ELSE
    else
#else
#define GPOS_ELSE
#endif

    if (lookup.lookupType == ltpChainedContextPositioning) {
        for (; i < lookup.subTableCount; i++)
            SFApplyChainingContextual(internal, lookup.subtables[i], lookup.lookupFlag, &SFApplyGPOSLookup);
    }
    
#endif
    
#ifdef GSUB_ELSE
#undef GSUB_ELSE
#endif
}

static void SFApplyGPOSFeatureList(SFInternal *internal, int featureIndex) {
    FeatureTable feature = internal->gpos->featureList.featureRecord[featureIndex].feature;
    
    SFGlyphIndex sidx;
    SFGlyphIndex eidx;
    
    int i;
    
    sidx.record = 0;
    sidx.glyph = 0;
    
    eidx.record = SFGetCharCount(internal) - 1;
    eidx.glyph = SFGetGlyphCount(internal, eidx.record) - 1;
    
    for (i = 0; i < feature.lookupCount; i++) {
        LookupTable currentLookup = internal->gpos->lookupList.lookupTables[feature.lookupListIndex[i]];
        SFApplyGPOSLookup(internal, currentLookup, sidx, eidx);
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

void SFApplyGPOS(SFInternal *internal) {
    SFBool arabScriptFound = SFFalse;
    int arabScriptIndex;
    
	int i;
    
    for (i = 0; i < internal->gpos->scriptList.scriptCount; i++) {
        if (strcmp((const char *)internal->gpos->scriptList.scriptRecord[i].scriptTag, "arab") == 0) {
            arabScriptFound = SFTrue;
            arabScriptIndex = i;
        }
    }
    
    if (arabScriptFound) {
        int totalFeatures = internal->gpos->scriptList.scriptRecord[arabScriptIndex].script.defaultLangSys.featureCount;
        
        int order1[GPOS_FEATURE_TAGS];
        
        int order2Len = 0;
		int *order2 = malloc(sizeof(int) * totalFeatures);

        for (i = 0; i < GPOS_FEATURE_TAGS; i++)
            order1[i] = -1;
        
        for (i = 0; i < totalFeatures; i++) {
            int featureIndex = internal->gpos->scriptList.scriptRecord[arabScriptIndex].script.defaultLangSys.featureIndex[i];
            
            int val = SFGetIndexOfGPOSFeatureTag((char *)internal->gpos->featureList.featureRecord[featureIndex].featureTag);
            if (val > -1)
                order1[val] = featureIndex;
            else
                order2[order2Len++] = featureIndex;
        }
        
        for (i = 0; i < GPOS_FEATURE_TAGS; i++) {
            if (order1[i] > -1)
                SFApplyGPOSFeatureList(internal, order1[i]);
        }
        
        for (i = 0; i < order2Len; i++)
            SFApplyGPOSFeatureList(internal, order2[i]);

		free(order2);
    }
}
