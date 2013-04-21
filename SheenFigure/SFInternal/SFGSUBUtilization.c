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
#include <string.h>

#include "SFGDEFUtilization.h"
#include "SFGSUBGPOSUtilization.h"
#include "SFGSUBUtilization.h"

#define ONE_PART_LETTER     0
#define TWO_PART_LETTER     1
#define FOUR_PART_LETTER    2

static const SFUInt ARABIC_LETTERS[] =
{
    ONE_PART_LETTER,  /* 0600; ARABIC NUMBER SIGN; U; No_Joining_Group */
    ONE_PART_LETTER,  /* 0601; ARABIC SIGN SANAH; U; No_Joining_Group */
    ONE_PART_LETTER,  /* 0602; ARABIC FOOTNOTE MARKER; U; No_Joining_Group */
    ONE_PART_LETTER,  /* 0603; ARABIC SIGN SAFHA; U; No_Joining_Group */
    ONE_PART_LETTER,  /* 0604; ARABIC SIGN SAMVAT; U; No_Joining_Group */
    ONE_PART_LETTER,  /* 0605 */
    ONE_PART_LETTER,  /* 0606 */
    ONE_PART_LETTER,  /* 0607 */
    ONE_PART_LETTER,  /* 0608; ARABIC RAY; U; No_Joining_Group */
    ONE_PART_LETTER,  /* 0609 */
    ONE_PART_LETTER,  /* 060A */
    ONE_PART_LETTER,  /* 060B; AFGHANI SIGN; U; No_Joining_Group */
    ONE_PART_LETTER,  /* 060C */
    ONE_PART_LETTER,  /* 060D */
    ONE_PART_LETTER,  /* 060E */
    ONE_PART_LETTER,  /* 060F */
    ONE_PART_LETTER,  /* 0610 */
    ONE_PART_LETTER,  /* 0611 */
    ONE_PART_LETTER,  /* 0612 */
    ONE_PART_LETTER,  /* 0613 */
    ONE_PART_LETTER,  /* 0614 */
    ONE_PART_LETTER,  /* 0615 */
    ONE_PART_LETTER,  /* 0616 */
    ONE_PART_LETTER,  /* 0617 */
    ONE_PART_LETTER,  /* 0618 */
    ONE_PART_LETTER,  /* 0619 */
    ONE_PART_LETTER,  /* 061A */
    ONE_PART_LETTER,  /* 061B */
    ONE_PART_LETTER,  /* 061C */
    ONE_PART_LETTER,  /* 061D */
    ONE_PART_LETTER,  /* 061E */
    ONE_PART_LETTER,  /* 061F */
    FOUR_PART_LETTER, /* 0620; DOTLESS YEH WITH SEPARATE RING BELOW; D; YEH */
    ONE_PART_LETTER,  /* 0621; HAMZA; U; No_Joining_Group */
    TWO_PART_LETTER,  /* 0622; ALEF WITH MADDA ABOVE; R; ALEF */
    TWO_PART_LETTER,  /* 0623; ALEF WITH HAMZA ABOVE; R; ALEF */
    TWO_PART_LETTER,  /* 0624; WAW WITH HAMZA ABOVE; R; WAW */
    TWO_PART_LETTER,  /* 0625; ALEF WITH HAMZA BELOW; R; ALEF */
    FOUR_PART_LETTER, /* 0626; DOTLESS YEH WITH HAMZA ABOVE; D; YEH */
    TWO_PART_LETTER,  /* 0627; ALEF; R; ALEF */
    FOUR_PART_LETTER, /* 0628; BEH; D; BEH */
    TWO_PART_LETTER,  /* 0629; TEH MARBUTA; R; TEH MARBUTA */
    FOUR_PART_LETTER, /* 062A; DOTLESS BEH WITH 2 DOTS ABOVE; D; BEH */
    FOUR_PART_LETTER, /* 062B; DOTLESS BEH WITH 3 DOTS ABOVE; D; BEH */
    FOUR_PART_LETTER, /* 062C; HAH WITH DOT BELOW; D; HAH */
    FOUR_PART_LETTER, /* 062D; HAH; D; HAH */
    FOUR_PART_LETTER, /* 062E; HAH WITH DOT ABOVE; D; HAH */
    TWO_PART_LETTER,  /* 062F; DAL; R; DAL */
    TWO_PART_LETTER,  /* 0630; DAL WITH DOT ABOVE; R; DAL */
    TWO_PART_LETTER,  /* 0631; REH; R; REH */
    TWO_PART_LETTER,  /* 0632; REH WITH DOT ABOVE; R; REH */
    FOUR_PART_LETTER, /* 0633; SEEN; D; SEEN */
    FOUR_PART_LETTER, /* 0634; SEEN WITH 3 DOTS ABOVE; D; SEEN */
    FOUR_PART_LETTER, /* 0635; SAD; D; SAD */
    FOUR_PART_LETTER, /* 0636; SAD WITH DOT ABOVE; D; SAD */
    FOUR_PART_LETTER, /* 0637; TAH; D; TAH */
    FOUR_PART_LETTER, /* 0638; TAH WITH DOT ABOVE; D; TAH */
    FOUR_PART_LETTER, /* 0639; AIN; D; AIN */
    FOUR_PART_LETTER, /* 063A; AIN WITH DOT ABOVE; D; AIN */
    FOUR_PART_LETTER, /* 063B; KEHEH WITH 2 DOTS ABOVE; D; GAF */
    FOUR_PART_LETTER, /* 063C; KEHEH WITH 3 DOTS BELOW; D; GAF */
    FOUR_PART_LETTER, /* 063D; FARSI YEH WITH INVERTED V ABOVE; D; FARSI YEH */
    FOUR_PART_LETTER, /* 063E; FARSI YEH WITH 2 DOTS ABOVE; D; FARSI YEH */
    FOUR_PART_LETTER, /* 063F; FARSI YEH WITH 3 DOTS ABOVE; D; FARSI YEH */
    FOUR_PART_LETTER, /* 0640; TATWEEL; C; No_Joining_Group */
    FOUR_PART_LETTER, /* 0641; FEH; D; FEH */
    FOUR_PART_LETTER, /* 0642; QAF; D; QAF */
    FOUR_PART_LETTER, /* 0643; KAF; D; KAF */
    FOUR_PART_LETTER, /* 0644; LAM; D; LAM */
    FOUR_PART_LETTER, /* 0645; MEEM; D; MEEM */
    FOUR_PART_LETTER, /* 0646; NOON; D; NOON */
    FOUR_PART_LETTER, /* 0647; HEH; D; HEH */
    TWO_PART_LETTER,  /* 0648; WAW; R; WAW */
    FOUR_PART_LETTER, /* 0649; DOTLESS YEH; D; YEH */
    FOUR_PART_LETTER, /* 064A; YEH; D; YEH */
    ONE_PART_LETTER,  /* 064B */
    ONE_PART_LETTER,  /* 064C */
    ONE_PART_LETTER,  /* 064D */
    ONE_PART_LETTER,  /* 064E */
    ONE_PART_LETTER,  /* 064F */
    ONE_PART_LETTER,  /* 0650 */
    ONE_PART_LETTER,  /* 0651 */
    ONE_PART_LETTER,  /* 0652 */
    ONE_PART_LETTER,  /* 0653 */
    ONE_PART_LETTER,  /* 0654 */
    ONE_PART_LETTER,  /* 0655 */
    ONE_PART_LETTER,  /* 0656 */
    ONE_PART_LETTER,  /* 0657 */
    ONE_PART_LETTER,  /* 0658 */
    ONE_PART_LETTER,  /* 0659 */
    ONE_PART_LETTER,  /* 065A */
    ONE_PART_LETTER,  /* 065B */
    ONE_PART_LETTER,  /* 065C */
    ONE_PART_LETTER,  /* 065D */
    ONE_PART_LETTER,  /* 065E */
    ONE_PART_LETTER,  /* 065F */
    ONE_PART_LETTER,  /* 0660 */
    ONE_PART_LETTER,  /* 0661 */
    ONE_PART_LETTER,  /* 0662 */
    ONE_PART_LETTER,  /* 0663 */
    ONE_PART_LETTER,  /* 0664 */
    ONE_PART_LETTER,  /* 0665 */
    ONE_PART_LETTER,  /* 0666 */
    ONE_PART_LETTER,  /* 0667 */
    ONE_PART_LETTER,  /* 0668 */
    ONE_PART_LETTER,  /* 0669 */
    ONE_PART_LETTER,  /* 066A */
    ONE_PART_LETTER,  /* 066B */
    ONE_PART_LETTER,  /* 066C */
    ONE_PART_LETTER,  /* 066D */
    FOUR_PART_LETTER, /* 066E; DOTLESS BEH; D; BEH */
    FOUR_PART_LETTER, /* 066F; DOTLESS QAF; D; QAF */
    ONE_PART_LETTER,  /* 0670 */
    TWO_PART_LETTER,  /* 0671; ALEF WITH WASLA ABOVE; R; ALEF */
    TWO_PART_LETTER,  /* 0672; ALEF WITH WAVY HAMZA ABOVE; R; ALEF */
    TWO_PART_LETTER,  /* 0673; ALEF WITH WAVY HAMZA BELOW; R; ALEF */
    ONE_PART_LETTER,  /* 0674; HIGH HAMZA; U; No_Joining_Group */
    TWO_PART_LETTER,  /* 0675; HIGH HAMZA ALEF; R; ALEF */
    TWO_PART_LETTER,  /* 0676; HIGH HAMZA WAW; R; WAW */
    TWO_PART_LETTER,  /* 0677; HIGH HAMZA WAW WITH DAMMA ABOVE; R; WAW */
    FOUR_PART_LETTER, /* 0678; HIGH HAMZA DOTLESS YEH; D; YEH */
    FOUR_PART_LETTER, /* 0679; DOTLESS BEH WITH TAH ABOVE; D; BEH */
    FOUR_PART_LETTER, /* 067A; DOTLESS BEH WITH VERTICAL 2 DOTS ABOVE; D; BEH */
    FOUR_PART_LETTER, /* 067B; DOTLESS BEH WITH VERTICAL 2 DOTS BELOW; D; BEH */
    FOUR_PART_LETTER, /* 067C; DOTLESS BEH WITH ATTACHED RING BELOW AND 2 DOTS ABOVE; D; BEH */
    FOUR_PART_LETTER, /* 067D; DOTLESS BEH WITH INVERTED 3 DOTS ABOVE; D; BEH */
    FOUR_PART_LETTER, /* 067E; DOTLESS BEH WITH 3 DOTS BELOW; D; BEH */
    FOUR_PART_LETTER, /* 067F; DOTLESS BEH WITH 4 DOTS ABOVE; D; BEH */
    FOUR_PART_LETTER, /* 0680; DOTLESS BEH WITH 4 DOTS BELOW; D; BEH */
    FOUR_PART_LETTER, /* 0681; HAH WITH HAMZA ABOVE; D; HAH */
    FOUR_PART_LETTER, /* 0682; HAH WITH VERTICAL 2 DOTS ABOVE; D; HAH */
    FOUR_PART_LETTER, /* 0683; HAH WITH 2 DOTS BELOW; D; HAH */
    FOUR_PART_LETTER, /* 0684; HAH WITH VERTICAL 2 DOTS BELOW; D; HAH */
    FOUR_PART_LETTER, /* 0685; HAH WITH 3 DOTS ABOVE; D; HAH */
    FOUR_PART_LETTER, /* 0686; HAH WITH 3 DOTS BELOW; D; HAH */
    FOUR_PART_LETTER, /* 0687; HAH WITH 4 DOTS BELOW; D; HAH */
    TWO_PART_LETTER,  /* 0688; DAL WITH TAH ABOVE; R; DAL */
    TWO_PART_LETTER,  /* 0689; DAL WITH ATTACHED RING BELOW; R; DAL */
    TWO_PART_LETTER,  /* 068A; DAL WITH DOT BELOW; R; DAL */
    TWO_PART_LETTER,  /* 068B; DAL WITH DOT BELOW AND TAH ABOVE; R; DAL */
    TWO_PART_LETTER,  /* 068C; DAL WITH 2 DOTS ABOVE; R; DAL */
    TWO_PART_LETTER,  /* 068D; DAL WITH 2 DOTS BELOW; R; DAL */
    TWO_PART_LETTER,  /* 068E; DAL WITH 3 DOTS ABOVE; R; DAL */
    TWO_PART_LETTER,  /* 068F; DAL WITH INVERTED 3 DOTS ABOVE; R; DAL */
    TWO_PART_LETTER,  /* 0690; DAL WITH 4 DOTS ABOVE; R; DAL */
    TWO_PART_LETTER,  /* 0691; REH WITH TAH ABOVE; R; REH */
    TWO_PART_LETTER,  /* 0692; REH WITH V ABOVE; R; REH */
    TWO_PART_LETTER,  /* 0693; REH WITH ATTACHED RING BELOW; R; REH */
    TWO_PART_LETTER,  /* 0694; REH WITH DOT BELOW; R; REH */
    TWO_PART_LETTER,  /* 0695; REH WITH V BELOW; R; REH */
    TWO_PART_LETTER,  /* 0696; REH WITH DOT BELOW AND DOT WITHIN; R; REH */
    TWO_PART_LETTER,  /* 0697; REH WITH 2 DOTS ABOVE; R; REH */
    TWO_PART_LETTER,  /* 0698; REH WITH 3 DOTS ABOVE; R; REH */
    TWO_PART_LETTER,  /* 0699; REH WITH 4 DOTS ABOVE; R; REH */
    FOUR_PART_LETTER, /* 069A; SEEN WITH DOT BELOW AND DOT ABOVE; D; SEEN */
    FOUR_PART_LETTER, /* 069B; SEEN WITH 3 DOTS BELOW; D; SEEN */
    FOUR_PART_LETTER, /* 069C; SEEN WITH 3 DOTS BELOW AND 3 DOTS ABOVE; D; SEEN */
    FOUR_PART_LETTER, /* 069D; SAD WITH 2 DOTS BELOW; D; SAD */
    FOUR_PART_LETTER, /* 069E; SAD WITH 3 DOTS ABOVE; D; SAD */
    FOUR_PART_LETTER, /* 069F; TAH WITH 3 DOTS ABOVE; D; TAH */
    FOUR_PART_LETTER, /* 06A0; AIN WITH 3 DOTS ABOVE; D; AIN */
    FOUR_PART_LETTER, /* 06A1; DOTLESS FEH; D; FEH */
    FOUR_PART_LETTER, /* 06A2; DOTLESS FEH WITH DOT BELOW; D; FEH */
    FOUR_PART_LETTER, /* 06A3; FEH WITH DOT BELOW; D; FEH */
    FOUR_PART_LETTER, /* 06A4; DOTLESS FEH WITH 3 DOTS ABOVE; D; FEH */
    FOUR_PART_LETTER, /* 06A5; DOTLESS FEH WITH 3 DOTS BELOW; D; FEH */
    FOUR_PART_LETTER, /* 06A6; DOTLESS FEH WITH 4 DOTS ABOVE; D; FEH */
    FOUR_PART_LETTER, /* 06A7; DOTLESS QAF WITH DOT ABOVE; D; QAF */
    FOUR_PART_LETTER, /* 06A8; DOTLESS QAF WITH 3 DOTS ABOVE; D; QAF */
    FOUR_PART_LETTER, /* 06A9; KEHEH; D; GAF */
    FOUR_PART_LETTER, /* 06AA; SWASH KAF; D; SWASH KAF */
    FOUR_PART_LETTER, /* 06AB; KEHEH WITH ATTACHED RING BELOW; D; GAF */
    FOUR_PART_LETTER, /* 06AC; KAF WITH DOT ABOVE; D; KAF */
    FOUR_PART_LETTER, /* 06AD; KAF WITH 3 DOTS ABOVE; D; KAF */
    FOUR_PART_LETTER, /* 06AE; KAF WITH 3 DOTS BELOW; D; KAF */
    FOUR_PART_LETTER, /* 06AF; GAF; D; GAF */
    FOUR_PART_LETTER, /* 06B0; GAF WITH ATTACHED RING BELOW; D; GAF */
    FOUR_PART_LETTER, /* 06B1; GAF WITH 2 DOTS ABOVE; D; GAF */
    FOUR_PART_LETTER, /* 06B2; GAF WITH 2 DOTS BELOW; D; GAF */
    FOUR_PART_LETTER, /* 06B3; GAF WITH VERTICAL 2 DOTS BELOW; D; GAF */
    FOUR_PART_LETTER, /* 06B4; GAF WITH 3 DOTS ABOVE; D; GAF */
    FOUR_PART_LETTER, /* 06B5; LAM WITH V ABOVE; D; LAM */
    FOUR_PART_LETTER, /* 06B6; LAM WITH DOT ABOVE; D; LAM */
    FOUR_PART_LETTER, /* 06B7; LAM WITH 3 DOTS ABOVE; D; LAM */
    FOUR_PART_LETTER, /* 06B8; LAM WITH 3 DOTS BELOW; D; LAM */
    FOUR_PART_LETTER, /* 06B9; NOON WITH DOT BELOW; D; NOON */
    FOUR_PART_LETTER, /* 06BA; DOTLESS NOON; D; NOON */
    FOUR_PART_LETTER, /* 06BB; DOTLESS NOON WITH TAH ABOVE; D; NOON */
    FOUR_PART_LETTER, /* 06BC; NOON WITH ATTACHED RING BELOW; D; NOON */
    FOUR_PART_LETTER, /* 06BD; NYA; D; NYA */
    FOUR_PART_LETTER, /* 06BE; KNOTTED HEH; D; KNOTTED HEH */
    FOUR_PART_LETTER, /* 06BF; HAH WITH 3 DOTS BELOW AND DOT ABOVE; D; HAH */
    TWO_PART_LETTER,  /* 06C0; DOTLESS TEH MARBUTA WITH HAMZA ABOVE; R; TEH MARBUTA */
    FOUR_PART_LETTER, /* 06C1; HEH GOAL; D; HEH GOAL */
    FOUR_PART_LETTER, /* 06C2; HEH GOAL WITH HAMZA ABOVE; D; HEH GOAL */
    TWO_PART_LETTER,  /* 06C3; TEH MARBUTA GOAL; R; TEH MARBUTA GOAL */
    TWO_PART_LETTER,  /* 06C4; WAW WITH ATTACHED RING WITHIN; R; WAW */
    TWO_PART_LETTER,  /* 06C5; WAW WITH BAR; R; WAW */
    TWO_PART_LETTER,  /* 06C6; WAW WITH V ABOVE; R; WAW */
    TWO_PART_LETTER,  /* 06C7; WAW WITH DAMMA ABOVE; R; WAW */
    TWO_PART_LETTER,  /* 06C8; WAW WITH ALEF ABOVE; R; WAW */
    TWO_PART_LETTER,  /* 06C9; WAW WITH INVERTED V ABOVE; R; WAW */
    TWO_PART_LETTER,  /* 06CA; WAW WITH 2 DOTS ABOVE; R; WAW */
    TWO_PART_LETTER,  /* 06CB; WAW WITH 3 DOTS ABOVE; R; WAW */
    FOUR_PART_LETTER, /* 06CC; FARSI YEH; D; FARSI YEH */
    TWO_PART_LETTER,  /* 06CD; YEH WITH TAIL; R; YEH WITH TAIL */
    FOUR_PART_LETTER, /* 06CE; FARSI YEH WITH V ABOVE; D; FARSI YEH */
    TWO_PART_LETTER,  /* 06CF; WAW WITH DOT ABOVE; R; WAW */
    FOUR_PART_LETTER, /* 06D0; DOTLESS YEH WITH VERTICAL 2 DOTS BELOW; D; YEH */
    FOUR_PART_LETTER, /* 06D1; DOTLESS YEH WITH 3 DOTS BELOW; D; YEH */
    TWO_PART_LETTER,  /* 06D2; YEH BARREE; R; YEH BARREE */
    TWO_PART_LETTER,  /* 06D3; YEH BARREE WITH HAMZA ABOVE; R; YEH BARREE */
    ONE_PART_LETTER,  /* 06D4 */
    TWO_PART_LETTER,  /* 06D5; DOTLESS TEH MARBUTA; R; TEH MARBUTA */
    ONE_PART_LETTER,  /* 06D6 */
    ONE_PART_LETTER,  /* 06D7 */
    ONE_PART_LETTER,  /* 06D8 */
    ONE_PART_LETTER,  /* 06D9 */
    ONE_PART_LETTER,  /* 06DA */
    ONE_PART_LETTER,  /* 06DB */
    ONE_PART_LETTER,  /* 06DC */
    ONE_PART_LETTER,  /* 06DD; ARABIC END OF AYAH; U; No_Joining_Group */
    ONE_PART_LETTER,  /* 06DE */
    ONE_PART_LETTER,  /* 06DF */
    ONE_PART_LETTER,  /* 06E0 */
    ONE_PART_LETTER,  /* 06E1 */
    ONE_PART_LETTER,  /* 06E2 */
    ONE_PART_LETTER,  /* 06E3 */
    ONE_PART_LETTER,  /* 06E4 */
    ONE_PART_LETTER,  /* 06E5 */
    ONE_PART_LETTER,  /* 06E6 */
    ONE_PART_LETTER,  /* 06E7 */
    ONE_PART_LETTER,  /* 06E8 */
    ONE_PART_LETTER,  /* 06E9 */
    ONE_PART_LETTER,  /* 06EA */
    ONE_PART_LETTER,  /* 06EB */
    ONE_PART_LETTER,  /* 06EC */
    ONE_PART_LETTER,  /* 06ED */
    TWO_PART_LETTER,  /* 06EE; DAL WITH INVERTED V ABOVE; R; DAL */
    TWO_PART_LETTER,  /* 06EF; REH WITH INVERTED V ABOVE; R; REH */
    ONE_PART_LETTER,  /* 06F0 */
    ONE_PART_LETTER,  /* 06F1 */
    ONE_PART_LETTER,  /* 06F2 */
    ONE_PART_LETTER,  /* 06F3 */
    ONE_PART_LETTER,  /* 06F4 */
    ONE_PART_LETTER,  /* 06F5 */
    ONE_PART_LETTER,  /* 06F6 */
    ONE_PART_LETTER,  /* 06F7 */
    ONE_PART_LETTER,  /* 06F8 */
    ONE_PART_LETTER,  /* 06F9 */
    FOUR_PART_LETTER, /* 06FA; SEEN WITH DOT BELOW AND 3 DOTS ABOVE; D; SEEN */
    FOUR_PART_LETTER, /* 06FB; SAD WITH DOT BELOW AND DOT ABOVE; D; SAD */
    FOUR_PART_LETTER, /* 06FC; AIN WITH DOT BELOW AND DOT ABOVE; D; AIN */
    ONE_PART_LETTER,  /* 06FD */
    ONE_PART_LETTER,  /* 06FE */
    FOUR_PART_LETTER, /* 06FF; KNOTTED HEH WITH INVERTED V ABOVE; D; KNOTTED HEH */
};

