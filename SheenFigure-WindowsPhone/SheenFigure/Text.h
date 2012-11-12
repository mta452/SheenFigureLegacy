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

#pragma once

#include "pch.h"
#include "Font.h"
#include "TextAlignment.h"

using namespace Platform;
using namespace Windows::UI;
using namespace Windows::Foundation::Collections;

using namespace SheenFigure;

namespace SheenFigure
{
	namespace Graphics
	{
		public delegate void GlyphDataDelegate(const Array<int32> ^pixels, int width, int height, float x, float y, Object ^resObj);

		public ref class Text sealed
		{
		private:
			SFTextRef refPtr;
			Color textColor;

			~Text(void);

		public:
			Text(void);

			void SetViewArea(int width);
			void SetColor(Color color);
			void SetTextAlignment(TextAlignment align);

			void ChangeFont(Font ^font);
			void ChangeText(String ^str);

			int GetCharIndexAfterLines(int initialIndex, int linesCount);
			int MeasureHeight();
			int RenderText(GlyphDataDelegate ^func, int initialIndex, int linesCount, int x, int y, Object ^resObj);
		};
	}
}
