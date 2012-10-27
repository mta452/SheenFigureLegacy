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

#ifndef _SF_GSUB_DATA_H
#define _SF_GSUB_DATA_H

#include "SFConfig.h"
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


#if defined(GSUB_CONTEXT) || defined(GSUB_CHAINING_CONTEXT)

typedef struct SubstLookupRecord {
    SFUShort sequenceIndex;         //Index into current glyph sequence-first glyph = 0
    SFUShort lookupListIndex;       //Lookup to apply to that position-zero-based
} SubstLookupRecord;

#endif


#ifdef GSUB_CONTEXT

#ifdef GSUB_CONTEXT_FORMAT1

typedef struct SubRuleTable {
    SFUShort glyphCount;            //Total number of glyphs in input glyph sequence-includes
                                    //the first glyph
    SFUShort substCount;            //Number of SubstLookupRecords
    SFGlyph *input;                 //[GlyphCount - 1]. Array of input GlyphIDs-start with second glyph
    SubstLookupRecord *substLookupRecord;
                                    //Array of SubstLookupRecords-in design order
} SubRuleTable;

typedef struct SubRuleSetTable {
    SFUShort subRuleCount;          //Number of SubRule tables
    SubRuleTable *subRule;          //Array of offsets to SubRule tables-from beginning of
                                    //SubRuleSet table-ordered by preference
} SubRuleSetTable;

#endif

#ifdef GSUB_CONTEXT_FORMAT2

typedef struct SubClassRuleTable {
    SFUShort glyphCount;            //Total number of classes specified for the context in
                                    //the rule-includes the first class
    SFUShort substCount;            //Number of SubstLookupRecords
    SFUShort *cls;                  //[GlyphCount - 1]. Array of classes-beginning with the
                                    //second class-to be matched to the input glyph class sequence
    SubstLookupRecord *substLookupRecord;
                                    //[SubstCount]. Array of Substitution lookups-in design order
} SubClassRuleTable;

typedef struct SubClassSetSubtable {
    SFUShort subClassRuleCnt;       //Number of SubClassRule tables
    SubClassRuleTable *subClassRule;//Array of offsets to SubClassRule tables-from beginning
                                    //of SubClassSet-ordered by preference
} SubClassSetSubtable;

#endif

typedef struct ContextSubstSubtable {
    SFUShort substFormat;           //Format identifier-format = 1

    union {
#ifdef GSUB_CONTEXT_FORMAT1
        struct {
            CoverageTable coverage; //Offset to Coverage table-from beginning of Substitution table
            SFUShort subRuleSetCount;
                                    //Number of SubRuleSet tables-must equal GlyphCount in Coverage table
            SubRuleSetTable *subRuleSet;
                                    //Array of offsets to SubRuleSet tables-from beginning
                                    //of Substitution table-ordered by Coverage Index
        } format1;
#endif
        
#ifdef GSUB_CONTEXT_FORMAT2
        struct {
            CoverageTable coverage; //Offset to Coverage table-from beginning of Substitution table
            ClassDefTable classDef; //Offset to glyph ClassDef table-from beginning of Substitution table
            SFUShort subClassSetCnt;//Number of SubClassSet tables
            SubClassSetSubtable *subClassSet;
                                    //Array of offsets to SubClassSet tables-from beginning of
                                    //Substitution table-ordered by class-may be NULL
        } format2;
#endif
        
#ifdef GSUB_CONTEXT_FORMAT3
        struct {
            SFUShort glyphCount;    //Number of glyphs in the input glyph sequence
            SFUShort substCount;    //Number of SubstLookupRecords
            CoverageTable *coverage;
                                    //Array of offsets to Coverage table-from beginning of
                                    //Substitution table-in glyph sequence order
            SubstLookupRecord *substLookupRecord;
                                    //Array of SubstLookupRecords-in design order
        } format3;
#endif
    } format;
} ContextSubstSubtable;

#endif


#ifdef GSUB_CHAINING_CONTEXT

#ifdef GSUB_CHAINING_CONTEXT_FORMAT1

typedef struct ChainSubRuleSubtable {
    SFUShort backtrackGlyphCount;   //Total number of glyphs in the backtrack sequence
                                    //(number of glyphs to be matched before the first glyph)
    SFGlyph *backtrack;             //Array of backtracking GlyphID's (to be matched
                                    //before the input sequence)
    SFUShort inputGlyphCount;       //Total number of glyphs in the input sequence
                                    //(includes the first glyph)
    SFGlyph *input;                 //[InputGlyphCount - 1].
                                    //Array of input GlyphIDs (start with second glyph)
    SFUShort lookaheadGlyphCount;   //Total number of glyphs in the look ahead sequence
                                    //(number of glyphs to be matched after the input sequence)
    SFGlyph *lookAhead;             //Array of lookahead GlyphID's (to be matched after
                                    //the input sequence)
    SFUShort substCount;            //Number of SubstLookupRecords
    SubstLookupRecord *substLookupRecord;
                                    //Array of SubstLookupRecords (in design order)
} ChainSubRuleSubtable;

