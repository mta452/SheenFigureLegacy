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

#include "SFConfig.h"

#include <stdlib.h>
#include <stdint.h>

#ifdef SF_IOS_CG

#include <CoreFoundation/CoreFoundation.h>
#include <CoreGraphics/CoreGraphics.h>

#else

#include <ft2build.h>
#include <freetype/freetype.h>
#include <freetype/ftbitmap.h>
#include <freetype/tttables.h>
#include <freetype/fttypes.h>
#include <freetype/tttags.h>

#endif

#include "SFTypes.h"
#include "SFCommonMethods.h"
#include "SFCMAPData.h"
#include "SFCMAPUtilization.h"
#include "SFGDEFData.h"
#include "SFGDEFUtilization.h"
#include "SFGSUBData.h"
#include "SFGSUBUtilization.h"
#include "SFGPOSData.h"
#include "SFGPOSUtilization.h"

typedef enum {
    itCMAP = 1,
    itGDEF = 2,
    itGSUB = 4,
    itGPOS = 8,
} ImplementedTables;


typedef struct SFFont {
    
#ifdef SF_IOS_CG
    CGFontRef _cgFont;
#else
    FT_Library _ftLib;
    FT_Face _ftFace;
    FT_GlyphSlot _ftSlot;
#endif
    
    SFFloat _size;				//In pixels
    SFFloat _sizeByEm;
    SFFloat _ascender;
    SFFloat _descender;
    SFFloat _leading;
    
    ImplementedTables _availableFontTables;
    
    SFTableCMAP _cmap;
    SFTableGDEF _gdef;
    SFTableGSUB _gsub;
    SFTableGPOS _gpos;
    
    struct SFFont *_parent;
    
    SFUInt _retainCount;
} SFFont;

#ifndef _SF_STRING_RECORD_REF
#define _SF_STRING_RECORD_REF

typedef struct SFStringRecord *SFStringRecordRef;

#endif

#ifndef _SF_FONT_REF
#define _SF_FONT_REF

typedef struct SFFont *SFFontRef;

#endif

#include "SFFont.h"

#ifdef SF_IOS_CG

static void readCMAPTable(SFFontRef sfFont) {
    CFDataRef cmapData = CGFontCopyTableForTag(sfFont->_cgFont, 'cmap');
    
    if (cmapData) {
        const SFUByte * const tableBytes = CFDataGetBytePtr(cmapData);
        SFReadCMAP(tableBytes, &sfFont->_cmap, CFDataGetLength(cmapData));
        sfFont->_availableFontTables |= itCMAP;
        
        CFRelease(cmapData);
    }
}

static void readGDEFTable(SFFontRef sfFont) {
    CFDataRef gdefData = CGFontCopyTableForTag(sfFont->_cgFont, 'GDEF');
    
    if (gdefData) {
        const SFUByte * const tableBytes = CFDataGetBytePtr(gdefData);
        SFReadGDEF(tableBytes, &sfFont->_gdef);
        sfFont->_availableFontTables |= itGDEF;
        
        CFRelease(gdefData);
    }
}

static void readGSUBTable(SFFontRef sfFont) {
    CFDataRef gsubTable = CGFontCopyTableForTag(sfFont->_cgFont, 'GSUB');
    
    if (gsubTable) {
        const SFUByte * const tableBytes = CFDataGetBytePtr(gsubTable);
        SFReadGSUB(tableBytes, &sfFont->_gsub);
        sfFont->_availableFontTables |= itGSUB;
        
        CFRelease(gsubTable);
    }
}

static void readGPOSTable(SFFontRef sfFont) {
    CFDataRef gposTable = CGFontCopyTableForTag(sfFont->_cgFont, 'GPOS');
    
    if (gposTable) {
        const SFUByte * const tableBytes = CFDataGetBytePtr(gposTable);
        SFReadGPOS(tableBytes, &sfFont->_gpos);
        sfFont->_availableFontTables |= itGPOS;
        
        CFRelease(gposTable);
    }
}

static void setFontSize(SFFontRef sfFont, SFFloat size) {
	sfFont->_size = size;
	sfFont->_sizeByEm = size / CGFontGetUnitsPerEm(sfFont->_cgFont);
	sfFont->_ascender = sfFont->_sizeByEm * CGFontGetAscent(sfFont->_cgFont);
	sfFont->_descender = sfFont->_sizeByEm * CGFontGetDescent(sfFont->_cgFont);
	sfFont->_leading = sfFont->_ascender - sfFont->_descender;
}

SFFontRef SFFontCreate(CGFontRef cgFont, CGFloat size) {
    SFFont *sfFont = malloc(sizeof(SFFont));
    
    sfFont->_cgFont = CGFontRetain(cgFont);
    sfFont->_availableFontTables = 0;
    sfFont->_parent = NULL;
    sfFont->_retainCount = 1;
    
    setFontSize(sfFont, size);
    
    return sfFont;
}

