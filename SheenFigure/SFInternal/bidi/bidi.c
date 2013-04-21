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

// Bidi.cpp - version 26

// Reference implementation for Unicode Bidirectional Algorithm

// Bidi include file

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

#include "SFTypes.h"
#include "SFCMAPUtilization.h"

#include "bidi.h"
#include "bidi_class.h"
#include "interaction.h"

/*------------------------------------------------------------------------
    File: Bidi.Cpp

    Description
    -----------

    Sample Implementation of the Unicode Bidirectional Algorithm as it
    was revised by Revision 5 of the Uniode Technical Report # 9
    (1999-8-17)

    Verified for changes to the algorithm up through Unicode 5.2.0 (2009).

    This implementation is organized into several passes, each implemen-
    ting one or more of the rules of the Unicode Bidi Algorithm. The
    resolution of Weak Types and of Neutrals each use a state table
    approach.

    Both a printf based interface and a Windows DlgProc are provided for
    interactive testing.

    A stress harness comparing this implementation (v24) to a Java based 
    implementation was used by Doug Felt to verify that the two 
    implementations produce identical results for all strings up to six
    bidi classes and stochastic strings up to length 20.

    Version 26 was verified by the author against the Unicode 5.2.0
    file BidiTest.txt, which provides an exhaustive text of strings of
    length 4 or less, but covers some important cases where the language
    in UAX#9 had been clarified.

    To see this code running in an actual Windows program,
    download the free Unibook uitlity from http://unicode.org/unibook
    The bidi demo is executed from the tools menu. It is build from
    this source file.

    Build Notes
    -----------

    To compile the sample implementation please set the #define
    directives above so the correct headers get included. Not all the
    files are needed for all purposes. For the commandline version
    only bidi.h and bidi.cpp are needed.

    The Win32 version is provided as a dialog procedure. To use as a
    standalone program compile with the the lbmain.cpp file. If all you
    need is the ability to run the code "as is", you can instead download
    the unibook utility from http://uincode.org/unibook/ which contains
    the bidi demo compiled from this source file.

    This code uses an extension to C++ that gives variables declared in
    a for() statement function the same scope as the for() statement.
    If your compiler does not support this extension, you may need to
    move the declaration, e.g. int ich = 0; in front of the for statement.

    Implementation Note
    -------------------

    NOTE: The Unicode Bidirectional Algorithm removes all explicit
        formatting codes in rule X9, but states that this can be
        simulated by conformant implementations. This implementation
        attempts to demonstrate such a simulation

        To demonstrate this, the current implementation does the
        following:

        in resolveExplicit()
            - change LRE, LRO, RLE, RLO, PDF to BN
            - assign nested levels to BN

        in resolveWeak and resolveNeutrals
            - assign L and R to BN's where they exist in place of
              sor and eor by changing the last BN in front of a
              level change to a strong type
            - skip over BN's for the purpose of determining actions
            - include BN in the count of deferred runs
                which will resolve some of them to EN, AN and N

        in resolveWhiteSpace
            - set the level of any surviving BN to the base level,
                or the level of the preceding character
            - include LRE,LRO, RLE, RLO, PDF and BN in the count
               whitespace to be reset

        This will result in the same order for non-BN characters as
        if the BN characters had been removed.

        The clean() function can be used to remove boundary marks for
        verification purposes.

    Notation
    --------
    Pointer variables generally start with the letter p
    Counter variables generally start with the letter c
    Index variables generally start with the letter i
    Boolean variables generally start with the letter f

    The enumerated bidirectional types have the same name as in the
    description for the Unicode Bidirectional Algorithm


    Using this code outside a demo context
    --------------------------------------

    The way the functions are broken down in this demo code is based
    on the needs of the demo to show the evolution in internal state
    as the algorithm proceeds. This obscures how the algorithm would
    be used in practice. These are the steps:

    1. Allocate a pair of arrays large enough to hold bidi class
       and calculated levels (one for each input character)

    2. Provide your own function to assign directional types (bidi
       classes) corresponding to each character in the input,
       conflating ON, WS, S to N. Unlike the classify function in this
       demo, the input would be actual Unicode characters.

    3. Process the first paragraph by calling BidiParagraph. That 
       function changes B into BN and returns a length including the 
       paragraph separator. (The iteration over multiple paragraphs 
       is left as excercise for the reader). 
    
    4. Assign directional types again, but now assign specific types
       to whitespace characters.

    5. Instead of reordering the input in place it is often desirable
       to calculate an array of offsets indicating the reordering.
       To that end, allocate such an array here and use it instead
       of the input array in the next step.
    
    6. Resolve and reorder the lines by calling BidiLines. That 
       function 'breaks' lines on LS characters. Provide an optional
       array of flags indicating the location of other line breaks as
       needed.


    Update History
    --------------
    Version 24 is the initial published and verified version of this
    reference implementation. Version 25 and its updates fix various
    minor issues with the scaffolding used for demonstrating the
    algorithm using pseudo-alphabets from the command line or dialog
    box. No changes to the implementation of the actual bidi algrithm
    are made in any of the minor updates to version 25. Version 26
    also makes no change to the actual algorithm but was verified
    against the official BidiTest.txt file for Unicode 5.2.0.

    - updated pseudo-alphabet

    - Last Revised 12-10-99 (25)

    - enable demo mode for release builds - no other changes

    - Last Revised 12-10-00 (25a)

    - fix regression in pseudo alphabet use for Windows UI

    - Last Revised 02-01-01 (25b)

    - fixed a few comments, renamed a variable

    - Last Revised 03-04-01 (25c)

    - make base level settable, enable mirror by default,
    fix dialog size

    - Last Revised 06-02-01 (25e)

    - fixed some comments

    - Last Revised 09-29-01 (25f)

    - fixed classification for LS,RLM,LRM in pseudo alphabet,
    focus issues in UI, regression fix to commandline from 25(e)
    fix DEMO switch

    - Last Revised 11-07-01 (25g)

    - fixed classification for plus/minus in pseudo alphabet
    to track changes made in Unicode 4.0.1

    - Last Revised 12-03-04 (25h)

    - now compiles as dialog-only program for WINDOWS_UI==1
    using new bidimain.cpp

    - Last Revised 12-02-07 (25i)

    - cleaned up whitespace and indenting in the source, 
    fixed two comments (table headers)

    - Last Revised 15-03-07 (25j)

    - named enumerations

    - Last Revised 30-05-07 (25k)

    - added usage notes, minor edits to comments, indentation, etc
    throughout. Added the bidiParagraph function. Checked against 
    changes in the Unicode Bidi Algorithm for Unicode 5.2.0. No
    changes needed to this implementation to match the values in
    the BidiTest.txt file in the Unicode Character Database.
    Minor fixes to dialog/windows proc, updated preprocessor directives.

    - Last Revised 03-08-09 (26)

    Credits:
    -------
    Written by: Asmus Freytag
    Command line interface by: Rick McGowan
    Verification (v24): Doug Felt

    Disclaimer and legal rights:
    ---------------------------
    Copyright (C) 1999-2009, ASMUS, Inc. All Rights Reserved.
    Distributed under the Terms of Use in http://www.unicode.org/copyright.html.

    THIS SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
    OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT OF THIRD PARTY RIGHTS.
    IN NO EVENT SHALL THE COPYRIGHT HOLDER OR HOLDERS INCLUDED IN THIS NOTICE
    BE LIABLE FOR ANY CLAIM, OR ANY SPECIAL INDIRECT OR CONSEQUENTIAL DAMAGES,
    OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
    WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
    ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THE SOFTWARE.

    The file bid.rc is included in the software covered by the above.
------------------------------------------------------------------------*/

