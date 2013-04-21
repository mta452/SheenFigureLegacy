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

#ifndef _SF_GSUB_GPOS_DATA_H
#define _SF_GSUB_GPOS_DATA_H

#include "SFConfig.h"
#include "SFTypes.h"
#include "SFCommonData.h"

#if defined(GSUB_GPOS_CONTEXT) || defined(GSUB_GPOS_CHAINING_CONTEXT)

typedef struct SubPosLookupRecord {
    SFUShort sequenceIndex;         //Index into current glyph sequence-first glyph = 0
    SFUShort lookupListIndex;       //Lookup to apply to that position-zero-based
} SubPosLookupRecord;

#endif


#ifdef GSUB_GPOS_CONTEXT

#ifdef GSUB_GPOS_CONTEXT_FORMAT1

typedef struct SubPosRuleTable {
    SFUShort glyphCount;            //Total number of glyphs in input glyph sequence-includes
                                    //the first glyph
    SFUShort subPosCount;           //Number of SubPosLookupRecords
    SFGlyph *input;                 //[GlyphCount - 1]. Array of input GlyphIDs-start with second glyph
    SubPosLookupRecord *subPosLookupRecord;
                                    //Array of SubPosLookupRecords-in design order
} SubPosRuleTable;

typedef struct SubPosRuleSetTable {
    SFUShort subPosRuleCount;       //Number of SubPosRule tables
    SubPosRuleTable *subPosRule;    //Array of offsets to SubPosRule tables-from beginning of
                                    //SubPosRuleSet table-ordered by preference
} SubPosRuleSetTable;

#endif

#ifdef GSUB_GPOS_CONTEXT_FORMAT2

typedef struct SubPosClassRuleTable {
    SFUShort glyphCount;            //Total number of classes specified for the context in
                                    //the rule-includes the first class
    SFUShort subPosCount;           //Number of SubPosLookupRecords
    SFUShort *cls;                  //[GlyphCount - 1]. Array of classes-beginning with the
                                    //second class-to be matched to the input glyph class sequence
    SubPosLookupRecord *subPosLookupRecord;
                                    //[SubPosCount]. Array of lookups-in design order
} SubPosClassRuleTable;

typedef struct SubPosClassSetSubtable {
    SFUShort subPosClassRuleCount;  //Number of SubPosClassRule tables
    SubPosClassRuleTable *subPosClassRule;
                                    //Array of offsets to SubPosClassRule tables-from beginning
                                    //of SubPosClassSet-ordered by preference
} SubPosClassSetSubtable;

#endif

typedef struct ContextSubPosSubtable {
    SFUShort subPosFormat;          //Format identifier-format = 1
    
    union {
#ifdef GSUB_GPOS_CONTEXT_FORMAT1
        struct {
            CoverageTable coverage; //Offset to Coverage table-from beginning of Substitution/Positioning
                                    //table
            SFUShort subPosRuleSetCount;
                                    //Number of SubPosRuleSet tables-must equal GlyphCount in Coverage table
            SubPosRuleSetTable *subPosRuleSet;
                                    //Array of offsets to SubPosRuleSet tables-from beginning
                                    //of Substitution/Positioning table-ordered by Coverage Index
        } format1;
#endif
        
#ifdef GSUB_GPOS_CONTEXT_FORMAT2
        struct {
            CoverageTable coverage; //Offset to Coverage table-from beginning of Substitution/Positioning
                                    //table
            ClassDefTable classDef; //Offset to glyph ClassDef table-from beginning of Substitution
                                    //or Positioning table
            SFUShort subPosClassSetCount;
                                    //Number of SubPosClassSet tables
            SubPosClassSetSubtable *subPosClassSet;
                                    //Array of offsets to SubPosClassSet tables-from beginning of
                                    //Substitution/Positioning table-ordered by class-may be NULL
        } format2;
#endif
        
#ifdef GSUB_GPOS_CONTEXT_FORMAT3
        struct {
            SFUShort glyphCount;    //Number of glyphs in the input glyph sequence
            SFUShort subPosCount;   //Number of SubPosLookupRecords
            CoverageTable *coverage;
                                    //Array of offsets to Coverage table-from beginning of
                                    //Substitution/Positioning table-in glyph sequence order
            SubPosLookupRecord *subPosLookupRecord;
                                    //Array of SubPosLookupRecords-in design order
        } format3;
#endif
    } format;
} ContextSubPosSubtable;

