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

/*****************JNI CLASS FOR USE IN ANDROID JAVA CLASSES*****************/

#include "SFConfig.h"

#ifdef SF_ANDROID

#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <math.h>

#include "SFTypes.h"
#include "SFFont.h"
#include "SFText.h"
#include "SheenFigure.h"

#define SF_UNABLE_INIT_ERROR		"Sheen Figure could not be initialized due to some internal error."
#define SF_INITIALIZATION_ERROR		"Sheen Figure has not been initialized."
#define SF_COLOR_CHANGE_ERROR		"Unable to change color. " SF_INITIALIZATION_ERROR
#define SF_TEXT_CHANGE_ERROR		"Unable to change text. " SF_INITIALIZATION_ERROR
#define SF_RENDER_TEXT_ERROR		"Unable to render text. " SF_INITIALIZATION_ERROR
#define SF_TEXT_ALIGNMENT_ERROR		"Unable to set text alignment. " SF_INITIALIZATION_ERROR
#define SF_FONT_CHANGE_ERROR		"Unable to change font." SF_INITIALIZATION_ERROR
#define SF_MEASURE_HEIGHT_ERROR		"Unable to measure height. " SF_INITIALIZATION_ERROR
#define SF_SET_VIEW_AREA_ERROR		"Unable to set view area. " SF_INITIALIZATION_ERROR
#define SF_FONT_ERROR				"A font needs to be set before measuring/rendering text."

typedef struct SFPropertiesOfJavaClass {
	SFFontRef _sfFont;
	SFTextRef _sfText;

	JNIEnv *_env;
	jobject _canvas;
	jmethodID _draw_bitmap_mid;
} SFPropertiesOfJavaClass;

static jfieldID getFieldID(JNIEnv *env, jobject obj) {
	jclass cls = (*env)->GetObjectClass(env, obj);
	jfieldID fid = (*env)->GetFieldID(env, cls, "mRefPtr", "I");

	return fid;
}

static SFPropertiesOfJavaClass *getClassProperties(JNIEnv *env, jobject obj) {
	jfieldID fid = getFieldID(env, obj);
    if (fid == NULL)
    	return NULL;

    uintptr_t refPtr = (*env)->GetIntField(env, obj, fid);
    return (SFPropertiesOfJavaClass *)refPtr;
}

static bool setClassProperties(JNIEnv *env, jobject obj, SFPropertiesOfJavaClass *refPtr) {
	jfieldID fid = getFieldID(env, obj);
	if (fid == NULL)
		return false;

	(*env)->SetIntField(env, obj, fid, (uintptr_t)refPtr);
	return true;
}

static SFUnichar *getUnistr(JNIEnv *env, jstring str, int *length) {
	jclass java_string_class = (jclass)(*env)->FindClass(env, "java/lang/String");
	jmethodID charAt_id = (*env)->GetMethodID(env, java_string_class, "charAt", "(I)C");
	jmethodID length_id = (*env)->GetMethodID(env, java_string_class, "length", "()I");
	*length = (*env)->CallIntMethod(env, str, length_id);

	SFUnichar *unistr = malloc(*length * sizeof(SFUnichar));
	int i = 0;
	for (; i < *length; i++) {
		SFUnichar ch = (*env)->CallCharMethod(env, str, charAt_id, i);
		unistr[i] = ch;
	}

	(*env)->DeleteLocalRef(env, java_string_class);

	return unistr;
}

static void render_glyph(void *pixels, int width, int rows, float x, float y, void *resObj) {
	SFPropertiesOfJavaClass *prop = (SFPropertiesOfJavaClass *)resObj;

	SFColor *cpixels = (SFColor *)pixels;
	int length = width * rows;

	jintArray colors_array = (*prop->_env)->NewIntArray(prop->_env, length);
	if (colors_array == NULL) {
		LOGE("Error in creating pixels array.");
		return;
	}

	(*prop->_env)->SetIntArrayRegion(prop->_env, colors_array, 0, length, cpixels);
	(*prop->_env)->CallVoidMethod(prop->_env, prop->_canvas, prop->_draw_bitmap_mid, colors_array, 0, width, x, y, width, rows, 1, NULL);
	(*prop->_env)->DeleteLocalRef(prop->_env, colors_array);
}

static void setViewArea(JNIEnv *env, jobject obj, jint width) {
	SFPropertiesOfJavaClass *prop = getClassProperties(env, obj);
	if (!prop) {
		LOGE(SF_SET_VIEW_AREA_ERROR);
		return;
	}

	SFTextChangePageWidth(prop->_sfText, width - 4);
}

static void setColor(JNIEnv *env, jobject obj, jint color) {
	SFPropertiesOfJavaClass *prop = getClassProperties(env, obj);
	if (!prop) {
		LOGE(SF_COLOR_CHANGE_ERROR);
		return;
	}

	SFTextSetColor(prop->_sfText, color);
}

static void setTextAlignment(JNIEnv *env, jobject obj, jint align) {
	SFPropertiesOfJavaClass *prop = getClassProperties(env, obj);
	if (!prop) {
		LOGE(SF_TEXT_ALIGNMENT_ERROR);
		return;
	}

	SFTextSetAlignment(prop->_sfText, align);
}

static void changeFont(JNIEnv *env, jobject obj, jlong refPtr) {
	SFPropertiesOfJavaClass *prop = getClassProperties(env, obj);
	if (!prop) {
		LOGE(SF_FONT_CHANGE_ERROR);
		return;
	}

	prop->_sfFont = (SFFontRef)((uintptr_t)refPtr);
	SFTextChangeFont(prop->_sfText, prop->_sfFont);
}

