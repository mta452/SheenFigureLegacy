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

package com.sheenfigure.graphics;

import android.graphics.Canvas;

public class Text extends Base {

	private int mRefPtr = 0;
	
	public Text() {
	    initialize();
	}
	
	public void changeFont(Font font) {
		changeFont(font.getRefPtr());
	}

	private native void initialize();

	public native void setViewArea(int width);
	public native void setColor(int color);
	public native void setTextAlignment(int align);
	
	private native void changeFont(long refPtr);
	public native void changeText(String str);
	
	public native int measureHeight();
	public native int getCharIndexAfterLines(int initialIndex, int linesCount);
	public native int renderText(Canvas canvas, int initialIndex, int linesCount, int x, int y);
	
	private native void destroy();
	
	@Override
	public void finalize() throws Throwable {
		super.finalize();

		destroy();
	}
}
