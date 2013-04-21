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

using namespace Platform;

namespace SheenFigure
{
	namespace Graphics
	{
		public ref class Font sealed
		{
		private:
			String ^path;

			SFFontRef sfFont;
			FT_Library ftLib;
			FT_Face ftFace;

			void Initialize(String ^path, float size, SFFontRef refPtr);

			Font(String ^path, float size, SFFontRef refPtr);
			~Font();

		internal:
			SFFontRef GetSFFont();

		public:
			Font(String ^path, float size);
			Font ^MakeClone(float size);

			property float Size {
				float get();
			};

			property float SizeByEm {
				float get();
			};

			property float Ascender {
				float get();
			};

			property float Descender {
				float get();
			};

			property float Leading {
				float get();
			};
		};
	}
}
