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
#include "Font.h"

#include <Windows.h>
#include <stdlib.h>

using namespace SheenFigure::Graphics;

void Font::Initialize(String ^path, float size, SFFontRef refPtr)
{
	this->path = path;

	const wchar_t *uniPath = path->Data();
	int length = path->Length();

	char *asciiPath = (char *)malloc(sizeof(char) * (length + 1));
	asciiPath[length] = '\0';

	for (int i = 0; i < length; i++) {
		asciiPath[i] = (char)uniPath[i];
	}

	float scale = Windows::Graphics::Display::DisplayProperties::LogicalDpi / 96.0f;

	FT_Init_FreeType(&ftLib);
	FT_New_Face(ftLib, asciiPath, 0, &ftFace);
	FT_Set_Char_Size(ftFace, 0, (int)(size * scale * 64), 72, 72);

	if (refPtr) {
		sfFont = SFFontMakeCloneForFTFace(refPtr, ftFace, size);
	} else {
		sfFont = SFFontCreateWithFTFace(ftFace, size);
	}

	if (!sfFont) {
		throw ref new Exception(0, "Could not initialize font.");
	}
}

Font::Font(String ^path, float size)
{
	Initialize(path, size, 0);
}

Font::Font(String ^path, float size, SFFontRef refPtr)
{
	Initialize(path, size, refPtr);
}

Font ^Font::MakeClone(float size)
{
	return (ref new Font(path, size, sfFont));
}

SFFontRef Font::GetSFFont()
{
	return sfFont;
}

float Font::Size::get()
{
	return SFFontGetSize(sfFont);
}

float Font::SizeByEm::get()
{
	return SFFontGetSizeByEm(sfFont);
}
	
float Font::Ascender::get()
{
	return SFFontGetAscender(sfFont);
}

float Font::Descender::get()
{
	return SFFontGetDescender(sfFont);
}

float Font::Leading::get()
{
	return SFFontGetLeading(sfFont);
}

Font::~Font() {
	SFFontRelease(sfFont);

	FT_Done_Face(ftFace);
	FT_Done_FreeType(ftLib);
}
