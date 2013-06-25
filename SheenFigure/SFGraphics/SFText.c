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
#include <stdint.h>
#include <math.h>

#include "bidi.h"
#include "SFCMAPUtilization.h"
#include "SFGDEFUtilization.h"
#include "SFGSUBUtilization.h"
#include "SFGPOSUtilization.h"
#include "SFGSUBGPOSUtilization.h"

#include "SFFontPrivate.h"
#include "SFFont.h"

#include "SFTextPrivate.h"
#include "SFText.h"

typedef struct SFMeasuredLine {
    SFFloat width;
    int startIndex;
    int endIndex;
} SFMeasuredLine;

typedef struct VaryingYGlyph {
    SFGlyph glyph;
    SFPoint position;
    
    struct VaryingYGlyph *next;
} VaryingYGlyph;


static int getAdvance(SFFontRef sfFont, SFStringRecord *record, int recordIndex, int glyphIndex) {
#ifdef SF_IOS_CG
    int adv;
#else
    FT_Fixed adv;
#endif
    
    SFGlyph glyph = record->charRecord[recordIndex].gRec[glyphIndex].glyph;
    
    if (record->charRecord[recordIndex].gRec[glyphIndex].glyphProp & gpAdvance) {
        return record->charRecord[recordIndex].gRec[glyphIndex].advance;
    }
    
#ifdef SF_IOS_CG
    CGFontGetGlyphAdvances(SFFontGetCGFont(sfFont), &glyph, 1, &adv);
#else
    FT_Get_Advance(SFFontGetFTFace(sfFont), glyph, FT_LOAD_NO_SCALE, &adv);
#endif
    
    record->charRecord[recordIndex].gRec[glyphIndex].advance = adv;
    record->charRecord[recordIndex].gRec[glyphIndex].glyphProp |= gpAdvance;
    
    return adv;
}

static SFStringRecord *applyFontTables(SFFontRef sfFont, SFStringRecord *record) {
    SFInternal internal;
    
    SFFontReadTables(sfFont);
    
    internal.record = record;
    internal.cmap = &sfFont->_tables->_cmap;
    internal.gdef = &sfFont->_tables->_gdef;
    internal.gsub = &sfFont->_tables->_gsub;
    internal.gpos = &sfFont->_tables->_gpos;
    
    if (sfFont->_tables->_availableTables & itCMAP) {
        SFApplyCMAP(&internal);
    }
    
    if (sfFont->_tables->_availableTables & itGSUB) {
        SFApplyGSUB(&internal);
    }
    
    if (sfFont->_tables->_availableTables & itGPOS) {
        SFApplyGPOS(&internal);
    }
    
    return record;
}

static int getBaseLevel(SFTextRef sfText) {
    if (sfText->_writingDirection == SFWritingDirectionAuto) {
        return baseLevel(sfText->_record->types, sfText->_record->charCount);
    }
    
    return sfText->_writingDirection - 1;
}

static SFMeasuredLine getLine(SFFontRef sfFont, SFStringRecord *record, SFFloat frameWidth, int startIndex) {
    SFFloat width = 0;
    SFFloat prevWidth = 0;
    
    SFGlyph glyph;
    SFPositionRecord crntPosRecord;
    SFMeasuredLine line;

    SFUnichar ch;
    
    int i;

    line.startIndex = startIndex;
    line.width = 0;
    
    if (startIndex >= record->charCount) {
        line.endIndex = -1;
        return line;
    }

    line.endIndex = startIndex;
    
    for (i = startIndex; i < record->charCount; i++) {
        int j, count;

        ch = record->chars[i];
        if (ch == ' ' || ch == '\n') {
            line.endIndex = i;
            line.width = width;
            
            if (ch == '\n') {
                return line;
            }
        }
        
        count = record->charRecord[i].glyphCount;
        
        prevWidth = width;
        
        for (j = 0; j < count; j++) {
            SFAnchorType aType;
            
            glyph = record->charRecord[i].gRec[j].glyph;
            if (!glyph) {
                continue;
            }
            
            crntPosRecord = record->charRecord[i].gRec[j].posRec;
            aType = crntPosRecord.anchorType;
            if (glyph && !(aType & atMark)) {
                if (aType & atEntry)
                    width -= crntPosRecord.anchor.x * SFFontGetSizeByEm(sfFont);
                else {
                    int adv = getAdvance(sfFont, record, i, j);
                    width += (crntPosRecord.advance.x + adv - crntPosRecord.placement.x) * SFFontGetSizeByEm(sfFont);
                }
            }
            
            if (ch != ' ' && ceil(width) >= floor(frameWidth)) {
                if (line.endIndex == line.startIndex) {
                    if (i == line.endIndex) {
                        line.width = width;
                    } else {
                        line.endIndex = i - 1;
                        line.width = prevWidth;
                    }
                }
                
                return line;
            }
        }
    }

    line.endIndex = record->charCount - 1;
    line.width = width;
    
    return line;
}

