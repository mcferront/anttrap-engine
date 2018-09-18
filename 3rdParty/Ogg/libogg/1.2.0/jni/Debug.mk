
LOCAL_PATH := $(call my-dir)/..

include $(CLEAR_VARS)

LOCAL_C_INCLUDES := $(LOCAL_PATH)/include
 
LOCAL_CFLAGS := -D_LIB \
                -DANDROID \
                -D_DEBUG \
                -D_USRDLL \
                -DLIBOGG_EXPORTS \
                -g
                
LOCAL_MODULE := Ogg
  
LOCAL_SRC_FILES := \
   src/bitwise.c \
   src/framing.c

include $(BUILD_STATIC_LIBRARY)