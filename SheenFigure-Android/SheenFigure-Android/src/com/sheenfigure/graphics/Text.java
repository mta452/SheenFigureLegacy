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
import android.graphics.Paint;

public class Text extends Base {
	
	private static class ReservedObjects {
		Font font;
		Canvas canvas;
		Paint paint;
	}

	int mNativeText = 0;
	
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
		mNativeText = nativeInitialize();
		
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
			nativeSetString(mNativeText, string);
			mString = string;
		}
	}
	
	public Font getFont() {
		return mFont;
	}
	
	public void setFont(Font font) {
		if (font != mFont) {
			nativeSetFont(mNativeText, font.mNativeFont);
			mFont = font;
		}
	}
	
	public int getColor() {
		return mColor;
	}
	
	public void setColor(int color) {
		if (color != mColor) {
			mColor = color;
		}
	}
	
	public int getAlignment() {
		return mAlignment;
	}
	
	public void setAlignment(int alignment) {
		if (alignment != mAlignment) {
			nativeSetAlignment(mNativeText, alignment);
			mAlignment = alignment;
		}
	}
	
	public int getWritingDirection() {
		return mWritingDirection;
	}
	
	public void setWritingDirection(int writingDirection) {
		if (writingDirection != mWritingDirection) {
			nativeSetWritingDirection(mNativeText, writingDirection);
			mWritingDirection = writingDirection;
		}
	}
	
	public int getNextLineCharIndex(float frameWidth, int startIndex, int linesCount) {
		return nativeGetNextLineCharIndex(mNativeText, frameWidth, startIndex, linesCount);
	}
	
	public int measureLines(float frameWidth) {
		return nativeMeasureLines(mNativeText, frameWidth);
	}
	
	public float measureHeight(float frameWidth) {
		return nativeMeasureHeight(mNativeText, frameWidth);
	}
	
	public int showString(Canvas canvas, float frameWidth, float x, float y, int startIndex, int lines) {
		ReservedObjects resObj = new ReservedObjects();
		resObj.font = mFont;
		resObj.canvas = canvas;
		resObj.paint = new Paint();
		resObj.paint.setColor(mColor);
		return nativeShowString(mNativeText, frameWidth, x, y, startIndex, lines, resObj);
	}
	
	@Override
	public void finalize() throws Throwable {
		super.finalize();
		
		if (mNativeText != 0) {
			nativeDestroy(mNativeText);
			mNativeText = 0;
		}
	}
	
	private static void renderGlyph(int glyph, float x, float y, Object resObj) {
		ReservedObjects resObjects = (ReservedObjects) resObj;
		Font.Glyph g = resObjects.font.getGlyph(glyph);
		if (!g.isEmpty()) {
			resObjects.canvas.drawBitmap(g.getBitmap(), x + g.getLeft(), y - g.getTop(), resObjects.paint);
		}
	}
	
	private static native int nativeInitialize();
	private static native void nativeDestroy(int nativeText);
	
	private static native void nativeSetString(int nativeText, String str);
	private static native void nativeSetFont(int nativeText, int nativeFont);
	private static native void nativeSetAlignment(int nativeText, int align);
	private static native void nativeSetWritingDirection(int nativeText, int writingDirection);
	
	private static native int nativeGetNextLineCharIndex(int nativeText, float frameWidth, int startIndex, int linesCount);
	private static native int nativeMeasureLines(int nativeText, float frameWidth);
	private static native float nativeMeasureHeight(int nativeText, float frameWidth);
	
	private static native int nativeShowString(int nativeText, float frameWidth, float x, float y, int startIndex, int lines, Object resObj);
}
