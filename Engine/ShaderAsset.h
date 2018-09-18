#pragma once

#include "EngineGlobal.h"

#ifdef DIRECTX9
   #include "Dx9Shader.h"
#elif defined DIRECTX12
    #include "Dx12Shader.h"
#elif defined OPENGL
   #include "GlShader.h"
#else
   #error Graphics API Undefined
#endif