// === HELPER FUNCTIONS AND DECLARATIONS =================================

/*----------------------------------------------------------------------
 The following array maps character codes to types for the purpose of
 this sample implementation. The legend string gives a human readable
 explanation of the pseudo alphabet.
 
 For simplicity, characters entered by buttons are given a 1:1 mapping
 between their type and pseudo character value. Pseudo characters that
 can be typed from the keyboard are explained in the legend string.
 
 Use the Unicode Character Database for the real values in real use.
 ---------------------------------------------------------------------*/

/***************************************
    Reverse, human readable reference:
    
    LRM:    0x4
    RLM:    0x5
      L:    0x16,a-z
    LRE:    0x11,[
    LRO:    0x10,{
      R:    0x17,G-Z
     AL:    A-F
    RLE:    0x14,]
    RLO:    0x13,}
    PDF:    0x12,^
     EN:    0-5
     ES:    /,+,[hyphen]
     ET:    #,$,%
     AN:    6-9
     CS:    [comma],.,:
    NSM:    `
     BN:    0x0-0x8,0xe,0xf,0x18-0x1b,~
      B:    0xa,0xd,0x1c-0x1e,|
      S:    0x9,0xb,0x1f,_
     WS:    0xc,0x15,[space]
     ON:    !,",&,',(,),*,;,<,=,>,?,@,\,0x7f
****************************************/

#define odd(x) ((x) & 1)

// WS, LS and S are not explicitly needed except for L1. Therefore this
// table conflates ON, S, WS, and LS to N, all others unchanged
int NTypes[] = {
    N,      // ON,
    L,      // L,
    R,      // R,
    AN,     // AN,
    EN,     // EN,
    AL,     // AL
    NSM,    // NSMf
    CS,     // CS
    ES,     // ES
    ET,     // ET
    BN,     // BN
    N,      // S
    N,      // WS
    B,      // B
    RLO,    // RLO
    RLE,    // RLE
    LRO,    // LRO
    LRE,    // LRE
    PDF,    // PDF
    ON,     // LS
};

