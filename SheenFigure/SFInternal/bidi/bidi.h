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

#ifndef _BIDI_H
#define _BIDI_H

#include "SFTypes.h"

typedef struct SFMirroredChar {
    int index;
    SFUnichar ch;
    
    struct SFMirroredChar *next;
} SFMirroredChar;

int baseLevel(const int *pcls, int cch);
SFMirroredChar *generateMirroredChars(const SFUnichar *pszInput, const int *plevel, int cch);
void generateBidiVisualOrder(int baselevel, const int *pclsLine, int *plevelLine, int *visOrder, int cchPara);
void generateBidiTypesAndLevels(int baselevel, const SFUnichar *pszInput, int *types, int *levels, int cch);

#endif
