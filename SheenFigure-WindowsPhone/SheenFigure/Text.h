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

#pragma once

#include "pch.h"
#include "Font.h"

using namespace Platform;
using namespace Windows::UI;
using namespace Windows::Foundation::Collections;

using namespace SheenFigure;

namespace SheenFigure
{
	namespace Graphics
	{
		public enum class TextAlignment
		{
			Right = SFTextAlignmentRight,
			Center = SFTextAlignmentCenter,
			Left = SFTextAlignmentLeft,
		};

		public enum class WritingDirection
		{
			Auto = SFWritingDirectionAuto,
			RTL = SFWritingDirectionRTL,
			LTR = SFWritingDirectionLTR,
		};

		public delegate void GlyphDataDelegate(const Array<int32> ^pixels, int width, int height, float x, float y, Object ^resObj);

		public ref class Text sealed
		{
		private:
			void Initialize(Platform::String ^string, SheenFigure::Graphics::Font ^font);

			SFTextRef sfText;
			String ^string;
			Font ^font;
			Color color;
			TextAlignment alignment;
			WritingDirection writingDirection;

			SFUnichar *unistr;
			uint32_t rgbColor;

			~Text(void);

		public:
			Text(void);
			Text(Platform::String ^string, SheenFigure::Graphics::Font ^font);

			property Platform::String ^String {
				Platform::String ^get();
				void set(Platform::String ^value);
			};

			property SheenFigure::Graphics::Font ^Font {
				SheenFigure::Graphics::Font ^get();
				void set(SheenFigure::Graphics::Font ^value);
			};

			property Windows::UI::Color Color {
				Windows::UI::Color get();
				void set(Windows::UI::Color value);
			};

			property SheenFigure::Graphics::TextAlignment Alignment {
				SheenFigure::Graphics::TextAlignment get();
				void set(SheenFigure::Graphics::TextAlignment value);
			};

			property SheenFigure::Graphics::WritingDirection WritingDirection {
				SheenFigure::Graphics::WritingDirection get();
				void set(SheenFigure::Graphics::WritingDirection value);
			};

			int GetNextLineCharIndex(float frameWidth, int startIndex, int linesCount);
			int MeasureLines(float frameWidth);
			float MeasureHeight(float frameWidth);

			int ShowString(GlyphDataDelegate ^delegate, float frameWidth, float x, float y, int startIndex, int lines, Object ^resObj);
		};
	}
}