#define GSUB_FEATURE_TAGS 8
static unsigned char gsubFeaturesTagOrder[GSUB_FEATURE_TAGS][5] = {
    "ccmp",
    "init",
    "medi",
    "fina",
    "isol",
    "rlig",
    "calt",
    "liga",
};

typedef enum {
    stInit,
    stMedi,
    stFina,
    stOther,
} ShortTag;

static void SFApplyGSUBLookup(SFInternal *internal, LookupTable lookup, SFGlyphIndex sidx, SFGlyphIndex eidx);

static ShortTag getShortTag(const unsigned char tag[5]) {
    ShortTag stag = stOther;
    if (tag) {
        if (tag[0] == 'i' && tag[1] == 'n' && tag[2] == 'i' && tag[3] == 't')
            stag = stInit;
        else if (tag[0] == 'm' && tag[1] == 'e' && tag[2] == 'd' && tag[3] == 'i')
            stag = stMedi;
        else if (tag[0] == 'f' && tag[1] == 'i' && tag[2] == 'n' && tag[3] == 'a')
            stag = stFina;
    }
    
    return stag;
}

static void applyArabicSubstitution(SFInternal *internal, void *stable, LookupFlag lookupFlag, LookupType type, short stag) {
    CoverageTable coverage;
    
#ifdef GSUB_SINGLE
    SingleSubstSubtable *singleSubst;
#endif

#ifdef GSUB_MULTIPLE
    MultipleSubstSubtable *multipleSubst;
#endif

#ifdef GSUB_ALTERNATE
    AlternateSubstSubtable *alternateSubst;
#endif
    
    // type of previous, current and next letter
    int tpl = ONE_PART_LETTER;
    int tcl;
    int tnl;
    
    int currentIndex;
    int nextIndex;
    int lastIndex = SFGetCharCount(internal) - 1;
    
    int i = 0;
    int ni = 0;
    
#ifdef GSUB_SINGLE
#define GSUB_ELSE

    if (type == ltsSingle) {
        singleSubst = (SingleSubstSubtable *)stable;
        coverage = singleSubst->coverage;
    }
    
#endif
    
#ifdef GSUB_MULTIPLE
#ifdef GSUB_ELSE
    else
#else
#define GSUB_ELSE
#endif
        
    if (type == ltsMultiple) {
        multipleSubst = (MultipleSubstSubtable *)stable;
        coverage = multipleSubst->coverage;
    }
    
#endif
    
#ifdef GSUB_ALTERNATE
#ifdef GSUB_ELSE
    else
#else
#define GSUB_ELSE
#endif

    if (type == ltsAlternate) {
        alternateSubst = (AlternateSubstSubtable *)stable;
        coverage = alternateSubst->coverage;
    }
    
#endif
    
#ifdef GSUB_ELSE
    else
#else
#undef GSUB_ELSE
#endif
        return;                 // don't go any further if no table is implemented or if lookup type
                                // is different from implemented tables

#ifdef GSUB_ELSE
#undef GSUB_ELSE
#endif

#define GET_TYPE_OF_LETTER(l) ((l >= 0x0600 && l <= 0x06FF) ? ARABIC_LETTERS[l - 0x0600] : ONE_PART_LETTER)

#define SET_NULL_VALUES()           \
    tnl = ONE_PART_LETTER;          \
    nextIndex = UNDEFINED_INDEX;
    
#define GET_NEXT_VALUES()                                                                       \
    for (; ni < SFGetCharCount(internal); ni++) {                                              \
        if (!SFIsIgnoredGlyph(internal, SFMakeGlyphIndex(ni, 0), lookupFlag)) {                \
            break;                                                                              \
        }                                                                                       \
    }                                                                                           \
\
    if (ni >= SFGetCharCount(internal)) {                                                      \
        SET_NULL_VALUES();                                                                      \
    } else {                                                                                    \
        tnl = GET_TYPE_OF_LETTER(SFGetChar(internal, ni));                                     \
        nextIndex = SFGetIndexOfGlyphInCoverage(&coverage, internal->record->charRecord[ni].gRec[0].glyph);\
    }
    
    GET_NEXT_VALUES();
    i = ni;
    
    for (; i < SFGetCharCount(internal);) {
        tcl = tnl;
        currentIndex = nextIndex;
        ni++;
        
        if (i == lastIndex) {
            SET_NULL_VALUES();
        } else {
            GET_NEXT_VALUES();
        }
        
        if (currentIndex != UNDEFINED_INDEX) {
            if ((stag == stInit && !(tpl != FOUR_PART_LETTER && (tnl == TWO_PART_LETTER || tnl == FOUR_PART_LETTER)))
                || (stag == stMedi && !(tpl == FOUR_PART_LETTER && (tnl == TWO_PART_LETTER || tnl == FOUR_PART_LETTER)))
                || (stag == stFina && !(tpl == FOUR_PART_LETTER && (tcl != FOUR_PART_LETTER ? 1 : tnl != FOUR_PART_LETTER)))) {
                goto continue_loop;
            }
            
#ifdef GSUB_SINGLE
#define GSUB_ELSE

            if (type == ltsSingle) {
                if (singleSubst->substFormat == 1)
                    internal->record->charRecord[i].gRec[0].glyph += singleSubst->format.format1.deltaGlyphID;
                else if (singleSubst->substFormat == 2)
                    internal->record->charRecord[i].gRec[0].glyph = singleSubst->format.format2.substitute[currentIndex];
                
                internal->record->charRecord[i].gRec[0].glyphProp = gpNotReceived;
            }
#endif

#ifdef GSUB_MULTIPLE
#ifdef GSUB_ELSE
            else
#else
#define GSUB_ELSE
#endif
            if (type == ltsMultiple) {
                if (multipleSubst->sequence[currentIndex].glyphCount == 1) {
                    internal->record->charRecord[i].gRec[0].glyph = multipleSubst->sequence[currentIndex].substitute[0];
                    
                    internal->record->charRecord[i].gRec[0].glyphProp = gpNotReceived;
                } else {
                    int length;
                    
                    SFInsertGlyphs(&internal->record->charRecord[i].gRec, multipleSubst->sequence[currentIndex].substitute, 1, multipleSubst->sequence[currentIndex].glyphCount, 0, &length);
                    
                    internal->record->glyphCount += length - internal->record->charRecord[i].glyphCount;
                    internal->record->charRecord[i].glyphCount = length;
                }
            }
#endif

#ifdef GSUB_ALTERNATE
#ifdef GSUB_ELSE
            else
#else
#define GSUB_ELSE
#endif
            if (type == ltsAlternate) {
                internal->record->charRecord[i].gRec[0].glyph = alternateSubst->alternateSet[currentIndex].substitute[0];
                internal->record->charRecord[i].gRec[0].glyphProp = gpNotReceived;
            }
#endif
        }
        
#ifdef GSUB_ELSE
#undef GSUB_ELSE
#endif
        
    continue_loop:
        tpl = tcl;
        i = ni;
    }
    
#undef SET_NULL_VALUES
#undef GET_NEXT_VALUES
}

