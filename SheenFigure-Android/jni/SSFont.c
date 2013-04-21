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

/***************************JNI WRAPPER FOR SFFONT***************************/

#include "SSJNI.h"

#ifdef SF_ANDROID

#include <stdbool.h>

#include "SSFont.h"

static jfieldID getRefPtrFieldID(JNIEnv *env, jobject obj) {
	jclass cls = (*env)->GetObjectClass(env, obj);
	jfieldID fid = (*env)->GetFieldID(env, cls, "mRefPtr", "I");

	return fid;
}

static SSFontClass *getRefPtr(JNIEnv *env, jobject obj) {
	jfieldID fid = getRefPtrFieldID(env, obj);
    if (fid) {
    	uintptr_t refPtr = (*env)->GetIntField(env, obj, fid);
    	return (SSFontClass *)refPtr;
    }

    return NULL;
}

static bool setRefPtr(JNIEnv *env, jobject obj, SSFontClass *font) {
	jfieldID fid = getRefPtrFieldID(env, obj);
	if (fid) {
		(*env)->SetIntField(env, obj, fid, (uintptr_t)font);
		return true;
	}

	return false;
}

static void initialize(JNIEnv *env, jobject obj, jstring path, jfloat size, uintptr_t refPtr) {
	SSFontClass *font = getRefPtr(env, obj);
	if (font) {
		LOGW("Font is already initialized for object 0x%x.", (int)font);
		return;
	}

	font = malloc(sizeof(SSFontClass));

	const char *utfPath = (*env)->GetStringUTFChars(env, path, 0);

	FT_Init_FreeType(&font->_ftLib);
	FT_New_Face(font->_ftLib, utfPath, 0, &font->_ftFace);
	FT_Set_Char_Size(font->_ftFace, 0, size * 64, 72, 72);

	if (refPtr) {
		font->_sfFont = SFFontMakeCloneForFTFace(((SSFontClass *)refPtr)->_sfFont, font->_ftFace, size);
	} else {
		font->_sfFont = SFFontCreateWithFTFace(font->_ftFace, size);
	}

	(*env)->ReleaseStringUTFChars(env, path, utfPath);

	if (setRefPtr(env, obj, font)) {
		LOGI("Font has been initialized for object 0x%x.", (int)font);
	} else {
		LOGE("Font could not be initialized due to some internal error.");
		free(font);
	}
}

static jfloat getSize(JNIEnv *env, jobject obj) {
	return SFFontGetSize(getRefPtr(env, obj)->_sfFont);
}

static jfloat getSizeByEm(JNIEnv *env, jobject obj) {
	return SFFontGetSizeByEm(getRefPtr(env, obj)->_sfFont);
}

static jfloat getAscender(JNIEnv *env, jobject obj) {
	return SFFontGetAscender(getRefPtr(env, obj)->_sfFont);
}

static jfloat getDescender(JNIEnv *env, jobject obj) {
	return SFFontGetDescender(getRefPtr(env, obj)->_sfFont);
}

static jfloat getLeading(JNIEnv *env, jobject obj) {
	return SFFontGetLeading(getRefPtr(env, obj)->_sfFont);
}

static void destroy(JNIEnv *env, jobject obj) {
	SSFontClass *font = getRefPtr(env, obj);

	SFFontRelease(font->_sfFont);
	FT_Done_Face(font->_ftFace);
	FT_Done_FreeType(font->_ftLib);

	free(font);

	setRefPtr(env, obj, NULL);

	LOGI("Font has been destroyed for object 0x%x.", (int)font);
}

static JNINativeMethod sfFontMethods[] = {
	{ "initialize", "(Ljava/lang/String;FI)V", (void *)initialize },
	{ "getSize", "()F", (void *)getSize },
	{ "getSizeByEm", "()F", (void *)getSizeByEm },
	{ "getAscender", "()F", (void *)getAscender },
	{ "getDescender", "()F", (void *)getDescender },
	{ "getLeading", "()F", (void *)getLeading },
	{ "destroy", "()V", (void *)destroy }
};

int register_com_sheenfigure_graphics_Font(JNIEnv *env) {
	static const char *const strClassName = "com/sheenfigure/graphics/Font";

	jclass cls = (*env)->FindClass(env, strClassName);
	if(cls == NULL){
		LOGE("Could not find class \"%s\".", strClassName);
		return JNI_ERR;
	}

	if ((*env)->RegisterNatives(env, cls, sfFontMethods, sizeof(sfFontMethods) / sizeof(sfFontMethods[0])) != JNI_OK) {
		LOGE("Method registration failure.");
		return JNI_ERR;
	}

    return JNI_OK;
}

#endif

/*****************************************************************************/
