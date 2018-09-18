#pragma once

#ifdef DIRECTX9
   #include "Dx9.h"
#elif defined DIRECTX12
   #include "Dx12.h"
#elif defined OPENGL
   #error What Engine Include goes here?
#else
   #error Graphics API not defined
#endif

