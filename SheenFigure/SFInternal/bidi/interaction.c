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

#include "SFTypes.h"
#include "fribidi_tab_char_type_9.i"
#include "fribidi_tab_mirroring.i"

#include "interaction.h"

SFBool getMirroredChar(SFUnichar ch, SFUnichar *mirror) {
    int pos, step;
    SFBool found;
    
    pos = step = (nFriBidiMirroredChars / 2) + 1;
    
    while (step > 1) {
        SFUnichar cmp_ch = FriBidiMirroredChars[pos].ch;
        step = (step + 1) / 2;
        
        if (cmp_ch < ch) {
            pos += step;
            if (pos > nFriBidiMirroredChars - 1) {
                pos = nFriBidiMirroredChars - 1;
            }
        } else if (cmp_ch > ch) {
            pos -= step;
            if (pos < 0) {
                pos = 0;
            }
        } else {
            break;
        }
    }
    found = FriBidiMirroredChars[pos].ch == ch;
    if (mirror) {
        *mirror = found ? FriBidiMirroredChars[pos].mirrored_ch : ch;
    }
    
    return found;
}

int getTypeOfChar(SFUnichar ch) {
    return (unsigned char)FRIBIDI_GET_TYPE(ch);
}
