#pragma once

#ifdef _DEBUG
#  define ENABLE_RTTI_CHECK
#  define ENABLE_ASSERT
#  define ENABLE_DEBUGMEMORY
#  define ENABLE_DEBUGPRINT
#  define ENABLE_TASKWORLD_MEMORY_VALIDATION
//#  define ENABLE_DEBUG_TASKWORLD_MEMORY
//#  define ENABLE_STRINGPOOL_VALIDATION
#  define D3D_DEBUG_INFO

#elif defined _RELEASE
#  ifndef NDEBUG
   #  define NDEBUG
#  endif
#  define ENABLE_ASSERT
#  define ENABLE_DEBUGPRINT
//#  define ENABLE_DEBUGMEMORY
//#  define ENABLE_DEBUG_TASKWORLD_MEMORY
//#  define ENABLE_STRINGPOOL_VALIDATION

#elif defined _DISTRIBUTION
#  ifndef NDEBUG
   #  define NDEBUG
#  endif

#else
   #error "Build configuration not defined"
#endif

#ifdef WIN32
#  define _BIND_TO_CURRENT_CRT_VERSION 1
#  define PlatformType "win"
#  define WINVER 0x0A00
#  define _WIN32_WINNT 0x0A00
#  define _CRT_SECURE_NO_DEPRECATE
#  define _CRT_SECURE_NO_WARNINGS
#elif defined IOS
#  define PlatformType "ios"
#  define OPENGL
#elif defined ANDROID
#  define PlatformType "android"
#  define OPENGL
#elif defined LINUX
#  define PlatformType "linux"
#elif defined MAC
#  define PlatformType "mac"
#  define OPENGL
#else
#  error "Platform not defined"
#endif


