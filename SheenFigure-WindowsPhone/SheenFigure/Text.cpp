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

#include "pch.h"
#include "Text.h"

#include <stdint.h>

using namespace SheenFigure;
using namespace SheenFigure::Graphics;

typedef struct GlyphDataDelegateReference {
	GlyphDataDelegate ^func;
	Object ^resObj;
} GlyphDataDelegateReference;

static SFUnichar *GetUnistr(String ^str, int *length) {
	*length = str->Length();

	const wchar_t *orgstr = str->Data();
	SFUnichar *unistr = (SFUnichar *)malloc(*length * sizeof(SFUnichar));
	for (int i = 0; i < *length; i++)
		unistr[i] = orgstr[i];

	return unistr;
}

static void CallDelegate(void *pixels, int width, int rows, float x, float y, void *resObj) {
	Array<int32>^ pixelsArray = ref new Array<int32>((int32 *)pixels, width * rows);
	GlyphDataDelegateReference *reference = (GlyphDataDelegateReference *)resObj;
	reference->func(pixelsArray, width, rows, x, y, reference->resObj);
}

Text::Text(void)
{
	refPtr = SFTextCreate(NULL, NULL, 0, 100);
}

void Text::SetViewArea(int width) {
	SFTextChangePageWidth(refPtr, (float)width - 4);
}

void Text::SetColor(Color color) {
	SFColor c = ((color.R << 16) | (color.G << 8)) | color.B;
	SFTextSetColor(refPtr, c);
}

void Text::SetTextAlignment(TextAlignment align) {
	SFTextSetAlignment(refPtr, (SFTextAlignment)align);
}

void Text::ChangeFont(Font ^font) {
	SFTextChangeFont(refPtr, font->GetRefPtr());
}

void Text::ChangeText(String ^str) {
	int length;
	SFUnichar *unistr = GetUnistr(str, &length);
	SFTextChangeString(refPtr, unistr, length);
}

int Text::RenderText(GlyphDataDelegate ^func, int initialIndex, int linesCount, int x, int y, Object ^resObj) {
	if (!SFTextGetFont(refPtr))
		throw (ref new Exception(0, "Font is not provided."));

	float singleLineHeight = SFFontGetLeading(refPtr);

	CGPoint pos;
	pos.x = (float)x;
	pos.y = SFFontGetAscender(SFTextGetFont(refPtr)) + y;
	SFTextSetInitialPosition(refPtr, pos);

	GlyphDataDelegateReference reference;
	reference.func = func;
	reference.resObj = resObj;

	SFTextSetReservedObject(refPtr, &reference);

	int numLines = linesCount;
	initialIndex = SFTextShowString(refPtr, CallDelegate, initialIndex, &numLines);

	return numLines;
}

int Text::GetCharIndexAfterLines(int initialIndex, int linesCount) {
	if (!SFTextGetFont(refPtr))
		throw (ref new Exception(0, "Font is not provided."));

	int tmpLines = 0;
	int startIndex = SFTextGetNextLineCharIndex(refPtr, linesCount, initialIndex, &tmpLines);

	if (startIndex == -1)
		return -tmpLines;

	if (startIndex > initialIndex)
		return startIndex;

	return ++initialIndex;
}

int Text::MeasureHeight() {
	if (!SFTextGetFont(refPtr))
		throw (ref new Exception(0, "Font is not provided."));

	int lines = 0;
	int startIndex = 0;

	while (startIndex > -1) {
		int tmpLines = 0;
		int tmpStartIndex = SFTextGetNextLineCharIndex(refPtr, 10, startIndex, &tmpLines);
		lines += tmpLines;

		if (tmpStartIndex > startIndex || tmpStartIndex == -1)
			startIndex = tmpStartIndex;
		else
			startIndex++;
	}

	int mHeight = (int)(lines * SFFontGetLeading(refPtr));
	return mHeight;
}

Text::~Text() {
	SFTextRelease(refPtr);
}
