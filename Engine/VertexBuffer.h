#pragma once

#include "EngineGlobal.h"

#ifdef DIRECTX9
   #include "Dx9VertexBuffer.h"
#elif defined DIRECTX12
   #include "Dx12VertexBuffer.h"
#elif defined OPENGL
   #include "GlVertexBuffer.h"
#else
   #error Graphics API Undefined
#endif
