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

#ifndef _SF_FONT_PRIVATE_H
#define _SF_FONT_PRIVATE_H

#include "SFConfig.h"
#include "SFTypes.h"

#ifdef SF_IOS_CG
#include <CoreGraphics/CoreGraphics.h>
#else
#include <ft2build.h>
#include <freetype/freetype.h>
#include <freetype/ftadvanc.h>
#include <freetype/ftbitmap.h>
#include <freetype/tttables.h>
#include <freetype/fttypes.h>
#include <freetype/tttags.h>
#endif

#include "SFCMAPData.h"
#include "SFGDEFData.h"
#include "SFGSUBData.h"
#include "SFGPOSData.h"

typedef enum {
    itCMAP = 1,
    itGDEF = 2,
    itGSUB = 4,
    itGPOS = 8,
} SFImplementedTable;


typedef struct SFFontTables {
    SFImplementedTable _availableTables;
    
    SFTableCMAP _cmap;
    SFTableGDEF _gdef;
    SFTableGSUB _gsub;
    SFTableGPOS _gpos;
    
    SFUInt _retainCount;
} SFFontTables;

typedef struct SFFontTables *SFFontTablesRef;


typedef struct SFFont {
#ifdef SF_IOS_CG
    CGFontRef _cgFont;
#else
    FT_Face _ftFace;
#endif
    
    SFFontTables *_tables;
    
    SFFloat _size;				//In pixels
    SFFloat _sizeByEm;
    SFFloat _ascender;
    SFFloat _descender;
    SFFloat _leading;
    
    SFUInt _retainCount;
} SFFont;

#ifndef _SF_FONT_REF
#define _SF_FONT_REF

typedef struct SFFont *SFFontRef;

#endif

void SFFontReadTables(SFFontRef sfFont);

#endif
