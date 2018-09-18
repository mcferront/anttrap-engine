
LOCAL_PATH := $(call my-dir)/../src

include $(CLEAR_VARS)

LOCAL_C_INCLUDES := $(LOCAL_PATH)
 
LOCAL_CFLAGS := -D_LIB \
                -DANDROID \
                -DNDEBUG \
                -D_LUA_ANSI
                
LOCAL_MODULE := Lua
  
LOCAL_SRC_FILES := \
   lapi.c \
   lauxlib.c \
   lbaselib.c \
   lcode.c \
   ldblib.c \
   ldebug.c \
   ldo.c \
   ldump.c \
   lfunc.c \
   lgc.c \
   linit.c \
   liolib.c \
   llex.c \
   lmathlib.c \
   lmem.c \
   loadlib.c \
   lobject.c \
   lopcodes.c \
   loslib.c \
   lparser.c \
   lstate.c \
   lstring.c \
   lstrlib.c \
   ltable.c \
   ltablib.c \
   ltm.c \
   luac.c \
   lundump.c \
   lvm.c \
   lzio.c \
   print.c
   
   include $(BUILD_STATIC_LIBRARY)