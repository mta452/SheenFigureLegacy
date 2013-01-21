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

#include "ssunistr.h"
#include "SFTypes.h"
#include "SFGlobal.h"
#include "SFCommonMethods.h"
#include "SFGDEFUtilization.h"
#include "SFGSUBUtilization.h"

#define FOUR_PART_URDU_LETTERS 34
static SFUnichar fourPartUrduLetters[FOUR_PART_URDU_LETTERS] =
{
	0x0640,				//ـ
    0x0628,             //ب
    0x067E,             //پ
    0x062A,             //ت
    0x0679,             //ٹ
    0x062B,             //ث
    0x062C,             //ج
    0x0686,             //چ
    0x062D,             //ح
    0x062E,             //خ
    0x0633,             //س
    0x0634,             //ش
    0x0635,             //ص
    0x0636,             //ض
    0x0637,             //ط
    0x0638,             //ظ
    0x0639,             //ع
    0x063A,             //غ
    0x0641,             //ف
    0x0642,             //ق
    0x06A9,             //ک
    0x06AF,             //گ
    0x0643,             //Arabic kaf
    0x0644,             //ل
    0x0645,             //م
    0x0646,             //ن
    0x0647,             //Arabic Gol Heh
    0x06BE,             //ھ
    0x06C1,             //ہ
    0x06C2,             //ۂ
    0x0626,             //ئ
    0x06CC,             //ی
	0x0649,				//ى
    0x064A,             //ي
};

#define TWO_PART_URDU_LETTERS 16
static SFUnichar twoPartUrduLetters[TWO_PART_URDU_LETTERS] =
{
    0x0622,             //آ
    0x0623,             //أ
    0x0625,             //إ
    0x0627,             //ا
    0x062F,             //د
    0x0688,             //ڈ
    0x0630,             //ذ
    0x0631,             //ر
    0x0691,             //ڑ
    0x0632,             //ز
    0x0698,             //ژ
    0x06BA,             //ں
    0x0648,             //و
    0x0624,             //ؤ
    0x06C3,             //ۃ
    0x06D2,             //ے
};

#define GSUB_FEATURE_TAGS 8
static unsigned char gsubFeaturesTagOrder[GSUB_FEATURE_TAGS][5] = {
    "ccmp",
    "init",
    "medi",
    "fina",
    "isol",
    "rlig",
    "calt",
    "liga",
};

typedef enum {
    stInit,
    stMedi,
    stFina,
    stOther,
} ShortTag;

static void SFApplyGSUBLookup(LookupTable lookup, unsigned char featureTag[5], SFGlyphIndex sindex, SFGlyphIndex eindex);

static ShortTag getShortTag(const unsigned char tag[5]) {
    ShortTag stag = stOther;
    if (tag) {
        if (tag[0] == 'i' && tag[1] == 'n' && tag[2] == 'i' && tag[3] == 't')
            stag = stInit;
        else if (tag[0] == 'm' && tag[1] == 'e' && tag[2] == 'd' && tag[3] == 'i')
            stag = stMedi;
        else if (tag[0] == 'f' && tag[1] == 'i' && tag[2] == 'n' && tag[3] == 'a')
            stag = stFina;
    }
    
    return stag;
}