#ifdef GSUB_SINGLE

static void SFApplySingleSubstitution(SFInternal *internal, SingleSubstSubtable *stable, LookupFlag lookupFlag, const unsigned char tag[5], SFGlyphIndex sidx, SFGlyphIndex eidx) {
    ShortTag stag = getShortTag(tag);
    if (stag == stOther) {
        int cidx;                   // coverage index
        int gidx;                   // end glyph index
        
        for (; ;) {
            if (sidx.record < eidx.record) {
                if (SFIsOddLevel(internal, sidx.record)) {
                    goto continue_loop;
                }
                
                gidx = SFGetGlyphCount(internal, sidx.record);
            } else if (sidx.record == eidx.record) {
                if (SFIsOddLevel(internal, sidx.record)) {
                    break;
                }
                
                gidx = eidx.glyph + 1;
            } else {
                break;
            }
            
            for (; sidx.glyph < gidx; sidx.glyph++) {
                if (SFIsIgnoredGlyph(internal, sidx, lookupFlag))
                    continue;
                
                cidx = SFGetIndexOfGlyphInCoverage(&stable->coverage, SFGetGlyph(internal, sidx));
                if (cidx != UNDEFINED_INDEX) {
                    if (stable->substFormat == 1) {
                        SFGetGlyph(internal, sidx) += stable->format.format1.deltaGlyphID;
                    } else if (stable->substFormat == 2) {
                        SFGetGlyph(internal, sidx) = stable->format.format2.substitute[cidx];
                    }
                    
                    SFGetGlyphProperties(internal, sidx) = gpNotReceived;
                }
            }
            
        continue_loop:
            sidx.record++;
            sidx.glyph = 0;
        }
    } else {
        applyArabicSubstitution(internal, stable, lookupFlag | lfIgnoreMarks, ltsSingle, stag);
    }
}