int ClassFromChN(SFUnichar ch) {
    return NTypes[getTypeOfChar(ch)];
}

int ClassFromChWS(SFUnichar ch) {
    return getTypeOfChar(ch);
}

// === HELPER FUNCTIONS ================================================

// reverse cch logical order
void reverseLogicalOrder(int *visOrder, int cch) {
    SFUnichar chTemp;
    int ich;
    
    for (ich = 0; ich < --cch; ich++) {
        chTemp = visOrder[ich];
        visOrder[ich] = visOrder[cch];
        visOrder[cch] = chTemp;
    }
}

// Set a run of cval values at locations all prior to, but not including
// iStart, to the new value nval.
void SetDeferredRun(int *pval, int cval, int iStart, int nval) {
    int i;
    
    for (i = iStart - 1; i >= iStart - cval; i--) {
        pval[i] = nval;
    }
}

// === ASSIGNING BIDI CLASSES ============================================

/*------------------------------------------------------------------------
    Function: classify

    Determines the character classes for all following
    passes of the algorithm

    Input: Text string
           Character count
           Whether to report types as WS, ON, S or as N (false)

    Output: Array of directional classes
------------------------------------------------------------------------*/
int classify(const SFUnichar *pszText, int *pcls, int cch, SFBool fWS) {
    int ich = 0;
    
    if (fWS) {
        for (; ich < cch; ich++) {
            pcls[ich] = ClassFromChWS(pszText[ich]);
        }
        
        return ich;
    }

    for (; ich < cch; ich++) {
        pcls[ich] = ClassFromChN(pszText[ich]);
    }
    
    return ich;
}

// === THE PARAGRAPH LEVEL ===============================================

/*------------------------------------------------------------------------
    Function: baseLevel

    Determines the base level

    Implements rule P2 of the Unicode Bidi Algorithm.

    Input: Array of directional classes
           Character count

    Note: Ignores explicit embeddings
------------------------------------------------------------------------*/
int baseLevel(const int *pcls, int cch) {
    int ich;

    for (ich = 0; ich < cch; ich++) {
        switch (pcls[ich]) {
            // strong left
            case L:
                return 1;
                break;
                
            // strong right
            case R:
            case AL:
                return 0;
                break;
        }
    }

    return 1;
}

//====== RESOLVE EXPLICIT ================================================

int GreaterEven(int i) {
    return odd(i) ? i + 1 : i + 2;
}

int GreaterOdd(int i) {
    return odd(i) ? i + 2 : i + 1;
}

int EmbeddingDirection(int level) {
    return odd(level) ? L : R;
}


/*------------------------------------------------------------------------
    Function: resolveExplicit

    Recursively resolves explicit embedding levels and overrides.
    Implements rules X1-X9, of the Unicode Bidirectional Algorithm.

    Input: Base embedding level and direction
           Character count

    Output: Array of embedding levels
            Caller must allocate (one level per input character)

    In/Out: Array of direction classes


    Note: The function uses two simple counters to keep track of
          matching explicit codes and PDF. Use the default argument for
          the outermost call. The nesting counter counts the recursion
          depth and not the embedding level.
------------------------------------------------------------------------*/

const int MAX_LEVEL = 61; // the real value

int resolveExplicit(int level, int dir, int *pcls, int *plevel, int cch, int nNest) {
    // always called with a valid nesting level
    // nesting levels are != embedding levels
    int nLastValid;
    int ich;
    
    nLastValid = nNest;

    // process the text
    for (ich = 0; ich < cch; ich++) {
        int cls = pcls[ich];
        switch (cls) {
            case LRO:
            case LRE:
                nNest++;
                
                if (GreaterEven(level) <= MAX_LEVEL) {
                    plevel[ich] = GreaterEven(level);
                    pcls[ich] = BN;
                    ich += resolveExplicit(plevel[ich], (cls == LRE ? N : L),
                                           &pcls[ich+1], &plevel[ich+1],
                                           cch - (ich+1), nNest);
                    nNest--;
                    continue;
                }
                
                cls = pcls[ich] = BN;
                break;
                
            case RLO:
            case RLE:
                nNest++;
                
                if (GreaterOdd(level) <= MAX_LEVEL) {
                    plevel[ich] = GreaterOdd(level);
                    pcls[ich] = BN;
                    ich += resolveExplicit(plevel[ich], (cls == RLE ? N : R),
                                           &pcls[ich+1], &plevel[ich+1],
                                           cch - (ich+1), nNest);
                    nNest--;
                    continue;
                }
                
                cls = pcls[ich] = BN;
                break;
                
            case PDF:
                cls = pcls[ich] = BN;
                
                if (nNest) {
                    if (nLastValid < nNest) {
                        nNest--;
                    } else {
                        cch = ich; // break the loop, but complete body
                    }
                }
                break;
        }
        
        // Apply the override
        if (dir != N) {
            cls = dir;
        }
        
        plevel[ich] = level;
        
        if (pcls[ich] != BN) {
            pcls[ich] = cls;
        }
    }
    
    return ich;
}

