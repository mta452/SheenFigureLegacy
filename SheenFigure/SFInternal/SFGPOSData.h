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

#ifndef _SF_GPOS_DATA_H
#define _SF_GPOS_DATA_H

#include "SFTypes.h"
#include "SFConfig.h"
#include "SFCommonData.h"

typedef enum {
    vfXPlacement = 0x0001,          //Includes horizontal adjustment for placement
    vfYPlacement = 0x0002,          //Includes vertical adjustment for placement
    vfXAdvance = 0x0004,            //Includes horizontal adjustment for advance
    vfYAdvance = 0x0008,            //Includes vertical adjustment for advance
    vfXPlaDevice = 0x0010,          //Includes horizontal Device table for placement
    vfYPlaDevice = 0x0020,          //Includes vertical Device table for placement
    vfXAdvDevice = 0x0040,          //Includes horizontal Device table for advance
    vfYAdvDevice = 0x0080,          //Includes vertical Device table for advance
    vfReserved = 0xF000,            //For future use (set to zero)
} ValueFormat;


#if defined(GPOS_SINGLE) || defined(GPOS_PAIR)

typedef struct ValueRecord {
    short xPlacement;               //Horizontal adjustment for placement-in design units
    short yPlacement;               //Vertical adjustment for placement-in design units
    short xAdvance;                 //Horizontal adjustment for advance-in design units
                                    //(only used for horizontal writing)
    short yAdvance;                 //Vertical adjustment for advance-in design units
                                    //(only used for vertical writing)
    DeviceTable xPlaDevice;         //Offset to Device table for horizontal placement-measured
                                    //from beginning of PosTable (may be NULL)
    DeviceTable	yPlaDevice;         //Offset to Device table for vertical placement-measured
                                    //from beginning of PosTable (may be NULL)
    DeviceTable	xAdvDevice;         //Offset to Device table for horizontal advance-measured
                                    //from beginning of PosTable (may be NULL)
    DeviceTable	yAdvDevice;         //Offset to Device table for vertical advance-measured from
                                    //beginning of PosTable (may be NULL)
} ValueRecord;

#endif

#ifdef GPOS_ANCHOR

typedef struct AnchorTable {
    SFUShort anchorFormat;
    
    short xCoordinate;              //Horizontal value-in design units
    short yCoordinate;              //Vertical value-in design units
    
    union {
        struct {
            SFUShort anchorPoint;   //Index to glyph contour point
        } format2;
        
        struct {
            DeviceTable xDeviceTable;
                                    //Offset to Device table for X coordinate- from
                                    //beginning of Anchor table (may be NULL)
            DeviceTable	yDeviceTable;
                                    //Offset to Device table for Y coordinate- from
                                    //beginning of Anchor table (may be NULL)
        } format3;
    } format;
} AnchorTable;

#endif


#ifdef GPOS_SINGLE

typedef struct SingleAdjustmentPosSubtable {
    SFUShort posFormat;             //Format identifier
    CoverageTable coverage;         //Offset to Coverage table-from beginning of SinglePos subtable
    ValueFormat valueFormat;        //Defines the types of data in the ValueRecord
    
    union {

#ifdef GPOS_SINGLE_FORMAT1
        struct {
            ValueRecord value;      //Defines positioning value(s)-applied to all glyphs
                                    //in the Coverage table
        } format1;
#endif
        
#ifdef GPOS_SINGLE_FORMAT2
        struct {
            SFUShort valueCount;    //Number of ValueRecords
            ValueRecord *value;     //Array of ValueRecords-positioning values applied to glyphs
        } format2;
#endif
    } format;
} SingleAdjustmentPosSubtable;

#endif

#ifdef GPOS_PAIR_FORMAT1

typedef struct PairValueRecord {
    SFGlyph secondGlyph;            //GlyphID of second glyph in the pair-first glyph is
                                    //listed in the Coverage table
    ValueRecord value1;             //Positioning data for the first glyph in the pair
    ValueRecord value2;             //Positioning data for the second glyph in the pair
} PairValueRecord;