static void saveVaryingYGlyph(VaryingYGlyph **head, SFGlyph glyph, SFPoint position) {
    VaryingYGlyph *current = malloc(sizeof(VaryingYGlyph));
    current->glyph = glyph;
    current->position = position;
    current->next = *head;
    *head = current;
}

static void drawVaryingYGlyphs(VaryingYGlyph **head, SFFloat varyingY, void *resObj, SFGlyphRenderFunction func) {
    VaryingYGlyph *current = *head;
    while (current) {
        VaryingYGlyph *previous;
        
        (*func)(current->glyph, current->position.x, current->position.y - varyingY, resObj);
        
        previous = current;
        current = current->next;
        
        free(previous);
    }
    
    *head = NULL;
}

static void drawLine(SFFontRef sfFont, SFStringRecord *record, int baselevel, SFMeasuredLine line, SFPoint *position, void *resObj, SFGlyphRenderFunction func) {
    if (line.endIndex - line.startIndex >= 0) {
        int length = line.endIndex - line.startIndex + 1;
        int *levels = malloc(length * sizeof(int));
        int *visOrder = malloc(length * sizeof(int));
        
        SFGlyph crntGlyph;
        
        SFBool hasExitAnchor = SFFalse;
        VaryingYGlyph *varyingYGlyphs = NULL;

        int i, j, k;

        SFMirroredChar *mchars;
        SFMirroredChar *crntmchar;
        
        SFFloat sizeByEm = SFFontGetSizeByEm(sfFont);
        SFFloat leading = position->y;

        for (i = line.startIndex, j = 0; i <= line.endIndex; i++, j++) {
            levels[j] = record->levels[i];
            visOrder[j] = i;
        }
        
        generateBidiVisualOrder(baselevel, record->types + line.startIndex, levels, visOrder, length);
        mchars = generateMirroredChars(record->chars + line.startIndex, levels, length);
        crntmchar = mchars->next;
        
        free(levels);

        for (i = line.startIndex, j = 0; i <= line.endIndex; i++, j++) {
            int vi = visOrder[j];
            int count = record->charRecord[vi].glyphCount;
            
            for (k = 0; k < count; k++) {
                SFPositionRecord pos;
                SFAnchorType aType;
                
                SFBool isMark;
                SFBool isCursive;
                
                if (crntmchar && crntmchar->index == (vi - line.startIndex)) {
                    SFMirroredChar *nextmchar = crntmchar->next;
                    crntGlyph = SFCharToGlyph(&sfFont->_tables->_cmap, crntmchar->ch);
                    free(crntmchar);
                    crntmchar = nextmchar;
                } else {
                    crntGlyph = record->charRecord[vi].gRec[k].glyph;
                }
                
                if (!crntGlyph) {
                    continue;
                }
                
                pos = record->charRecord[vi].gRec[k].posRec;
                aType = pos.anchorType;
                
                isMark = (aType & atMark);
                isCursive = (aType & atEntry);
                
                if (isMark || (aType & atCursiveIgnored)) {
                    SFFloat markX = position->x - ((pos.anchor.x - pos.placement.x) * sizeByEm);
                    SFFloat markY = position->y + ((pos.anchor.y - pos.placement.y) * sizeByEm);
                    
                    if (hasExitAnchor) {
                        SFPoint p;
                        p.x = markX;
                        p.y = markY;
                        
                        saveVaryingYGlyph(&varyingYGlyphs, crntGlyph, p);
                    } else {
                        (*func)(crntGlyph, markX, markY, resObj);
                    }
                } else {
                    if (isCursive) {
                        position->x += pos.anchor.x * sizeByEm;
                        position->y -= pos.anchor.y * sizeByEm;
                        
                        hasExitAnchor = (aType & atExit);
                        if (hasExitAnchor) {
                            saveVaryingYGlyph(&varyingYGlyphs, crntGlyph, *position);
                        } else {
                            drawVaryingYGlyphs(&varyingYGlyphs, position->y - leading, resObj, func);
                            position->y = leading;
                            
                            (*func)(crntGlyph, position->x, position->y, resObj);
                        }
                    } else {
                        int adv = getAdvance(sfFont, record, vi, k);
                        
                        position->x -= (pos.advance.x + adv - pos.placement.x) * sizeByEm;
                        position->y -= pos.placement.y * sizeByEm;
                        
                        if (aType & atExit) {
                            saveVaryingYGlyph(&varyingYGlyphs, crntGlyph, *position);
                            hasExitAnchor = SFTrue;
                        } else {
                            if (hasExitAnchor) {
                                drawVaryingYGlyphs(&varyingYGlyphs, position->y - leading, resObj, func);
                                position->y = leading;
                            }
                            
                            hasExitAnchor = SFFalse;
                            (*func)(crntGlyph, position->x, position->y, resObj);
                        }
                    }
                }
            }
        }
        
        free(mchars);
        free(visOrder);
    }
}

