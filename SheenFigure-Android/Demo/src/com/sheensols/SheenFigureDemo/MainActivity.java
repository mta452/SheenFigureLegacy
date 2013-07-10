package com.sheensols.SheenFigureDemo;

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
import com.sheenfigure.widget.TextView;

public class MainActivity extends Activity {

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
	
    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        
        String fontPath = copyFile("NafeesWeb.ttf");
        Font font = new Font(fontPath, dpToPixels(45.0f));
        
        TextView txtDesc = (TextView)findViewById(R.id.text_view_description);
        txtDesc.setFont(font);
    }

    @Override
    public void onConfigurationChanged(Configuration newConfig) {
		super.onConfigurationChanged(newConfig);
    }
}
