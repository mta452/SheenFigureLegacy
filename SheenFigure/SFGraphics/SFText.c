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

#include <stdlib.h>
#include <stdint.h>
#include <ctype.h>
#include <math.h>

#include "SFConfig.h"

#ifdef SF_IOS_CG
#include <CoreGraphics/CoreGraphics.h>
#else
#include <ft2build.h>
#include <freetype/freetype.h>
#include <freetype/ftadvanc.h>
#endif

#include "SFCommonData.h"
#include "SFCommonMethods.h"

#ifndef _SF_STRING_RECORD_REF
#define _SF_STRING_RECORD_REF
typedef struct SFStringRecord *SFStringRecordRef;
#endif

#include "SFFont.h"
#include "SFText.h"

typedef struct SFMeasuredLine {
    float width;
    SFRange range1;
    SFRange range2;
    SFRange range3;
    int ranges;
    int nextIndex;
} SFMeasuredLine;

typedef struct SFText {
    SFFloat _pageWidth;
    CGPoint _initialPosition;
    
    SFFontRef _sfFont;
    SFStringRecordRef _record;
    
#ifdef SF_IOS_CG
	CGContextRef _context;
	CGColorRef _txtColor;
#else
	SFColor _txtColor;
#endif
    
    SFTextAlignment _txtAlign;
    
    void *_resObj;
    
    SFUInt _retainCount;
} SFText;

#define ZERO_RANGE(range) range.start = 0; range.end = 0;

static int SFGetAdvance(SFStringRecord *record, SFFontRef sfFont, int recordIndex, int glyphIndex) {
#ifdef SF_IOS_CG
    int adv;
#else
    FT_Fixed adv;
#endif
    
    SFGlyph glyph = record->charRecord[recordIndex].gRec[glyphIndex].glyph;
    
    if (record->charRecord[recordIndex].gRec[glyphIndex].glyphProp & gpAdvance)
    	return record->charRecord[recordIndex].gRec[glyphIndex].advance;
    
#ifdef SF_IOS_CG
    CGFontGetGlyphAdvances(SFFontGetCGFont(sfFont), &glyph, 1, &adv);
#else
    FT_Get_Advance(SFFontGetFTFace(sfFont), glyph, FT_LOAD_NO_SCALE, &adv);
#endif
    
    record->charRecord[recordIndex].gRec[glyphIndex].advance = adv;
    record->charRecord[recordIndex].gRec[glyphIndex].glyphProp |= gpAdvance;
    
    return adv;
}

static void mergeRanges(SFMeasuredLine *line, SFRange additionalRange) {
    if (line->ranges <= 0)
        return;
    
    if (line->range1.end == line->range2.start || (line->range1.end + 1) == line->range2.start) {
        line->range1.end = line->range2.end;
        line->ranges--;
    }
    
    if (line->ranges == 2) {
        if (line->range2.end == line->range3.start || (line->range2.end + 1) == line->range3.start) {
            line->range2.end = line->range3.end;
            line->ranges--;
        }
        
        if (additionalRange.end > 0 && line->range3.start <= additionalRange.start) {
            line->range3.end = additionalRange.end;
        }
    }
}

static void addRange(SFMeasuredLine *line, SFRange range) {
    if (line->ranges == -1) {
        line->range1 = range;
        line->ranges++;
    } else if (line->ranges == 0) {
        line->range2 = range;
        line->ranges++;
    } else if (line->ranges == 1) {
        line->range3 = range;
        ZERO_RANGE(range);
        line->ranges++;
    }
    
    mergeRanges(line, range);
}

