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

#ifndef _SF_COMMON_DATA_H
#define _SF_COMMON_DATA_H

#include "SFConfig.h"
#include "SFTypes.h"

#define UNDEFINED_INDEX     -1

SFUShort SFReadUShort(const SFUByte *base, uintptr_t offset);
SFUInt SFReadUInt(const SFUByte *base, uintptr_t offset);

/************************************SCRIPT TABLES***************************************/

typedef struct LangSysTable {
    //Offset lookupOrder = NULL;    (reserved for an offset to a reordering table)
    SFUShort reqFeatureIndex;       //Index of a feature required for this language
                                    //system- if no required features = 0xFFFF
    SFUShort featureCount;          //Number of FeatureIndex values for this language system-excludes
                                    //the required feature
    SFUShort *featureIndex;         //Array of indices into the FeatureList-in arbitrary order
} LangSysTable;

typedef struct LangSysRecord {
    unsigned char LangSysTag[5];    //4-byte LangSysTag identifier
    LangSysTable LangSys;           //Offset to LangSys table-from beginning of Script table
} LangSysRecord;

typedef struct ScriptTable {
    LangSysTable defaultLangSys;    //Offset to DefaultLangSys table-from beginning
                                    //of Script table-may be NULL
    SFUShort langSysCount;          //Number of LangSysRecords for this script-excluding
                                    //the DefaultLangSys
    LangSysRecord *langSysRecord;   //Array of LangSysRecords-listed alphabetically by LangSysTag
} ScriptTable;

typedef struct ScriptRecord {
    unsigned char scriptTag[5];     //4-byte ScriptTag identifier
    ScriptTable	script;             //Offset to Script table-from beginning of ScriptList
} ScriptRecord;

typedef struct ScriptListTable {
    SFUShort scriptCount;           //Number of ScriptRecords
    ScriptRecord *scriptRecord;     //Array of ScriptRecords -listed alphabetically by ScriptTag
} ScriptListTable;

/********************************END SCRIPT TABLES**************************************/

/**********************************FEATURE TABLES***************************************/

typedef struct FeatureTable {
    //void *FeatureParams	= NULL; (reserved for offset to FeatureParams)
    SFUShort lookupCount;           //Number of LookupList indices for this feature
    SFUShort *lookupListIndex;      //Array of LookupList indices for this feature -zero-based
                                    //(first lookup is LookupListIndex = 0)
} FeatureTable;

typedef struct FeatureRecord {
    unsigned char featureTag[5];    //4-byte feature identification tag
    FeatureTable feature;           //Offset to Feature table-from beginning of FeatureList
} FeatureRecord;

typedef struct FeatureListTable {
    SFUShort featureCount;          //Number of FeatureRecords in this table
    FeatureRecord *featureRecord;   //Array of FeatureRecords-zero-based (first feature has
                                    //FeatureIndex = 0)-listed alphabetically by FeatureTag
} FeatureListTable;

/*********************************END FEATURE TABLES*************************************/

/************************************LOOKUP TABLES***************************************/

typedef enum {
    //////////////////////////////////GSUB LOOKUP TYPES///////////////////////////////////

    ltsSingle = 1,                  //Replace one glyph with one glyph
    ltsMultiple = 2,                //Replace one glyph with more than one glyph
    ltsAlternate = 3,               //Replace one glyph with one of many glyphs
    ltsLigature = 4,                //Replace multiple glyphs with one glyph
    ltsContext = 5,                 //One or more glyphs in context
    ltsChainingContext = 6,         //Replace one or more glyphs in chained context
    ltsExtensionSubstitution = 7,   //Extension mechanism for other substitutions (i.e.
                                    //this excludes the Extension type substitution itself)
    ltsReverseChainingContextSingle = 8,
                                    //Applied in reverse order, replace single glyph in chaining context
    ltsReserved = 9,                //For future use (set to zero)
    
    
    //////////////////////////////////GPOS LOOKUP TYPES///////////////////////////////////
    
    ltpSingleAdjustment = 1,        //Adjust position of a single glyph
    ltpPairAdjustment = 2,          //Adjust position of a pair of glyphs
    ltpCursiveAttachment = 3,       //Attach cursive glyphs
    ltpMarkToBaseAttachment = 4,    //Attach a combining mark to a base glyph
    ltpMarkToLigatureAttachment = 5,//Attach a combining mark to a ligature
    ltpMarkToMarkAttachment = 6,    //Attach a combining mark to another mark
    ltpContextPositioning = 7,      //Position one or more glyphs in context
    ltpChainedContextPositioning = 8,
                                    //Position one or more glyphs in chained context
    ltpExtensionPositioning = 9,    //Extension mechanism for other positionings
    ltpReserved = 10,               //For future use (set to zero)
} LookupType;