static int drawText(SFFontRef sfFont, SFStringRecord *record, int baselevel, SFTextAlignment align, SFFloat frameWidth, SFPoint *position, int *lines, int startIndex, void *resObj, SFGlyphRenderFunction func) {
    SFFloat initialX = position->x;
    
    SFMeasuredLine line;
    int nextIndex = startIndex;
    int endIndex = record->charCount - 1;
    
    int i = 0;
    while (i < *lines) {
        line = getLine(sfFont, record, frameWidth, nextIndex);
        
        if (align == SFTextAlignmentRight) {
            position->x += frameWidth;
        } else if (align == SFTextAlignmentCenter) {
            position->x += line.width + (frameWidth - line.width) / 2;
        } else if (align == SFTextAlignmentLeft) {
            position->x += line.width;
        }
        
        drawLine(sfFont, record, baselevel, line, position, resObj, func);
        
        position->x = initialX;
        position->y += SFFontGetLeading(sfFont);
        
        i++;
        
        if (line.endIndex < endIndex) {
            nextIndex = line.endIndex + 1;
        } else {
            nextIndex = -1;
            break;
        }
    }
    
    *lines = i;
    return nextIndex;
}

static int getNextLineCharIndex(SFFontRef sfFont, SFStringRecord *record, SFFloat frameWidth, int startIndex, int *countLines) {
    SFMeasuredLine line;
    int nextIndex = startIndex;
    int endIndex = record->charCount - 1;
    
    int i = 0;
    while (i < *countLines) {
        line = getLine(sfFont, record, frameWidth, nextIndex);
        i++;
        
        if (line.endIndex < endIndex) {
            nextIndex = line.endIndex + 1;
        } else {
            nextIndex = -1;
            break;
        }
    }
    
    *countLines = i;
    return nextIndex;
}

static int measureLines(SFFontRef sfFont, SFStringRecord *record, SFFloat frameWidth) {
    SFMeasuredLine line;
    int startIndex = 0;
    int endIndex = record->charCount - 1;
    
    int lineCount = 0;
    
    while (startIndex < endIndex) {
        line = getLine(sfFont, record, frameWidth, startIndex);
        lineCount++;
        
        startIndex = line.endIndex + 1;
    }
    
    return lineCount;
}

SFTextRef SFTextCreateWithString(SFUnichar *str, int length, SFFontRef sfFont) {
    SFTextRef sfText = malloc(sizeof(SFText));

    sfText->_sfFont = SFFontRetain(sfFont);
    sfText->_writingDirection = SFWritingDirectionRTL;
    
    if (sfFont && str && length > -1) {
        sfText->_record = SFMakeStringRecordForBaseLevel(str, length, sfText->_writingDirection - 1);
        applyFontTables(sfFont, sfText->_record);
    } else {
        sfText->_record = NULL;
    }
    
    sfText->_txtAlign = SFTextAlignmentRight;
    
    pthread_mutex_init(&sfText->_retainMutex, NULL);
    sfText->_retainCount = 1;
    
    return sfText;
}

void SFTextSetString(SFTextRef sfText, SFUnichar *str, int length) {
    SFStringRecord *refRecord = sfText->_record;

    if (str && length > -1) {
        SFStringRecord *newRecord = SFMakeStringRecordForBaseLevel(str, length, sfText->_writingDirection - 1);
        if (sfText->_sfFont) {
            applyFontTables(sfText->_sfFont, newRecord);
        }
        
        sfText->_record = newRecord;
        
        if (refRecord && str == refRecord->chars) {
            refRecord->retainChars = SFTrue;
        }
    } else {
        sfText->_record = NULL;
    }
    
    SFReleaseStringRecord(refRecord);
}

