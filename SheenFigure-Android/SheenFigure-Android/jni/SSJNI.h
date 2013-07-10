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

#ifndef _SS_JNI_H
#define _SS_JNI_H

#include <jni.h>
#include <android/log.h>

#include <stdbool.h>
#include <pthread.h>

#include <SFConfig.h>
#include <SFTypes.h>
#include <SFFont.h>
#include <SFText.h>

#define SF_TAG 		"sheenfigure"

#define LOGV(...) 	__android_log_print(ANDROID_LOG_VERBOSE, SF_TAG, __VA_ARGS__)
#define LOGI(...)  	__android_log_print(ANDROID_LOG_INFO, SF_TAG, __VA_ARGS__)
#define LOGW(...) 	__android_log_print(ANDROID_LOG_WARN, SF_TAG, __VA_ARGS__)
#define LOGE(...) 	__android_log_print(ANDROID_LOG_ERROR, SF_TAG, __VA_ARGS__)

#define _SS_JAVA_CLASS_PREFIX	"com/sheenfigure/graphics/"
#define _SS_FONT_JAVA_CLASS		_SS_JAVA_CLASS_PREFIX"Font"
#define _SS_TEXT_JAVA_CLASS		_SS_JAVA_CLASS_PREFIX"Text"

#endif
