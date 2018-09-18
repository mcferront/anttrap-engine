#pragma once

#include "EngineGlobal.h"
#include "List.h"
#include "ThreadLocks.h"

#ifdef WIN32
   #include "Win32Thread.h"
#elif defined IOS
   #include "iOSThread.h"
#elif defined ANDROID
   #include "AndroidThread.h"
#elif defined MAC
   #include "MacThread.h"
#elif defined LINUX
   #include "LinuxThread.h"
#else
   #error "Platform not defined"
#endif

#define MainThreadCheck\
   Debug::Assert( Condition(Thread::IsMainThread()), "This function can only be called from the main thread" )