static SFMeasuredLine getLine(SFTextRef sfText, SFStringRecord *record, int startIndex) {
	SFFloat width = 0.0f;
    SFFontRef sfFont = sfText->_sfFont;
    
    SFGlyph glyph;
    SFPositionRecord crntPosRecord;
    SFMeasuredLine line;
    SFMeasuredLine tmpLine;
    
    SFRange crntSeg;
    
    SFUnichar ch;
    int chIndex;
    int level;
    
	int i;

    if (startIndex >= record->charCount) {
        line.ranges = -1;
        line.nextIndex = -1;
        line.width = 0;
        
        return line;
    }
    
    tmpLine.ranges = -1;
    tmpLine.nextIndex = startIndex + 1;
    ZERO_RANGE(tmpLine.range1);
    ZERO_RANGE(tmpLine.range2);
    ZERO_RANGE(tmpLine.range3);
    
    crntSeg.start = crntSeg.end = record->lOrder[startIndex];
    level = record->levels[startIndex];
    
    for (i = startIndex; i < record->charCount; i++) {
        int j, count;
        
        chIndex = record->lOrder[i];
        ch = record->chars[chIndex];
        
        if (ch == ' ' || ch == '\n' || i == record->charCount - 1) {
            line = tmpLine;
            line.nextIndex = i + 1;
            line.width = width;
            addRange(&line, crntSeg);
            
            if (ch == '\n')
                return line;
        }
        
        if (level != record->levels[i]) {
            addRange(&tmpLine, crntSeg);
            
            crntSeg.end = chIndex;
            crntSeg.start = chIndex;
            
            level = record->levels[i];
        } else {
            if (odd(record->levels[i]))
                crntSeg.start = chIndex;
            else
                crntSeg.end = chIndex;
        }
        
        count = record->charRecord[chIndex].glyphCount;
        
        for (j = 0; j < count; j++) {
            SFAnchorType aType;
            
            glyph = record->charRecord[chIndex].gRec[j].glyph;
            if (!glyph)
                continue;
            
            crntPosRecord = record->charRecord[chIndex].gRec[j].posRec;
            aType = crntPosRecord.anchorType;
            if (glyph && !(aType & atMark)) {
                if (aType & atEntry)
                    width -= crntPosRecord.anchor.x * SFFontGetSizeByEm(sfFont);
                else {
                    int adv = SFGetAdvance(record, sfFont, chIndex, j);
                    width += (crntPosRecord.advance.x + adv - crntPosRecord.placement.x) * SFFontGetSizeByEm(sfFont);
                }
            }
            
            if (ceil(width) >= floor(sfText->_pageWidth))
                return line;
        }
    }
    
    line = tmpLine;
    line.nextIndex =-1;
    line.width = width;
    addRange(&line, crntSeg);
    
    return line;
}

#ifdef SF_IOS_CG

SFTextRef SFTextCreate(SFFontRef sfFont, CFStringRef str, SFFloat pageWidth) {
    SFTextRef sfText = malloc(sizeof(SFText));
    
    SFFloat colorComponents[4] = {0, 0, 0, 1};
    CGColorSpaceRef devRGB = CGColorSpaceCreateDeviceRGB();
    sfText->_txtColor = CGColorCreate(devRGB, colorComponents);
    CGColorSpaceRelease(devRGB);
    
    sfText->_context = NULL;
    sfText->_sfFont = SFFontRetain(sfFont);
    sfText->_initialPosition = CGPointMake(0, 0);
    sfText->_record = NULL;
    sfText->_resObj = NULL;
    
    if (sfFont && str) {
        int textlen = CFStringGetLength(str);
        if (textlen > 0) {
            SFUnichar *text = malloc(sizeof(SFUnichar) * textlen);
            CFStringGetCharacters(str, CFRangeMake(0, textlen), text);
            
            sfText->_record = SFFontAllocateStringRecordForString(sfFont, text, textlen);
        }
    }
    
    sfText->_txtAlign = SFTextAlignmentRight;
    sfText->_pageWidth = pageWidth;
    sfText->_retainCount = 1;

    return sfText;
}

void SFTextChangeString(SFTextRef sfText, CFStringRef str) {
    UniChar *characters;
    CFIndex length;
    
    length = CFStringGetLength(str);
    if (length == 0)
        return;
    
    SFFontDeallocateStringRecord(sfText->_sfFont, sfText->_record);
    
    characters = malloc(sizeof(UniChar) * length);
    CFStringGetCharacters(str, CFRangeMake(0, length), characters);
    
    sfText->_record = SFFontAllocateStringRecordForString(sfText->_sfFont, characters, length);
}