static void applyArabicSubstitution(void *stable, LookupFlag lookupFlag, LookupType type, short stag) {
    CoverageTable coverage;
    
    SingleSubstSubtable *singleSubst;
    MultipleSubstSubtable *multipleSubst;
    AlternateSubstSubtable *alternateSubst;
    
    //Index of next letter in twoPartUrduLetters.
    int inlt;
    
    //Index of previous, current and next letter in fourPartUrduLetters.
    int iplf = -1;
    int iclf;
    int inlf;
    
    int currentIndex;
    int nextIndex;
    int lastIndex = record->charCount - 1;
    
    int i = 0;
    int ni = 0;
    
    if (type == ltsSingle) {
        singleSubst = (SingleSubstSubtable *)stable;
        coverage = singleSubst->coverage;
    } else if (type == ltsMultiple) {
        multipleSubst = (MultipleSubstSubtable *)stable;
        coverage = multipleSubst->coverage;
    } else if (type == ltsAlternate) {
        alternateSubst = (AlternateSubstSubtable *)stable;
        coverage = alternateSubst->coverage;
    } else
        return;
    
#define SET_NULL_VALUES()           \
inlt = -1;                      \
inlf = -1;                      \
nextIndex = UNDEFINED_INDEX;
    
#define GET_NEXT_VALUES()                                                                       \
for (; ni < record->charCount; ni++) {                                                      \
if (!SFIsIgnoredGlyph(ni, 0, lookupFlag)) {        \
break;                                                                              \
}                                                                                       \
}                                                                                           \
\
if (ni >= record->charCount) {                                                              \
SET_NULL_VALUES();                                                                      \
} else {                                                                                    \
inlt = idxunichar(twoPartUrduLetters, record->chars[ni], 0, TWO_PART_URDU_LETTERS);     \
inlf = idxunichar(fourPartUrduLetters, record->chars[ni], 0, FOUR_PART_URDU_LETTERS);   \
nextIndex = SFGetIndexOfGlyphInCoverage(&coverage, record->charRecord[ni].gRec[0].glyph);\
}
    
    GET_NEXT_VALUES();
    i = ni;
    
    for (; i < record->charCount;) {
        iclf = inlf;
        currentIndex = nextIndex;
        ni++;
        
        if (i == lastIndex) {
            SET_NULL_VALUES();
        } else {
            GET_NEXT_VALUES();
        }
        
        if (currentIndex != UNDEFINED_INDEX) {
            if ((stag == stInit && !(iplf == -1 && (inlt > -1 || inlf > -1)))
                || (stag == stMedi && !(iplf > -1 && (inlt > -1 || inlf > -1)))
                || (stag == stFina && !(iplf > -1 && (iclf == -1 ? 1 : inlf == -1))))
                goto continue_loop;
            
            if (type == ltsSingle) {
                if (singleSubst->substFormat == 1)
                    record->charRecord[i].gRec[0].glyph += singleSubst->format.format1.deltaGlyphID;
                else if (singleSubst->substFormat == 2)
                    record->charRecord[i].gRec[0].glyph = singleSubst->format.format2.substitute[currentIndex];
                
                record->charRecord[i].gRec[0].glyphProp = gpNotReceived;
            } else if (type == ltsMultiple) {
                if (multipleSubst->sequence[currentIndex].glyphCount == 1) {
                    record->charRecord[i].gRec[0].glyph = multipleSubst->sequence[currentIndex].substitute[0];
                    
                    record->charRecord[i].gRec[0].glyphProp = gpNotReceived;
                } else {
                    int length;
                    
                    SFInsertGlyphs(&record->charRecord[i].gRec, multipleSubst->sequence[currentIndex].substitute, 1, multipleSubst->sequence[currentIndex].glyphCount, 0, &length);
                    
                    record->glyphCount += length - record->charRecord[i].glyphCount;
                    record->charRecord[i].glyphCount = length;
                }
            } else if (type == ltsAlternate) {
                record->charRecord[i].gRec[0].glyph = alternateSubst->alternateSet[currentIndex].substitute[0];
                
                record->charRecord[i].gRec[0].glyphProp = gpNotReceived;
            }
        }
        
    continue_loop:
        iplf = iclf;
        i = ni;
    }
    
#undef SET_NULL_VALUES
#undef GET_NEXT_VALUES
}

