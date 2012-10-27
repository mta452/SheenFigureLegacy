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

/*------------------------------------------------------------------------
 Bidirectional Character Types
 
 as defined by the Unicode Bidirectional Algorithm Table 3-7.
 
 Note:
 
 The list of bidirectional character types here is not grouped the
 same way as the table 3-7, since the numberic values for the types
 are chosen to keep the state and action tables compact.
 ------------------------------------------------------------------------*/

#ifndef BIDI_CLASS_H
#define BIDI_CLASS_H

enum bidi_class
{
	// input types
    // ON MUST be zero, code relies on ON = N = 0
	ON = 0,  // Other Neutral
	L,		 // Left Letter
	R,		 // Right Letter
	AN, 	 // Arabic Number
	EN, 	 // European Number
	AL, 	 // Arabic Letter (Right-to-left)
	NSM,	 // Non-spacing Mark
	CS, 	 // Common Separator
	ES, 	 // European Separator
	ET, 	 // European Terminator (post/prefix e.g. $ and %)
    
             // resolved types
	BN, 	 // Boundary neutral (type of RLE etc after explicit levels)
    
             // input types,
	S,		 // Segment Separator (TAB)		// used only in L1
	WS, 	 // White space					// used only in L1
	B,		 // Paragraph Separator (aka as PS)
    
             // types for explicit controls
	RLO,	 // these are used only in X1-X9
	RLE,
	LRO,
	LRE,
	PDF,
    
	// resolved types, also resolved directions
	N = ON,  // alias, where ON, WS and S are treated the same
};

#endif
