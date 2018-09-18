#pragma once
   
#include "EngineGlobal.h"

#ifdef DIRECTX9
   #include "Dx9Material.h"
#elif defined DIRECTX12
   #include "Dx12Material.h"
#elif defined OPENGL
   #include "GlMaterial.h"
#else
   #error Graphics API Undefined
#endif