static void SFApplySingleSubstitution(SingleSubstSubtable *stable, LookupFlag lookupFlag, const unsigned char tag[5], SFGlyphIndex sindex, SFGlyphIndex eindex) {
    ShortTag stag = getShortTag(tag);
    if (stag == stOther) {
        int currentIndex;
        int endGlyphIndex;

        int endRecordIndex = eindex.recordIndex + 1;

		int i = sindex.recordIndex;
        int j = sindex.glyphIndex;

        for (; i < endRecordIndex; i++) {
            if (odd(record->levels[i]))
                goto continue_loop;
            
            if (i == (endRecordIndex - 1))
                endGlyphIndex = eindex.glyphIndex + 1;
            else
                endGlyphIndex = record->charRecord[i].glyphCount;
            
            for (; j < endGlyphIndex; j++) {
                if (SFIsIgnoredGlyph(i, j, lookupFlag))
                    continue;
                
                currentIndex = SFGetIndexOfGlyphInCoverage(&stable->coverage, record->charRecord[i].gRec[j].glyph);
                
                if (currentIndex != UNDEFINED_INDEX) {
                    if (stable->substFormat == 1)
                        record->charRecord[i].gRec[j].glyph += stable->format.format1.deltaGlyphID;
                    else if (stable->substFormat == 2)
                        record->charRecord[i].gRec[j].glyph = stable->format.format2.substitute[currentIndex];
                    
                    record->charRecord[i].gRec[j].glyphProp = gpNotReceived;
                }
            }
            
        continue_loop:
            j = 0;
        }
    } else
        applyArabicSubstitution(stable, lookupFlag | lfIgnoreMarks, ltsSingle, stag);
}

static void SFApplyMultipleSubstitution(MultipleSubstSubtable *stable, LookupFlag lookupFlag, unsigned char tag[5], SFGlyphIndex sindex, SFGlyphIndex eindex) {
    ShortTag stag;
    
    if (record->charCount == 0)
        return;
    
    stag = getShortTag(tag);
    if (stag == stOther) {
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
                int currentIndex;
                
                if (SFIsIgnoredGlyph(i, j, lookupFlag))
                    continue;
                
                currentIndex = SFGetIndexOfGlyphInCoverage(&stable->coverage, record->charRecord[i].gRec[j].glyph);
                if (currentIndex != UNDEFINED_INDEX) {
                    if (stable->sequence[currentIndex].glyphCount == 1) {
                        record->charRecord[i].gRec[j].glyph = stable->sequence[currentIndex].substitute[0];
                        
                        record->charRecord[i].gRec[j].glyphProp = gpNotReceived;
                    } else {
                        int length;
                        SFInsertGlyphs(&record->charRecord[i].gRec, stable->sequence[currentIndex].substitute, record->charRecord[i].glyphCount, stable->sequence[currentIndex].glyphCount, j, &length);
                        
                        record->glyphCount += length - record->charRecord[i].glyphCount;
                        record->charRecord[i].glyphCount = length;
                    }
                }
            }
            
        continue_loop:
            j = 0;
        }
    } else
        applyArabicSubstitution(stable, lookupFlag | lfIgnoreMarks, ltsMultiple, stag);
}

static void SFApplyAlternateSubstitution(AlternateSubstSubtable *stable, LookupFlag lookupFlag, const unsigned char tag[5], SFGlyphIndex sindex, SFGlyphIndex eindex) {
    ShortTag stag = getShortTag(tag);
    if (stag == stOther) {
        int currentIndex;
		int endGlyphIndex;
        
        int endRecordIndex = eindex.recordIndex + 1;

		int i = sindex.recordIndex;
        int j = sindex.glyphIndex;

        for (; i < endRecordIndex; i++) {
            if (odd(record->levels[i]))
                goto continue_loop;
            
            if (i == (endRecordIndex - 1))
                endGlyphIndex = eindex.glyphIndex + 1;
            else
                endGlyphIndex = record->charRecord[i].glyphCount;
            
            for (; j < endGlyphIndex; j++) {
                if (SFIsIgnoredGlyph(i, j, lookupFlag))
                    continue;
                
                currentIndex = SFGetIndexOfGlyphInCoverage(&stable->coverage, record->charRecord[i].gRec[j].glyph);
                
                if (currentIndex != UNDEFINED_INDEX)
                    record->charRecord[i].gRec[j].glyph = stable->alternateSet[currentIndex].substitute[0];
            }
            
        continue_loop:
            j = 0;
        }
    } else
        applyArabicSubstitution(stable, lookupFlag, ltsAlternate, stag);
}

