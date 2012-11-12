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

#include <stdio.h>
#include <stdlib.h>

#include "SFCommonData.h"
#include "SFGlobal.h"
#include "SFGDEFData.h"
#include "SFGDEFUtilization.h"

void SFAddGlyphProperties(int charIndex, int glyphIndex) {
    SFGlyph glyph = record->charRecord[charIndex].gRec[glyphIndex].glyph;
    
    record->charRecord[charIndex].gRec[glyphIndex].glyphProp |= gpReceived;
    
    if (gdef->glyphClassDef.classFormat == 1) {
        SFGlyph startGlyph = gdef->glyphClassDef.format.format1.startGlyph;
        SFGlyph endGlyph = startGlyph + gdef->glyphClassDef.format.format1.glyphCount;
        
        if (glyph >= startGlyph && glyph <= endGlyph) {
            int cls = gdef->glyphClassDef.format.format1.classValueArray[glyph - startGlyph];
            
            switch (cls) {
                case gcvBase:
                    record->charRecord[charIndex].gRec[glyphIndex].glyphProp |= gpBase;
                    break;
                case gcvMark:
                    record->charRecord[charIndex].gRec[glyphIndex].glyphProp |= gpMark;
                    break;
                case gcvLigature:
                    record->charRecord[charIndex].gRec[glyphIndex].glyphProp |= gpLigature;
                    break;
                case gcvComponent:
                    record->charRecord[charIndex].gRec[glyphIndex].glyphProp |= gpComponent;
                    break;
            }
        }
    } else if (gdef->glyphClassDef.classFormat == 2) {
        SFUShort classRangeCount = gdef->glyphClassDef.format.format2.classRangeCount;
        SFUShort i;

        for (i = 0; i < classRangeCount; i++) {
            SFGlyph startGlyph = gdef->glyphClassDef.format.format2.classRangeRecord[i].start;
            SFGlyph endGlyph = gdef->glyphClassDef.format.format2.classRangeRecord[i].end;
            
            if (glyph >= startGlyph && glyph <= endGlyph) {
                int cls = gdef->glyphClassDef.format.format2.classRangeRecord[i].cls;
                
                switch (cls) {
                    case gcvBase:
                        record->charRecord[charIndex].gRec[glyphIndex].glyphProp |= gpBase;
                        break;
                    case gcvMark:
                        record->charRecord[charIndex].gRec[glyphIndex].glyphProp |= gpMark;
                        break;
                    case gcvLigature:
                        record->charRecord[charIndex].gRec[glyphIndex].glyphProp |= gpLigature;
                        break;
                    case gcvComponent:
                        record->charRecord[charIndex].gRec[glyphIndex].glyphProp |= gpComponent;
                        break;
                }
            }
        }
    }
}

SFBool SFDoesGlyphExistInGlyphClassDef(ClassDefTable *cls, GlyphClassValue clsValue, SFGlyph glyph) {
    if (cls->classFormat == 1) {
        SFGlyph startGlyph = cls->format.format1.startGlyph;
        SFUShort endGlyph = startGlyph + cls->format.format1.glyphCount;
        
        if (glyph >= startGlyph && glyph <= endGlyph && cls->format.format1.classValueArray[glyph - startGlyph] == clsValue)
            return SFTrue;

    } else if (cls->classFormat == 2) {
        SFUShort classRangeCount = cls->format.format2.classRangeCount;
        SFUShort i;

        for (i = 0; i < classRangeCount; i++) {
            if (cls->format.format2.classRangeRecord[i].cls == clsValue) {
                SFGlyph startGlyph = cls->format.format2.classRangeRecord[i].start;
                SFGlyph endGlyph = cls->format.format2.classRangeRecord[i].end;
                
                if (glyph >= startGlyph && glyph <= endGlyph)
                    return SFTrue;
            }
        }
    }
    
    return SFFalse;
}

SFBool SFIsIgnoredGlyph(int charIndex, int glyphIndex, LookupFlag lookupFlag) {
    SFGlyph glyph;
    SFGlyphProperty glyphProp;
    
    SFBool hasMarkProperty;
    
    glyph = record->charRecord[charIndex].gRec[glyphIndex].glyph;
    if (glyph == 0)
        return SFTrue;
    
    glyphProp = record->charRecord[charIndex].gRec[glyphIndex].glyphProp;
    
    if (glyphProp == gpNotReceived) {
        SFAddGlyphProperties(charIndex, glyphIndex);
        glyphProp = record->charRecord[charIndex].gRec[glyphIndex].glyphProp;
    }
    
    hasMarkProperty = (glyphProp & gpMark);
    if ((lookupFlag & lfIgnoreMarks) && hasMarkProperty)
        return SFTrue;
    
    if ((lookupFlag & lfIgnoreLigatures) && (glyphProp & gpLigature))
        return SFTrue;
    
    if ((lookupFlag & lfIgnoreBaseGlyphs) && (glyphProp & gpBase))
        return SFTrue;
    
    if (lookupFlag & lfMarkAttachmentType) {
        if (gdef->hasMarkAttachClassDef
            && hasMarkProperty
            && !SFDoesGlyphExistInGlyphClassDef(&gdef->markAttachClassDef, lookupFlag >> 8, glyph))
            return SFTrue;
    }
    
    return SFFalse;
}