typedef enum {
    lfRightToLeft = 0x0001,
                                    //This bit relates only to the correct processing of the cursive attachment lookup type (GPOS lookup type 3). When this bit is set, the last glyph in a given sequence to which the cursive attachment lookup is applied, will be positioned on the baseline. 
                                    //Note: Setting of this bit is not intended to be used by operating systems or applications to determine text direction. 
    lfIgnoreBaseGlyphs = 0x0002,
                                    //If set, skips over base glyphs
    lfIgnoreLigatures = 0x0004,
                                    //If set, skips over ligatures
    lfIgnoreMarks = 0x0008,
                                    //If set, skips over all combining marks
    lfUseMarkFilteringSet = 0x0010,
                                    //If set, indicates that the lookup table structure is followed by a MarkFilteringSet field. The layout engine skips over all mark glyphs not in the mark filtering set indicated.
    lfReserved = 0x00E0,
                                    //Reserved for future use (Set to zero)
    lfMarkAttachmentType = 0xFF00,
                                    //If not zero, skips over all marks of attachment type different from specified.
} LookupFlag;

typedef struct LookupTable {
    LookupType lookupType;          //Different enumerations for GSUB and GPOS
    LookupFlag lookupFlag;          //Lookup qualifiers
    SFUShort subTableCount;         //Number of SubTables for this lookup
    void **subtables;               //Array of offsets to SubTables-from beginning of Lookup table
    SFUShort markFilteringSet;      //Index (base 0) into GDEF mark glyph sets structure.
                                    //This field is only present if bit UseMarkFilteringSet
                                    //of lookup flags is set.
} LookupTable;

typedef struct LookupListTable {
    SFUShort lookupCount;           //Number of lookups in this table
    LookupTable *lookupTables;      //Array of offsets to Lookup tables-from beginning of
                                    //LookupList -zero based (first lookup is Lookup index = 0)
} LookupListTable;

typedef void *(*SubtableFunction)(const SFUByte * const, LookupType *type);
typedef void (*FreeSubtableFunction)(void *subtable, LookupType type);

/**********************************END LOOKUP TABLES*************************************/

typedef struct ClassRangeRecord {
    SFGlyph start;                  //First GlyphID in the range
    SFGlyph end;                    //Last GlyphID in the range
    SFUShort cls;                   //Applied to all glyphs in the range
} ClassRangeRecord;

typedef struct ClassDefTable {
    SFUShort classFormat;           //Format identifier
    
    union {
        struct {
            SFUShort startGlyph;    //First GlyphID of the ClassValueArray
            SFUShort glyphCount;    //Size of the ClassValueArray
            SFUShort *classValueArray;
                                    //Array of Class Values-one per GlyphID
        } format1;
        
        struct {
            SFUShort classRangeCount;
                                    //Number of ClassRangeRecords
            ClassRangeRecord *classRangeRecord;
                                    //Array of ClassRangeRecords-ordered by Start GlyphID
        } format2;
    } format;
} ClassDefTable;

typedef struct RangeRecord {
    SFGlyph start;                  //First GlyphID in the range
    SFGlyph end;                    //Last GlyphID in the range
    
    SFUShort startCoverageIndex;    //Coverage Index of first GlyphID in range
} RangeRecord;

typedef struct CoverageTable {
    SFUShort coverageFormat;        //Format identifier-format = 1
    
    union {
        struct {
            SFUShort  glyphCount;   //Number of glyphs in the GlyphArray
            SFGlyph *glyphArray;    //Array of GlyphIDs-in numerical order
        } format1;
        
        struct {
            SFUShort	rangeCount; //Number of RangeRecords
            RangeRecord *rangeRecord;
                                    //Array of glyph ranges-ordered by Start GlyphID
        } format2;
    } format;
    
} CoverageTable;

typedef struct DeviceTable {
    SFUShort startSize;             //Smallest size to correct-in ppem
    SFUShort endSize;               //Largest size to correct-in ppem
    SFUShort deltaFormat;           //Format of DeltaValue array data: 1, 2, or 3
    
    //Format
    //1     2   Signed 2-bit value, 8 values per uint16
    //2     4   Signed 4-bit value, 4 values per uint16
    //3     8   Signed 8-bit value, 2 values per uint16
    
    SFUShort deltaValue;            //Array of compressed data
} DeviceTable;


void SFReadScriptListTable(const SFUByte * const slTable, ScriptListTable *tablePtr);
void SFFreeScriptListTable(ScriptListTable *tablePtr);

void SFReadFeatureListTable(const SFUByte * const flTable, FeatureListTable *tablePtr);
void SFFreeFeatureListTable(FeatureListTable *tablePtr);

void SFReadLookupListTable(const SFUByte * const llTable, LookupListTable *tablePtr, SubtableFunction func);
void SFFreeLookupListTable(LookupListTable *tablePtr, FreeSubtableFunction func);

void SFReadClassDefTable(const SFUByte * const cdTable, ClassDefTable *tablePtr);
void SFFreeClassDefTable(ClassDefTable *tablePtr);

void SFReadCoverageTable(const SFUByte * const cTable, CoverageTable *tablePtr);
void SFFreeCoverageTable(CoverageTable *tablePtr);

void SFReadDeviceTable(const SFUByte * const dTable, DeviceTable *tablePtr);

int SFGetIndexOfGlyphInCoverage(CoverageTable *tablePtr, SFGlyph glyph);

#endif
