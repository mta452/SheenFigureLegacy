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

#ifndef _SF_CMAP_DATA_H
#define _SF_CMAP_DATA_H

#include "SFTypes.h"

typedef enum {
    piUnicode = 0,                      //Indicates Unicode version.
    piMacintosh = 1,                    //Script Manager code.
    piReserved = 2,                     //(2 = reserved; do not use)
    piMicrosoft = 3,                    //Microsoft encoding.
} PlatformID;

typedef enum {
    uniDefault_semantics = 0,
    uniVersion_11_semantics = 1,
    uniISO_10646_1993_semantics = 2,    //(deprecated)
    uniVersion_20_or_later_semantics = 3,
} UnicodeEncodingID;

typedef enum {
    macRoman = 0,
    macJapanese = 1,
    macTraditionalChinese = 2,
    macKorean = 3,
    macArabic = 4,
    macHebrew = 5,
    macGreek = 6,
    macRussian = 7,
    macRSymbol = 8,
    macDevanagari = 9,
    macGurmukhi = 10,
    macGujarati = 11,
    macOriya = 12,
    macBengali = 13,
    macTamil = 14,
    macTelugu = 15,
    macKannada = 16,
    macMalayalam = 17,
    macSinhalese = 18,
    macBurmese = 19,
    macKhmer = 20,
    macThai = 21,
    macLaotian = 22,
    macGeorgian = 23,
    macArmenian = 24,
    macSimplifiedChinese = 25,
    macTibetan = 26,
    macMongolian = 27,
    macGeez = 28,
    macSlavic = 29,
    macVietnamese = 30,
    macSindhi = 31,
    macUninterpreted = 32,
} MacintoshEncodingID;
    
typedef enum {
    msSymbol = 0, 
    msUnicodeBMP_UCS_2 = 1,
    msShiftJIS = 2,
    msPRC = 3,
    msBig5 = 4,
    msWansung = 5,
    msJohab = 6,
    msReserved = 7,                     //(7 - 9) Reserved
    msUnicode_UCS_4 = 10,
} MicrosoftEncodingID;

typedef struct EncodingFormat {
    SFUShort formatNumber;
    
    union {
        struct {
            SFUShort length;            //Length of subtable in bytes	 
            SFUShort language;          //Language code for this encoding subtable, or zero
                                        //if language-independent	 
            SFUShort segCountX2;        //2 * segCount	 
            SFUShort searchRange;       //2 * (2**floor(log2(segCount)))	 
            SFUShort entrySelector;     //log2(searchRange/2)	 
            SFUShort rangeShift;        //(2 * segCount) - searchRange	 
            SFUShort *endCount;         //Ending character code for each segment, last = 0xFFFF.	
            SFUShort reservedPad;       //This value should be zero	
            SFUShort *startCount;       //Starting character code for each segment	
            SFUShort *idDelta;          //Delta for all character codes in segment	 
            SFUShort *idRangeOffset;    //Offset in bytes to glyph indexArray, or 0	 
            SFUShort *glyphIdArray;     //Glyph index array
        } format4;
    } format;
} EncodingFormat;

typedef struct EncodingSubtable {
    PlatformID platformID;              //Platform identifier
    union {
        UnicodeEncodingID unicodeEncodingID;
        MacintoshEncodingID macEncodingID;
        MicrosoftEncodingID msEncodingID;
    } encodingID;
    
    EncodingFormat encodingFormat;
} EncodingSubtable;
    
typedef struct SFTableCMAP {
    SFUShort version;                   //Table version number (0).
    SFUShort numTables;                 //Number of encoding tables that follow.
    EncodingSubtable encodingSubtable;
} SFTableCMAP;

void SFReadCMAP(const SFUByte * const table, SFTableCMAP *tablePtr, long cmapLength);
void SFFreeCMAP(SFTableCMAP *tablePtr);

#endif
