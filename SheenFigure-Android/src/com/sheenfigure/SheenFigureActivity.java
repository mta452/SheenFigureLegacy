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
	Label textView;
	
	private void copyFile(InputStream in, OutputStream out) throws IOException {
	    byte[] buffer = new byte[1024];
	    int read;
	    while((read = in.read(buffer)) != -1){
	      out.write(buffer, 0, read);
	    }
	}
	
	private String copyFile(String fileName) {
	    AssetManager assetManager = getAssets();

	    InputStream in = null;
        OutputStream out = null;
        
        String absoulutePath = "/data/data/" + this.getPackageName() + '/' + fileName;
        
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
        Font fnt = new Font(fontPath, dpToPixels(19));
        
        textView = (Label)findViewById(R.id.sf_label);
        textView.setFont(fnt);
        textView.setTextColor(0xFFFFFF);
        textView.setText("محمول (جمع: محمولات / mobile) کو ہاتفِ خلوی (cell phone) بھی کہا جاتا ہے اور یہ جدید طرزیات کی مدد سے تیار کی جانے والی ایک ایسی برقی اختراع (electronic device) ہوتی ہے کہ جسکے زریعے ہاتف (telephone) کا استعمال آزادانہ اور دوران حرکت و سفر کسی بھی جگہ بلا کسی قابل دید رابطے (یعنی تار وغیرہ کے بغیر) کیا جاسکتا ہے۔ آج کل جو جدید محمولات تیار کیے جارہے ہیں ان میں ناصرف یہ کہ ہاتف اور جال محیط عالم سے روابط (برقی خط اور رزمی بدیل (packet switching) وغیرہ) کی سہولیات میسر ہیں بلکہ اسکے ساتھ ساتھ ان میں تصاویر بھیجنے اور موصول کرنے کیلئے کثیرالوسیط پیغامی خدمت (multimedia messaging service) ، عکاسہ (camera) اور منظرہ (video) بنانے کی خصوصیات بھی موجود ہوتی ہیں۔");
    }
    
    @Override
    public void onConfigurationChanged(Configuration newConfig) {
		super.onConfigurationChanged(newConfig);
    }
}