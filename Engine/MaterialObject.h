#pragma once
   
#include "EngineGlobal.h"

#if defined DIRECTX12
   #include "Dx12MaterialObject.h"
   #include "Dx12ComputeMaterialObject.h"
#else
   #error Graphics API Undefined
#endif