typedef struct ChainSubRuleSetTable {
    SFUShort chainSubRuleCount;     //Number of ChainSubRule tables
    ChainSubRuleSubtable *chainSubRule;
                                    //Array of offsets to ChainSubRule tables-from beginning
                                    //of ChainSubRuleSet table-ordered by preference
} ChainSubRuleSetTable;

#endif

#ifdef GSUB_CHAINING_CONTEXT_FORMAT2

typedef struct ChainSubClassRuleTable {
    SFUShort backtrackGlyphCount;   //Total number of glyphs in the backtrack sequence
                                    //(number of glyphs to be matched before the first glyph)
    SFUShort *backtrack;            //Array of backtracking classes(to be matched before
                                    //the input sequence)
    SFUShort inputGlyphCount;       //Total number of classes in the input sequence
                                    //(includes the first class)
    SFUShort *input;                //[InputGlyphCount - 1]. Array of input classes(start with
                                    //second class; to be matched with the input glyph sequence)
    SFUShort lookaheadGlyphCount;   //Total number of classes in the look ahead sequence
                                    //(number of classes to be matched after the input sequence)
    SFUShort *lookAhead;            //Array of lookahead classes(to be matched after the input sequence)
    SFUShort substCount;            //Number of SubstLookupRecords
    SubstLookupRecord *substLookupRecord;
                                    //Array of SubstLookupRecords (in design order)
} ChainSubClassRuleTable;

typedef struct ChainSubClassSetSubtable {
    SFUShort chainSubClassRuleCnt;  //Number of ChainSubClassRule tables
    ChainSubClassRuleTable *chainSubClassRule;
                                    //Array of offsets to ChainSubClassRule tables-from beginning
                                    //of ChainSubClassSet-ordered by preference
} ChainSubClassSetSubtable;

#endif

typedef struct ChainingContextualSubstSubtable {
    SFUShort substFormat;           //Format identifier

    union {
#ifdef GSUB_CHAINING_CONTEXT_FORMAT1
        struct {
            CoverageTable coverage; //Offset to Coverage table-from beginning of Substitution table
            SFUShort chainSubRuleSetCount;
                                    //Number of ChainSubRuleSet tables-must equal GlyphCount
                                    //in Coverage table
            ChainSubRuleSetTable *chainSubRuleSet;
                                    //Array of offsets to ChainSubRuleSet tables-from beginning
                                    //of Substitution table-ordered by Coverage Index
        } format1;
#endif
        
#ifdef GSUB_CHAINING_CONTEXT_FORMAT2
        struct {
            CoverageTable coverage; //Offset to Coverage table-from beginning of Substitution table
            ClassDefTable backtrackClassDef;
                                    //Offset to glyph ClassDef table containing backtrack
                                    //sequence data-from beginning of Substitution table
            ClassDefTable inputClassDef;
                                    //Offset to glyph ClassDef table containing input
                                    //sequence data-from beginning of Substitution table
            ClassDefTable lookaheadClassDef;
                                    //Offset to glyph ClassDef table containing lookahead
                                    //sequence data-from beginning of Substitution table
            SFUShort chainSubClassSetCnt;
                                    //Number of ChainSubClassSet tables
            ChainSubClassSetSubtable *chainSubClassSet;
                                    //Array of offsets to ChainSubClassSet tables-from beginning
                                    //of Substitution table-ordered by input class-may be NULL
        } format2;
#endif
        
#ifdef GSUB_CHAINING_CONTEXT_FORMAT3
        struct {
            SFUShort backtrackGlyphCount;
                                    //Number of glyphs in the backtracking sequence
            CoverageTable *backtrackGlyphCoverage;
                                    //Array of offsets to coverage tables in backtracking
                                    //sequence, in glyph sequence order
            SFUShort inputGlyphCount;
                                    //Number of glyphs in input sequence
            CoverageTable *inputGlyphCoverage;
                                    //Array of offsets to coverage tables in input sequence,
                                    //in glyph sequence order
            SFUShort lookaheadGlyphCount;
                                    //Number of glyphs in lookahead sequence
            CoverageTable *lookaheadGlyphCoverage;
                                    //Array of offsets to coverage tables in lookahead sequence,
                                    //in glyph sequence order
            SFUShort substCount;    //Number of SubstLookupRecords
            SubstLookupRecord *SubstLookupRecord;
                                    //Array of SubstLookupRecords, in design order
        } format3;
#endif
    } format;
} ChainingContextualSubstSubtable;

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

