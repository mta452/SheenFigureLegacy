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

package com.sheenfigure.widget;

import java.util.ArrayList;

import com.sheenfigure.R;
import com.sheenfigure.graphics.Font;
import com.sheenfigure.graphics.Text;

import android.content.Context;
import android.content.res.TypedArray;
import android.graphics.Canvas;
import android.graphics.Color;
import android.graphics.Rect;

import android.view.View;
import android.util.AttributeSet;
import android.util.FloatMath;

public class TextView extends View {

	static class CancellableThread extends Thread {
		private boolean mCancelled = false;
		
		public CancellableThread(Runnable runnable) {
			super(runnable);
		}
		
		public void cancel() {
			mCancelled = true;
		}

		public boolean isCancelled() {
			return mCancelled;
		}
	}
	
	private static final int PADDING = 3;

	private Text mText;
	private int mMeasuredWidth;
	private int mMeasuredHeight;

	private float mLineHeight;
	private Rect mVisibleBounds;
	private ArrayList<Integer> mLinesMap;
	
	private void init() {
		mText = new Text();
		mMeasuredHeight = 0;

		mVisibleBounds = new Rect();
		mLinesMap = new ArrayList<Integer>();
	}
	
	public TextView(Context context) {
		super(context);
		init();
	}

	public TextView(Context context, AttributeSet attrs) {
		this(context, attrs, 0);
	}

	public TextView(Context context, AttributeSet attrs, int defStyle) {
		super(context, attrs, defStyle);
		init();
		
		TypedArray values = context.getTheme().obtainStyledAttributes(attrs, R.styleable.TextView, 0, 0);
		try {
			setTextColor(values.getInteger(R.styleable.TextView_textColor, Color.BLACK));
			setTextAlignment(values.getInteger(R.styleable.TextView_textAlignment, Text.TEXT_ALIGNMENT_RIGHT));
			setWritingDirection(values.getInteger(R.styleable.TextView_writingDirection, Text.WRITING_DIRECTION_RTL));
			setText(values.getString(R.styleable.TextView_text));
		} finally {
			values.recycle();
		}
	}

	@Override
    protected void onMeasure(final int widthMeasureSpec, final int heightMeasureSpec) {
		int width = MeasureSpec.getSize(widthMeasureSpec);
		if (width != mMeasuredWidth || mMeasuredHeight == 0) {
			mLinesMap.clear();
			
			float lineHeight = mText.getFont().getLeading();
            
            int index = 0;
            int countLines = 1;
            int totalLines = 0;
            
            float frameWidth = width - (PADDING * 2);

            while (index > -1) {
                mLinesMap.add(index);
                index = mText.getNextLineCharIndex(frameWidth, index, countLines);
                
                totalLines += countLines;
            }

            mMeasuredWidth = width;
            mMeasuredHeight = Math.round(totalLines * lineHeight);
		}

		int hSpec = heightMeasureSpec;
		if (MeasureSpec.getMode(hSpec) == MeasureSpec.UNSPECIFIED) {
			hSpec = MeasureSpec.makeMeasureSpec(mMeasuredHeight, MeasureSpec.EXACTLY);
		}

		super.onMeasure(widthMeasureSpec, hSpec);
	}
	
	@Override
	protected void onDraw(Canvas canvas) {
		super.onDraw(canvas);

		canvas.getClipBounds(mVisibleBounds);
		final int visibleLines = (int) (FloatMath.floor(mVisibleBounds.height() / mLineHeight) + 2);
		final int lineIndex = (int) FloatMath.floor(mVisibleBounds.top / mLineHeight);
		final int y = (int) (lineIndex * mLineHeight);
		final int startIndex = mLinesMap.get(lineIndex);
	    
		mText.showString(canvas, mMeasuredWidth - (PADDING * 2), PADDING, y, startIndex, visibleLines);
	}

	public String getText() {
		return mText.getString();
	}
	
	public void setText(String text) {
		if (text == null) {
			text = "";
		}
		 
		if (text != mText.getString()) {
			mText.setString(text);

			mMeasuredHeight = 0;
			invalidate();
		}
	}
	 
	public Font getFont() {
		return mText.getFont();
	}
	 
	public void setFont(Font font) {
		if (font != mText.getFont()) {
			mText.setFont(font);
			mLineHeight = font.getLeading();
			mMeasuredHeight = 0;
			requestLayout();
		}
	}
	
	public int getTextColor() {
		return mText.getColor();
	}
	
	public void setTextColor(int color) {
		if (color != mText.getColor()) {
			mText.setColor(color);

			invalidate();
		}
	}

	public int getTextAlignment() {
		return mText.getAlignment();
	}
	
	public void setTextAlignment(int align) {
		if (align != mText.getAlignment()) {
			mText.setAlignment(align);

			invalidate();
		}
	}

	public int getWritingDirection() {
		return mText.getAlignment();
	}
	
	public void setWritingDirection(int writingDirection) {
		if (writingDirection != mText.getWritingDirection()) {
			mText.setWritingDirection(writingDirection);
			mMeasuredHeight = 0;
			requestLayout();
		}
	}
}
