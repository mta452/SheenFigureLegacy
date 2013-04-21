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

#include "pch.h"
#include "Text.h"

#include <stdint.h>

using namespace SheenFigure;
using namespace SheenFigure::Graphics;

typedef struct SSReservedObjects {
	GlyphDataDelegate ^_func;
	Object ^_resObj;

	FT_Face _ftFace;
	uint32_t _rgbColor;
} SSReservedObjects;

static SFUnichar *GetUnistr(String ^str, int *length) {
	*length = str->Length();

	if (*length)
	{
		const wchar_t *orgstr = str->Data();
		SFUnichar *unistr = (SFUnichar *)malloc(*length * sizeof(SFUnichar));
		for (int i = 0; i < *length; i++)
			unistr[i] = orgstr[i];

		return unistr;
	}

	return NULL;
}

void Text::Initialize(Platform::String ^string, SheenFigure::Graphics::Font ^font)
{
	sfText = SFTextCreateWithString(NULL, 0, NULL);

	Windows::UI::Color color;
	color.R = color.G = color.B = 0;

	Font = font;
	Color = color;
	Alignment = SheenFigure::Graphics::TextAlignment::Right;
	WritingDirection = SheenFigure::Graphics::WritingDirection::RTL;
	String = string;
}

Text::Text()
{
	Initialize(nullptr, nullptr);
}

Text::Text(Platform::String ^string, SheenFigure::Graphics::Font ^font)
{
	Initialize(string, font);
}

Platform::String ^Text::String::get()
{
	return string;
}

void Text::String::set(Platform::String ^value)
{
	if (!value)
		value = "";

	string = value;

	free(unistr);
	unistr = NULL;

	int length;
	unistr = GetUnistr(string, &length);
	SFTextSetString(sfText, unistr, length);
}

SheenFigure::Graphics::Font ^Text::Font::get()
{
	return font;
}

void Text::Font::set(SheenFigure::Graphics::Font ^value)
{
	if (value != font)
	{
		font = value;
		SFTextSetFont(sfText, font->GetSFFont());
	}
}

Windows::UI::Color Text::Color::get()
{
	return color;
}

void Text::Color::set(Windows::UI::Color value)
{
	color = value;
	rgbColor = ((color.R << 16) | (color.G << 8)) | color.B;
}

SheenFigure::Graphics::TextAlignment Text::Alignment::get()
{
	return alignment;
}

void Text::Alignment::set(SheenFigure::Graphics::TextAlignment value)
{
	if (value != alignment)
	{
		alignment = value;
		SFTextSetAlignment(sfText, (SFTextAlignment)alignment);
	}
}

SheenFigure::Graphics::WritingDirection Text::WritingDirection::get()
{
	return writingDirection;
}

void Text::WritingDirection::set(SheenFigure::Graphics::WritingDirection value)
{
	if (value != writingDirection)
	{
		writingDirection = value;
		SFTextSetWritingDirection(sfText, (SFWritingDirection)writingDirection);
	}
}

int Text::GetNextLineCharIndex(float frameWidth, int startIndex, int linesCount) {
	if (!font)
		throw (ref new Exception(0, "Font is not provided."));

	int tmpLines = linesCount;
	int nextIndex = SFTextGetNextLineCharIndex(sfText, frameWidth, startIndex, &tmpLines);
	if (nextIndex == -1) {
		return (-tmpLines-1);
    }

	return nextIndex;
}

int Text::MeasureLines(float frameWidth)
{
	return SFTextMeasureLines(sfText, frameWidth);
}

float Text::MeasureHeight(float frameWidth)
{
	return SFTextMeasureHeight(sfText, frameWidth);
}

static void renderGlyph(SFTextRef sfText, SFGlyph glyph, SFFloat x, SFFloat y, void *resObj) {
	SSReservedObjects *resObjects = (SSReservedObjects *)resObj;

	FT_Error error;
	FT_Face face = resObjects->_ftFace;
	FT_GlyphSlot slot = face->glyph;

	error = FT_Load_Glyph(face, glyph, FT_LOAD_DEFAULT);
	if (error) {
		return;
	}

	error = FT_Render_Glyph(face->glyph, FT_RENDER_MODE_NORMAL);
	if (error) {
		return;
	}

	FT_Bitmap *bmp = &slot->bitmap;
	int length = bmp->width * bmp->rows;
	if (!length) {
	    return;
	}

	Array<int32>^ pixels = ref new Array<int32>(length);

	//Here we will treat grayscale pixels as alpha
	for (int i = 0; i < length; i++)
		pixels[i] = (bmp->buffer[i] << 24) | resObjects->_rgbColor;

	float scale = Windows::Graphics::Display::DisplayProperties::LogicalDpi / 96.0f;
	resObjects->_func(pixels, bmp->width, bmp->rows, (x * scale) + slot->bitmap_left, (y * scale) - slot->bitmap_top, resObjects->_resObj);
}

int Text::ShowString(GlyphDataDelegate ^func, float frameWidth, float x, float y, int startIndex, int lines, Object ^resObj) {
	SSReservedObjects resObjects;
	resObjects._func = func;
	resObjects._resObj = resObj;

	FT_Face ftFace = SFFontGetFTFace(font->GetSFFont());
	FT_Reference_Face(ftFace);

	resObjects._ftFace = ftFace;
	resObjects._rgbColor = rgbColor;

	SFPoint pos;
	pos.x = x;
	pos.y = y;

	int numLines = lines;
	int result = SFTextShowString(sfText, frameWidth, pos, startIndex, &numLines, &resObjects, &renderGlyph);

	FT_Done_Face(ftFace);

	return numLines;
}

Text::~Text()
{
	SFTextRelease(sfText);
	free(unistr);
}