static void SFApplyLigatureSubstitution(LigatureSubstSubtable *stable, LookupFlag lookupFlag, SFGlyphIndex sindex, SFGlyphIndex eindex) {
    SFGlyphIndex index = sindex;
    if (SFIsIgnoredGlyph(index.recordIndex, index.glyphIndex, lookupFlag)) {
        if (!SFGetNextValidGlyphIndex(&index, lookupFlag))
            return;
    }
    
    do {
        int coverageIndex;
        LigatureSetTable currentLigSet;
        
        int i;
        
        coverageIndex = SFGetIndexOfGlyphInCoverage(&stable->coverage, record->charRecord[index.recordIndex].gRec[index.glyphIndex].glyph);
        
        if (coverageIndex == UNDEFINED_INDEX)
            continue;
        
        currentLigSet = stable->ligatureSet[coverageIndex];
        
        //Loop to match any of the ligature table with current glyphs.
        for (i = 0; i < currentLigSet.ligatureCount; i++) {
			SFGlyphIndex *inputIndexes = malloc(sizeof(SFGlyphIndex) * currentLigSet.ligature[i].compCount);

            int inputIndexesCount;
            int j = 1, k = 0, l = 1;
            
            inputIndexesCount = currentLigSet.ligature[i].compCount;
            inputIndexes[0] = index;
            
            //Loop to match next required glyphs.
            for (; j < currentLigSet.ligature[i].compCount; j++) {
                SFGlyphIndex tmpIndex;
                SFGlyph nextGlyph;
                
                tmpIndex = inputIndexes[k];
                if (!SFGetNextValidGlyphIndex(&tmpIndex, lookupFlag))
                    goto continue_parent_loop;
                
                nextGlyph = record->charRecord[tmpIndex.recordIndex].gRec[tmpIndex.glyphIndex].glyph;
                if (nextGlyph != currentLigSet.ligature[i].component[j])
                    goto continue_parent_loop;
                
                inputIndexes[++k] = tmpIndex;
            }
            
            //Now apply substitution
            record->charRecord[index.recordIndex].gRec[index.glyphIndex].glyph = currentLigSet.ligature[i].ligGlyph;
            record->charRecord[index.recordIndex].gRec[index.glyphIndex].glyphProp = gpNotReceived;
            
            for (; l < inputIndexesCount; l++) {
                record->charRecord[inputIndexes[l].recordIndex].gRec[inputIndexes[l].glyphIndex].glyph = 0;
                record->glyphCount -= 1;
            }
            
            index = inputIndexes[l - 1];

			free(inputIndexes);
            break;
            
        continue_parent_loop:
            {
				free(inputIndexes);
            }
        }
    } while (SFGetNextValidGlyphIndex(&index, lookupFlag)
             && SFCompareGlyphIndex(index, eindex) <= 0);
}


static void SFApplyChainingContextualSubstitution(ChainingContextualSubstSubtable *stable, LookupFlag lookupFlag) {
    SFGlyphIndex index;
	SFGlyphIndex *inputIndexes = malloc(sizeof(SFGlyphIndex) * stable->format.format3.inputGlyphCount);
    
    if (stable->substFormat != 3)
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
        for (; n < stable->format.format3.substCount; n++) {
            SubstLookupRecord currentRecord = stable->format.format3.SubstLookupRecord[n];
            
            SFApplyGSUBLookup(gsub->lookupList.lookupTables[currentRecord.lookupListIndex], NULL, inputIndexes[currentRecord.sequenceIndex], inputIndexes[stable->format.format3.inputGlyphCount - 1]);
        }
        
        index = inputIndexes[stable->format.format3.inputGlyphCount - 1];
        
    continue_parent_loop:
        {
            //continue;
        }
        
    } while (SFGetNextValidGlyphIndex(&index, lookupFlag));

	free(inputIndexes);
}