// === RESOLVE WEAK TYPES ================================================

enum bidi_state // possible states
{
    xa,         //    arabic letter
    xr,         //    right leter
    xl,         //    left letter
    
    ao,         //    arabic lett. foll by ON
    ro,         //    right lett. foll by ON
    lo,         //    left lett. foll by ON
    
    rt,         //    ET following R
    lt,         //    ET following L
    
    cn,         //    EN, AN following AL
    ra,         //    arabic number foll R
    re,         //    european number foll R
    la,         //    arabic number foll L
    le,         //    european number foll L
    
    ac,         //    CS following cn
    rc,         //    CS following ra
    rs,         //    CS,ES following re
    lc,         //    CS following la
    ls,         //    CS,ES following le
    
    ret,        //    ET following re
    let,        //    ET following le
};

int stateWeak[][10] = {
    //    N,  L,  R,  AN, EN, AL,NSM, CS, ES, ET,
    /*xa*/    ao, xl, xr, cn, cn, xa, xa, ao, ao, ao, /* arabic letter          */
    /*xr*/    ro, xl, xr, ra, re, xa, xr, ro, ro, rt, /* right letter           */
    /*xl*/    lo, xl, xr, la, le, xa, xl, lo, lo, lt, /* left letter              */
    
    /*ao*/    ao, xl, xr, cn, cn, xa, ao, ao, ao, ao, /* arabic lett. foll by ON*/
    /*ro*/    ro, xl, xr, ra, re, xa, ro, ro, ro, rt, /* right lett. foll by ON */
    /*lo*/    lo, xl, xr, la, le, xa, lo, lo, lo, lt, /* left lett. foll by ON  */
    
    /*rt*/    ro, xl, xr, ra, re, xa, rt, ro, ro, rt, /* ET following R          */
    /*lt*/    lo, xl, xr, la, le, xa, lt, lo, lo, lt, /* ET following L          */
    
    /*cn*/    ao, xl, xr, cn, cn, xa, cn, ac, ao, ao, /* EN, AN following AL      */
    /*ra*/    ro, xl, xr, ra, re, xa, ra, rc, ro, rt, /* arabic number foll R   */
    /*re*/    ro, xl, xr, ra, re, xa, re, rs, rs,ret, /* european number foll R */
    /*la*/    lo, xl, xr, la, le, xa, la, lc, lo, lt, /* arabic number foll L   */
    /*le*/    lo, xl, xr, la, le, xa, le, ls, ls,let, /* european number foll L */
    
    /*ac*/    ao, xl, xr, cn, cn, xa, ao, ao, ao, ao, /* CS following cn          */
    /*rc*/    ro, xl, xr, ra, re, xa, ro, ro, ro, rt, /* CS following ra          */
    /*rs*/    ro, xl, xr, ra, re, xa, ro, ro, ro, rt, /* CS,ES following re      */
    /*lc*/    lo, xl, xr, la, le, xa, lo, lo, lo, lt, /* CS following la          */
    /*ls*/    lo, xl, xr, la, le, xa, lo, lo, lo, lt, /* CS,ES following le      */
    
    /*ret*/ ro, xl, xr, ra, re, xa,ret, ro, ro,ret, /* ET following re          */
    /*let*/ lo, xl, xr, la, le, xa,let, lo, lo,let, /* ET following le          */
};

enum bidi_action // possible actions
{
    // primitives
    IX = 0x100,                     // increment
    XX = 0xF,                       // no-op
    
    // actions
    xxx = (XX << 4) + XX,           // no-op
    xIx = IX + xxx,                 // increment run
    xxN = (XX << 4) + ON,           // set current to N
    xxE = (XX << 4) + EN,           // set current to EN
    xxA = (XX << 4) + AN,           // set current to AN
    xxR = (XX << 4) + R,            // set current to R
    xxL = (XX << 4) + L,            // set current to L
    Nxx = (ON << 4) + 0xF,          // set run to neutral
    Axx = (AN << 4) + 0xF,          // set run to AN
    ExE = (EN << 4) + EN,           // set run to EN, set current to EN
    NIx = (ON << 4) + 0xF + IX,     // set run to N, increment
    NxN = (ON << 4) + ON,           // set run to N, set current to N
    NxR = (ON << 4) + R,            // set run to N, set current to R
    NxE = (ON << 4) + EN,           // set run to N, set current to EN
    
