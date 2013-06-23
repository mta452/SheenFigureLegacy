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

/*****************JNI CLASS FOR USE IN ANDROID JAVA CLASSES*****************/

#include "SSJNI.h"

#ifdef SF_ANDROID

#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <math.h>

#include "SSFont.h"
#include "SSText.h"

#define SF_FONT_ERROR				"A font needs to be set before measuring/rendering text."

typedef struct SSReservedObjects {
	JNIEnv *_env;

	FT_Face _ftFace;
	uint32_t _rgbColor;

	jobject _canvas;
	jmethodID _draw_bitmap_mid;
} SSReservedObjects;

static jfieldID getRefPtrFieldID(JNIEnv *env, jobject obj) {
	jclass cls = (*env)->GetObjectClass(env, obj);
	jfieldID fid = (*env)->GetFieldID(env, cls, "mRefPtr", "I");

	return fid;
}

static SSTextClass *getRefPtr(JNIEnv *env, jobject obj) {
	jfieldID fid = getRefPtrFieldID(env, obj);
    if (fid) {
    	uintptr_t refPtr = (*env)->GetIntField(env, obj, fid);
    	return (SSTextClass *)refPtr;
    }

    return NULL;
}

static bool setRefPtr(JNIEnv *env, jobject obj, SSTextClass *text) {
	jfieldID fid = getRefPtrFieldID(env, obj);
	if (fid) {
		(*env)->SetIntField(env, obj, fid, (uintptr_t)text);
		return true;
	}

	return false;
}

static SFUnichar *getUnistr(JNIEnv *env, jstring str, int *length) {
	jclass java_string_class = (jclass)(*env)->FindClass(env, "java/lang/String");
	jmethodID charAt_id = (*env)->GetMethodID(env, java_string_class, "charAt", "(I)C");
	jmethodID length_id = (*env)->GetMethodID(env, java_string_class, "length", "()I");
	*length = (*env)->CallIntMethod(env, str, length_id);

    if (*length) {
        SFUnichar *unistr = malloc(*length * sizeof(SFUnichar));

        int i = 0;
        for (; i < *length; i++) {
            SFUnichar ch = (*env)->CallCharMethod(env, str, charAt_id, i);
            unistr[i] = ch;
        }
        
        (*env)->DeleteLocalRef(env, java_string_class);
        
        return unistr;
    }

	return NULL;
}

static void initialize(JNIEnv *env, jobject obj) {
	SSTextClass *text = getRefPtr(env, obj);
	if (text) {
		LOGW("Text is already initialized for object 0x%x.", (int)text);
		return;
	}

	text = malloc(sizeof(SSTextClass));
	text->_sfText = SFTextCreateWithString(NULL, 0, NULL);

	if (setRefPtr(env, obj, text)) {
		LOGI("Text has been initialized for object 0x%x.", (int)text);
	} else {
		LOGE("Text could not be initialized due to some internal error.");
		free(text);
	}
}

static void setString(JNIEnv *env, jobject obj, jstring str) {
	SSTextClass *text = getRefPtr(env, obj);

	int length = 0;
	SFUnichar *unistr = getUnistr(env, str, &length);
    SFTextSetString(text->_sfText, unistr, length);
}

static void setFont(JNIEnv *env, jobject obj, jint refPtr) {
	SSTextClass *text = getRefPtr(env, obj);
	SSFontClass *font = (SSFontClass *)((uintptr_t)refPtr);
	text->_sfFont = font->_sfFont;

	SFTextSetFont(text->_sfText, text->_sfFont);
}

static void setColor(JNIEnv *env, jobject obj, jint color) {
	SSTextClass *text = getRefPtr(env, obj);
	text->_rgbColor = color & 0xFFFFFF;
}

static void setAlignment(JNIEnv *env, jobject obj, jint alignment) {
	SFTextSetAlignment(getRefPtr(env, obj)->_sfText, alignment);
}

static void setWritingDirection(JNIEnv *env, jobject obj, jint writingDirection) {
	SFTextSetWritingDirection(getRefPtr(env, obj)->_sfText, writingDirection);
}

static int getNextLineCharIndex(JNIEnv *env, jobject obj, jfloat frameWidth, int startIndex, int linesCount) {
	SSTextClass *text = getRefPtr(env, obj);
	if (!text->_sfFont) {
		LOGW(SF_FONT_ERROR);
		return 0;
	}

	int tmpLines = linesCount;
	int nextIndex = SFTextGetNextLineCharIndex(text->_sfText, frameWidth, startIndex, &tmpLines);
	if (nextIndex == -1) {
		return (-tmpLines-1);
    }

	return nextIndex;
}

static int measureLines(JNIEnv *env, jobject obj, jfloat frameWidth) {
    return SFTextMeasureLines(getRefPtr(env, obj)->_sfText, frameWidth);
}