void SFReadContextSubPos(const SFUByte * const csTable, ContextSubPosSubtable *tablePtr);
void SFFreeContextSubPos(ContextSubPosSubtable *tablePtr);

#endif


#ifdef GSUB_GPOS_CHAINING_CONTEXT

#ifdef GSUB_GPOS_CHAINING_CONTEXT_FORMAT1

typedef struct ChainSubPosRuleSubtable {
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
    SFUShort subPosCount;           //Number of SubPosLookupRecords
    SubPosLookupRecord *subPosLookupRecord;
                                    //Array of SubPosLookupRecords (in design order)
} ChainSubPosRuleSubtable;

typedef struct ChainSubPosRuleSetTable {
    SFUShort chainSubPosRuleCount;  //Number of ChainSubPosRule tables
    ChainSubPosRuleSubtable *chainSubPosRule;
                                    //Array of offsets to ChainSubPosRule tables-from beginning
                                    //of ChainSubPosRuleSet table-ordered by preference
} ChainSubPosRuleSetTable;

#endif

#ifdef GSUB_GPOS_CHAINING_CONTEXT_FORMAT2

typedef struct ChainSubPosClassRuleTable {
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
    SFUShort substCount;            //Number of SubPosLookupRecords
    SubPosLookupRecord *subPosLookupRecord;
                                    //Array of SubPosLookupRecords (in design order)
} ChainSubPosClassRuleTable;

typedef struct ChainSubPosClassSetSubtable {
    SFUShort chainSubPosClassRuleCount;
                                    //Number of ChainSubPosClassRule tables
    ChainSubPosClassRuleTable *chainSubPosClassRule;
                                    //Array of offsets to ChainSubPosClassRule tables-from beginning
                                    //of ChainSubPosClassSet-ordered by preference
} ChainSubPosClassSetSubtable;

#endif

typedef struct ChainingContextualSubPosSubtable {
    SFUShort subPosFormat;          //Format identifier
    
    union {
#ifdef GSUB_GPOS_CHAINING_CONTEXT_FORMAT1
        struct {
            CoverageTable coverage; //Offset to Coverage table-from beginning of Substitution/Positioning
                                    //table
            SFUShort chainSubPosRuleSetCount;
                                    //Number of ChainSubRuleSet tables-must equal GlyphCount in
                                    //Coverage table
            ChainSubPosRuleSetTable *chainSubPosRuleSet;
                                    //Array of offsets to ChainSubPosRuleSet tables-from beginning
                                    //of Substitution/Positioning table-ordered by Coverage Index
        } format1;
#endif

#ifdef GSUB_GPOS_CHAINING_CONTEXT_FORMAT2
        struct {
            CoverageTable coverage; //Offset to Coverage table-from beginning of Substitution/Positioning
                                    //table
            ClassDefTable backtrackClassDef;
                                    //Offset to glyph ClassDef table containing backtrack
                                    //sequence data-from beginning of Substitution/Positioning table
            ClassDefTable inputClassDef;
                                    //Offset to glyph ClassDef table containing input sequence
                                    //data-from beginning of Substitution/Positioning table
            ClassDefTable lookaheadClassDef;
                                    //Offset to glyph ClassDef table containing lookahead
                                    //sequence data-from beginning of Substitution/Positioning table
            SFUShort chainSubPosClassSetCount;
                                    //Number of ChainSubPosClassSet tables
            ChainSubPosClassSetSubtable *chainSubPosClassSet;
                                    //Array of offsets to ChainSubPosClassSet tables-from beginning of
                                    //Substitution/Positioning table-ordered by input class-may be NULL
        } format2;
#endif
        
#ifdef GSUB_GPOS_CHAINING_CONTEXT_FORMAT3
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
            SFUShort subPosCount;   //Number of SubPosLookupRecords
            SubPosLookupRecord *subPosLookupRecord;
                                    //Array of SubPosLookupRecords, in design order
        } format3;
#endif
    } format;
} ChainingContextualSubPosSubtable;

void SFReadChainingContextSubPos(const SFUByte * const ccsTable, ChainingContextualSubPosSubtable *tablePtr);
void SFFreeChainingContextSubPos(ChainingContextualSubPosSubtable *tablePtr);

#endif

#endif