    AxA = (AN << 4) + AN,           // set run to AN, set current to AN
    NxL = (ON << 4) + L,            // set run to N, set current to L
    LxL = (L << 4) + L,             // set run to L, set current to L
};


int actionWeak[][10] = {
    //   N,.. L,   R,  AN,  EN,  AL, NSM,  CS,..ES,  ET,
    /*xa*/ xxx, xxx, xxx, xxx, xxA, xxR, xxR, xxN, xxN, xxN, /* arabic letter            */
    /*xr*/ xxx, xxx, xxx, xxx, xxE, xxR, xxR, xxN, xxN, xIx, /* right leter             */
    /*xl*/ xxx, xxx, xxx, xxx, xxL, xxR, xxL, xxN, xxN, xIx, /* left letter             */
    
    /*ao*/ xxx, xxx, xxx, xxx, xxA, xxR, xxN, xxN, xxN, xxN, /* arabic lett. foll by ON    */
    /*ro*/ xxx, xxx, xxx, xxx, xxE, xxR, xxN, xxN, xxN, xIx, /* right lett. foll by ON    */
    /*lo*/ xxx, xxx, xxx, xxx, xxL, xxR, xxN, xxN, xxN, xIx, /* left lett. foll by ON    */
    
    /*rt*/ Nxx, Nxx, Nxx, Nxx, ExE, NxR, xIx, NxN, NxN, xIx, /* ET following R            */
    /*lt*/ Nxx, Nxx, Nxx, Nxx, LxL, NxR, xIx, NxN, NxN, xIx, /* ET following L            */
    
    /*cn*/ xxx, xxx, xxx, xxx, xxA, xxR, xxA, xIx, xxN, xxN, /* EN, AN following  AL    */
    /*ra*/ xxx, xxx, xxx, xxx, xxE, xxR, xxA, xIx, xxN, xIx, /* arabic number foll R    */
    /*re*/ xxx, xxx, xxx, xxx, xxE, xxR, xxE, xIx, xIx, xxE, /* european number foll R    */
    /*la*/ xxx, xxx, xxx, xxx, xxL, xxR, xxA, xIx, xxN, xIx, /* arabic number foll L    */
    /*le*/ xxx, xxx, xxx, xxx, xxL, xxR, xxL, xIx, xIx, xxL, /* european number foll L    */
    
    /*ac*/ Nxx, Nxx, Nxx, Axx, AxA, NxR, NxN, NxN, NxN, NxN, /* CS following cn         */
    /*rc*/ Nxx, Nxx, Nxx, Axx, NxE, NxR, NxN, NxN, NxN, NIx, /* CS following ra         */
    /*rs*/ Nxx, Nxx, Nxx, Nxx, ExE, NxR, NxN, NxN, NxN, NIx, /* CS,ES following re        */
    /*lc*/ Nxx, Nxx, Nxx, Axx, NxL, NxR, NxN, NxN, NxN, NIx, /* CS following la         */
    /*ls*/ Nxx, Nxx, Nxx, Nxx, LxL, NxR, NxN, NxN, NxN, NIx, /* CS,ES following le        */
    
    /*ret*/xxx, xxx, xxx, xxx, xxE, xxR, xxE, xxN, xxN, xxE, /* ET following re            */
    /*let*/xxx, xxx, xxx, xxx, xxL, xxR, xxL, xxN, xxN, xxL, /* ET following le            */
};

int GetDeferredType(int action) {
    return (action >> 4) & 0xF;
}

int GetResolvedType(int action) {
    return action & 0xF;
}

/* Note on action table:

    States can be of two kinds:
     - Immediate Resolution State, where each input token
       is resolved as soon as it is seen. These states havve
       only single action codes (xxN) or the no-op (xxx)
       for static input tokens.
     - Deferred Resolution State, where input tokens either
       either extend the run (xIx) or resolve its Type (e.g. Nxx).

    Input classes are of three kinds
     - Static Input Token, where the class of the token remains
       unchanged on output (AN, L, N, R)
     - Replaced Input Token, where the class of the token is
       always replaced on output (AL, BN, NSM, CS, ES, ET)
     - Conditional Input Token, where the class of the token is
       changed on output in some but not all cases (EN)

     Where tokens are subject to change, a double action
     (e.g. NxA, or NxN) is _required_ after deferred states,
     resolving both the deferred state and changing the current token.

    These properties of the table are verified by assertions below.
    This code is needed only during debugging and maintenance
*/


