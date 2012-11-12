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

//#define GDEF_TEST

#ifndef _SF_GDEF_DATA_H
#define _SF_GDEF_DATA_H

#include "SFConfig.h"
#include "SFTypes.h"
#include "SFCommonData.h"


#ifdef GDEF_GLYPH_CLASS_DEF

typedef enum {
    gcvBase = 1,                     //(single character, spacing glyph)
    gcvLigature = 2,                 //(multiple character, spacing glyph)
    gcvMark = 3,                     //(non-spacing combining glyph)
    gcvComponent = 4,                //glyph (part of single character, spacing glyph)
} GlyphClassValue;

#endif


#ifdef GDEF_ATTACH_LIST

typedef struct AttachPointTable {
    SFUShort pointCount;            //Number of attachment points on this glyph
    SFUShort *pointIndex;           //Array of contour point indices -in increasing numerical order
} AttachPointTable;

typedef struct AttachListTable {
    CoverageTable coverage;         //Offset to Coverage table - from beginning of AttachList table
    SFUShort glyphCount;            //Number of glyphs with attachment points
    AttachPointTable *attachPoint;  //Array of offsets to AttachPoint tables-from beginning of
                                    //AttachList table-in Coverage Index order
} AttachListTable;

#endif


#ifdef GDEF_LIG_CARET_LIST

typedef struct CaretValuesTable {
    SFUShort caretValueFormat;      //Format identifier
    
    union {
        struct {
            short coordinate;       //X or Y value, in design units
        } format1;
        
        struct {
            SFUShort caretValuePoint;
                                    //Contour point index on glyph;
        } format2;
        
        struct {
            short coordinate;       //X or Y value, in design units
            DeviceTable deviceTable;
                                    //Offset to Device table for X or Y value-from beginning
                                    //of CaretValue table
        } format3;
    } format;
} CaretValuesTable;

typedef struct LigatureGlyphTable {
    SFUShort caretCount;            //Number of CaretValues for this ligature (components - 1)
    CaretValuesTable *caretValue;   //Array of offsets to CaretValue tables-from beginning
                                    //of LigGlyph table-in increasing coordinate order
} LigatureGlyphTable;

typedef struct LigatureCaretListTable {
    CoverageTable coverage;         //Offset to Coverage table - from beginning of LigCaretList table
    SFUShort ligGlyphCount;         //Number of ligature glyphs
    LigatureGlyphTable *LigGlyph;   //Array of offsets to LigGlyph tables-from beginning of
                                    //LigCaretList table-in Coverage Index order
} LigatureCaretListTable;

#endif


#ifdef GDEF_MARK_ATTACH_CLASS_DEF

typedef struct MarkGlyphSetsDefTable {
    SFUShort markSetTableFormat;    //Format identifier == 1
    SFUShort markSetCount;          //Number of mark sets defined
    CoverageTable *coverage;        //Array of offsets to mark set coverage tables.
} MarkGlyphSetsDefTable;

#endif


typedef struct SFTableGDEF {
    SFUInt version;                 //Version of the GDEF table-0x00010000 or 0x00010002
    
#ifdef GDEF_GLYPH_CLASS_DEF
    
    SFBool hasGlyphClassDef;
    ClassDefTable glyphClassDef;    //Offset to class definition table for glyph type-from
                                    //beginning of GDEF header (may be NULL)
    
#endif


#ifdef GDEF_ATTACH_LIST
    
    SFBool hasAttachList;
    AttachListTable attachList;     //Offset to list of glyphs with attachment points-from
                                    //beginning of GDEF header (may be NULL)
    
#endif


#ifdef GDEF_LIG_CARET_LIST
    
    SFBool hasLigCaretList;
    LigatureCaretListTable ligCaretList;
                                    //Offset to list of positioning points for ligature carets-from
                                    //beginning of GDEF header (may be NULL)
    
#endif
    
    
#ifdef GDEF_MARK_ATTACH_CLASS_DEF
    
    SFBool hasMarkAttachClassDef;
    ClassDefTable markAttachClassDef;
                                    //Offset to class definition table for mark attachment type-from
                                    //beginning of GDEF header (may be NULL)
    
#endif


#ifdef GDEF_MARK_GLYPH_SETS_DEF
    
    SFBool hasMarkGlyphSetsDef;
    MarkGlyphSetsDefTable markGlyphSetsDef;
                                    //Offset to the table of mark set definitions - from beginning
                                    //of GDEF header (may be NULL)
    
#endif

} SFTableGDEF;

void SFReadGDEF(const SFUByte * const table, SFTableGDEF *tablePtr);
void SFFreeGDEF(SFTableGDEF *tablePtr);


#endif
