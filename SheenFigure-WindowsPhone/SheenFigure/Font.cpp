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
#include "Font.h"

#include <Windows.h>
#include <stdlib.h>

using namespace SheenFigure::Graphics;

Font::Font(SFFontRef ptr)
{
	refPtr = ptr;
}

Font::Font(String ^path, float size)
{
	const wchar_t *uniPath = path->Data();
	int length = path->Length();

	char *asciiPath = (char *)malloc(sizeof(char) * (length + 1));
	asciiPath[length] = '\0';

	for (int i = 0; i < length; i++)
		asciiPath[i] = (char)uniPath[i];

	OutputDebugStringA(asciiPath);

	refPtr = SFFontCreateWithFileName(asciiPath, size);
	if (!refPtr)
		throw ref new Exception(0, "Could not initialize font.");
}

SFFontRef Font::GetRefPtr()
{
	return refPtr;
}

Font ^Font::MakeClone(float size)
{
	SFFontRef clonePtr = SFFontMakeClone(refPtr, size);

	return (ref new Font(clonePtr));
}

float Font::GetSize()
{
	return SFFontGetSize(refPtr);
}

float Font::GetSizeByEm()
{
	return SFFontGetSizeByEm(refPtr);
}
	
float Font::GetAscender()
{
	return SFFontGetAscender(refPtr);
}

float Font::GetDescender()
{
	return SFFontGetDescender(refPtr);
}

float Font::GetLeading()
{
	return SFFontGetLeading(refPtr);
}

Font::~Font() {
	SFFontRelease(refPtr);
}