/*------------------------------------------------------------------------
    Function: resolveWeak

    Resolves the directionality of numeric and other weak character types

    Implements rules X10 and W1-W6 of the Unicode Bidirectional Algorithm.

    Input: Array of embedding levels
           Character count

    In/Out: Array of directional classes

    Note: On input only these directional classes are expected
          AL, HL, R, L,  ON, BN, NSM, AN, EN, ES, ET, CS,
------------------------------------------------------------------------*/
void resolveWeak(int baselevel, int *pcls, int *plevel, int cch) {
    int state = odd(baselevel) ? xl : xr;
    int action;
    
    int cls;
    int clsRun;
    int clsNew;
    
    int level = baselevel;
    
    int cchRun = 0;
    
    int ich = 0;
    for (; ich < cch; ich++) {
        // ignore boundary neutrals
        if (pcls[ich] == BN) {
            // must flatten levels unless at a level change;
            plevel[ich] = level;
            
            // lookahead for level changes
            if (ich + 1 == cch && level != baselevel) {
                // have to fixup last BN before end of the loop, since
                // its fix-upped value will be needed below the assert
                pcls[ich] = EmbeddingDirection(level);
            } else if (ich + 1 < cch && level != plevel[ich+1] && pcls[ich+1] != BN) {
                // fixup LAST BN in front / after a level run to make
                // it act like the SOR/EOR in rule X10
                int newlevel = plevel[ich+1];
                if (level > newlevel) {
                    newlevel = level;
                }
                plevel[ich] = newlevel;
                
                // must match assigned level
                pcls[ich] = EmbeddingDirection(newlevel);
                level = plevel[ich+1];
            } else {
                // don't interrupt runs
                if (cchRun) {
                    cchRun++;
                }
                
                continue;
            }
        }
        
        cls = pcls[ich];
        
        action = actionWeak[state][cls];
        
        // resolve the directionality for deferred runs
        clsRun = GetDeferredType(action);
        if (clsRun != XX) {
            SetDeferredRun(pcls, cchRun, ich, clsRun);
            cchRun = 0;
        }
        
        // resolve the directionality class at the current location
        clsNew = GetResolvedType(action);
        if (clsNew != XX) {
            pcls[ich] = clsNew;
        }
        
        // increment a deferred run
        if (IX & action) {
            cchRun++;
        }
        
        state = stateWeak[state][cls];
    }
    
    // resolve any deferred runs
    // use the direction of the current level to emulate PDF
    cls = EmbeddingDirection(level);
    
    // resolve the directionality for deferred runs
    clsRun = GetDeferredType(actionWeak[state][cls]);
    if (clsRun != XX) {
        SetDeferredRun(pcls, cchRun, ich, clsRun);
    }
}

// === RESOLVE NEUTAL TYPES ==============================================

// action values
enum neutral_action
{
    // action to resolve previous input
    nL = L,         // resolve EN to L
    En = 3 << 4,    // resolve neutrals run to embedding level direction
    Rn = R << 4,    // resolve neutrals run to strong right
    Ln = L << 4,    // resolved neutrals run to strong left
    In = (1<<8),    // increment count of deferred neutrals
    LnL = (1<<4)+L, // set run and EN to L
};

int GetDeferredNeutrals(int action, int level) {
    action = (action >> 4) & 0xF;
    if (action == (En >> 4)) {
        return EmbeddingDirection(level);
    }
    
    return action;
}

int GetResolvedNeutrals(int action) {
    action = action & 0xF;
    if (action == In) {
        return 0;
    }

    return action;
}

// state values
enum neutral_state
{
    // new temporary class
    r,      // R and characters resolved to R
    l,      // L and characters resolved to L
    rn,     // N preceded by right
    ln,     // N preceded by left
    a,      // AN preceded by left (the abbrev 'la' is used up above)
    na,     // N preceeded by a
} ;


/*------------------------------------------------------------------------
    Notes:

    By rule W7, whenever a EN is 'dominated' by an L (including start of
    run with embedding direction = L) it is resolved to, and further treated
    as L.

    This leads to the need for 'a' and 'na' states.
------------------------------------------------------------------------*/

int actionNeutrals[][5] = {
    //    N,    L,    R, AN, EN, = cls
                            // state =
    In,  0,  0,  0,  0,     // r    right
    In,  0,  0,  0,  L,     // l    left
    
    In, En, Rn, Rn, Rn,     // rn   N preceded by right
    In, Ln, En, En, LnL,    // ln   N preceded by left
    
    In,  0,  0,  0,  L,     // a    AN preceded by left
    In, En, Rn, Rn, En,     // na   N  preceded by a
} ;

int stateNeutrals[][5] = {
    //     N, L,    R,    AN, EN = cls
                                    // state =
    rn, l,    r,    r,    r,        // r   right
    ln, l,    r,    a,    l,        // l   left
    
    rn, l,    r,    r,    r,        // rn  N preceded by right
    ln, l,    r,    a,    l,        // ln  N preceded by left
    
    na, l,    r,    a,    l,        // a  AN preceded by left
    na, l,    r,    a,    l,        // na  N preceded by la
} ;

