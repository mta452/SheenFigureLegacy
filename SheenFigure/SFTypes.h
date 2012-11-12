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

#ifndef _SF_TYPES_H
#define _SF_TYPES_H

#include <stdint.h>
#include "SFConfig.h"

#ifdef SF_IOS_CG

#include <CoreGraphics/CoreGraphics.h>
typedef CGFloat							SFFloat;

#else

typedef float							SFFloat;

#ifndef COREGRAPHICS_H_
typedef struct CGPoint {
	SFFloat x;
	SFFloat y;
} CGPoint;
#endif

#endif

typedef char							SFBool;
#define SFFalse							0
#define SFTrue							1

typedef uint8_t                         SFUByte;
typedef uint16_t                        SFUShort;
typedef uint32_t                        SFUInt;

typedef SFUShort 						SFUnichar;

typedef SFUShort                        SFGlyph;
typedef SFUInt                          SFColor;

#endif
