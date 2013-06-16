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
import android.graphics.Bitmap;
import android.graphics.Bitmap.Config;
import android.graphics.Canvas;
import android.graphics.Color;

import android.view.ViewGroup;
import android.util.AttributeSet;
import android.util.DisplayMetrics;

import android.widget.ImageView;

public class Label extends ViewGroup {

	private static final int PADDING = 3;

	private Text mText;

	private int mMeasuredHeight;
	
	private void init() {
		mText = new Text();
		mMeasuredHeight = 0;
	}
	
	public Label(Context context) {
		super(context);
		init();
	}

	public Label(Context context, AttributeSet attrs) {
		this(context, attrs, 0);
	}

	public Label(Context context, AttributeSet attrs, int defStyle) {
		super(context, attrs, defStyle);
		init();
		
		TypedArray values = context.getTheme().obtainStyledAttributes(attrs, R.styleable.Label, 0, 0);
		try {
			setTextColor(values.getInteger(R.styleable.Label_textColor, Color.BLACK));
			setTextAlignment(values.getInteger(R.styleable.Label_textAlignment, Text.TEXT_ALIGNMENT_RIGHT));
			setWritingDirection(values.getInteger(R.styleable.Label_writingDirection, Text.WRITING_DIRECTION_RTL));
			setText(values.getString(R.styleable.Label_text));
		} finally {
			values.recycle();
		}
	}
	
	@Override
    protected void onMeasure(final int widthMeasureSpec, final int heightMeasureSpec) {
		mMeasuredHeight = 0;
		
		int hSpec = heightMeasureSpec;
		if (MeasureSpec.getMode(hSpec) == MeasureSpec.UNSPECIFIED) {
			hSpec = MeasureSpec.makeMeasureSpec(mMeasuredHeight, MeasureSpec.EXACTLY);
		}

		super.onMeasure(widthMeasureSpec, hSpec);
	}

	@Override
	protected void onLayout(boolean changed, int left, int top, int right, int bottom) {

	}
	
	@Override
	protected void onDraw(Canvas canvas) {
	
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
			requestLayout();
		}
	}
	 
	public Font getFont() {
		return mText.getFont();
	}
	 
	public void setFont(Font font) {
		if (font != mText.getFont()) {
			mText.setFont(font);

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

			requestLayout();
		}
	}

	public int getTextAlignment() {
		return mText.getAlignment();
	}
	
	public void setTextAlignment(int align) {
		if (align != mText.getAlignment()) {
			mText.setAlignment(align);

			requestLayout();
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
