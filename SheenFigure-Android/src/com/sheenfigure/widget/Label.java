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

class CachedPage {
	
	private int mStartIndex;
	private Bitmap mBitmap;
	private ImageView mBitmapView;
	
	public CachedPage(Context context, int startIndex, Bitmap bitmap) {
		mStartIndex = startIndex;
		mBitmap = bitmap;
		mBitmapView = new ImageView(context);
		mBitmapView.setImageBitmap(mBitmap);
	}
	
	public int getStartIndex() {
		return mStartIndex;
	}
	
	public void setBitmap(Bitmap bitmap) {
		mBitmap = bitmap;
		mBitmapView.setImageBitmap(bitmap);
	}
	
	public Bitmap getBitmap() {
		return mBitmap;
	}
	
	public ImageView getBitmapView() {
		return mBitmapView;
	}
}

public class Label extends ViewGroup {

	private static final int PADDING = 3;

	private Text mText;
	
	private ArrayList<CachedPage> mCachedPages;

	private int mMeasuredHeight;
	private int mLinesInPage;
	
	private boolean mDrawn;
	
	private void init() {
		mText = new Text();
		mCachedPages = new ArrayList<CachedPage>();
		
		mDrawn = false;
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
		mDrawn = false;
		mMeasuredHeight = 0;
		
		refreshDrawing(MeasureSpec.getSize(widthMeasureSpec));
		
		int hSpec = heightMeasureSpec;
		if (MeasureSpec.getMode(hSpec) == MeasureSpec.UNSPECIFIED) {
			hSpec = MeasureSpec.makeMeasureSpec(mMeasuredHeight, MeasureSpec.EXACTLY);
		}

		super.onMeasure(widthMeasureSpec, hSpec);
	}

	@Override
	protected void onLayout(boolean changed, int left, int top, int right, int bottom) {

	}
	
	private int getMaxAllowedPixelsInAPage() {
		DisplayMetrics displayMetrics = getResources().getDisplayMetrics();
		int maxPixels = displayMetrics.widthPixels * displayMetrics.heightPixels;
	    
	    return maxPixels;
	}
	
	private void clearCachedPages() {
		for (int i = mCachedPages.size() - 1; i >= 0; i--) {
			CachedPage page = mCachedPages.get(i);
			page.getBitmap().recycle();
			removeView(page.getBitmapView());
			
			mCachedPages.remove(i);
		}
		
		System.gc();
	}

	private int generateCachedPages(int pageWidth) {
		clearCachedPages();

		float lineHeight = mText.getFont().getLeading();
		
		float posX = PADDING;
		float posY = lineHeight / 2.0f;
		
		int imageWidth = pageWidth - getPaddingRight() - getPaddingLeft();
		int imageHeight = getMaxAllowedPixelsInAPage() / imageWidth;
		
		float frameWidth = imageWidth - (PADDING * 2);
		mLinesInPage = (int) Math.floor((imageHeight - lineHeight) / lineHeight);
		float pageHeight = lineHeight * mLinesInPage;

		Canvas canvas = new Canvas();

		int countLines = 0;
		int totalLines = 0;
		
		int index = 0;
		int prevIndex = 0;
		
		for (int i = 0; index > -1; i++) {
			prevIndex = index;
			countLines = mLinesInPage;
			
			index = mText.getNextLineCharIndex(frameWidth, index, countLines);
			if (index < 0) {
				countLines = -index - 1;
			}
			
			imageHeight = (int)(lineHeight * (countLines + 1));
			
			Bitmap bmp = Bitmap.createBitmap(imageWidth, imageHeight, Config.ARGB_4444);
			canvas.setBitmap(bmp);
			mText.showString(canvas, frameWidth, posX, posY, prevIndex, countLines);
			
			CachedPage page = new CachedPage(getContext(), prevIndex, bmp);
			mCachedPages.add(page);
			
			ImageView bmpView = page.getBitmapView();
			ViewGroup.LayoutParams params = new LayoutParams(imageWidth, imageHeight);
			addView(bmpView, params);
			
			int layoutY = getPaddingTop() + (int) ((i * pageHeight) - posY);
			bmpView.layout(getPaddingLeft(), layoutY, getPaddingLeft() + imageWidth, layoutY + imageHeight);
			
			totalLines += countLines;
		}

		mDrawn = true;
		mMeasuredHeight = Math.round((totalLines * lineHeight)) + getPaddingTop() + getPaddingBottom();
		
		return totalLines;
	}

	private void updateCachedPages() {
		Canvas canvas = new Canvas();
		
		float posX = PADDING;
		float posY = getFont().getLeading() / 2.0f;
		
		for (int i = 0; i < mCachedPages.size(); i++) {
			CachedPage prevPage = mCachedPages.get(i);
			
			int imageWidth = prevPage.getBitmap().getWidth();
			int imageHeight = prevPage.getBitmap().getHeight();
			
			int frameWidth = imageWidth - (PADDING * 2);
			
			prevPage.getBitmap().recycle();
			prevPage.setBitmap(null);
			
			Bitmap bmp = Bitmap.createBitmap(imageWidth, imageHeight, Config.ARGB_8888);
			canvas.setBitmap(bmp);
			mText.showString(canvas, frameWidth, posX, posY, prevPage.getStartIndex(), mLinesInPage);
			
			prevPage.setBitmap(bmp);
		}
		
		mDrawn = true;
	}
	
	private void refreshDrawing(int width) {
		final Font font = mText.getFont();
		final String text = mText.getString();
		
		width = width - getPaddingRight() - getPaddingLeft();
	    if (width > 0 && font != null && text != null && text.length() > 0) {
	        if (!mDrawn) {
	        	if (mMeasuredHeight == 0) {
	        		generateCachedPages(width);
	        	} else {
	        		updateCachedPages();
	        	}
	        }
	    } else {
	        clearCachedPages();
	        mDrawn = true;
	    }
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
			 
			mDrawn = false;
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
			 
			mDrawn = false;
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
			
			mDrawn = false;
			requestLayout();
		}
	}

	public int getTextAlignment() {
		return mText.getAlignment();
	}
	
	public void setTextAlignment(int align) {
		if (align != mText.getAlignment()) {
			mText.setAlignment(align);
			
			mDrawn = false;
			requestLayout();
		}
	}

	public int getWritingDirection() {
		return mText.getAlignment();
	}
	
	public void setWritingDirection(int writingDirection) {
		if (writingDirection != mText.getWritingDirection()) {
			mText.setWritingDirection(writingDirection);
			
			mDrawn = false;
			mMeasuredHeight = 0;
			requestLayout();
		}
	}
	 
	 @Override
	 protected void onDetachedFromWindow() {
		 super.onDetachedFromWindow();
		 
		 this.clearCachedPages();
	 }
}
