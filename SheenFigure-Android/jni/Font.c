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

/***************************JNI WRAPPER FOR SFFONT***************************/

#ifdef SF_ANDROID

#include <stdbool.h>

#include "SheenFigure.h"
#include "SFFont.h"

static jfieldID getFieldID(JNIEnv *env, jobject obj) {
	jclass cls = (*env)->GetObjectClass(env, obj);
	jfieldID fid = (*env)->GetFieldID(env, cls, "mRefPtr", "I");

	return fid;
}

static SFFontRef getRefPtr(JNIEnv *env, jobject obj) {
	jfieldID fid = getFieldID(env, obj);
    if (fid == NULL)
    	return NULL;

    uintptr_t refPtr = (*env)->GetIntField(env, obj, fid);
    return (SFFontRef)refPtr;
}

static bool setRefPtr(JNIEnv *env, jobject obj, SFFontRef refPtr) {
	jfieldID fid = getFieldID(env, obj);
	if (fid == NULL)
		return false;

	(*env)->SetIntField(env, obj, fid, (uintptr_t)refPtr);
	return true;
}

static void initialize(JNIEnv *env, jobject obj, jstring path, jfloat size) {
	const char *fontPath = (*env)->GetStringUTFChars(env, path, 0);
	SFFontRef sfFont = SFFontCreateWithFileName(fontPath, size);
	(*env)->ReleaseStringUTFChars(env, path, fontPath);

	setRefPtr(env, obj, sfFont);
}

static jint makeClone(JNIEnv *env, jobject obj, jfloat size) {
	SFFontRef sfFont = getRefPtr(env, obj);
	SFFontRef clone = SFFontMakeClone(sfFont, size);

	return (uintptr_t)clone;
}

static jfloat getSize(JNIEnv *env, jobject obj) {
	SFFontRef sfFont = getRefPtr(env, obj);
	return SFFontGetSize(sfFont);
}

static jfloat getSizeByEm(JNIEnv *env, jobject obj) {
	SFFontRef sfFont = getRefPtr(env, obj);
	return SFFontGetSizeByEm(sfFont);
}

static jfloat getAscender(JNIEnv *env, jobject obj) {
	SFFontRef sfFont = getRefPtr(env, obj);
	return SFFontGetAscender(sfFont);
}

static jfloat getDescender(JNIEnv *env, jobject obj) {
	SFFontRef sfFont = getRefPtr(env, obj);
	return SFFontGetDescender(sfFont);
}

static jfloat getLeading(JNIEnv *env, jobject obj) {
	SFFontRef sfFont = getRefPtr(env, obj);
	return SFFontGetLeading(sfFont);
}

static void destroy(JNIEnv *env, jobject obj) {
	SFFontRef sfFont = getRefPtr(env, obj);
	SFFontRelease(sfFont);
}

static JNINativeMethod sfFontMethods[] = {
	{ "initialize", "(Ljava/lang/String;F)V", (void *)initialize },
	{ "makeClone", "(F)I", (void *)makeClone },
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
