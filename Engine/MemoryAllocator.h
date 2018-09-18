#pragma once

#include "EngineGlobal.h"

#ifdef WIN32
   #include "Win32MemoryAllocator.h"
#else
   #include "GenericMemoryAllocator.h"
#endif