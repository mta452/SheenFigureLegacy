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

import java.lang.ref.WeakReference;

import android.graphics.Bitmap;
import android.util.SparseArray;

public class Font extends Base {

	static class Glyph {
		private final int mTop;
		private final int mLeft;
		private final Bitmap mBitmap;
		
		public Glyph(int top, int left, Bitmap bitmap) {
			mTop = top;
			mLeft = left;
			mBitmap = bitmap;
		}
		
		public Glyph() {
			this(0, 0, null);
		}
		
		public boolean isEmpty() {
			return (mBitmap == null);
		}
		
		public int getTop() {
			return mTop;
		}
		
		public int getLeft() {
			return mLeft;
		}
		
		public int getWidth() {
			return mBitmap.getWidth();
		}
		
		public int getHeight() {
			return mBitmap.getHeight();
		}
		
		public Bitmap getBitmap() {
			return mBitmap;
		}
	}
	
	int mNativeFont = 0;
	
	private String mPath;
	private SparseArray<WeakReference<Glyph>> mCache;

	private Font(String path, float size, int refPtr) {
		mNativeFont = nativeInitialize(path, size, refPtr);
		mPath = path;
		mCache = new SparseArray<WeakReference<Glyph>>();
	}
	
	public Font(String path, float size) {
		this(path, size, 0);
	}

	public Font makeClone(float size) {
		return (new Font(mPath, size, mNativeFont));
	}

	public float getSize() {
		return nativeGetSize(mNativeFont);
	}
	
	public float getSizeByEm() {
		return nativeGetSizeByEm(mNativeFont);
	}
	
	public float getAscender() {
		return nativeGetAscender(mNativeFont);
	}
	
	public float getDescender() {
		return nativeGetDescender(mNativeFont);
	}
	
	public float getLeading() {
		return nativeGetLeading(mNativeFont);
	}

	Glyph getGlyph(int glyph) {
		Glyph cacheGlyph;
		synchronized (mCache) {
			int cacheIndex = mCache.indexOfKey(glyph);
			if (cacheIndex < 0) {
				cacheGlyph = nativeGenerateGlyph(mNativeFont, glyph);
				mCache.append(glyph, new WeakReference<Glyph>(cacheGlyph));
			} else {
				WeakReference<Glyph> weakGlyph = mCache.valueAt(cacheIndex);
				cacheGlyph = weakGlyph.get();
				if (cacheGlyph == null) {
					cacheGlyph = nativeGenerateGlyph(mNativeFont, glyph);
					mCache.setValueAt(cacheIndex, new WeakReference<Glyph>(cacheGlyph));
				}
			}
		}
		
		return cacheGlyph;
	}
	
	public void clearCache() {
		synchronized (mCache) {
			mCache.clear();
		}
	}
	
	@Override
	public void finalize() throws Throwable {
		super.finalize();
		
		if (mNativeFont != 0) {
			nativeDestroy(mNativeFont);
			mNativeFont = 0;
		}
	}
	
	private static native int nativeInitialize(String path, float size, int refPtr);
	private static native void nativeDestroy(int nativeFont);
	
	private static native Glyph nativeGenerateGlyph(int nativeFont, int glyph);

	private static native float nativeGetSize(int nativeFont);
	private static native float nativeGetSizeByEm(int nativeFont);
	
	private static native float nativeGetAscender(int nativeFont);
	private static native float nativeGetDescender(int nativeFont);
	private static native float nativeGetLeading(int nativeFont);
}