/*------------------------------------------------------------------------
    Function: resolveNeutrals

    Resolves the directionality of neutral character types.

    Implements rules W7, N1 and N2 of the Unicode Bidi Algorithm.

    Input: Array of embedding levels
           Character count
           Baselevel

    In/Out: Array of directional classes

    Note: On input only these directional classes are expected
          R,  L,  N, AN, EN and BN

          W8 resolves a number of ENs to L
------------------------------------------------------------------------*/
void resolveNeutrals(int baselevel, int *pcls, const int *plevel, int cch) {
    // the state at the start of text depends on the base level
    int state = odd(baselevel) ? l : r;
    int action;
    
    int cls;
    int clsRun;
    int clsNew;
    
    int cchRun = 0;
    int level = baselevel;
    
    int ich = 0;
    for (; ich < cch; ich++) {
        // ignore boundary neutrals
        if (pcls[ich] == BN) {
            // include in the count for a deferred run
            if (cchRun) {
                cchRun++;
            }
            
            // skip any further processing
            continue;
        }
        
        cls = pcls[ich];
        
        action = actionNeutrals[state][cls];
        
        // resolve the directionality for deferred runs
        clsRun = GetDeferredNeutrals(action, level);
        if (clsRun != N) {
            SetDeferredRun(pcls, cchRun, ich, clsRun);
            cchRun = 0;
        }
        
        // resolve the directionality class at the current location
        clsNew = GetResolvedNeutrals(action);
        if (clsNew != N) {
            pcls[ich] = clsNew;
        }
        
        if (In & action) {
            cchRun++;
        }
        
        state = stateNeutrals[state][cls];
        level = plevel[ich];
    }
    
    // resolve any deferred runs
    cls = EmbeddingDirection(level);    // eor has type of current level
    
    // resolve the directionality for deferred runs
    clsRun = GetDeferredNeutrals(actionNeutrals[state][cls], level);
    if (clsRun != N) {
        SetDeferredRun(pcls, cchRun, ich, clsRun);
    }
}

// === RESOLVE IMPLLICIT =================================================

/*------------------------------------------------------------------------
    Function: resolveImplicit

    Recursively resolves implicit embedding levels.
    Implements rules I1 and I2 of the Unicode Bidirectional Algorithm.

    Input: Array of direction classes
           Character count
           Base level

    In/Out: Array of embedding levels

    Note: levels may exceed 15 on output.
          Accepted subset of direction classes
          R, L, AN, EN
------------------------------------------------------------------------*/
int addLevel[][4] = {
    // L, R, AN, EN = cls
                                            // level =
    /* even */    1,    0,    1,    1,      // EVEN
    /* odd  */    0,    1,    2,    2,      // ODD
    
    //Original
    /* even */    //0,    1,    2,    2,    // EVEN
    /* odd  */    //1,    0,    1,    1,    // ODD
};

void resolveImplicit(const int *pcls, int *plevel, int cch) {
    int ich;

    for (ich = 0; ich < cch; ich++) {
        // cannot resolve bn here, since some bn were resolved to strong
        // types in resolveWeak. To remove these we need the original
        // types, which are available again in resolveWhiteSpace
        if (pcls[ich] == BN) {
            continue;
        }
        
        plevel[ich] += addLevel[odd(plevel[ich])][pcls[ich] - 1];
    }
}

/*------------------------------------------------------------------------
    Function: resolveWhiteSpace

    Resolves levels for WS and S
    Implements rule L1 of the Unicode bidi Algorithm.

    Input:  Base embedding level
            Character count
            Array of direction classes (for one line of text)

    In/Out: Array of embedding levels (for one line of text)

    Note: this should be applied a line at a time. The default driver
          code supplied in this file assumes a single line of text; for
          a real implementation, cch and the initial pointer values
          would have to be adjusted.
------------------------------------------------------------------------*/
void resolveWhitespace(int baselevel, const int *pcls, int *plevel, int cch) {
    int cchrun = 0;
    int oldlevel = baselevel;
    
    int ich = 0;
    for (; ich < cch; ich++) {
        switch(pcls[ich]) {
            default:
                cchrun = 0; // any other character breaks the run
                break;
            case WS:
                cchrun++;
                break;
                
            case RLE:
            case LRE:
            case LRO:
            case RLO:
            case PDF:
            case BN:
                plevel[ich] = oldlevel;
                cchrun++;
                break;
                
            case S:
            case B:
                // reset levels for WS before eot
                SetDeferredRun(plevel, cchrun, ich, baselevel);
                cchrun = 0;
                plevel[ich] = baselevel;
                break;
        }

        oldlevel = plevel[ich];
    }

    // reset level before eot
    SetDeferredRun(plevel, cchrun, ich, baselevel);
}