typedef struct PairSetTable {
    SFUShort pairValueCount;        //Number of PairValueRecords
    PairValueRecord	*pairValueRecord;
                                    //Array of PairValueRecords-ordered by GlyphID of the second glyph
} PairSetTable;

#endif

#ifdef GPOS_PAIR_FORMAT2

typedef struct Class2Record {
    ValueRecord value1;             //Positioning for first glyph-empty if ValueFormat1 = 0
    ValueRecord value2;             //Positioning for second glyph-empty if ValueFormat2 = 0
} Class2Record;

typedef struct Class1Record {
    Class2Record *class2Record;     //Array of Class2 records-ordered by Class2
} Class1Record;

#endif

#ifdef GPOS_PAIR

typedef struct PairAdjustmentPosSubtable {
    SFUShort posFormat;             //Format identifier
    CoverageTable coverage;         //Offset to Coverage table-from beginning of PairPos
                                    //subtable-for the first glyph of the pair
    ValueFormat valueFormat1;       //ValueRecord definition-for the first glyph of the
                                    //pair-may be zero (0)
    ValueFormat valueFormat2;       //ValueRecord definition-for the second glyph of the
                                    //pair-may be zero (0)
    
    union {

#ifdef GPOS_PAIR_FORMAT1
        struct {
            SFUShort pairSetCount;  //Number of PairSet tables
            PairSetTable *pairSetTable;
                                    //Array of offsets to PairSet tables-from beginning of
                                    //PairPos subtable-ordered by Coverage Index
        } format1;
#endif
        
#ifdef GPOS_PAIR_FORMAT2
        struct {
            ClassDefTable classDef1;
                                    //Offset to ClassDef table-from beginning of PairPos
                                    //subtable-for the first glyph of the pair
            ClassDefTable classDef2;
                                    //Offset to ClassDef table-from beginning of PairPos
                                    //subtable-for the second glyph of the pair
            SFUShort class1Count;   //Number of classes in ClassDef1 table-includes Class0
            SFUShort class2Count;   //Number of classes in ClassDef2 table-includes Class0
            Class1Record *class1Record;
                                    //Array of Class1 records-ordered by Class1
        } format2;
#endif
    } format;
} PairAdjustmentPosSubtable;

#endif


#ifdef GPOS_CURSIVE

typedef struct EntryExitRecord {
    SFBool hasEntryAnchor;
    SFBool hasExitAnchor;
    
    AnchorTable entryAnchor;        //Offset to EntryAnchor table-from beginning of
                                    //CursivePos subtable-may be NULL
    AnchorTable exitAnchor;         //Offset to ExitAnchor table-from beginning of
                                    //CursivePos subtable-may be NULL
} EntryExitRecord;

typedef struct CursiveAttachmentPosSubtable {
    SFUShort posFormat;             //Format identifier-format = 1
    CoverageTable coverage;         //Offset to Coverage table-from beginning of CursivePos subtable
    SFUShort entryExitCount;        //Number of EntryExit records
    EntryExitRecord *entryExitRecord;
                                    //Array of EntryExit records-in Coverage Index order
} CursiveAttachmentPosSubtable;

#endif


#ifdef GPOS_MARK

typedef struct MarkRecord {
    SFUShort cls;                   //Class defined for this mark
    AnchorTable markAnchor;         //Offset to Anchor table-from beginning of MarkArray table
} MarkRecord;

typedef struct MarkArrayTable {
    SFUShort markCount;             //Number of MarkRecords
    MarkRecord *markRecord;         //Array of MarkRecords-in Coverage order
} MarkArrayTable;

#endif

#ifdef GPOS_MARK_TO_BASE

typedef struct BaseRecord {
    AnchorTable *baseAnchor;        //Array of offsets (one per class) to Anchor tables-from
                                    //beginning of BaseArray table-ordered by class-zero-based
} BaseRecord;

typedef struct BaseArrayTable {
    SFUShort baseCount;             //Number of BaseRecords
    BaseRecord *baseRecord;         //Array of BaseRecords-in order of BaseCoverage Index
} BaseArrayTable;