static void SFApplyGSUBLookup(LookupTable lookup, unsigned char featureTag[5], SFGlyphIndex sindex, SFGlyphIndex eindex) {
	int i = 0;

    if (lookup.lookupType == ltsSingle) {
        for (; i < lookup.subTableCount; i++)
            SFApplySingleSubstitution(lookup.subtables[i], lookup.lookupFlag, featureTag, sindex, eindex);
    } else if (lookup.lookupType == ltsAlternate) {
        for (; i < lookup.subTableCount; i++)
            SFApplyAlternateSubstitution(lookup.subtables[i], lookup.lookupFlag, featureTag, sindex, eindex);
    } else if (lookup.lookupType == ltsMultiple) {
        for (; i < lookup.subTableCount; i++)
            SFApplyMultipleSubstitution(lookup.subtables[i], lookup.lookupFlag, featureTag, sindex, eindex);
    } else if (lookup.lookupType == ltsLigature) {
        for (; i < lookup.subTableCount; i++)
            SFApplyLigatureSubstitution(lookup.subtables[i], lookup.lookupFlag, sindex, eindex);
    } else if (lookup.lookupType == ltsChainingContext) {
        for (; i < lookup.subTableCount; i++)
            SFApplyChainingContextualSubstitution(lookup.subtables[i], lookup.lookupFlag);
    }
}

static void SFApplyGSUBFeatureList(int featureIndex) {
    FeatureTable feature = gsub->featureList.featureRecord[featureIndex].feature;
    
    SFGlyphIndex sindex;
    SFGlyphIndex eindex;
    
	int i;

    sindex.recordIndex = 0;
    sindex.glyphIndex = 0;
    
    eindex.recordIndex = record->charCount - 1;
    eindex.glyphIndex = record->charRecord[eindex.recordIndex].glyphCount - 1;
    
    for (i = 0; i < feature.lookupCount; i++) {
        LookupTable currentLookup = gsub->lookupList.lookupTables[feature.lookupListIndex[i]];
        
        SFApplyGSUBLookup(currentLookup, gsub->featureList.featureRecord[featureIndex].featureTag, sindex, eindex);
    }
}

static int getIndexOfGSUBFeatureTag(char tag[5]) {
	int i;
    for (i = 0; i < GSUB_FEATURE_TAGS; i++) {
        if (strcmp((const char *)gsubFeaturesTagOrder[i], tag) == 0)
            return i;
    }
    
    return -1;
}

void SFApplyGSUB(SFTableGSUB *gsubTable, SFTableGDEF *gdefTable, SFStringRecord *strRecord) {
    SFBool arabScriptFound = SFFalse;
    int arabScriptIndex;
    
	int i;

    gsub = gsubTable;
    gdef = gdefTable;
    record = strRecord;
    record->glyphCount = record->charCount;
    
    for (i = 0; i < gsub->scriptList.scriptCount; i++) {
        if (strcmp((const char *)gsub->scriptList.scriptRecord[i].scriptTag, "arab") == 0) {
            arabScriptFound = SFTrue;
            arabScriptIndex = i;
        }
    }
    
    if (arabScriptFound) {
        int totalFeatures = gsub->scriptList.scriptRecord[arabScriptIndex].script.defaultLangSys.featureCount;
        
        int order1[GSUB_FEATURE_TAGS];
        
        int order2Len = 0;
		int *order2 = malloc(sizeof(int) * totalFeatures);

        for (i = 0; i < GSUB_FEATURE_TAGS; i++)
            order1[i] = -1;
        
        for (i = 0; i < totalFeatures; i++) {
            int featureIndex = gsub->scriptList.scriptRecord[arabScriptIndex].script.defaultLangSys.featureIndex[i];
            
            int val = getIndexOfGSUBFeatureTag((char *)gsub->featureList.featureRecord[featureIndex].featureTag);
            if (val > -1)
                order1[val] = featureIndex;
            else
                order2[order2Len++] = featureIndex;
        }
        
        for (i = 0; i < GSUB_FEATURE_TAGS; i++) {
            if (order1[i] > -1)
                SFApplyGSUBFeatureList(order1[i]);
        }
        
		free(order2);

        /*for (int i = 0; i < record->charCount; i++) {
         for (int j = 0; j < record->charRecord[i].glyphCount; j++)
         memset(&record->charRecord[i].gRec[j].posRec, 0, sizeof(SFPositionRecord));
         }*/
        
        //for (int i = 0; i < order2Len; i++)
        //    applyGSUBFeatureList(order2[i]);
    }
}