void SFTextSetColor(SFTextRef sfText, CGColorRef color) {
    if (sfText->_txtColor)
        CGColorRelease(sfText->_txtColor);
    
    sfText->_txtColor = CGColorRetain(color);
}

#else

SFTextRef SFTextCreate(SFFontRef sfFont, SFUnichar *str, int length, SFFloat pageWidth) {
	SFTextRef sfText = malloc(sizeof(SFText));
    
	sfText->_sfFont = SFFontRetain(sfFont);
	sfText->_initialPosition.x = 0;
	sfText->_initialPosition.y = 0;
	sfText->_record = NULL;
    sfText->_resObj = NULL;
    
	if (sfFont)
		sfText->_record = SFFontAllocateStringRecordForString(sfFont, str, length);
    
	sfText->_txtAlign = SFTextAlignmentRight;
	sfText->_pageWidth = pageWidth;
	sfText->_retainCount = 1;
	sfText->_txtColor = 0;
    
	return sfText;
}

void SFTextChangeString(SFTextRef sfText, SFUnichar *str, int length) {
	SFFontDeallocateStringRecord(sfText->_sfFont, sfText->_record);

    if (sfText->_sfFont)
    	sfText->_record = SFFontAllocateStringRecordForString(sfText->_sfFont, str, length);
    else
    	sfText->_record = NULL;
}

void SFTextSetColor(SFTextRef sfText, SFColor color) {
#ifdef SF_IOS
    //For iOS, in one uint, bytes are stored in reversed order
    //as BGRA, i.e. blue at first place and alpha at last place.
    SFUByte r = color >> 16;
    SFUByte g = (color >> 8) & 0xF;
    SFUByte b = color & 0xF;
    sfText->_txtColor = ((b << 16) | (g << 8)) | r;
#else
    //For android, bytes are stored in correct order as ARGB.
	sfText->_txtColor = color;
#endif
}

#endif

void SFTextSetReservedObject(SFTextRef sfText, void *obj) {
	sfText->_resObj = obj;
}

void SFTextChangeFont(SFTextRef sfText, SFFontRef sfFont) {
	if (sfFont) {
		SFFontDeallocateStringRecord(sfText->_sfFont, sfText->_record);
		SFFontRelease(sfText->_sfFont);
        
		sfText->_sfFont = SFFontRetain(sfFont);
	}
    
    sfText->_record = NULL;
}

void SFTextChangePageWidth(SFTextRef sfText, SFFloat pageWidth) {
    sfText->_pageWidth = pageWidth;
}

void SFTextSetAlignment(SFTextRef sfText, SFTextAlignment align) {
    sfText->_txtAlign = align;
}

void SFTextSetInitialPosition(SFTextRef sfText, CGPoint pos) {
    sfText->_initialPosition = pos;
}

SFFontRef SFTextGetFont(SFTextRef sfText) {
    return sfText->_sfFont;
}

SFTextRef SFTextRetain(SFTextRef sfText) {
    sfText->_retainCount++;
    
    return sfText;
}

void SFTextRelease(SFTextRef sfText) {
	sfText->_retainCount--;
    
    if (sfText->_retainCount == 0) {
        
#ifdef SF_IOS_CG
    	if (sfText->_txtColor)
    		CGColorRelease(sfText->_txtColor);
#endif
        
        SFFontDeallocateStringRecord(sfText->_sfFont, sfText->_record);
        SFFontRelease(sfText->_sfFont);
        free(sfText);
    }
}