typedef struct MarkToBaseAttachmentPosSubtable {
    SFUShort posFormat;             //Format identifier-format = 1
    CoverageTable markCoverage;     //Offset to MarkCoverage table-from beginning of MarkBasePos subtable
    CoverageTable baseCoverage;     //Offset to BaseCoverage table-from beginning of MarkBasePos subtable
    SFUShort classCount;            //Number of classes defined for marks
    MarkArrayTable markArray;       //Offset to MarkArray table-from beginning of MarkBasePos subtable
    BaseArrayTable baseArray;       //Offset to BaseArray table-from beginning of MarkBasePos subtable
} MarkToBaseAttachmentPosSubtable;

#endif


#ifdef GPOS_MARK_TO_LIGATURE

typedef struct ComponentRecord {
    AnchorTable *ligatureAnchor;    //Array of offsets (one per class) to Anchor tables-from
                                    //beginning of LigatureAttach table-ordered by class-NULL
                                    //if a component does not have an attachment for a
                                    //class-zero-based array
} ComponentRecord;

typedef struct LigatureAttachTable {
    SFUShort componentCount;        //Number of ComponentRecords in this ligature
    ComponentRecord	*componentRecord;
                                    //Array of Component records-ordered in writing direction
} LigatureAttachTable;

typedef struct LigatureArrayTable {
    SFUShort ligatureCount;         //Number of LigatureAttach table offsets
    LigatureAttachTable	*ligatureAttach;
                                    //Array of offsets to LigatureAttach tables-from beginning
                                    //of LigatureArray table-ordered by LigatureCoverage Index
} LigatureArrayTable;

typedef struct MarkToLigatureAttachmentPosSubtable {
    SFUShort posFormat;             //Format identifier-format = 1
    CoverageTable markCoverage;     //Offset to Mark Coverage table-from beginning of MarkLigPos subtable
    CoverageTable ligatureCoverage; //Offset to Ligature Coverage table-from beginning of
                                    //MarkLigPos subtable
    SFUShort classCount;            //Number of defined mark classes
    MarkArrayTable markArray;       //Offset to MarkArray table-from beginning of MarkLigPos subtable
    LigatureArrayTable ligatureArray;
                                    //Offset to LigatureArray table-from beginning of MarkLigPos subtable
} MarkToLigatureAttachmentPosSubtable;

#endif


#ifdef GPOS_MARK_TO_MARK

typedef struct Mark2Record {
    AnchorTable *mark2Anchor;       //Array of offsets (one per class) to Anchor tables-from
                                    //beginning of Mark2Array table-zero-based array
} Mark2Record;

typedef struct Mark2ArrayTable {
    SFUShort mark2Count;            //Number of Mark2 records
    Mark2Record *mark2Record;       //Array of Mark2 records-in Coverage order
} Mark2ArrayTable;

typedef struct MarkToMarkAttachmentPosSubtable {
    SFUShort posFormat;             //Format identifier-format = 1
    CoverageTable mark1Coverage;    //Offset to Combining Mark Coverage table-from beginning
                                    //of MarkMarkPos subtable
    CoverageTable mark2Coverage;    //Offset to Base Mark Coverage table-from beginning of
                                    //MarkMarkPos subtable
    SFUShort classCount;            //Number of Combining Mark classes defined
    MarkArrayTable mark1Array;      //Offset to MarkArray table for Mark1-from beginning of
                                    //MarkMarkPos subtable
    Mark2ArrayTable mark2Array;     //Offset to Mark2Array table for Mark2-from beginning of
                                    //MarkMarkPos subtable
} MarkToMarkAttachmentPosSubtable;

#endif

#if defined(GPOS_CONTEXT) || defined (GPOS_CHAINING_CONTEXT)

typedef struct PosLookupRecord {
    SFUShort sequenceIndex;         //Index into current glyph sequence-first glyph = 0
    SFUShort lookupListIndex;       //Lookup to apply to that position-zero-based
} PosLookupRecord;

