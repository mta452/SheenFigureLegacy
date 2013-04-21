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

#include "SFGDEFUtilization.h"
#include "SFGSUBGPOSUtilization.h"

#ifdef GSUB_GPOS_CHAINING_CONTEXT

void SFApplyChainingContextual(SFInternal *internal, ChainingContextualSubPosSubtable *stable, LookupFlag lookupFlag, SFApplyLookupFunction applyLookupFunction) {
    SFGlyphIndex pidx;              // previous index
    SFGlyphIndex cidx;              // current index
    SFGlyphIndex nidx;              // next index
    
	SFGlyphIndex *inputIndexes;
    
    if (stable->subPosFormat != 3)
        return;
    
    cidx = SFMakeGlyphIndex(0, 0);
    if (SFIsIgnoredGlyph(internal, cidx, lookupFlag)) {
        if (!SFGetNextValidGlyphIndex(internal, &cidx, lookupFlag))
            return;
    }
    
    inputIndexes = malloc(sizeof(SFGlyphIndex) * stable->format.format3.inputGlyphCount);
    
    do {
        int coverageIndex;

        int i, j;
        
        coverageIndex = SFGetIndexOfGlyphInCoverage(&stable->format.format3.inputGlyphCoverage[0], SFGetGlyph(internal, cidx));
        if (coverageIndex == UNDEFINED_INDEX) {
            continue;
        }
        
        inputIndexes[0] = cidx;
        
        // loop to check input glyphs.
        for (i = 1, j = 0; i < stable->format.format3.inputGlyphCount; i++) {
            SFGlyphIndex tmpIndex;
            SFGlyph currentGlyph;
            
            tmpIndex = inputIndexes[j];
            
            // checking if we have another glyph (except ignored glyphs) to match
            if (!SFGetNextValidGlyphIndex(internal, &tmpIndex, lookupFlag)) {
                goto exit;
            }
            
            currentGlyph = SFGetGlyph(internal, tmpIndex);
            coverageIndex = SFGetIndexOfGlyphInCoverage(&stable->format.format3.inputGlyphCoverage[i], currentGlyph);
            
            inputIndexes[++j] = tmpIndex;
            
            // current glyph does not match the input glyph,
            // so try checking from next glyph.
            if (coverageIndex == UNDEFINED_INDEX)
                goto continue_parent_loop;
        }
        
        nidx = cidx;
        
        // loop to check lookahead glyphs.
        for (i = 0; i < stable->format.format3.lookaheadGlyphCount; i++) {
            SFGlyph currentGlyph;
            
            if (!SFGetNextValidGlyphIndex(internal, &nidx, lookupFlag)) {
                goto exit;
            }
            
            currentGlyph = SFGetGlyph(internal, nidx);
            coverageIndex = SFGetIndexOfGlyphInCoverage(&stable->format.format3.lookaheadGlyphCoverage[i], currentGlyph);
            
            // current glyph does not match the lookahead glyph,
            // so try checking from next glyph.
            if (coverageIndex == UNDEFINED_INDEX)
                goto continue_parent_loop;
        }
        
        pidx = cidx;
        
        // loop to check backtrack glyphs.
        for (i = 0; i < stable->format.format3.backtrackGlyphCount; i++) {
            SFGlyph currentGlyph;
            
            if (!SFGetPreviousValidGlyphIndex(internal, &pidx, lookupFlag))
                goto continue_parent_loop;
            
            currentGlyph = SFGetGlyph(internal, pidx);
            coverageIndex = SFGetIndexOfGlyphInCoverage(&stable->format.format3.backtrackGlyphCoverage[i], currentGlyph);
            
            // current glyph does not match the backtrack glyph,
            // so try checking from next glyph.
            if (coverageIndex == UNDEFINED_INDEX)
                goto continue_parent_loop;
        }
        
        // finally all conditions are satisfied, so we apply substitution/positioning here.
        for (i = 0; i < stable->format.format3.subPosCount; i++) {
            SubPosLookupRecord currentRecord = stable->format.format3.subPosLookupRecord[i];
            
            (*applyLookupFunction)(internal, internal->gsub->lookupList.lookupTables[currentRecord.lookupListIndex], inputIndexes[currentRecord.sequenceIndex], inputIndexes[stable->format.format3.inputGlyphCount - 1]);
        }
        
        cidx = inputIndexes[stable->format.format3.inputGlyphCount - 1];
        
    continue_parent_loop:
        {
            //continue;
        }
    } while (SFGetNextValidGlyphIndex(internal, &cidx, lookupFlag));
    
exit:
	free(inputIndexes);
}

#endif
