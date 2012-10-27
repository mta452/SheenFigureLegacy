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
#include <stdbool.h>

#include "ssunistr.h"

int unislen(const SFUnichar value[]) {
    if (value == NULL)
        return 0;
    
    int x = 0;
    
    while (value[x] != '\0')
        x++;
    
    return(x);
}

bool hasmaxlen(const SFUnichar *value, int maxLength) {
    int length = -1;
    while (++length < maxLength) {
        if (value[length] == 0)
            return false;
    }
    
    return true;
}

int idxunichar(const SFUnichar value[], SFUnichar word, int startIndex, int count) {
    int length = startIndex + count;
    
    while (startIndex < length) {
        if (value[startIndex] == word)
            return startIndex;
        
        startIndex++;
    }
    
    return -1;
}

SFUnichar *subustrl(const SFUnichar *value, int startIndex, int length) {
    if (length <= 0)
        return NULL;
    
    SFUnichar *dest = (SFUnichar *)malloc(sizeof(SFUnichar) * (length + 1));
    
    int i = 0;
    while (i < length && (dest[i++] = value[startIndex++]) != 0);
    
    dest[i] = 0;
    
    return dest;
}

SFUnichar *subustrs(const SFUnichar *value, int startIndex) {
    return subustrl(value, startIndex, unislen(value) - startIndex);
}

SFUnichar *catustr(const SFUnichar *value1, const SFUnichar *value2) {
    SFUnichar *dest = (SFUnichar *)malloc(sizeof(SFUnichar) * ((unislen(value1) + unislen(value2)) + 1));
    
    int i = -1;
    while ((dest[++i] = value1[i]) != 0);
    
    int j = -1;
    while ((dest[i++] = value2[++j]) != 0);
    
    return dest;
}

SFUnichar *catustrch(const SFUnichar value1, SFUnichar *value2) {
    SFUnichar *dest = (SFUnichar *)malloc(sizeof(SFUnichar) * (unislen(value2) + 2));
    dest[0] = value1;
    
    int i = 0, j = -1;
    while ((dest[++i] = value2[++j]) != 0);
    
    return dest;
}

