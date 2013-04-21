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

package com.sheenfigure.graphics;

import android.graphics.Canvas;
import android.graphics.Color;

public class Text extends Base {

	int mRefPtr = 0;
	
	public static final int TEXT_ALIGNMENT_RIGHT = 0;
	public static final int TEXT_ALIGNMENT_CENTER = 1;
	public static final int TEXT_ALIGNMENT_LEFT = 2;
	
	public static final int WRITING_DIRECTION_AUTO = 0;
	public static final int WRITING_DIRECTION_RTL = 1;
	public static final int WRITING_DIRECTION_LTR = 2;

	private String mString;
	private Font mFont;
	private int mColor;
	private int mAlignment;
	private int mWritingDirection;
	
	public Text() {
	    this(null, null);
	}
	
	public Text(String string, Font font) {
		initialize();
		
		setFont(font);
		setColor(Color.BLACK);
		setAlignment(TEXT_ALIGNMENT_RIGHT);
		setWritingDirection(WRITING_DIRECTION_RTL);
		setString(string);
	}

	public String getString() {
		return mString;
	}
	
	public void setString(String string) {
		if (string != mString) {
			mString = string;
			setNativeString(mString);
		}
	}
	
	public Font getFont() {
		return mFont;
	}
	
	public void setFont(Font font) {
		if (font != mFont) {
			mFont = font;
			setNativeFont(mFont.mRefPtr);
		}
	}
	
	public int getColor() {
		return mColor;
	}
	
	public void setColor(int color) {
		if (color != mColor) {
			mColor = color;
			setNativeColor(mColor);
		}
	}
	
	public int getAlignment() {
		return mAlignment;
	}
	
	public void setAlignment(int alignment) {
		if (alignment != mAlignment) {
			mAlignment = alignment;
			setNativeAlignment(mAlignment);
		}
	}
	
	public int getWritingDirection() {
		return mWritingDirection;
	}
	
	public void setWritingDirection(int writingDirection) {
		if (writingDirection != mWritingDirection) {
			mWritingDirection = writingDirection;
			setNativeWritingDirection(mWritingDirection);
		}
	}
	
	private native void initialize();
	
	private native void setNativeString(String str);
	private native void setNativeFont(int refPtr);
	private native void setNativeColor(int color);
	private native void setNativeAlignment(int align);
	private native void setNativeWritingDirection(int writingDirection);
	
	public native int getNextLineCharIndex(float frameWidth, int startIndex, int linesCount);
	public native int measureLines(float frameWidth);
	public native float measureHeight(float frameWidth);
	
	public native int showString(Canvas canvas, float frameWidth, float x, float y, int startIndex, int lines);
	
	private native void destroy();
	
	@Override
	public void finalize() throws Throwable {
		super.finalize();
		destroy();
	}
}
