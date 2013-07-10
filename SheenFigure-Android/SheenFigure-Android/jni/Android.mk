#
# Copyright (C) 2013 SheenFigure
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#
 
LOCAL_PATH := $(call my-dir)

#########################FREETYPE##########################
include $(CLEAR_VARS)

LOCAL_MODULE := freetype

FT_ROOT_PATH := $(LOCAL_PATH)/../../../FreeType
FT_SRC_PATH := $(FT_ROOT_PATH)/src

FILE_LIST := $(FT_SRC_PATH)/base/ftbase.c
FILE_LIST += $(FT_SRC_PATH)/base/ftapi.c
FILE_LIST += $(FT_SRC_PATH)/base/ftbbox.c
FILE_LIST += $(FT_SRC_PATH)/base/ftbitmap.c
FILE_LIST += $(FT_SRC_PATH)/base/ftdebug.c
FILE_LIST += $(FT_SRC_PATH)/base/ftgasp.c
FILE_LIST += $(FT_SRC_PATH)/base/ftglyph.c
FILE_LIST += $(FT_SRC_PATH)/base/ftinit.c
FILE_LIST += $(FT_SRC_PATH)/base/ftlcdfil.c
FILE_LIST += $(FT_SRC_PATH)/base/ftmm.c
FILE_LIST += $(FT_SRC_PATH)/base/ftpatent.c
FILE_LIST += $(FT_SRC_PATH)/base/ftsynth.c
FILE_LIST += $(FT_SRC_PATH)/base/ftstroke.c
FILE_LIST += $(FT_SRC_PATH)/base/ftsystem.c
FILE_LIST += $(FT_SRC_PATH)/bzip2/ftbzip2.c
FILE_LIST += $(FT_SRC_PATH)/cff/cff.c
FILE_LIST += $(FT_SRC_PATH)/pshinter/pshinter.c
FILE_LIST += $(FT_SRC_PATH)/psnames/psnames.c
FILE_LIST += $(FT_SRC_PATH)/raster/raster.c
FILE_LIST += $(FT_SRC_PATH)/sfnt/sfnt.c
FILE_LIST += $(FT_SRC_PATH)/smooth/smooth.c
FILE_LIST += $(FT_SRC_PATH)/truetype/truetype.c
LOCAL_SRC_FILES := $(FILE_LIST:$(LOCAL_PATH)/%=%)

LOCAL_CFLAGS := "-DFT2_BUILD_LIBRARY -std=gnu99"
LOCAL_C_INCLUDES := $(FT_ROOT_PATH)/include

LOCAL_EXPORT_C_INCLUDES := $(FT_ROOT_PATH)/include

include $(BUILD_STATIC_LIBRARY)
#########################FREETYPE##########################

#######################SHEEN FIGURE########################
include $(CLEAR_VARS)

LOCAL_MODULE := sheenfigure

SF_ROOT_PATH := $(LOCAL_PATH)/../../../SheenFigure

FILE_LIST := $(wildcard $(SF_ROOT_PATH)/SFInternal/*.c)
FILE_LIST += $(wildcard $(SF_ROOT_PATH)/SFInternal/**/*.c)
FILE_LIST += $(wildcard $(SF_ROOT_PATH)/SFGraphics/*.c)
LOCAL_SRC_FILES := $(FILE_LIST:$(LOCAL_PATH)/%=%)

LOCAL_CFLAGS := -std=gnu99

LOCAL_C_INCLUDES := $(SF_ROOT_PATH)
LOCAL_C_INCLUDES += $(SF_ROOT_PATH)/SFInternal/bidi
LOCAL_C_INCLUDES += $(SF_ROOT_PATH)/SFInternal
LOCAL_C_INCLUDES += $(SF_ROOT_PATH)/SFGraphics

LOCAL_EXPORT_C_INCLUDES := $(SF_ROOT_PATH)
LOCAL_EXPORT_C_INCLUDES += $(SF_ROOT_PATH)/SFInternal/bidi
LOCAL_EXPORT_C_INCLUDES += $(SF_ROOT_PATH)/SFInternal
LOCAL_EXPORT_C_INCLUDES += $(SF_ROOT_PATH)/SFGraphics

LOCAL_STATIC_LIBRARIES := freetype

include $(BUILD_STATIC_LIBRARY)
###########################################################

############################JNI############################
include $(CLEAR_VARS)

LOCAL_MODULE := SheenFigureJNI

FILE_LIST := $(LOCAL_PATH)/SSJNI.c
FILE_LIST += $(LOCAL_PATH)/SSFont.c
FILE_LIST += $(LOCAL_PATH)/SSText.c
LOCAL_SRC_FILES := $(FILE_LIST:$(LOCAL_PATH)/%=%)

LOCAL_CFLAGS := "-DSF_ANDROID -std=gnu99"
LOCAL_LDLIBS := -llog
LOCAL_LDLIBS += -ljnigraphics

LOCAL_C_INCLUDES := $(LOCAL_PATH)

LOCAL_STATIC_LIBRARIES := sheenfigure

include $(BUILD_SHARED_LIBRARY)
###########################################################