#endif

#ifdef GSUB_MULTIPLE

static void SFApplyMultipleSubstitution(SFInternal *internal, MultipleSubstSubtable *stable, LookupFlag lookupFlag, unsigned char tag[5], SFGlyphIndex sidx, SFGlyphIndex eidx) {
    ShortTag stag;
    
    if (!SFGetCharCount(internal))
        return;
    
    stag = getShortTag(tag);
    if (stag == stOther) {
        int cidx;                   // coverage index
        int gidx;                   // end glyph index
        
        for (; ;) {
            if (sidx.record < eidx.record) {
                if (SFIsOddLevel(internal, sidx.record)) {
                    goto continue_loop;
                }
                
                gidx = SFGetGlyphCount(internal, sidx.record);
            } else if (sidx.record == eidx.record) {
                if (SFIsOddLevel(internal, sidx.record)) {
                    break;
                }
                
                gidx = eidx.glyph + 1;
            } else {
                break;
            }
            
            for (; sidx.glyph < gidx; sidx.glyph++) {
                if (SFIsIgnoredGlyph(internal, sidx, lookupFlag))
                    continue;
                
                cidx = SFGetIndexOfGlyphInCoverage(&stable->coverage, SFGetGlyph(internal, sidx));
                if (cidx != UNDEFINED_INDEX) {
                    if (stable->sequence[cidx].glyphCount == 1) {
                        SFGetGlyph(internal, sidx) = stable->sequence[cidx].substitute[0];
                        SFGetGlyphProperties(internal, sidx) = gpNotReceived;
                    } else {
                        int length;
                        SFInsertGlyphs(&SFGetGlyphRecord(internal, sidx.record), stable->sequence[cidx].substitute, SFGetGlyphCount(internal, sidx.record), stable->sequence[cidx].glyphCount, sidx.glyph, &length);
                        
                        SFGetTotalGlyphCount(internal) += length - SFGetGlyphCount(internal, sidx.record);
                        SFGetGlyphCount(internal, sidx.record) = length;
                    }
                }
            }
            
        continue_loop:
            sidx.record++;
            sidx.glyph = 0;
        }
    } else {
        applyArabicSubstitution(internal, stable, lookupFlag | lfIgnoreMarks, ltsMultiple, stag);
    }
}

