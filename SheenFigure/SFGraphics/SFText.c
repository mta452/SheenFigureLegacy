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


static int SFGetAdvance(SFStringRecord *record, SFFontRef sfFont, int recordIndex, int glyphIndex) {
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

static int SFTextGetBaseLevel(SFTextRef sfText) {
    if (sfText->_writingDirection == SFWritingDirectionAuto) {
        return baseLevel(sfText->_record->types, sfText->_strLength);
    }
    
    return sfText->_writingDirection - 1;
}

static SFMeasuredLine getLine(SFTextRef sfText, SFFloat frameWidth, SFStringRecord *record, int startIndex) {
    SFFloat width = 0;
    SFFloat prevWidth = 0;
    SFFontRef sfFont = sfText->_sfFont;
    
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
                    int adv = SFGetAdvance(record, sfFont, i, j);
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

SFTextRef SFTextCreateWithString(const SFUnichar *str, int length, SFFontRef sfFont) {
    SFTextRef sfText = malloc(sizeof(SFText));
    
    sfText->_str = str;
    sfText->_strLength = length;
    sfText->_sfFont = SFFontRetain(sfFont);
    sfText->_writingDirection = SFWritingDirectionRTL;

    sfText->_record = NULL;
    
    if (str && length > -1 && sfFont) {
        sfText->_record = SFMakeStringRecordForBaseLevel(str, length, sfText->_writingDirection - 1);
        applyFontTables(sfFont, sfText->_record);
    }
    
    sfText->_txtAlign = SFTextAlignmentRight;
    sfText->_retainCount = 1;
    
    return sfText;
}

SFTextRef SFTextRetain(SFTextRef sfText) {
    sfText->_retainCount++;
    
    return sfText;
}

void SFTextRelease(SFTextRef sfText) {
    sfText->_retainCount--;
    
    if (sfText->_retainCount == 0) {
        SFFreeStringRecord(sfText->_record);
        SFFontRelease(sfText->_sfFont);
        free(sfText);
    }
}

void SFTextSetString(SFTextRef sfText, const SFUnichar *str, int length) {
    SFFreeStringRecord(sfText->_record);
    
    sfText->_str = str;
    sfText->_strLength = length;
    
    if (str && length > -1) {
        sfText->_record = SFMakeStringRecordForBaseLevel(str, length, sfText->_writingDirection - 1);
        if (sfText->_sfFont) {
            applyFontTables(sfText->_sfFont, sfText->_record);
        }
    } else {
        sfText->_record = NULL;
    }
}

void SFTextSetFont(SFTextRef sfText, SFFontRef sfFont) {
    if (sfFont) {
        if (sfFont != sfText->_sfFont) {
            SFFontRelease(sfText->_sfFont);
            sfText->_sfFont = SFFontRetain(sfFont);
            
            if (sfText->_record) {
                SFClearCharRecord(sfText->_record);
                applyFontTables(sfText->_sfFont, sfText->_record);
            }
        }
    } else {
        SFFreeStringRecord(sfText->_record);
        sfText->_record = NULL;
    }
}

void SFTextSetAlignment(SFTextRef sfText, SFTextAlignment alignment) {
    sfText->_txtAlign = alignment;
}

void SFTextSetWritingDirection(SFTextRef sfText, SFWritingDirection writingDirection) {
    if (writingDirection != sfText->_writingDirection) {
        sfText->_writingDirection = writingDirection;
        
        if (sfText->_record) {
            SFClearStringRecordForBaseLevel(sfText->_record, sfText->_writingDirection - 1);
            applyFontTables(sfText->_sfFont, sfText->_record);
        }
    }
}

static void saveVaryingYGlyph(VaryingYGlyph **head, SFGlyph glyph, SFPoint position) {
    VaryingYGlyph *current = malloc(sizeof(VaryingYGlyph));
    current->glyph = glyph;
    current->position = position;
    current->next = *head;
    *head = current;
}

static void drawVaryingYGlyphs(VaryingYGlyph **head, SFGlyphRenderFunction func, SFTextRef sfText, SFFloat varyingY, void *resObj) {
    VaryingYGlyph *current = *head;
    while (current) {
        VaryingYGlyph *previous;
        
        (*func)(sfText, current->glyph, current->position.x, current->position.y - varyingY, resObj);
        
        previous = current;
        current = current->next;
        
        free(previous);
    }
    
    *head = NULL;
}

static void drawLine(int baselevel, SFText *sfText, SFMeasuredLine line, SFPoint *position, void *resObj, SFGlyphRenderFunction func) {
    if (line.endIndex - line.startIndex >= 0) {
        int length = line.endIndex - line.startIndex + 1;
        int *levels = malloc(length * sizeof(int));
        int *visOrder = malloc(length * sizeof(int));
        
        SFGlyph crntGlyph;
        
        SFBool hasExitAnchor = SFFalse;
        VaryingYGlyph *varyingYGlyphs = NULL;

        int i, j, k;

        SFStringRecord *record = sfText->_record;
        SFMirroredChar *mchars;
        SFMirroredChar *crntmchar;
        
        SFFloat sizeByEm = SFFontGetSizeByEm(sfText->_sfFont);
        SFFloat leading = position->y;
    
        for (i = line.startIndex, j = 0; i <= line.endIndex; i++, j++) {
            levels[j] = sfText->_record->levels[i];
            visOrder[j] = i;
        }
        
        generateBidiVisualOrder(baselevel, sfText->_record->types + line.startIndex, levels, visOrder, length);
        mchars = generateMirroredChars(sfText->_record->chars + line.startIndex, levels, length);
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
                    crntGlyph = SFCharToGlyph(&sfText->_sfFont->_tables->_cmap, crntmchar->ch);
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
                        (*func)(sfText, crntGlyph, markX, markY, resObj);
                    }
                } else {
                    if (isCursive) {
                        position->x += pos.anchor.x * sizeByEm;
                        position->y -= pos.anchor.y * sizeByEm;
                        
                        hasExitAnchor = (aType & atExit);
                        if (hasExitAnchor) {
                            saveVaryingYGlyph(&varyingYGlyphs, crntGlyph, *position);
                        } else {
                            drawVaryingYGlyphs(&varyingYGlyphs, func, sfText, position->y - leading, resObj);
                            position->y = leading;
                            
                            (*func)(sfText, crntGlyph, position->x, position->y, resObj);
                        }
                    } else {
                        int adv = SFGetAdvance(record, sfText->_sfFont, vi, k);
                        
                        position->x -= (pos.advance.x + adv - pos.placement.x) * sizeByEm;
                        position->y -= pos.placement.y * sizeByEm;
                        
                        if (aType & atExit) {
                            saveVaryingYGlyph(&varyingYGlyphs, crntGlyph, *position);
                            hasExitAnchor = SFTrue;
                        } else {
                            if (hasExitAnchor) {
                                drawVaryingYGlyphs(&varyingYGlyphs, func, sfText, position->y - leading, resObj);
                                position->y = leading;
                            }
                            
                            hasExitAnchor = SFFalse;
                            (*func)(sfText, crntGlyph, position->x, position->y, resObj);
                        }
                    }
                }
            }
        }
        
        free(mchars);
        free(visOrder);
    }
}

