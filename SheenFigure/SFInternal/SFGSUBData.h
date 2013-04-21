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

#ifndef _SF_GSUB_DATA_H
#define _SF_GSUB_DATA_H

#include "SFConfig.h"
#include "SFTypes.h"
#include "SFCommonData.h"

#ifdef GSUB_SINGLE

typedef struct SingleSubstSubtable {
    SFUShort substFormat;           //Format identifier-format = 1
    CoverageTable coverage;         //Offset to Coverage table-from beginning of Substitution table
    
    union {
        
#ifdef GSUB_SINGLE_FORMAT1
        struct {
            int deltaGlyphID;       //Add to original GlyphID to get substitute GlyphID
        } format1;
#endif
        
#ifdef GSUB_SINGLE_FORMAT2
        struct {
            SFUShort glyphCount;    //Number of GlyphIDs in the Substitute array
            SFGlyph *substitute;    //Array of substitute GlyphIDs-ordered by Coverage Index
        } format2;
#endif
    } format;
    
} SingleSubstSubtable;

#endif


#ifdef GSUB_MULTIPLE

typedef struct SequenceTable {
    SFUShort glyphCount;            //Number of GlyphIDs in the Substitute array.
                                    //This should always be greater than 0.
    SFGlyph *substitute;            //String of GlyphIDs to substitute
} SequenceTable;

typedef struct MultipleSubstSubtable {
    SFUShort substFormat;           //Format identifier-format = 2
    CoverageTable coverage;         //Offset to Coverage table-from beginning of Substitution table
    SFUShort sequenceCount;         //Number of Sequence table offsets in the Sequence array
    SequenceTable *sequence;        //Array of offsets to Sequence tables-from beginning of
                                    //Substitution table-ordered by Coverage Index
} MultipleSubstSubtable;

#endif


#ifdef GSUB_ALTERNATE

typedef struct AlternateSetTable {
    SFUShort glyphCount;            //Number of GlyphIDs in the Alternate array
    SFGlyph *substitute;            //Array of alternate GlyphIDs-in arbitrary order
} AlternateSetTable;

typedef struct AlternateSubstSubtable {
    SFUShort substFormat;           //Format identifier-format = 1
    CoverageTable coverage;         //Offset to Coverage table-from beginning of Substitution table
    SFUShort alternateSetCount;     //Number of AlternateSet tables
    AlternateSetTable *alternateSet;//Array of offsets to AlternateSet tables-from beginning
                                    //of Substitution table-ordered by Coverage Index
} AlternateSubstSubtable;

#endif


#ifdef GSUB_LIGATURE

typedef struct LigatureTable {
    SFGlyph ligGlyph;               //GlyphID of ligature to substitute
    SFUShort compCount;             //Number of components in the ligature
    SFGlyph	*component;             //[CompCount - 1]. Array of component GlyphIDs-start with
                                    //the second component-ordered in writing direction
} LigatureTable;

typedef struct LigatureSetTable {
    SFUShort ligatureCount;         //Number of Ligature tables
    LigatureTable *ligature;        //Array of offsets to Ligature tables-from beginning of
                                    //LigatureSet table-ordered by preference
} LigatureSetTable;

typedef struct LigatureSubstSubtable {
    SFUShort substFormat;           //Format identifier-format = 1
    CoverageTable coverage;         //Offset to Coverage table-from beginning of Substitution table
    SFUShort ligSetCount;           //Number of LigatureSet tables
    LigatureSetTable *ligatureSet;  //Array of offsets to LigatureSet tables-from beginning of
                                    //Substitution table-ordered by Coverage Index
} LigatureSubstSubtable;

#endif


#ifdef GSUB_REVERSE_CHAINING_CONTEXT

typedef struct ReverseChainingContextSubstSubtable {
    SFUShort substFormat;           //Format identifier-format = 1
    CoverageTable Coverage;         //Offset to Coverage table - from beginning of Substitution table
    SFUShort backtrackGlyphCount;   //Number of glyphs in the backtracking sequence
    CoverageTable *backtrackGlyphCoverage;
                                    //Array of offsets to coverage tables in backtracking
                                    //sequence, in glyph sequence order
    SFUShort lookaheadGlyphCount;   //Number of glyphs in lookahead sequence
    CoverageTable *lookaheadGlyphCoverage;
                                    //Array of offsets to coverage tables in lookahead sequence,
                                    //in glyph sequence order
    SFUShort glyphCount;            //Number of GlyphIDs in the Substitute array
    SFGlyph *substitute;            //Array of substitute GlyphIDs-ordered by Coverage Index
} ReverseChainingContextSubstSubtable;

#endif


typedef struct SFTableGSUB {
    SFUInt version;                 //Version of the GSUB table-initially set to 0x00010000
    ScriptListTable scriptList;     //Offset to ScriptList table-from beginning of GSUB table
    FeatureListTable featureList;   //Offset to FeatureList table-from beginning of GSUB table
    LookupListTable lookupList;     //Offset to LookupList table-from beginning of GSUB table
} SFTableGSUB;

void SFReadGSUB(const SFUByte * const table, SFTableGSUB *tablePtr);
void SFFreeGSUB(SFTableGSUB *tablePtr);

#endif

