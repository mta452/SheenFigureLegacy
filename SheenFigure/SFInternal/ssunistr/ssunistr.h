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

#ifndef _SSUNISTR_H
#define _SSUNISTR_H

#include "SFTypes.h"

int unislen(const SFUnichar value[]);
SFBool hasmaxlen(const SFUnichar *value, int maxLength);
int idxunichar(const SFUnichar value[], SFUnichar word, int startIndex, int count);

SFUnichar *subustrl(const SFUnichar *value, int startIndex, int length);
SFUnichar *subustrs(const SFUnichar *value, int startIndex);

SFUnichar *catustr(const SFUnichar *value1, const SFUnichar *value2);
SFUnichar *catustrch(const SFUnichar value1, SFUnichar *value2);

#endif