static void callGlyphDrawingFunction(SFGlyphRenderFunction func, SFText *sfText, SFGlyph glyph, float x, float y) {
#ifdef SF_IOS_CG
	CGContextShowGlyphsAtPoint(sfText->_context, x, y, &glyph, 1);
#else
	FT_Face face;
    FT_GlyphSlot slot;
    FT_Bitmap *bmp;
    
    unsigned char *buffer;
    int length;
    
    SFColor *pixels;
    
    int i;
    
    FT_Error error, error1;
    
	face = SFFontGetFTFace(sfText->_sfFont);
	//FT_Set_Char_Size(face, 0, SFFontGetSize(sfText->_sfFont) * 64, 72, 72);
	FT_Set_Pixel_Sizes(face, 0, SFFontGetSize(sfText->_sfFont));
	slot = face->glyph;
    
	error = FT_Load_Glyph(face, glyph, FT_LOAD_DEFAULT);
	error1 = FT_Render_Glyph(face->glyph, FT_RENDER_MODE_NORMAL);
	if (error || error1)
		return;
    
	bmp = &slot->bitmap;
    
	buffer = (unsigned char *)bmp->buffer;
	length = bmp->width * bmp->rows;
    
    if (!length)
        return;
    
	pixels = malloc(sizeof(SFColor) * length);
    
    //Here we will treat grayscale pixels as alpha
	for (i = 0; i < length; i++) {
#ifdef SF_IOS
		//For iOS, in one uint, bytes are stored in reversed order
        //as BGRA, i.e. blue at first place and alpha at last place.
		pixels[i] = (sfText->_txtColor << 8) | buffer[i];
#else
        //For android, bytes are stored in correct order as ARGB.
        pixels[i] = (buffer[i] << 24) | sfText->_txtColor;
#endif
    }
    
	(*func)(pixels, bmp->width, bmp->rows, x + slot->bitmap_left, y - slot->bitmap_top, sfText->_resObj);
    
    free(pixels);
#endif
}

/////////////////////////////////CGContext Additions/////////////////////////////////
typedef struct VaryingYGlyph {
    SFGlyph glyph;
    CGPoint position;
    
    struct VaryingYGlyph *next;
} VaryingYGlyph;

static void saveVaryingYGlyph(VaryingYGlyph **head, SFGlyph glyph, CGPoint position) {
    VaryingYGlyph *current = malloc(sizeof(VaryingYGlyph));
    current->glyph = glyph;
    current->position = position;
    current->next = *head;
    *head = current;
}

static void drawVaryingYGlyphs(VaryingYGlyph **head, SFGlyphRenderFunction func, SFTextRef sfText, SFFloat varyingY) {
    VaryingYGlyph *current = *head;
    while (current) {
        VaryingYGlyph *previous;
        
    	callGlyphDrawingFunction(func, sfText, current->glyph, current->position.x, current->position.y - varyingY);
        
        previous = current;
        current = current->next;
        
        free(previous);
    }
    
    *head = NULL;
}

static void drawLine(SFGlyphRenderFunction func, SFText *sfText, SFRange range, CGPoint *position) {
    if (range.end - range.start > 0) {
        SFGlyph crntGlyph;
        
        SFBool hasExitAnchor = SFFalse;
        VaryingYGlyph *varyingYGlyphs = NULL;
        
		int i;

        SFFloat sizeByEm = SFFontGetSizeByEm(sfText->_sfFont);
        SFFloat leading = position->y;
        
        SFStringRecord *record = sfText->_record;
        
        for (i = range.start; i <= range.end; i++) {
            int count = record->charRecord[i].glyphCount;
			int j;
			
            for (j = 0; j < count; j++) {
                SFPositionRecord pos;
                SFAnchorType aType;
                
                SFBool isMark;
                SFBool isCursive;
                
                crntGlyph = record->charRecord[i].gRec[j].glyph;
                if (!crntGlyph)
                    continue;
                
                pos = record->charRecord[i].gRec[j].posRec;
                aType = pos.anchorType;
                
                isMark = (aType & atMark);
                isCursive = (aType & atEntry);
                
                if (isMark || (aType & atCursiveIgnored)) {
                    SFFloat markX = position->x - ((pos.anchor.x - pos.placement.x) * sizeByEm);
                    SFFloat markY = position->y + ((pos.anchor.y - pos.placement.y) * sizeByEm);
                    
                    if (hasExitAnchor) {
                        CGPoint p;
                        p.x = markX;
                        p.y = markY;
                        
                        saveVaryingYGlyph(&varyingYGlyphs, crntGlyph, p);
                    } else
                        callGlyphDrawingFunction(func, sfText, crntGlyph, markX, markY);
                } else {
                    if (isCursive) {
                        position->x += pos.anchor.x * sizeByEm;
                        position->y -= pos.anchor.y * sizeByEm;
                        
                        hasExitAnchor = (aType & atExit);
                        if (hasExitAnchor) {
                            saveVaryingYGlyph(&varyingYGlyphs, crntGlyph, *position);
                        } else {
                            drawVaryingYGlyphs(&varyingYGlyphs, func, sfText, position->y - leading);
                            position->y = leading;
                            
                            callGlyphDrawingFunction(func, sfText, crntGlyph, position->x, position->y);
                        }
                    } else {
                        int adv = SFGetAdvance(record, sfText->_sfFont, i, j);
                        
                        position->x -= (pos.advance.x + adv - pos.placement.x) * sizeByEm;
                        position->y -= pos.placement.y * sizeByEm;
                        
                        if (aType & atExit) {
                            saveVaryingYGlyph(&varyingYGlyphs, crntGlyph, *position);
                            hasExitAnchor = SFTrue;
                        } else {
                            if (hasExitAnchor) {
                                drawVaryingYGlyphs(&varyingYGlyphs, func, sfText, position->y - leading);
                                position->y = leading;
                            }
                            
                            hasExitAnchor = SFFalse;
                            callGlyphDrawingFunction(func, sfText, crntGlyph, position->x, position->y);
                        }
                    }
                }
            }
        }
    }
}