void SFTextSetFont(SFTextRef sfText, SFFontRef sfFont) {
    if (sfFont) {
        if (sfFont != sfText->_sfFont) {
            SFStringRecord *refRecord = sfText->_record;
            SFFontRef refFont = sfText->_sfFont;
            
            sfText->_sfFont = SFFontRetain(sfFont);
            SFFontRelease(refFont);
            
            if (refRecord) {
                SFStringRecord *newRecord = SFMakeStringRecordForBaseLevel(refRecord->chars, refRecord->charCount, sfText->_writingDirection - 1);
                applyFontTables(sfFont, newRecord);
                
                sfText->_record = newRecord;
                
                refRecord->retainChars = SFTrue;
                SFReleaseStringRecord(refRecord);
            }
        }
    } else {
        SFStringRecord *refRecord = sfText->_record;
        sfText->_record = NULL;
        SFReleaseStringRecord(refRecord);
    }
}

void SFTextSetAlignment(SFTextRef sfText, SFTextAlignment alignment) {
    sfText->_txtAlign = alignment;
}

void SFTextSetWritingDirection(SFTextRef sfText, SFWritingDirection writingDirection) {
    if (writingDirection != sfText->_writingDirection) {
        SFStringRecord *refRecord = sfText->_record;
        SFFontRef refFont = SFFontRetain(sfText->_sfFont);
        
        sfText->_writingDirection = writingDirection;
        
        if (refFont && refRecord) {
            SFStringRecord *newRecord = SFMakeStringRecordForBaseLevel(refRecord->chars, refRecord->charCount, sfText->_writingDirection - 1);
            applyFontTables(refFont, newRecord);
            
            sfText->_record = newRecord;
            
            refRecord->retainChars = SFTrue;
            SFReleaseStringRecord(refRecord);
        }
        
        SFFontRelease(refFont);
    }
}

int SFTextGetNextLineCharIndex(SFTextRef sfText, SFFloat frameWidth, int startIndex, int *countLines) {
    int retIndex = -1;
    SFFontRef font = SFFontRetain(sfText->_sfFont);
    SFStringRecord *record = SFRetainStringRecord(sfText->_record);
    
    if (font && record) {
        retIndex = getNextLineCharIndex(font, record, frameWidth, startIndex, countLines);
    }
    
    SFFontRelease(font);
    SFReleaseStringRecord(record);
    
    return retIndex;
}

int SFTextMeasureLines(SFTextRef sfText, SFFloat frameWidth) {
    int lineCount = 0;
    SFFontRef font = SFFontRetain(sfText->_sfFont);
    SFStringRecord *record = SFRetainStringRecord(sfText->_record);
    
    if (font && record) {
        lineCount = measureLines(font, record, frameWidth);
    }
    
    SFFontRelease(font);
    SFReleaseStringRecord(record);
    
    return lineCount;
}

SFFloat SFTextMeasureHeight(SFTextRef sfText, SFFloat frameWidth) {
    int height = 0;
    SFFontRef font = SFFontRetain(sfText->_sfFont);
    SFStringRecord *record = SFRetainStringRecord(sfText->_record);
    
    if (font && record) {
        height = measureLines(font, record, frameWidth) * font->_leading;
    }
    
    SFFontRelease(font);
    SFReleaseStringRecord(record);
    
    return height;
}

int SFTextShowString(SFTextRef sfText, SFFloat frameWidth, SFPoint position, int startIndex, int *lines, void *resObj, SFGlyphRenderFunction func) {
    int retIndex = -1;
    SFFontRef font = SFFontRetain(sfText->_sfFont);
    SFStringRecord *record = SFRetainStringRecord(sfText->_record);
    
    if (font && record) {
        int maxLines = INT32_MAX;
        if (!lines) {
            lines = &maxLines;
        } else if (*lines < 0) {
            *lines = maxLines;
        }
        
        position.y += sfText->_sfFont->_ascender;
        retIndex = drawText(font, record, getBaseLevel(sfText), sfText->_txtAlign, frameWidth, &position, lines, startIndex, resObj, func);
    } else {
        if (lines) {
            *lines = 0;
        }
    }
    
    SFFontRelease(font);
    SFReleaseStringRecord(record);
    
    return retIndex;
}

SFTextRef SFTextRetain(SFTextRef sfText) {
    if (sfText) {
        pthread_mutex_lock(&sfText->_retainMutex);
        
        sfText->_retainCount++;
        
        pthread_mutex_unlock(&sfText->_retainMutex);
    }
    
    return sfText;
}

void SFTextRelease(SFTextRef sfText) {
    if (sfText) {
        pthread_mutex_lock(&sfText->_retainMutex);
        
        sfText->_retainCount--;
        
        pthread_mutex_unlock(&sfText->_retainMutex);
        
        if (sfText->_retainCount == 0) {
            SFReleaseStringRecord(sfText->_record);
            SFFontRelease(sfText->_sfFont);
            pthread_mutex_destroy(&sfText->_retainMutex);
            free(sfText);
        }
    }
}
