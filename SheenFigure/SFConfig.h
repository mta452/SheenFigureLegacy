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

#ifndef _SF_CONFIG_H
#define _SF_CONFIG_H

//#define CMAP_TEST
//#define GDEF_TEST
//#define SCRIPT_TEST
//#define FEATURE_TEST
//#define LOOKUP_TEST

/////////////////DEFINE TABLES THAT ARE IMPLEMENTED IN THE LIBRARY//////////////////////

//////////////////////////////////////GDEF TABLE////////////////////////////////////////

#define GDEF_GLYPH_CLASS_DEF
#define GDEF_ATTACH_LIST
#define GDEF_LIG_CARET_LIST
#define GDEF_MARK_ATTACH_CLASS_DEF
#define GDEF_MARK_GLYPH_SETS_DEF

////////////////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////GSUB TABLE////////////////////////////////////////

#define GSUB_SINGLE_FORMAT1
#define GSUB_SINGLE_FORMAT2
#if defined(GSUB_SINGLE_FORMAT1) || defined(GSUB_SINGLE_FORMAT2)
#define GSUB_SINGLE
#endif

#define GSUB_MULTIPLE

#define GSUB_ALTERNATE

#define GSUB_LIGATURE

#define GSUB_EXTENSION

#define GSUB_REVERSE_CHAINING_CONTEXT

////////////////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////GPOS TABLE////////////////////////////////////////

#define GPOS_SINGLE_FORMAT1
#define GPOS_SINGLE_FORMAT2
#if defined(GPOS_SINGLE_FORMAT1) || defined(GPOS_SINGLE_FORMAT2)
#define GPOS_SINGLE
#endif

#define GPOS_PAIR_FORMAT1
#define GPOS_PAIR_FORMAT2
#if defined(GPOS_PAIR_FORMAT1) || defined(GPOS_PAIR_FORMAT2)
#define GPOS_PAIR
#endif

#define GPOS_CURSIVE

#define GPOS_MARK_TO_BASE

#define GPOS_MARK_TO_LIGATURE

#define GPOS_MARK_TO_MARK

#if defined(GPOS_MARK_TO_BASE) || defined(GPOS_MARK_TO_LIGATURE) || defined(GPOS_MARK_TO_MARK)
#define GPOS_MARK
#endif

#if defined(GPOS_CURSIVE) || defined(GPOS_MARK)
#define GPOS_ANCHOR
#endif

#define GPOS_EXTENSION

////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////GSUB GPOS COMMON TABLE//////////////////////////////////

//#define GSUB_GPOS_CONTEXT_FORMAT1
//#define GSUB_GPOS_CONTEXT_FORMAT2
//#define GSUB_GPOS_CONTEXT_FORMAT3

#if defined(GSUB_GPOS_CONTEXT_FORMAT1) || defined(GSUB_GPOS_CONTEXT_FORMAT2) || defined(GSUB_GPOS_CONTEXT_FORMAT3)
#define GSUB_GPOS_CONTEXT
#endif

//#define GSUB_GPOS_CHAINING_CONTEXT_FORMAT1
//#define GSUB_GPOS_CHAINING_CONTEXT_FORMAT2
#define GSUB_GPOS_CHAINING_CONTEXT_FORMAT3

#if defined(GSUB_GPOS_CHAINING_CONTEXT_FORMAT1) || defined(GSUB_GPOS_CHAINING_CONTEXT_FORMAT2) || defined(GSUB_GPOS_CHAINING_CONTEXT_FORMAT3)
#define GSUB_GPOS_CHAINING_CONTEXT
#endif

////////////////////////////////////////////////////////////////////////////////////////

#endif