static int drawText(int baselevel, SFText *sfText, SFFloat frameWidth, SFPoint *position, int *lines, int startIndex, void *resObj, SFGlyphRenderFunction func) {
    if (sfText->_record) {
        SFFloat initialX = position->x;

        SFMeasuredLine line;
        int nextIndex = startIndex;
        int endIndex = sfText->_record->charCount - 1;
        
        int i = 0;
        while (i < *lines) {
            line = getLine(sfText, frameWidth, sfText->_record, nextIndex);

            if (sfText->_txtAlign == SFTextAlignmentRight) {
                position->x += frameWidth;
            } else if (sfText->_txtAlign == SFTextAlignmentCenter) {
                position->x += line.width + (frameWidth - line.width) / 2;
            } else if (sfText->_txtAlign == SFTextAlignmentLeft) {
                position->x += line.width;
            }

            drawLine(baselevel, sfText, line, position, resObj, func);

            position->x = initialX;
            position->y += SFFontGetLeading(sfText->_sfFont);

            i++;
            
            if (line.endIndex < endIndex) {
                nextIndex = line.endIndex + 1;
            } else {
                nextIndex = -1;
                break;
            }
        }
        
        *lines = i;
        return startIndex;
    }
    
    *lines = 0;
    return -1;
}

int SFTextGetNextLineCharIndex(SFTextRef sfText, SFFloat frameWidth, int startIndex, int *countLines) {
    if (sfText->_sfFont && sfText->_record) {
        SFMeasuredLine line;
        int nextIndex = startIndex;
        int endIndex = sfText->_record->charCount - 1;
        
        int i = 0;
        while (i < *countLines) {
            line = getLine(sfText, frameWidth, sfText->_record, nextIndex);
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
    
    *countLines = 0;
    return -1;
}

int SFTextMeasureLines(SFTextRef sfText, SFFloat frameWidth) {
    if (sfText->_sfFont && sfText->_record) {
        SFMeasuredLine line;
        int startIndex = 0;
        int endIndex = sfText->_record->charCount - 1;
        
        int lineCount = 0;
        while (startIndex < endIndex) {
            line = getLine(sfText, frameWidth, sfText->_record, startIndex);
            lineCount++;

            startIndex = line.endIndex + 1;
        }
        
        return lineCount;
    }
    
    return 0;
}

SFFloat SFTextMeasureHeight(SFTextRef sfText, SFFloat frameWidth) {
    if (sfText->_sfFont) {
        return SFTextMeasureLines(sfText, frameWidth) * sfText->_sfFont->_leading;
    }
    
    return 0;
}

int SFTextShowString(SFTextRef sfText, SFFloat frameWidth, SFPoint position, int startIndex, int *lines, void *resObj, SFGlyphRenderFunction func) {
    position.y += sfText->_sfFont->_ascender;
    return drawText(SFTextGetBaseLevel(sfText), sfText, frameWidth, &position, lines, startIndex, resObj, func);
}