#endif

#ifdef GSUB_ALTERNATE

static void SFApplyAlternateSubstitution(SFInternal *internal, AlternateSubstSubtable *stable, LookupFlag lookupFlag, const unsigned char tag[5], SFGlyphIndex sidx, SFGlyphIndex eidx) {
    ShortTag stag = getShortTag(tag);
    if (stag == stOther) {
        int cidx;                   // coverage index
        int gidx;                   // end glyph index
        
        for (; ;) {
            if (sidx.record < eidx.record) {
                if (SFIsOddLevel(internal, sidx.record)) {
                    goto continue_loop;
                }
                
                gidx = SFGetGlyphCount(internal, sidx.record);
            } else if (sidx.record == eidx.record) {
                if (SFIsOddLevel(internal, sidx.record)) {
                    break;
                }
                
                gidx = eidx.glyph + 1;
            } else {
                break;
            }
            
            for (; sidx.glyph < gidx; sidx.glyph++) {
                if (SFIsIgnoredGlyph(internal, sidx, lookupFlag)) {
                    continue;
                }
                
                cidx = SFGetIndexOfGlyphInCoverage(&stable->coverage, SFGetGlyph(internal, sidx));
                if (cidx != UNDEFINED_INDEX)
                    SFGetGlyph(internal, sidx) = stable->alternateSet[cidx].substitute[0];
            }
            
        continue_loop:
            sidx.record++;
            sidx.glyph = 0;
        }
    } else
        applyArabicSubstitution(internal, stable, lookupFlag, ltsAlternate, stag);
}