SFFontRef SFFontCreateWithFileName(CFStringRef name, CFStringRef extension, CGFloat size) {
    CFBundleRef mainBundle = CFBundleGetMainBundle();
    CFURLRef url = CFBundleCopyResourceURL(mainBundle, name, (CFStringRef)extension, NULL);
    
    CGDataProviderRef dataProvider = CGDataProviderCreateWithURL(url);
    CGFontRef fnt = CGFontCreateWithDataProvider(dataProvider);
    
    SFFontRef sfFont = SFFontCreate(fnt, size);
    
    CGFontRelease(fnt);
    CGDataProviderRelease(dataProvider);
    CFRelease(url);
    
    return sfFont;
}

CGFontRef SFFontGetCGFont(SFFontRef sfFont) {
    return sfFont->_cgFont;
}

#else

static void readCMAPTable(SFFontRef sfFont) {
	FT_ULong length = 0;
    
    FT_Tag tag;
    FT_Byte *buffer;
    
    FT_Error error;
    
	tag = FT_MAKE_TAG('c', 'm', 'a', 'p');
	error = FT_Load_Sfnt_Table(sfFont->_ftFace, tag, 0, NULL, &length);
	if (error)
		return;
    
	buffer = malloc(length);
	error = FT_Load_Sfnt_Table(sfFont->_ftFace, tag, 0, buffer, &length);
	if (!error) {
		SFReadCMAP(buffer, &sfFont->_cmap, length);
		sfFont->_availableFontTables |= itCMAP;
	}
    
	free(buffer);
}

static void readGDEFTable(SFFontRef sfFont) {
	FT_ULong length = 0;
    
    FT_Tag tag;
    FT_Byte *buffer;
    
    FT_Error error;
    
	tag = FT_MAKE_TAG('G', 'D', 'E', 'F');
	error = FT_Load_Sfnt_Table(sfFont->_ftFace, tag, 0, NULL, &length);
	if (error)
		return;
    
	buffer = malloc(length);
	error = FT_Load_Sfnt_Table(sfFont->_ftFace, tag, 0, buffer, &length);
	if (!error) {
		SFReadGDEF(buffer, &sfFont->_gdef);
		sfFont->_availableFontTables |= itGDEF;
	}
    
	free(buffer);
}

static void readGSUBTable(SFFontRef sfFont) {
	FT_ULong length = 0;
    
    FT_Tag tag;
    FT_Byte *buffer;
    
    FT_Error error;
    
	tag = FT_MAKE_TAG('G', 'S', 'U', 'B');
	error = FT_Load_Sfnt_Table(sfFont->_ftFace, tag, 0, NULL, &length);
	if (error)
		return;
    
	buffer = malloc(length);
	error = FT_Load_Sfnt_Table(sfFont->_ftFace, tag, 0, buffer, &length);
	if (!error) {
		SFReadGSUB(buffer, &sfFont->_gsub);
		sfFont->_availableFontTables |= itGSUB;
	}
    
	free(buffer);
}

static void readGPOSTable(SFFontRef sfFont) {
	FT_ULong length = 0;
    
    FT_Tag tag;
    FT_Byte *buffer;
    
    FT_Error error;
    
	tag = FT_MAKE_TAG('G', 'P', 'O', 'S');
	error = FT_Load_Sfnt_Table(sfFont->_ftFace, tag, 0, NULL, &length);
	if (error)
		return;
    
	buffer = malloc(length);
    error = FT_Load_Sfnt_Table(sfFont->_ftFace, tag, 0, buffer, &length);
	if (!error) {
		SFReadGPOS(buffer, &sfFont->_gpos);
		sfFont->_availableFontTables |= itGPOS;
	}
    
	free(buffer);
}

static void setFontSize(SFFontRef sfFont, SFFloat size) {
	sfFont->_size = size;
	sfFont->_sizeByEm = size / sfFont->_ftFace->units_per_EM;
	sfFont->_ascender = sfFont->_sizeByEm * sfFont->_ftFace->ascender;
	sfFont->_descender = sfFont->_sizeByEm * sfFont->_ftFace->descender;
	sfFont->_leading = sfFont->_ascender - sfFont->_descender;
}

SFFontRef SFFontCreateWithFileName(const char *name, SFFloat size) {
	SFFont *sfFont = malloc(sizeof(SFFont));
	int error = FT_Init_FreeType(&sfFont->_ftLib);

	if (error) {
		printf("Could not init freetype library\n");
		return NULL;
	}
    
	error = FT_New_Face(sfFont->_ftLib, name, 0, &sfFont->_ftFace);
	if (error) {
		printf("Could not open font from path \"%s\"", name);
        
		FT_Done_FreeType(sfFont->_ftLib);
        
		return NULL;
	}

	FT_Set_Char_Size(sfFont->_ftFace, 0, size * 64, 72, 72);
    
	setFontSize(sfFont, size);
    
	sfFont->_ftSlot = sfFont->_ftFace->glyph;
	sfFont->_availableFontTables = 0;
	sfFont->_parent = NULL;
	sfFont->_retainCount = 1;
    
	return sfFont;
}