/*------------------------------------------------------------------------
    Functions: reorder/reorderLevel

    Recursively reorders the display string
    "From the highest level down, reverse all characters at that level and
    higher, down to the lowest odd level"

    Implements rule L2 of the Unicode bidi Algorithm.

    Input: Array of embedding levels
           Character count
           Flag enabling reversal (set to false by initial caller)

    In/Out: Text to reorder

    Note: levels may exceed 15 resp. 61 on input.

    Rule L3 - reorder combining marks is not implemented here
    Rule L4 - glyph mirroring is implemented as a display option below

    Note: this should be applied a line at a time
-------------------------------------------------------------------------*/

int reorderLevel(int level, int *plevel, int *visOrder, int cch, SFBool fReverse) {
    int ich;
    
    // true as soon as first odd level encountered
    fReverse = fReverse || odd(level);
    
    for (ich = 0; ich < cch; ich++)
    {
        if (plevel[ich] < level)
            break;

        if (plevel[ich] > level) {
            ich += reorderLevel(level + 1, plevel + ich, visOrder + ich,
                                cch - ich, fReverse) - 1;
        }
    }
    
    if (fReverse) {
        reverseLogicalOrder(visOrder, ich);
    }

    return ich;
}

int reorder(int baselevel, int *plevel, int *visOrder, int cch) {
    int ich = 0;
    
    while (ich < cch) {
        ich += reorderLevel(baselevel, plevel + ich, visOrder + ich, cch - ich, SFFalse);
    }

    return ich;
}

static void saveMirroredChar(SFMirroredChar **current, int index, SFUnichar ch) {
    SFMirroredChar *next = malloc(sizeof(SFMirroredChar));
    next->index = index;
    next->ch = ch;
    next->next = NULL;
    
    (*current)->next = next;
    *current = next;
}

// === DISPLAY OPTIONS ================================================
/*-----------------------------------------------------------------------
    Function:    generateMirroredChars

    Crudely implements rule L4 of the Unicode Bidirectional Algorithm
    Demonstrate mirrored brackets, braces and parens


    Input:  Array of levels
            Count of characters

    In/Out: Array of characters (should be array of glyph ids)

    Note:
    A full implementation would need to substitute mirrored glyphs even
    for characters that are not paired (e.g. integral sign).
-----------------------------------------------------------------------*/
SFMirroredChar *generateMirroredChars(const SFUnichar *pszInput, const int *plevel, int cch) {
    SFMirroredChar *main = malloc(sizeof(SFMirroredChar));
    SFMirroredChar *current = main;
    
    SFUnichar tmp = 0;
    int ich = 0;

    main->index = -1;
    main->ch = -1;
    main->next = NULL;
    
    for (; ich < cch; ich ++) {
        if (odd(plevel[ich])) {
            continue;
        }
        
        if (getMirroredChar(pszInput[ich], &tmp)) {
            saveMirroredChar(&current, ich, tmp);
        }
    }
    
    return main;
}

// === BIDI INTERFACE FUNCTIONS ========================================
/*------------------------------------------------------------------------
    Function: generateBidiVisualOrder

    Implements the Line-by-Line phases of the Unicode Bidi Algorithm

    Input:     Count of characters
             Flag (Mirror output if true)
             Optional: Array of flags, true for last character on each line 

    Inp/Out: Input text
             Array of character directions
             Array of levels
 ------------------------------------------------------------------------*/
 
void generateBidiVisualOrder(int baselevel, const int *pclsLine, int *plevelLine, int *visOrder, int cchPara) {
    // resolve whitespace
    resolveWhitespace(baselevel, pclsLine, plevelLine, cchPara);
    
    //if (fMirror)
    //    mirror(pszLine, plevelLine, cchPara);
    
    // reorder each line in place
    reorder(baselevel, plevelLine, visOrder, cchPara);
}

void generateBidiTypesAndLevels(int baselevel, const SFUnichar *pszInput, int *types, int *levels, int cch) {
    // assign directional types
    classify(pszInput, types, cch, SFFalse);
    
    if (baselevel < 0) {
        baselevel = baseLevel(types, cch);
    }
    
    // resolve explicit
    resolveExplicit(baselevel, N, types, levels, cch, 0);
    
    // resolve weak
    resolveWeak(baselevel, types, levels, cch);
    
    // resolve neutrals
    resolveNeutrals(baselevel,types, levels, cch);
    
    // resolveImplicit
    resolveImplicit(types, levels, cch);
    
    // assign directional types again, but for WS, S this time
    classify(pszInput, types, cch, SFTrue);
}