#endif

#ifdef GSUB_LIGATURE

static void SFApplyLigatureSubstitution(SFInternal *internal, LigatureSubstSubtable *stable, LookupFlag lookupFlag, SFGlyphIndex sidx, SFGlyphIndex eidx) {
    SFGlyphIndex index = sidx;
    if (SFIsIgnoredGlyph(internal, index, lookupFlag)) {
        if (!SFGetNextValidGlyphIndex(internal, &index, lookupFlag)) {
            return;
        }
    }
    
    do {
        int coverageIndex;
        LigatureSetTable currentLigSet;
        
        int i;
        
        coverageIndex = SFGetIndexOfGlyphInCoverage(&stable->coverage, SFGetGlyph(internal, index));
        
        if (coverageIndex == UNDEFINED_INDEX)
            continue;
        
        currentLigSet = stable->ligatureSet[coverageIndex];
        
        //Loop to match any of the ligature table with current glyphs.
        for (i = 0; i < currentLigSet.ligatureCount; i++) {
			SFGlyphIndex *inputIndexes = malloc(sizeof(SFGlyphIndex) * currentLigSet.ligature[i].compCount);

            int inputIndexesCount;
            int j = 1, k = 0, l = 1;
            
            inputIndexesCount = currentLigSet.ligature[i].compCount;
            inputIndexes[0] = index;
            
            //Loop to match next required glyphs.
            for (; j < currentLigSet.ligature[i].compCount; j++) {
                SFGlyphIndex tmpIndex = inputIndexes[k];
                if (!SFGetNextValidGlyphIndex(internal, &tmpIndex, lookupFlag))
                    goto continue_parent_loop;
                
                if (SFGetGlyph(internal, tmpIndex) != currentLigSet.ligature[i].component[j])
                    goto continue_parent_loop;
                
                inputIndexes[++k] = tmpIndex;
            }
            
            // now apply substitution
            SFGetGlyph(internal, index) = currentLigSet.ligature[i].ligGlyph;
            SFGetGlyphProperties(internal, index) = gpNotReceived;
            
            for (; l < inputIndexesCount; l++) {
                SFGetGlyph(internal, inputIndexes[l]) = 0;
                SFGetTotalGlyphCount(internal) -= 1;
            }
            
            index = inputIndexes[l - 1];

			free(inputIndexes);
            break;
            
        continue_parent_loop:
            {
				free(inputIndexes);
            }
        }
    } while (SFGetNextValidGlyphIndex(internal, &index, lookupFlag)
             && SFCompareGlyphIndex(index, eidx) <= 0);
}