#endif

#ifdef GPOS_CONTEXT

#ifdef GPOS_CONTEXT_FORMAT1

typedef struct PosRuleTable {
    SFUShort glyphCount;            //Total number of glyphs in input glyph sequence-includes
                                    //the first glyph
    SFUShort posCount;              //Number of PosLookupRecords
    SFGlyph *input;                 //[GlyphCount - 1]. Array of input GlyphIDs-start with second glyph
    PosLookupRecord *posLookupRecord;
                                    //Array of PosLookupRecords-in design order
} PosRuleTable;

typedef struct PosRuleSetTable {
    SFUShort posRuleCount;          //Number of PosRule tables
    PosRuleTable *posRule;          //Array of offsets to PosRule tables-from beginning of
                                    //PosRuleSet table-ordered by preference
} PosRuleSetTable;

#endif

#ifdef GPOS_CONTEXT_FORMAT2

typedef struct PosClassRuleTable {
    SFUShort glyphCount;            //Total number of classes specified for the context in
                                    //the rule-includes the first class
    SFUShort posCount;              //Number of PosLookupRecords
    SFUShort *cls;                  //[GlyphCount - 1]. Array of classes-beginning with the
                                    //second class-to be matched to the input glyph class sequence
    PosLookupRecord *posLookupRecord;
                                    //[PosCount]. Array of Position lookups-in design order
} PosClassRuleTable;

typedef struct PosClassSetTable {
    SFUShort posClassRuleCnt;       //Number of SubClassRule tables
    PosClassRuleTable *posClassRule;//Array of offsets to SubClassRule tables-from beginning
                                    //of SubClassSet-ordered by preference
} PosClassSetTable;

#endif

typedef struct ContextPosSubtable {
    SFUShort posFormat;             //Format identifier
    
    union {

#ifdef GPOS_CONTEXT_FORMAT1
        struct {
            CoverageTable coverage; //Offset to Coverage table-from beginning of Substitution table
            SFUShort posRuleSetCount;
                                    //Number of PosRuleSet tables-must equal GlyphCount in Coverage table
            PosRuleSetTable *posRuleSet;
                                    //Array of offsets to SubRuleSet tables-from beginning
                                    //of Substitution table-ordered by Coverage Index
        } format1;
#endif
        
#ifdef GPOS_CONTEXT_FORMAT2
        struct {
            CoverageTable coverage; //Offset to Coverage table-from beginning of Substitution table
            ClassDefTable classDef; //Offset to glyph ClassDef table-from beginning of Substitution table
            SFUShort posClassSetCnt;//Number of PosClassSet tables
            PosClassSetTable *posClassSet;
                                    //Array of offsets to PosClassSet tables-from beginning of
                                    //Position table-ordered by class-may be NULL
        } format2;
#endif
        
#ifdef GPOS_CONTEXT_FORMAT3
        struct {
            SFUShort glyphCount;    //Number of glyphs in the input glyph sequence
            SFUShort posCount;      //Number of PosLookupRecords
            CoverageTable *coverage;
                                    //Array of offsets to Coverage table-from beginning of
                                    //Substitution table-in glyph sequence order
            PosLookupRecord *posLookupRecord;
                                    //Array of PosLookupRecords-in design order
        } format3;
#endif
    } format;
} ContextPosSubtable;

#endif


#ifdef GPOS_CHAINING_CONTEXT

#ifdef GPOS_CHAINING_CONTEXT_FORMAT1

typedef struct ChainPosRuleSubtable {
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
    SFUShort posCount;              //Number of PosLookupRecords
    PosLookupRecord *posLookupRecord;
                                    //Array of PosLookupRecords (in design order)
} ChainPosRuleSubtable;

typedef struct ChainPosRuleSetTable {
    SFUShort chainPosRuleCount;     //Number of ChainPosRule tables
    ChainPosRuleSubtable *chainPosRule;
                                    //Array of offsets to ChainPosRule tables-from beginning
                                    //of ChainPosRuleSet table-ordered by preference
} ChainPosRuleSetTable;

