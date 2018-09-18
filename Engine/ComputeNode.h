#pragma once

#include "EngineGlobal.h"
#include "Identifiable.h"
#include "Renderer.h"

class ComputeMaterialObject;
class GpuTimer;

class ComputeNode : public Renderer
{
public:
   typedef void (*PCALLBACK) (ComputeMaterialObject *pCompute, RenderContext context, const void *pUserData);

private:
   List<ComputeMaterialObject *>  m_ComputeObjects;
   List<ComputeMaterialObject *>  m_ObjectsReady;
   
   PCALLBACK   m_pComputeSettingsProc;
   const void *m_pUserData;
   const char *m_pPass;
   bool m_UseAsyncCompute;

   GpuTimer *m_pGpuTimer;

public:
   ComputeNode(
      const char *pPass,
      ResourceHandle computeMaterials[],
      bool useAsyncCompute = false,
      PCALLBACK pComputeSettingsProc = NULL,
      const void *pUserData = NULL
   );

   virtual ~ComputeNode( void );

   virtual void GetRenderData(
      const Viewport &viewport
   );

   virtual void Render(
      const Viewport &viewport,
       GpuDevice::CommandList *pBatchCommandList
   );

   GpuTimer *AddGpuTimer( const char *pName );
};
