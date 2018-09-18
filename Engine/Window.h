#pragma once

#include "EngineGlobal.h"

#ifdef WIN32
   #include "Win32Window.h"
#elif defined IOS
   #include "iOSWindow.h"
#elif defined ANDROID
   #include "AndroidWindow.h"
#elif defined MAC
   #include "MacWindow.h"
#endif