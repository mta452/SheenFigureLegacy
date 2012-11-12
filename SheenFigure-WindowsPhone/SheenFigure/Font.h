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

using namespace Platform;

namespace SheenFigure
{
	namespace Graphics
	{
		public ref class Font sealed
		{
		private:
			SFFontRef refPtr;

			Font(SFFontRef ptr);
			~Font();

		internal:
			SFFontRef GetRefPtr();

		public:
			Font(String ^path, float size);
			Font ^MakeClone(float size);

			float GetSize();
			float GetSizeByEm();
	
			float GetAscender();
			float GetDescender();
			float GetLeading();
		};
	}
}
