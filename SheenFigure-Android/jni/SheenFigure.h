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

#ifndef _SHEEN_FIGURE_JNI_H
#define _SHEEN_FIGURE_JNI_H

#include <jni.h>
#include <android/log.h>

#include "SFConfig.h"
#include "SFTypes.h"

#define SF_TAG "sheenfigure"
#define LOGV(...) 	__android_log_print(ANDROID_LOG_VERBOSE, SF_TAG, __VA_ARGS__)
#define LOGI(...)  	__android_log_print(ANDROID_LOG_INFO, SF_TAG, __VA_ARGS__)
#define LOGW(...) 	__android_log_print(ANDROID_LOG_WARN, SF_TAG, __VA_ARGS__)
#define LOGE(...) 	__android_log_print(ANDROID_LOG_ERROR, SF_TAG, __VA_ARGS__)

int register_com_sheenfigure_graphics_Font(JNIEnv *env);
int register_com_sheenfigure_graphics_Text(JNIEnv *env);

#endif