static float measureHeight(JNIEnv *env, jobject obj, jfloat frameWidth) {
    return SFTextMeasureHeight(getRefPtr(env, obj)->_sfText, frameWidth);
}

static void renderGlyph(SFGlyph glyph, SFFloat x, SFFloat y, void *resObj) {
	SSReservedObjects *resObjects = (SSReservedObjects *)resObj;

	FT_Error error;
	FT_Face face = resObjects->_ftFace;
	FT_GlyphSlot slot = face->glyph;

	error = FT_Load_Glyph(face, glyph, FT_LOAD_DEFAULT);
	if (error) {
		return;
	}

	error = FT_Render_Glyph(face->glyph, FT_RENDER_MODE_NORMAL);
	if (error) {
		return;
	}

	FT_Bitmap *bmp = &slot->bitmap;
	int length = bmp->width * bmp->rows;
	if (!length) {
	    return;
	}

	jintArray pixelsArray = (*resObjects->_env)->NewIntArray(resObjects->_env, length);
	jint *pixels = (*resObjects->_env)->GetIntArrayElements(resObjects->_env, pixelsArray, NULL);

	if (!pixelsArray) {
		LOGE("Error in creating pixels array.");
		return;
	}

	int i = 0;
	//Here we will treat grayscale pixels as alpha
	for (; i < length; i++) {
		pixels[i] = bmp->buffer[i] << 24 | resObjects->_rgbColor;
	}

	(*resObjects->_env)->CallVoidMethod(resObjects->_env, resObjects->_canvas, resObjects->_draw_bitmap_mid, pixelsArray, 0, bmp->width, x + slot->bitmap_left, y - slot->bitmap_top, bmp->width, bmp->rows, 1, NULL);
	(*resObjects->_env)->ReleaseIntArrayElements(resObjects->_env, pixelsArray, pixels, JNI_ABORT);
	(*resObjects->_env)->DeleteLocalRef(resObjects->_env, pixelsArray);
}

static int showString(JNIEnv *env, jobject obj, jobject canvas, jfloat frameWidth, jfloat x, jfloat y, int startIndex, int lines) {
	SSTextClass *text = getRefPtr(env, obj);
	if (!text->_sfFont) {
		LOGW(SF_FONT_ERROR);
		return;
	}

	SSReservedObjects resObj;
	resObj._env = env;
	resObj._canvas = canvas;

	FT_Face ftFace = SFFontGetFTFace(text->_sfFont);
	FT_Reference_Face(ftFace);

	resObj._ftFace = ftFace;
	resObj._rgbColor = text->_rgbColor;

	SFPoint point;
	point.x = x;
	point.y = y;

	jclass java_canvas_class = (jclass)(*env)->GetObjectClass(env, canvas);
	resObj._draw_bitmap_mid = (*env)->GetMethodID(env, java_canvas_class, "drawBitmap", "([IIIFFIIZLandroid/graphics/Paint;)V");

	int numLines = lines;
	int result = SFTextShowString(text->_sfText, frameWidth, point, startIndex, &numLines, &resObj, &renderGlyph);

	FT_Done_Face(ftFace);

	return numLines;
}

static void destroy(JNIEnv *env, jobject obj) {
	SSTextClass *text = getRefPtr(env, obj);
	SFTextRelease(text->_sfText);
	free(text);

	setRefPtr(env, obj, NULL);

	LOGI("Text has been destroyed for object 0x%x.", (int)text);
}

static JNINativeMethod sfTextMethods[] = {
    { "setNativeString", "(Ljava/lang/String;)V", (void *)setString },
	{ "setNativeFont", "(I)V", (void *)setFont },
	{ "setNativeColor", "(I)V", (void *)setColor },
    { "setNativeAlignment", "(I)V", (void *)setAlignment },
    { "setNativeWritingDirection", "(I)V", (void *)setWritingDirection },
	{ "getNextLineCharIndex", "(FII)I", (void *)getNextLineCharIndex },
	{ "measureLines", "(F)I", (void *)measureLines },
	{ "measureHeight", "(F)F", (void *)measureHeight },
	{ "showString", "(Landroid/graphics/Canvas;FFFII)I", (void *)showString },
	{ "destroy", "()V", (void *)destroy },
	{ "initialize", "()V", (void *)initialize }
};

int register_com_sheenfigure_graphics_Text(JNIEnv *env) {
	static const char *const strClassName = "com/sheenfigure/graphics/Text";

	jclass cls = (*env)->FindClass(env, strClassName);
	if(cls == NULL){
		LOGE("Could not find class \"%s\".", strClassName);
		return JNI_ERR;
	}

	if ((*env)->RegisterNatives(env, cls, sfTextMethods, sizeof(sfTextMethods) / sizeof(sfTextMethods[0])) != JNI_OK) {
		LOGE("Method registration failure.");
		return JNI_ERR;
	}

    return JNI_OK;
}

#endif

/***************************************************************************/
