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

#include "SFGDEFUtilization.h"

void SFAddGlyphProperties(SFInternal *internal, SFGlyphIndex index) {
    SFGetGlyphProperties(internal, index) |= gpReceived;
    
    if (internal->gdef->glyphClassDef.classFormat == 1) {
        SFGlyph startGlyph = internal->gdef->glyphClassDef.format.format1.startGlyph;
        SFGlyph endGlyph = startGlyph + internal->gdef->glyphClassDef.format.format1.glyphCount;
        
        if (SFGetGlyph(internal, index) >= startGlyph && SFGetGlyph(internal, index) <= endGlyph) {
            int cls = internal->gdef->glyphClassDef.format.format1.classValueArray[SFGetGlyph(internal, index) - startGlyph];
            switch (cls) {
                case gcvBase:
                    SFGetGlyphProperties(internal, index) |= gpBase;
                    break;
                    
                case gcvMark:
                    SFGetGlyphProperties(internal, index) |= gpMark;
                    break;
                    
                case gcvLigature:
                    SFGetGlyphProperties(internal, index) |= gpLigature;
                    break;
                    
                case gcvComponent:
                    SFGetGlyphProperties(internal, index) |= gpComponent;
                    break;
            }
        }
    } else if (internal->gdef->glyphClassDef.classFormat == 2) {
        SFUShort classRangeCount = internal->gdef->glyphClassDef.format.format2.classRangeCount;
        SFUShort i;

        for (i = 0; i < classRangeCount; i++) {
            SFGlyph startGlyph = internal->gdef->glyphClassDef.format.format2.classRangeRecord[i].start;
            SFGlyph endGlyph = internal->gdef->glyphClassDef.format.format2.classRangeRecord[i].end;
            
            if (SFGetGlyph(internal, index) >= startGlyph && SFGetGlyph(internal, index) <= endGlyph) {
                int cls = internal->gdef->glyphClassDef.format.format2.classRangeRecord[i].cls;
                switch (cls) {
                    case gcvBase:
                        SFGetGlyphProperties(internal, index) |= gpBase;
                        break;
                        
                    case gcvMark:
                        SFGetGlyphProperties(internal, index) |= gpMark;
                        break;
                        
                    case gcvLigature:
                        SFGetGlyphProperties(internal, index) |= gpLigature;
                        break;
                        
                    case gcvComponent:
                        SFGetGlyphProperties(internal, index) |= gpComponent;
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

SFBool SFIsIgnoredGlyph(SFInternal *internal, SFGlyphIndex index, LookupFlag lookupFlag) {
    SFBool hasMarkProperty;
    
    if (!SFGetGlyph(internal, index))
        return SFTrue;

    if (SFGetGlyphProperties(internal, index) == gpNotReceived) {
        SFAddGlyphProperties(internal, index);
    }
    
    hasMarkProperty = (SFGetGlyphProperties(internal, index) & gpMark);
    if ((lookupFlag & lfIgnoreMarks) && hasMarkProperty)
        return SFTrue;
    
    if ((lookupFlag & lfIgnoreLigatures) && (SFGetGlyphProperties(internal, index) & gpLigature))
        return SFTrue;
    
    if ((lookupFlag & lfIgnoreBaseGlyphs) && (SFGetGlyphProperties(internal, index) & gpBase))
        return SFTrue;
    
    if (lookupFlag & lfMarkAttachmentType) {
        if (internal->gdef->hasMarkAttachClassDef
            && hasMarkProperty
            && !SFDoesGlyphExistInGlyphClassDef(&internal->gdef->markAttachClassDef, lookupFlag >> 8, SFGetGlyph(internal, index)))
            return SFTrue;
    }
    
    return SFFalse;
}

