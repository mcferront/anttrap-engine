
LOCAL_PATH := $(call my-dir)/..

include $(CLEAR_VARS)

LOCAL_C_INCLUDES := $(LOCAL_PATH)
 
LOCAL_CFLAGS := -D_LIB \
                -DANDROID \
                -DNDEBUG
                
LOCAL_MODULE := ZLib
  
LOCAL_SRC_FILES := \
   adler32.c \
   compress.c \
   crc32.c \
   deflate.c \
   example.c \
   gzclose.c \
   gzlib.c \
   gzread.c \
   gzwrite.c \
   infback.c \
   inffast.c \
   inflate.c \
   inftrees.c \
   minigzip.c \
   trees.c \
   uncompr.c \
   zutil.c

include $(BUILD_STATIC_LIBRARY)