static int drawText(SFGlyphRenderFunction func, SFText *sfText, CGPoint *position, int *lines, int startIndex) {
	if (sfText->_record) {
        SFFloat initialX = position->x;
        int i = 0;
        
        SFMeasuredLine line;
        line.range1.start = 0;
        line.range1.end = 0;
        
        while (startIndex > -1 && i < *lines) {
            line = getLine(sfText, sfText->_record, startIndex);
            
            if (sfText->_txtAlign == SFTextAlignmentRight)
                position->x += sfText->_pageWidth;
            else if (sfText->_txtAlign == SFTextAlignmentCenter)
                position->x += line.width + (sfText->_pageWidth - line.width) / 2;
            else if (sfText->_txtAlign == SFTextAlignmentLeft)
                position->x += line.width;
            
            drawLine(func, sfText, line.range1, position);
            
            if (line.ranges > 0)
                drawLine(func, sfText, line.range2, position);
            
            if (line.ranges > 1)
                drawLine(func, sfText, line.range3, position);
            
            position->x = initialX;
            position->y += SFFontGetLeading(sfText->_sfFont);
            
            startIndex = line.nextIndex;
            
            i++;
        }
        
        *lines = i;
        
        return startIndex;
    }
    
    *lines = 0;
    
    return -1;
}

int SFTextGetNextLineCharIndex(SFTextRef sfText, int maxLines, int startIndex, int *createdLines) {
    if (sfText->_record) {
        int i = 0;
        
    	SFMeasuredLine line;
        line.nextIndex = -1;
        
        while (startIndex > -1 && i < maxLines) {
            line = getLine(sfText, sfText->_record, startIndex);
            startIndex = line.nextIndex;
            i++;
        }
        
        *createdLines = i;
        
        return line.nextIndex;
    }
    
    *createdLines = 0;
    
    return -1;
}

#ifdef SF_IOS_CG

void CGContextDrawText(CGContextRef context, SFTextRef sfText, int startIndex, int *lines) {
    CGPoint position = sfText->_initialPosition;
    CGAffineTransform flip = CGAffineTransformMake(1.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f);
    
    CGContextSaveGState(context);

	CGContextSetTextMatrix(context, flip);
    CGContextSetTextDrawingMode(context, kCGTextFill);
    
    CGContextSetFont(context, SFFontGetCGFont(sfText->_sfFont));
    CGContextSetFontSize(context, SFFontGetSize(sfText->_sfFont));
    CGContextSetFillColorWithColor(context, sfText->_txtColor);
    
    sfText->_context = context;
    drawText(NULL, sfText, &position, lines, startIndex);
    sfText->_context = NULL;
    
    CGContextRestoreGState(context);
}

#else

int SFTextShowString(SFTextRef sfText, SFGlyphRenderFunction func, int startIndex, int *lines) {
	CGPoint position = sfText->_initialPosition;
	return drawText(func, sfText, &position, lines, startIndex);
}

#endif
