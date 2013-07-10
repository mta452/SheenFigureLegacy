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

#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <math.h>

#include "SSFont.h"
#include "SSText.h"

#define SF_FONT_ERROR				"A font needs to be set before measuring/rendering text."

typedef struct SSReservedObjects {
	JNIEnv *_env;
	jobject _jResObj;
	jobject _textClass;
	jmethodID _renderGlyphMID;
} SSReservedObjects;

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

static SSTextClass *initialize(JNIEnv *env, jobject obj) {
	SSTextClass *text = malloc(sizeof(SSTextClass));
	text->_sfText = SFTextCreateWithString(NULL, 0, NULL);

	LOGI("Native text has been initialized for object 0x%x.", (uintptr_t)text);

	return text;
}

static void destroy(JNIEnv *env, jobject obj, SSTextClass *text) {
	SFTextRelease(text->_sfText);
	free(text);

	LOGI("Text has been destroyed for object 0x%x.", (int)text);
}

static void setString(JNIEnv *env, jobject obj, SSTextClass *text, jstring str) {
	int length = 0;
    SFUnichar *unistr = getUnistr(env, str, &length);
    SFTextSetString(text->_sfText, unistr, length);
}

static void setFont(JNIEnv *env, jobject obj, SSTextClass *text, SSFontClass *font) {
	if (font) {
		text->_font = font;
		SFTextSetFont(text->_sfText, text->_font->_sfFont);
	} else {
		text->_font = NULL;
		SFTextSetFont(text->_sfText, NULL);
	}
}

static void setAlignment(JNIEnv *env, jobject obj, SSTextClass *text, jint alignment) {
	SFTextSetAlignment(text->_sfText, alignment);
}

static void setWritingDirection(JNIEnv *env, jobject obj, SSTextClass *text, jint writingDirection) {
	SFTextSetWritingDirection(text->_sfText, writingDirection);
}

static int getNextLineCharIndex(JNIEnv *env, jobject obj, SSTextClass *text, jfloat frameWidth, int startIndex, int linesCount) {
	if (!text->_font || !text->_font->_sfFont) {
		LOGW(SF_FONT_ERROR);
		return;
	}

	int tmpLines = linesCount;
	int nextIndex = SFTextGetNextLineCharIndex(text->_sfText, frameWidth, startIndex, &tmpLines);

	if (nextIndex == -1) {
		return (-tmpLines-1);
    }

	return nextIndex;
}

static int measureLines(JNIEnv *env, jobject obj, SSTextClass *text, jfloat frameWidth) {
    return SFTextMeasureLines(text->_sfText, frameWidth);
}

static float measureHeight(JNIEnv *env, jobject obj, SSTextClass *text, jfloat frameWidth) {
    return SFTextMeasureHeight(text->_sfText, frameWidth);
}

static void renderGlyph(SFGlyph glyph, SFFloat x, SFFloat y, void *resObj) {
	SSReservedObjects *resObjects = (SSReservedObjects *)resObj;
	(*resObjects->_env)->CallStaticVoidMethod(resObjects->_env, resObjects->_textClass, resObjects->_renderGlyphMID, (jint)glyph, (jfloat)x, (jfloat)y, resObjects->_jResObj);
}

static int showString(JNIEnv *env, jobject obj, SSTextClass *text, jfloat frameWidth, jfloat x, jfloat y, jint startIndex, jint lines, jobject jResObj) {
	if (!text->_font || !text->_font->_sfFont) {
		LOGW(SF_FONT_ERROR);
		return;
	}

	jclass textClass = (jclass)(*env)->FindClass(env, _SS_TEXT_JAVA_CLASS);

	SSReservedObjects resObj;
	resObj._env = env;
	resObj._jResObj = jResObj;
	resObj._textClass = textClass;
	resObj._renderGlyphMID = (*env)->GetStaticMethodID(env, textClass, "renderGlyph", "(IFFLjava/lang/Object;)V");

	SFPoint point;
	point.x = x;
	point.y = y;

	int numLines = lines;
	int result = SFTextShowString(text->_sfText, frameWidth, point, startIndex, &numLines, &resObj, &renderGlyph);

	return numLines;
}

static JNINativeMethod sfTextMethods[] = {
	{ "nativeInitialize", "()I", (void *)initialize },
	{ "nativeDestroy", "(I)V", (void *)destroy },
    { "nativeSetString", "(ILjava/lang/String;)V", (void *)setString },
	{ "nativeSetFont", "(II)V", (void *)setFont },
    { "nativeSetAlignment", "(II)V", (void *)setAlignment },
    { "nativeSetWritingDirection", "(II)V", (void *)setWritingDirection },
	{ "nativeGetNextLineCharIndex", "(IFII)I", (void *)getNextLineCharIndex },
	{ "nativeMeasureLines", "(IF)I", (void *)measureLines },
	{ "nativeMeasureHeight", "(IF)F", (void *)measureHeight },
	{ "nativeShowString", "(IFFFIILjava/lang/Object;)I", (void *)showString }
};

int register_com_sheenfigure_graphics_Text(JNIEnv *env) {
	jclass cls = (*env)->FindClass(env, _SS_TEXT_JAVA_CLASS);
	if(cls == NULL){
		LOGE("Could not find class \"%s\".", _SS_TEXT_JAVA_CLASS);
		return JNI_ERR;
	}

	if ((*env)->RegisterNatives(env, cls, sfTextMethods, sizeof(sfTextMethods) / sizeof(sfTextMethods[0])) != JNI_OK) {
		LOGE("Method registration failure.");
		return JNI_ERR;
	}

    return JNI_OK;
}

/***************************************************************************/
