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

#include "SFFontPrivate.h"
#include "SFFont.h"

static SFFontTablesRef SFFontTablesCreate() {
    SFFontTablesRef sfFontTables = malloc(sizeof(SFFontTables));
    sfFontTables->_availableTables = 0;
    sfFontTables->_retainCount = 1;
    
    return sfFontTables;
}

static SFFontTablesRef SFFontTablesRetain(SFFontTablesRef sfFontTables) {
    if (sfFontTables) {
        sfFontTables->_retainCount++;
    }
    
    return sfFontTables;
}

static void SFFontTablesRelease(SFFontTablesRef sfFontTables) {
    if (!sfFontTables) {
        return;
    }
    
    sfFontTables->_retainCount--;
    
    if (sfFontTables->_retainCount == 0) {
        if (sfFontTables->_availableTables & itCMAP) {
            SFFreeCMAP(&sfFontTables->_cmap);
        }
        
        if (sfFontTables->_availableTables & itGDEF) {
            SFFreeGDEF(&sfFontTables->_gdef);
        }
        
        if (sfFontTables->_availableTables & itGSUB) {
            SFFreeGSUB(&sfFontTables->_gsub);
        }
        
        if (sfFontTables->_availableTables & itGPOS) {
            SFFreeGPOS(&sfFontTables->_gpos);
        }
        
        free(sfFontTables);
    }
}

#ifdef SF_IOS_CG

static void readCMAPTable(SFFontRef sfFont) {
    CFDataRef cmapData = CGFontCopyTableForTag(sfFont->_cgFont, 'cmap');
    
    if (cmapData) {
        const SFUByte * const tableBytes = CFDataGetBytePtr(cmapData);
        SFReadCMAP(tableBytes, &sfFont->_tables->_cmap, CFDataGetLength(cmapData));
        sfFont->_tables->_availableTables |= itCMAP;
        
        CFRelease(cmapData);
    }
}

static void readGDEFTable(SFFontRef sfFont) {
    CFDataRef gdefData = CGFontCopyTableForTag(sfFont->_cgFont, 'GDEF');
    
    if (gdefData) {
        const SFUByte * const tableBytes = CFDataGetBytePtr(gdefData);
        SFReadGDEF(tableBytes, &sfFont->_tables->_gdef);
        sfFont->_tables->_availableTables |= itGDEF;
        
        CFRelease(gdefData);
    }
}

static void readGSUBTable(SFFontRef sfFont) {
    CFDataRef gsubTable = CGFontCopyTableForTag(sfFont->_cgFont, 'GSUB');
    
    if (gsubTable) {
        const SFUByte * const tableBytes = CFDataGetBytePtr(gsubTable);
        SFReadGSUB(tableBytes, &sfFont->_tables->_gsub);
        sfFont->_tables->_availableTables |= itGSUB;
        
        CFRelease(gsubTable);
    }
}

static void readGPOSTable(SFFontRef sfFont) {
    CFDataRef gposTable = CGFontCopyTableForTag(sfFont->_cgFont, 'GPOS');
    
    if (gposTable) {
        const SFUByte * const tableBytes = CFDataGetBytePtr(gposTable);
        SFReadGPOS(tableBytes, &sfFont->_tables->_gpos);
        sfFont->_tables->_availableTables |= itGPOS;
        
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

SFFontRef SFFontCreateWithCGFont(CGFontRef cgFont, SFFloat size) {
    SFFont *sfFont = malloc(sizeof(SFFont));
    sfFont->_cgFont = CGFontRetain(cgFont);
    sfFont->_tables = SFFontTablesCreate();
    sfFont->_retainCount = 1;

    setFontSize(sfFont, size);
    
    return sfFont;
}

SFFontRef SFFontMakeCloneForCGFont(SFFontRef sfFont, CGFontRef cgFont, SFFloat size) {
    SFFont *clone = malloc(sizeof(SFFont));
    clone->_cgFont = CGFontRetain(cgFont);
    clone->_tables = SFFontTablesRetain(sfFont->_tables);
    clone->_retainCount = 1;

    setFontSize(clone, size);
    
    return clone;
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
		SFReadCMAP(buffer, &sfFont->_tables->_cmap, length);
		sfFont->_tables->_availableTables |= itCMAP;
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
		SFReadGDEF(buffer, &sfFont->_tables->_gdef);
		sfFont->_tables->_availableTables |= itGDEF;
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
		SFReadGSUB(buffer, &sfFont->_tables->_gsub);
		sfFont->_tables->_availableTables |= itGSUB;
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
		SFReadGPOS(buffer, &sfFont->_tables->_gpos);
		sfFont->_tables->_availableTables |= itGPOS;
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

SFFontRef SFFontCreateWithFTFace(FT_Face ftFace, SFFloat size) {
    SFFont *sfFont = malloc(sizeof(SFFont));
    sfFont->_ftFace = ftFace;
    sfFont->_tables = SFFontTablesCreate();
	sfFont->_retainCount = 1;

    FT_Reference_Face(ftFace);
	setFontSize(sfFont, size);

	return sfFont;
}

SFFontRef SFFontMakeCloneForFTFace(SFFontRef sfFont, FT_Face ftFace, SFFloat size) {
	SFFont *clone = malloc(sizeof(SFFont));
	clone->_ftFace = ftFace;
    clone->_tables = SFFontTablesRetain(sfFont->_tables);
    clone->_retainCount = 1;
    
    FT_Reference_Face(ftFace);
	setFontSize(clone, size);
    
	return clone;
}

FT_Face SFFontGetFTFace(SFFontRef sfFont) {
	return sfFont->_ftFace;
}

#endif

void SFFontReadTables(SFFontRef sfFont) {
    if (!sfFont->_tables->_availableTables) {
        readCMAPTable(sfFont);
        readGDEFTable(sfFont);
        readGSUBTable(sfFont);
        readGPOSTable(sfFont);
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
    if (sfFont) {
        sfFont->_retainCount++;
    }
    
    return sfFont;
}

void SFFontRelease(SFFontRef sfFont) {
    if (!sfFont) {
        return;
    }
    
    sfFont->_retainCount--;
    
    if (sfFont->_retainCount == 0) {
#ifdef SF_IOS_CG
        if (sfFont->_cgFont) {
            CGFontRelease(sfFont->_cgFont);
        }
#else
        if (sfFont->_ftFace) {
            FT_Done_Face(sfFont->_ftFace);
        }
#endif
            
        SFFontTablesRelease(sfFont->_tables);
        
        free(sfFont);
    }
}
