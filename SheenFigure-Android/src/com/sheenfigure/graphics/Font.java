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

public class Font extends Base {

	int mRefPtr = 0;
	
	private String mPath;

	private Font(String path, float size, int refPtr) {
		mPath = path;
		initialize(path, size, refPtr);
	}
	
	public Font(String path, float size) {
		this(path, size, 0);
	}

	public Font makeClone(float size) {
		return (new Font(mPath, size, mRefPtr));
	}

	@Override
	public void finalize() throws Throwable {
		super.finalize();
		destroy();
	}
	
	private native void initialize(String path, float size, int refPtr);

	public native float getSize();
	public native float getSizeByEm();
	
	public native float getAscender();
	public native float getDescender();
	public native float getLeading();
	
	private native void destroy();
}
