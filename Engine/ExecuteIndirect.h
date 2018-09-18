#pragma once

#include "EngineGlobal.h"

#ifdef DIRECTX12
#include "Dx12ExecuteIndirect.h"
#else
#error Graphics API Undefined
#endif
