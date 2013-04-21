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

#ifndef _SF_TEXT_PRIVATE_H
#define _SF_TEXT_PRIVATE_H

#include "SFConfig.h"
#include "SFTypes.h"
#include "SFInternal.h"
#include "SFFontPrivate.h"

typedef struct SFText {
    const SFUnichar *_str;
    int _strLength;

    SFFont *_sfFont;
    SFStringRecord *_record;
    int _writingDirection;
    int _txtAlign;
    
    SFUInt _retainCount;
} SFText;

#ifndef _SF_TEXT_REF
#define _SF_TEXT_REF

typedef struct SFText *SFTextRef;

#endif

#endif