#endif

static void SFApplyGSUBLookupWithFeatureTag(SFInternal *internal, LookupTable lookup, unsigned char featureTag[5], SFGlyphIndex sidx, SFGlyphIndex eidx) {
	int i = 0;

#ifdef GSUB_SINGLE
    
    if (lookup.lookupType == ltsSingle) {
        for (; i < lookup.subTableCount; i++)
            SFApplySingleSubstitution(internal, lookup.subtables[i], lookup.lookupFlag, featureTag, sidx, eidx);
    }
    
#define GSUB_ELSE
    
#endif
    
#ifdef GSUB_ALTERNATE
    
#ifdef GSUB_ELSE
    else
#else
#define GSUB_ELSE
#endif

    if (lookup.lookupType == ltsAlternate) {
        for (; i < lookup.subTableCount; i++)
            SFApplyAlternateSubstitution(internal, lookup.subtables[i], lookup.lookupFlag, featureTag, sidx, eidx);
    }
    
#endif
    
#ifdef GSUB_MULTIPLE
    
#ifdef GSUB_ELSE
    else
#else
#define GSUB_ELSE
#endif
        
    if (lookup.lookupType == ltsMultiple) {
        for (; i < lookup.subTableCount; i++)
            SFApplyMultipleSubstitution(internal, lookup.subtables[i], lookup.lookupFlag, featureTag, sidx, eidx);
    }
    
#endif
    
#ifdef GSUB_LIGATURE
    
#ifdef GSUB_ELSE
    else
#else
#define GSUB_ELSE
#endif
    
    if (lookup.lookupType == ltsLigature) {
        for (; i < lookup.subTableCount; i++)
            SFApplyLigatureSubstitution(internal, lookup.subtables[i], lookup.lookupFlag, sidx, eidx);
    }
    
#endif
    
#ifdef GSUB_GPOS_CHAINING_CONTEXT
    
#ifdef GSUB_ELSE
    else
#else
#define GSUB_ELSE
#endif
        
    if (lookup.lookupType == ltsChainingContext) {
        for (; i < lookup.subTableCount; i++)
            SFApplyChainingContextual(internal, lookup.subtables[i], lookup.lookupFlag, &SFApplyGSUBLookup);
    }
    
#endif

#ifdef GSUB_ELSE
#undef GSUB_ELSE
#endif
}