#endif

#ifdef GPOS_CHAINING_CONTEXT_FORMAT2

typedef struct ChainPosClassRuleSubTable {
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
    SFUShort posCount;              //Number of PosLookupRecords
    PosLookupRecord *posLookupRecord;
                                    //Array of PosLookupRecords (in design order)
} ChainPosClassRuleSubTable;

typedef struct ChainPosClassSetTable {
    SFUShort ChainPosClassRuleCnt;  //Number of ChainPosClassRule tables
    ChainPosClassRuleSubTable *chainPosClassRule;
                                    //Array of offsets to ChainPosClassRule tables-from beginning
                                    //of ChainPosClassSet-ordered by preference
} ChainPosClassSetTable;

#endif

typedef struct ChainingContextualPosSubtable {
    SFUShort posFormat;             //Format identifier
    
    union {
        
#ifdef GPOS_CHAINING_CONTEXT_FORMAT1
        struct {
            CoverageTable coverage; //Offset to Coverage table-from beginning of Positution table
            SFUShort chainPosRuleSetCount;
                                    //Number of ChainPosRuleSet tables-must equal GlyphCount
                                    //in Coverage table
            ChainPosRuleSetTable *chainPosRuleSet;
                                    //Array of offsets to ChainPosRuleSet tables-from beginning
                                    //of Positution table-ordered by Coverage Index
        } format1;
#endif
        
#ifdef GPOS_CHAINING_CONTEXT_FORMAT2
        struct {
            CoverageTable coverage; //Offset to Coverage table-from beginning of Positution table
            ClassDefTable backtrackClassDef;
                                    //Offset to glyph ClassDef table containing backtrack
                                    //sequence data-from beginning of Positution table
            ClassDefTable inputClassDef;
                                    //Offset to glyph ClassDef table containing input
                                    //sequence data-from beginning of Positution table
            ClassDefTable lookaheadClassDef;
                                    //Offset to glyph ClassDef table containing lookahead
                                    //sequence data-from beginning of Positution table
            SFUShort chainPosClassSetCnt;
                                    //Number of ChainPosClassSet tables
            ChainPosClassSetTable *chainPosClassSet;
                                    //Array of offsets to ChainPosClassSet tables-from beginning
                                    //of Positution table-ordered by input class-may be NULL
        } format2;
#endif
        
#ifdef GPOS_CHAINING_CONTEXT_FORMAT3
        struct {
            SFUShort backtrackGlyphCount;
                                    //Number of glyphs in the backtracking sequence
            CoverageTable *backtrackGlyphCoverage;
                                    //Array of offsets to coverage tables in backtracking
                                    //sequence, in glyph sequence order
            SFUShort inputGlyphCount; //Number of glyphs in input sequence
            CoverageTable *inputGlyphCoverage;
                                    //Array of offsets to coverage tables in input sequence,
                                    //in glyph sequence order
            SFUShort lookaheadGlyphCount;
                                    //Number of glyphs in lookahead sequence
            CoverageTable *lookaheadGlyphCoverage;
                                    //Array of offsets to coverage tables in lookahead sequence,
                                    //in glyph sequence order
            SFUShort posCount;      //Number of PosLookupRecords
            PosLookupRecord *PosLookupRecord;
                                    //Array of PosLookupRecords, in design order
        } format3;
#endif
    } format;
} ChainingContextualPosSubtable;

#endif


typedef struct SFTableGPOS {
    SFUInt version;                 //Version of the GSUB table-initially set to 0x00010000
    ScriptListTable scriptList;     //Offset to ScriptList table-from beginning of GPOS table
    FeatureListTable featureList;   //Offset to FeatureList table-from beginning of GPOS table
    LookupListTable lookupList;     //Offset to LookupList table-from beginning of GPOS table
} SFTableGPOS;

void SFReadGPOS(const SFUByte * const table, SFTableGPOS *tablePtr);
void SFFreeGPOS(SFTableGPOS *tablePtr);

#endif
