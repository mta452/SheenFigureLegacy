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

#include "SFTypes.h"

SFUShort SFReadUShort(const volatile SFUByte *base, uintptr_t offset) {
    return (base[offset] << 8) | base[offset + 1];
}

SFUInt SFReadUInt(const volatile SFUByte *base, uintptr_t offset) {
    return ((((SFUInt)base[offset] << 24) | base[offset + 1] << 16) | base[offset + 2] << 8) | base[offset + 3];
}
