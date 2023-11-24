# Copyright 2011 The Android Open Source Project



#$(warning debug info BUILD_TINY_ANDROID = $(BUILD_TINY_ANDROID) ) 
#$(error debug info then exit)

LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

LOCAL_SRC_FILES := htfsk.c debug.c

LOCAL_MODULE := htfsk
#LOCAL_MODULE_TAGS := optional
LOCAL_FORCE_STATIC_EXECUTABLE := true
#LOCAL_MODULE_PATH := $(TARGET_ROOT_OUT)
#LOCAL_UNSTRIPPED_PATH := $(TARGET_ROOT_OUT_UNSTRIPPED)

LOCAL_C_INCLUDES := bootable/recovery external/openssl/include 
LOCAL_STATIC_LIBRARIES := libminui libpixelflinger_static libpng
LOCAL_STATIC_LIBRARIES += libz libstdc++ libcutils liblog libm libc

include $(BUILD_EXECUTABLE)

#$(warning BUILD_EXECUTABLE = $(BUILD_EXECUTABLE) ) 
#$(error debug info then exit)













