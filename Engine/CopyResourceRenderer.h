#pragma once

#include "EngineGlobal.h"
#include "Identifiable.h"
#include "Renderer.h"

class CopyResourceRenderer : public Renderer
{
   ResourceHandle m_Destination;
   ResourceHandle m_Source;

public:
   CopyResourceRenderer(
      ResourceHandle destination,
      ResourceHandle source
      );

   virtual ~CopyResourceRenderer( void );

   virtual void GetRenderData(
      const Viewport &viewport
      );

   virtual void Render(
      const Viewport &viewport,
       GpuDevice::CommandList *pBatchCommandList
      );
};
