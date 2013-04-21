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

package com.sheenfigure;

import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;

import android.app.Activity;
import android.content.res.AssetManager;
import android.content.res.Configuration;
import android.os.Bundle;
import android.util.DisplayMetrics;
import android.util.Log;

import com.sheenfigure.graphics.Font;
import com.sheenfigure.widget.Label;

public class SheenFigureActivity extends Activity {
	private void copyFile(InputStream in, OutputStream out) throws IOException {
	    byte[] buffer = new byte[1024];
	    int read;
	    while ((read = in.read(buffer)) != -1) {
	    	out.write(buffer, 0, read);
	    }
	}
	
	private String copyFile(String fileName) {
	    AssetManager assetManager = getAssets();

	    InputStream in = null;
        OutputStream out = null;
        
        String absoulutePath = getFilesDir().getAbsolutePath() + '/' + fileName;
        
        File file = new File(absoulutePath);
        if (!file.exists()) {
        	 try {
                 in = assetManager.open(fileName);
                 out = new FileOutputStream(absoulutePath);
                 
                 copyFile(in, out);
                 
                 in.close();
                 in = null;
                 
                 out.flush();
                 out.close();
                 out = null;
             } catch(Exception e) {
                 Log.e("tag", e.getMessage());
             }
        }
        
        return absoulutePath;
	}
	
	public float dpToPixels(float dp) {
	    DisplayMetrics metrics = getResources().getDisplayMetrics();
	    float px = dp * (metrics.densityDpi / 160f);
	    return px;
	}
	
    /** Called when the activity is first created. */
    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.main);

        String fontPath = copyFile("NafeesWeb.ttf");
        Font font = new Font(fontPath, dpToPixels(20.0f));
        
        Label lblDesc = (Label)findViewById(R.id.sf_label_description);
        lblDesc.setFont(font);
    }
    
    @Override
    public void onConfigurationChanged(Configuration newConfig) {
		super.onConfigurationChanged(newConfig);
    }
}