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

import android.util.Log;

public class Font extends Base {

	private int mRefPtr = 0;

	public Font(String path, float size) {
		initialize(path, size);
	}
	
	private Font(int refPtr) {
		mRefPtr = refPtr;
		Log.i("asdf", mRefPtr + "");
	}
	
	public Font makeCloneOfDifferentSize(float size) {
		int cloneRefPtr = makeClone(size);
		Font clone = new Font(cloneRefPtr);
		
		return clone;
	}
	
	protected long getRefPtr() {
		return mRefPtr;
	}

	@Override
	public void finalize() throws Throwable {
		super.finalize();

		destroy();
	}
	
	private native void initialize(String path, float size);
	private native int makeClone(float size);

	public native float getSize();
	public native float getSizeByEm();
	
	public native float getAscender();
	public native float getDescender();
	public native float getLeading();
	
	private native void destroy();
}
