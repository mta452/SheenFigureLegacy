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

#include <android/bitmap.h>

#include <stdbool.h>
#include <string.h>

#include "SSFont.h"

static jobject _alpha8Config = NULL;

static jclass _glyphClass = NULL;
static jmethodID _createGlyphMethodID = NULL;

static jclass _bitmapClass = NULL;
static jmethodID _createBitmapMethodID = NULL;

static jobject getAlpha8Config(JNIEnv *env) {
	if (!_alpha8Config) {
		jclass configClass = (jclass)(*env)->FindClass(env, "android/graphics/Bitmap$Config");
		jfieldID alpha8ConfigFID = (*env)->GetStaticFieldID(env, configClass, "ALPHA_8", "Landroid/graphics/Bitmap$Config;");
		_alpha8Config = (*env)->GetStaticObjectField(env, configClass, alpha8ConfigFID);
		_alpha8Config = (*env)->NewGlobalRef(env, _alpha8Config);
	}

	return _alpha8Config;
}

static jclass getGlyphClass(JNIEnv *env) {
	if (!_glyphClass) {
		_glyphClass = (jclass)(*env)->FindClass(env, _SS_FONT_JAVA_CLASS"$Glyph");
		_glyphClass = (*env)->NewGlobalRef(env, _glyphClass);
	}

	return _glyphClass;
}

static jmethodID getCreateGlyphMethodID(JNIEnv *env) {
	if (!_createGlyphMethodID) {
		_createGlyphMethodID = (*env)->GetMethodID(env, getGlyphClass(env), "<init>", "(IILandroid/graphics/Bitmap;)V");
	}

	return _createGlyphMethodID;
}

static jclass getBitmapClass(JNIEnv *env) {
	if (!_bitmapClass) {
		_bitmapClass = (jclass)(*env)->FindClass(env, "android/graphics/Bitmap");
		_bitmapClass = (*env)->NewGlobalRef(env, _bitmapClass);
	}

	return _bitmapClass;
}

static jmethodID getCreateBitmapMethodID(JNIEnv *env) {
	if (!_createBitmapMethodID) {
		_createBitmapMethodID = (*env)->GetStaticMethodID(env, getBitmapClass(env), "createBitmap", "(IILandroid/graphics/Bitmap$Config;)Landroid/graphics/Bitmap;");
	}

	return _createBitmapMethodID;
}

static SSFontClass *initialize(JNIEnv *env, jobject obj, jstring path, jfloat size, const SSFontClass *refPtr) {
	SSFontClass *font = malloc(sizeof(SSFontClass));

	const char *utfPath = (*env)->GetStringUTFChars(env, path, 0);

	FT_Init_FreeType(&font->_ftLib);
	FT_New_Face(font->_ftLib, utfPath, 0, &font->_ftFace);
	FT_Set_Char_Size(font->_ftFace, 0, size * 64, 72, 72);

	pthread_mutex_init(&font->_renderMutex, NULL);

	if (refPtr) {
		font->_sfFont = SFFontMakeCloneForFTFace(((SSFontClass *)refPtr)->_sfFont, font->_ftFace, size);
	} else {
		font->_sfFont = SFFontCreateWithFTFace(font->_ftFace, size);
	}

	(*env)->ReleaseStringUTFChars(env, path, utfPath);

	LOGI("Native font has been initialized for object 0x%x.", (uintptr_t)font);

	return font;
}

static void destroy(JNIEnv *env, jobject obj, SSFontClass *font) {
	if (font->_hasRenderMutex) {
		pthread_mutex_destroy(&font->_renderMutex);
	}

	SFFontRelease(font->_sfFont);
	FT_Done_Face(font->_ftFace);
	FT_Done_FreeType(font->_ftLib);

	free(font);

	LOGI("Font has been destroyed for object 0x%x.", (uintptr_t)font);
}

static jfloat getSize(JNIEnv *env, jobject obj, SSFontClass *font) {
	return SFFontGetSize(font->_sfFont);
}

static jfloat getSizeByEm(JNIEnv *env, jobject obj, SSFontClass *font) {
	return SFFontGetSizeByEm(font->_sfFont);
}

static jfloat getAscender(JNIEnv *env, jobject obj, SSFontClass *font) {
	return SFFontGetAscender(font->_sfFont);
}

static jfloat getDescender(JNIEnv *env, jobject obj, SSFontClass *font) {
	return SFFontGetDescender(font->_sfFont);
}

static jfloat getLeading(JNIEnv *env, jobject obj, SSFontClass *font) {
	return SFFontGetLeading(font->_sfFont);
}

static pthread_mutex_t *getFontRenderMutex(SSFontClass *font) {
	if (!font->_hasRenderMutex) {
		pthread_mutex_init(&font->_renderMutex, NULL);
	}

	return &font->_renderMutex;
}

static jobject generateGlyph(JNIEnv *env, jobject obj, SSFontClass *font, jint glyph, jintArray sizeArray) {
	pthread_mutex_t *renderMutex = getFontRenderMutex(font);
	pthread_mutex_lock(renderMutex);

	jobject glyphBitmap = NULL;

	FT_Error error;
	FT_Face face = font->_ftFace;
	FT_GlyphSlot slot = face->glyph;

	error = FT_Load_Glyph(face, glyph, FT_LOAD_RENDER);
	if (error) {
		goto unlock;
	}

	FT_Bitmap *bmp = &slot->bitmap;
	int length = bmp->width * bmp->rows;
	if (!length) {
	    goto unlock;
	}

	glyphBitmap = (*env)->CallStaticObjectMethod(env, getBitmapClass(env), getCreateBitmapMethodID(env), bmp->width, bmp->rows, getAlpha8Config(env));

	void *pixels = NULL;
	AndroidBitmap_lockPixels(env, glyphBitmap, &pixels);
	memcpy(pixels, bmp->buffer, length);
	AndroidBitmap_unlockPixels(env, glyphBitmap);

unlock:
	pthread_mutex_unlock(renderMutex);

	return (*env)->NewObject(env, getGlyphClass(env), getCreateGlyphMethodID(env), (jint)slot->bitmap_top, (jint)slot->bitmap_left, glyphBitmap);
}

static JNINativeMethod sfFontMethods[] = {
	{ "nativeInitialize", "(Ljava/lang/String;FI)I", (void *)initialize },
	{ "nativeDestroy", "(I)V", (void *)destroy },
	{ "nativeGetSize", "(I)F", (void *)getSize },
	{ "nativeGetSizeByEm", "(I)F", (void *)getSizeByEm },
	{ "nativeGetAscender", "(I)F", (void *)getAscender },
	{ "nativeGetDescender", "(I)F", (void *)getDescender },
	{ "nativeGetLeading", "(I)F", (void *)getLeading },
	{ "nativeGenerateGlyph", "(II)L"_SS_FONT_JAVA_CLASS"$Glyph;", (void *)generateGlyph }
};

int register_com_sheenfigure_graphics_Font(JNIEnv *env) {
	jclass cls = (*env)->FindClass(env, _SS_FONT_JAVA_CLASS);
	if(cls == NULL){
		LOGE("Could not find class \"%s\".", _SS_FONT_JAVA_CLASS);
		return JNI_ERR;
	}

	if ((*env)->RegisterNatives(env, cls, sfFontMethods, sizeof(sfFontMethods) / sizeof(sfFontMethods[0])) != JNI_OK) {
		LOGE("Method registration failure.");
		return JNI_ERR;
	}

    return JNI_OK;
}

/*****************************************************************************/