static void SFApplyGSUBLookup(SFInternal *internal, LookupTable lookup, SFGlyphIndex sidx, SFGlyphIndex eidx) {
    SFApplyGSUBLookupWithFeatureTag(internal, lookup, NULL, sidx, eidx);
}

static void SFApplyGSUBFeatureList(SFInternal *internal, int featureIndex) {
    FeatureTable feature = internal->gsub->featureList.featureRecord[featureIndex].feature;
    
    SFGlyphIndex sidx;
    SFGlyphIndex eidx;
    
	int i;

    sidx.record = 0;
    sidx.glyph = 0;
    
    eidx.record = SFGetCharCount(internal) - 1;
    eidx.glyph = SFGetGlyphCount(internal, eidx.record);
    
    for (i = 0; i < feature.lookupCount; i++) {
        LookupTable currentLookup = internal->gsub->lookupList.lookupTables[feature.lookupListIndex[i]];
        SFApplyGSUBLookupWithFeatureTag(internal, currentLookup, internal->gsub->featureList.featureRecord[featureIndex].featureTag, sidx, eidx);
    }
}

static int getIndexOfGSUBFeatureTag(char tag[5]) {
	int i;
    for (i = 0; i < GSUB_FEATURE_TAGS; i++) {
        if (strcmp((const char *)gsubFeaturesTagOrder[i], tag) == 0)
            return i;
    }
    
    return -1;
}

void SFApplyGSUB(SFInternal *internal) {
    SFBool arabScriptFound = SFFalse;
    int arabScriptIndex;
    
	int i;
    for (i = 0; i < internal->gsub->scriptList.scriptCount; i++) {
        if (strcmp((const char *)internal->gsub->scriptList.scriptRecord[i].scriptTag, "arab") == 0) {
            arabScriptFound = SFTrue;
            arabScriptIndex = i;
        }
    }
    
    if (arabScriptFound) {
        int totalFeatures = internal->gsub->scriptList.scriptRecord[arabScriptIndex].script.defaultLangSys.featureCount;
        
        int order1[GSUB_FEATURE_TAGS];
        
        int order2Len = 0;
		int *order2 = malloc(sizeof(int) * totalFeatures);

        for (i = 0; i < GSUB_FEATURE_TAGS; i++)
            order1[i] = -1;
        
        for (i = 0; i < totalFeatures; i++) {
            int featureIndex = internal->gsub->scriptList.scriptRecord[arabScriptIndex].script.defaultLangSys.featureIndex[i];
            
            int val = getIndexOfGSUBFeatureTag((char *)internal->gsub->featureList.featureRecord[featureIndex].featureTag);
            if (val > -1)
                order1[val] = featureIndex;
            else
                order2[order2Len++] = featureIndex;
        }
        
        for (i = 0; i < GSUB_FEATURE_TAGS; i++) {
            if (order1[i] > -1)
                SFApplyGSUBFeatureList(internal, order1[i]);
        }
        
		free(order2);

        /*for (int i = 0; i < record->charCount; i++) {
         for (int j = 0; j < record->charRecord[i].glyphCount; j++)
         memset(&record->charRecord[i].gRec[j].posRec, 0, sizeof(SFPositionRecord));
         }*/
        
        //for (int i = 0; i < order2Len; i++)
        //    applyGSUBFeatureList(order2[i]);
    }
}

