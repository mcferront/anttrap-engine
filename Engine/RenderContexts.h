#pragma once
   
#include "EngineGlobal.h"

#if defined DIRECTX12
   #include "Dx12Contexts.h"
#else
   #error Graphics API Undefined
#endif
