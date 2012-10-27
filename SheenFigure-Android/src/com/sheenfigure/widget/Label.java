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

package com.sheenfigure.widget;

import java.util.ArrayList;

import com.sheenfigure.graphics.Font;
import com.sheenfigure.graphics.Text;

import android.content.Context;
import android.graphics.Bitmap;
import android.graphics.Bitmap.Config;
import android.graphics.Canvas;
import android.graphics.Color;
import android.graphics.Rect;

import android.view.View;
import android.util.AttributeSet;

import android.widget.TableLayout.LayoutParams;

class CachedPage {
	
	private int mStartIndex;
	private Bitmap mBitmap;
	
	public CachedPage(int startIndex, Bitmap bitmap) {
		mStartIndex = startIndex;
		mBitmap = bitmap;
	}
	
	public int getStartIndex() {
		return mStartIndex;
	}
	
	public void setCachedBitmap(Bitmap bitmap) {
		mBitmap = bitmap;
	}
	
	public Bitmap getBitmap() {
		return mBitmap;
	}
}

public class Label extends View {

	public static final int TEXT_ALIGNMENT_RIGHT = 0;
	public static final int TEXT_ALIGNMENT_CENTER = 1;
	public static final int TEXT_ALIGNMENT_LEFT = 2;
	
	private static final int MIN_WIDTH = 100;
	private static final int MIN_HEIGHT = 50;
	private static final int DEFAULT_COLOR = Color.WHITE;
	
	private static final int MAX_ALLOWED_PIXELS_IN_A_PAGE = 320 * 480;
	private static final int PADDING = 3;
	
	private Font mFont;
	private Text mSheenFigure;
	
	private ArrayList<CachedPage> mCachedPages;
	
	private boolean mNeedsRedraw;
	private int mMeasuredHeight;
	private int mPageHeight;
	private int mLinesInPage;

	private String mText;
	private int mTextColor;
	private int mAlign;
	
	private void init() {
		setMinimumWidth(MIN_WIDTH);
		setMinimumHeight(MIN_HEIGHT);

		mSheenFigure = new Text();
		
		mCachedPages = new ArrayList<CachedPage>();
		
		mNeedsRedraw = true;
		mMeasuredHeight = 0;
		
		mText = "";
		mTextColor = DEFAULT_COLOR;
		mAlign = TEXT_ALIGNMENT_RIGHT;
	}
	
	public Label(Context context) {
		super(context);
		
		init();
	}

	public Label(Context context, AttributeSet attrs) {
		super(context, attrs);

		init();
	}

	public Label(Context context, AttributeSet attrs, int defStyle) {
		super(context, attrs, defStyle);

		init();
	}
	
	private int calculatePageHeight(int width) {
		return MAX_ALLOWED_PIXELS_IN_A_PAGE / width;
	}
	
	private void clearCachedPages() {
		for (int i = mCachedPages.size() - 1; i >= 0; i--) {
			mCachedPages.get(i).getBitmap().recycle();
			mCachedPages.remove(i);
		}
		
		System.gc();
	}
	
	private int createCachedPages(int width) {
		clearCachedPages();

		float lineHeight = mFont.getLeading();
		mPageHeight = this.calculatePageHeight(width);
		mLinesInPage = (int)Math.floor((mPageHeight - lineHeight) / lineHeight);

		Canvas c = new Canvas();

		int totalLines = 0;
		
		int index = 0;
		while (index > -1) {
			int prevIndex = index;
			index = mSheenFigure.getCharIndexAfterLines(index, mLinesInPage);

			int measuredLines;
			int pageHeight;
			
			if (index < 0) {
				measuredLines = -index;
				pageHeight = (int)(lineHeight * (measuredLines + 1));
			} else {
				measuredLines = mLinesInPage;
				pageHeight = mPageHeight;
			}
			
			Bitmap bmp = Bitmap.createBitmap(width, pageHeight, Config.ARGB_8888);
			c.setBitmap(bmp);
			mSheenFigure.renderText(c, prevIndex, mLinesInPage, 0, (int)lineHeight);
			
			CachedPage page = new CachedPage(prevIndex, bmp);
			mCachedPages.add(page);
			
			totalLines += measuredLines;
		}
		
		return totalLines;
	}

	private void refreshDrawing() {
		if (mNeedsRedraw) {
			int width = this.getWidth() - (PADDING * 2);

			if (width < MIN_WIDTH)
				return;

			if (mMeasuredHeight == 0) {
				mSheenFigure.setViewArea(width);
				int lines = this.createCachedPages(width);
				mMeasuredHeight = Math.round((lines * mFont.getLeading()));

				if (mMeasuredHeight < MIN_HEIGHT)
					mMeasuredHeight = MIN_HEIGHT;

				this.setLayoutParams(new LayoutParams(width, mMeasuredHeight + PADDING));
			}

			mNeedsRedraw = false;
		}
	}
	
	@Override
	protected void onSizeChanged(int w, int h, int oldw, int oldh) {
        if (w != oldw) {
        	mNeedsRedraw = true;
        	mMeasuredHeight = 0;
        }
    }
	
	@Override
	protected void onDraw(Canvas canvas) {
		super.onDraw(canvas);
		
		this.refreshDrawing();
		
		Rect rect = new Rect();
		this.getHitRect(rect);
		this.getLocalVisibleRect(rect);

		int lineHeight = (int)mFont.getLeading();
		int correctHeight = lineHeight * mLinesInPage;
		
		int startPage = (int)Math.floor((float)rect.top / (float)correctHeight);
		int endPage = (int)Math.floor((float)rect.bottom / (float)correctHeight);

		int padding = PADDING;
		for (; startPage <= endPage; startPage++) {
			if (mCachedPages.size() > startPage && mCachedPages.get(startPage).getBitmap() != null)
				canvas.drawBitmap(mCachedPages.get(startPage).getBitmap(), PADDING, (startPage * correctHeight) - lineHeight + padding, null);
			
			padding = 0;
		}
	 }

	 public void setText(String text) {
		 if (text == null)
			 text = "";
		 
		 mText = text;
		 mSheenFigure.changeText(mText);
		 mNeedsRedraw = true;
		 mMeasuredHeight = 0;

		 this.invalidate();
	 }
	 
	 public String getText() {
		 return mText;
	 }
	 
	 public void setFont(Font font) {
		 mFont = font;
		 mSheenFigure.changeFont(mFont);
		 mNeedsRedraw = true;
		 mMeasuredHeight = 0;

		 this.invalidate();
	 }
	 
	 public void setTextColor(int color) {
		 mTextColor = color;
		 mSheenFigure.setColor(mTextColor);
		 mNeedsRedraw = true;

		 this.invalidate();
	 }
	 
	 public int getTextColor() {
		 return mTextColor;
	 }

	 public void setTextAlignment(int align) {
		 if (mAlign == align)
			 return;
		 
		 mAlign = align;
		 mSheenFigure.setTextAlignment(mAlign);
		 mNeedsRedraw = true;
		 
		 this.invalidate();
	 }
	 
	 public int getTextAlignment() {
		 return mTextColor;
	 }
	 
	 @Override
	 protected void onDetachedFromWindow() {
		 super.onDetachedFromWindow();

		 try {
			 mSheenFigure.finalize(); 
		 } catch (Throwable e) {
			e.printStackTrace();
		}
		 
		 this.clearCachedPages();
	 }
}
