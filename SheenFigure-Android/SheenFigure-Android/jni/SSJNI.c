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

#include "SSJNI.h"

#ifdef SF_ANDROID

#include <stdint.h>

#include "SSFont.h"
#include "SSText.h"

JNIEXPORT jint JNICALL
JNI_OnLoad(JavaVM *jvm, void *reserved) {
	JNIEnv *env;
	int result;

	if ((*jvm)->GetEnv(jvm, (void **)&env, JNI_VERSION_1_6)) {
		LOGE("JNI Check failure");
		return JNI_ERR;
	}

	if (env == NULL) {
		LOGE("Env is NULL");
		return JNI_ERR;
	}

	result = register_com_sheenfigure_graphics_Text(env);
	result = register_com_sheenfigure_graphics_Font(env);

	if (result != JNI_OK)
		return JNI_ERR;

	return JNI_VERSION_1_6;
}

#endif