static void changeText(JNIEnv *env, jobject obj, jstring str) {
	SFPropertiesOfJavaClass *prop = getClassProperties(env, obj);
	if (!prop) {
		LOGE(SF_TEXT_CHANGE_ERROR);
		return;
	}

	int length = 0;
	SFUnichar *unistr = getUnistr(env, str, &length);
	SFTextChangeString(prop->_sfText, unistr, length);
}

static int renderText(JNIEnv *env, jobject obj, jobject canvas, int initialIndex, int linesCount, int x, int y) {
	SFPropertiesOfJavaClass *prop = getClassProperties(env, obj);
	if (!prop) {
		LOGE(SF_RENDER_TEXT_ERROR);
		return;
	}

	if (!prop->_sfFont) {
		LOGW(SF_FONT_ERROR);
		return;
	}

	prop->_canvas = canvas;

	float singleLineHeight = SFFontGetLeading(prop->_sfFont);

	CGPoint pos;
	pos.x = x;
	pos.y = SFFontGetAscender(prop->_sfFont) + y;
	SFTextSetInitialPosition(prop->_sfText, pos);

	jclass java_canvas_class = (jclass)(*prop->_env)->GetObjectClass(prop->_env, prop->_canvas);
	prop->_draw_bitmap_mid = (*prop->_env)->GetMethodID(prop->_env, java_canvas_class, "drawBitmap", "([IIIFFIIZLandroid/graphics/Paint;)V");

	int numLines = linesCount;
	initialIndex = SFTextShowString(prop->_sfText, &render_glyph, initialIndex, &numLines);

	return numLines;
}

static int getCharIndexAfterLines(JNIEnv *env, jobject obj, int initialIndex, int linesCount) {
	SFPropertiesOfJavaClass *prop = getClassProperties(env, obj);
	if (!prop) {
		LOGE(SF_MEASURE_HEIGHT_ERROR);
		return 0;
	}

	if (!prop->_sfFont) {
		LOGW(SF_FONT_ERROR);
		return 0;
	}

	int tmpLines = 0;
	int startIndex = SFTextGetNextLineCharIndex(prop->_sfText, linesCount, initialIndex, &tmpLines);

	if (startIndex == -1)
		return -tmpLines;

	if (startIndex > initialIndex)
		return startIndex;

	return ++initialIndex;
}

static int measureHeight(JNIEnv *env, jobject obj) {
	SFPropertiesOfJavaClass *prop = getClassProperties(env, obj);
	if (!prop) {
		LOGE(SF_MEASURE_HEIGHT_ERROR);
		return 0;
	}

	if (!prop->_sfFont) {
		LOGW(SF_FONT_ERROR);
		return 0;
	}

	int lines = 0;
	int startIndex = 0;

	while (startIndex > -1) {
		int tmpLines = 0;
		int tmpStartIndex = SFTextGetNextLineCharIndex(prop->_sfText, 10, startIndex, &tmpLines);
		lines += tmpLines;

		if (tmpStartIndex > startIndex || tmpStartIndex == -1)
			startIndex = tmpStartIndex;
		else
			startIndex++;

		LOGV("st: %d   lines: %d", startIndex, lines);
	}

	int mHeight = (int)(lines * SFFontGetLeading(prop->_sfFont));

	LOGV("Measured height is: %d", mHeight);

	return mHeight;
}

static void destroy(JNIEnv *env, jobject obj) {
	SFPropertiesOfJavaClass *prop = getClassProperties(env, obj);

	if (prop) {
		SFFontRelease(prop->_sfFont);
		SFTextRelease(prop->_sfText);
		free(prop);

		setClassProperties(env, obj, NULL);

		LOGI("Sheen Figure has been destroyed for object 0x%x.", (long)prop);
	} else
		LOGW("Sheen Figure cannot be destroyed as it is not initialized.");
}

static void initialize(JNIEnv *env, jobject obj) {
	SFPropertiesOfJavaClass *prevProp = getClassProperties(env, obj);
	if (prevProp) {
		LOGW("Sheen Figure is already initialized for object 0x%x.", (long)prevProp);
		return;
	}

	SFPropertiesOfJavaClass *prop = malloc(sizeof(SFPropertiesOfJavaClass));
	prop->_sfFont = NULL;

	prop->_sfText = SFTextCreate(NULL, NULL, 0, 100);
	prop->_env = env;
	prop->_canvas = NULL;
	prop->_draw_bitmap_mid = NULL;

	SFTextSetReservedObject(prop->_sfText, prop);

	if (!setClassProperties(env, obj, prop)) {
		LOGE(SF_UNABLE_INIT_ERROR);
		free(prop);
	} else
		LOGI("Sheen Figure has been initialized for object 0x%x.", (long)prop);
}

static JNINativeMethod sfTextMethods[] = {
	{ "setViewArea", "(I)V", (void *)setViewArea },
	{ "setColor", "(I)V", (void *)setColor },
	{ "setTextAlignment", "(I)V", (void *)setTextAlignment },
	{ "changeFont", "(J)V", (void *)changeFont },
	{ "changeText", "(Ljava/lang/String;)V", (void *)changeText },
	{ "renderText", "(Landroid/graphics/Canvas;IIII)I", (void *)renderText },
	{ "getCharIndexAfterLines", "(II)I", (void *)getCharIndexAfterLines },
	{ "measureHeight", "()I", (void *)measureHeight },
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