FT_Face SFFontGetFTFace(SFFontRef sfFont) {
	return sfFont->_ftFace;
}

#endif

SFFontRef SFFontMakeClone(SFFontRef sfFont, SFFloat size) {
	SFFont *clone = malloc(sizeof(SFFont));
    
    sfFont = SFFontRetain(sfFont);
    
#ifdef SF_IOS_CG
	clone->_cgFont = sfFont->_cgFont;
#else
	clone->_ftLib = sfFont->_ftLib;
	clone->_ftFace = sfFont->_ftFace;
	clone->_ftSlot = sfFont->_ftSlot;
#endif
    
	setFontSize(clone, size);
    
	clone->_availableFontTables = sfFont->_availableFontTables;
    
	clone->_cmap = sfFont->_cmap;
	clone->_gdef = sfFont->_gdef;
	clone->_gsub = sfFont->_gsub;
	clone->_gpos = sfFont->_gpos;
	clone->_parent = sfFont;
	clone->_retainCount = 1;
    
	return clone;
}

static void readFontTables(SFFontRef sfFont) {
    if (!sfFont->_availableFontTables) {
        readCMAPTable(sfFont);
        readGDEFTable(sfFont);
        readGSUBTable(sfFont);
        readGPOSTable(sfFont);
    }
}

SFStringRecordRef SFFontAllocateStringRecordForString(SFFontRef sfFont, SFUnichar *inputString, int length) {
	if (inputString && length) {
        SFStringRecordRef record = malloc(sizeof(SFStringRecord));
        
        int allocSize = (sizeof(int) * length);
        int *types = malloc(allocSize);
        int *levels = malloc(allocSize);
        int *lOrder = malloc(allocSize);
        
        resolveBidi(inputString, types, levels, length, lOrder);
        
        // SFDeallocateStringRecord will be responsible for freeing inputString, levels and lOrder
        SFAllocateStringRecord(record, inputString, levels, lOrder, length);
        
        free(types);
        
        readFontTables(sfFont);
        
        if (sfFont->_availableFontTables & itCMAP)
            SFApplyCMAP(&sfFont->_cmap, record);
        
        if (sfFont->_availableFontTables & itGSUB)
            SFApplyGSUB(&sfFont->_gsub,
                        (sfFont->_availableFontTables & itGDEF) ? &sfFont->_gdef : NULL
                        , record);
        
        if (sfFont->_availableFontTables & itGPOS)
            SFApplyGPOS(&sfFont->_gpos,
                        (sfFont->_availableFontTables & itGDEF) ? &sfFont->_gdef : NULL
                        , record);
        
        return record;
    }
    
	return NULL;
}

void SFFontDeallocateStringRecord(SFFontRef sfFont, SFStringRecordRef strRecord) {
    if (strRecord) {
        SFFreeStringRecord(strRecord);
        free(strRecord);
    }
}

SFFloat SFFontGetSize(SFFontRef sfFont) {
    return sfFont->_size;
}

SFFloat SFFontGetSizeByEm(SFFontRef sfFont) {
    return sfFont->_sizeByEm;
}

SFFloat SFFontGetAscender(SFFontRef sfFont) {
    return sfFont->_ascender;
}

SFFloat SFFontGetDescender(SFFontRef sfFont) {
    return sfFont->_descender;
}

SFFloat SFFontGetLeading(SFFontRef sfFont) {
    return sfFont->_leading;
}

SFFontRef SFFontRetain(SFFontRef sfFont) {
    if (sfFont)
        sfFont->_retainCount++;
    
    return sfFont;
}

void SFFontRelease(SFFontRef sfFont) {
    if (!sfFont)
        return;
    
    sfFont->_retainCount--;
    
    if (sfFont->_retainCount == 0) {
        if (sfFont->_parent)
    		SFFontRelease(sfFont->_parent);
    	else {
            
#ifdef SF_IOS_CG
    		CGFontRelease(sfFont->_cgFont);
#else
    		if (sfFont->_ftFace)
    			FT_Done_Face(sfFont->_ftFace);
            
    		if (sfFont->_ftLib)
    			FT_Done_FreeType(sfFont->_ftLib);
#endif
            
    		if (sfFont->_availableFontTables & itCMAP)
    			SFFreeCMAP(&sfFont->_cmap);
            
    		if (sfFont->_availableFontTables & itGDEF)
    			SFFreeGDEF(&sfFont->_gdef);
            
    		if (sfFont->_availableFontTables & itGSUB)
    			SFFreeGSUB(&sfFont->_gsub);
            
    		if (sfFont->_availableFontTables & itGPOS)
    			SFFreeGPOS(&sfFont->_gpos);
    	}
        
        free(sfFont);
    }
}
