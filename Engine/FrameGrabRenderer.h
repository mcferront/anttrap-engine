#pragma once

#include "EngineGlobal.h"
#include "Renderer.h"
#include "Identifiable.h"
#include "RenderObject.h"
#include "MaterialAsset.h"
#include "HashTable.h"
#include "List.h"

class Viewport;
class IRenderModifier;

class FrameGrabRenderer : public Renderer
{
public:
   typedef HashTable<RenderContext, List<RenderObjectDesc *> *> PassHash;
   typedef Enumerator<RenderContext, List<RenderObjectDesc *> *> PassEnum;

private:
   static const uint32 MaxCustomSamplePositions = 16;

private:
   ResourceHandle m_ImageBuffer;
   ImageBuffer *m_pCapturedImage;
   int m_Width;
   int m_Height;
   size_t m_ImageSize;
   bool m_GrabFrame;

public:
   FrameGrabRenderer(
      ResourceHandle imageBuffer
      );

   virtual ~FrameGrabRenderer( void );
   
   virtual void GetRenderData(
      const Viewport &viewport
   );

   virtual void Render(
      const Viewport &viewport,
       GpuDevice::CommandList *pBatchCommandList
   );

   bool SaveFile(
      const char *pFilename
      );

   void GrabFrame( void ) { m_GrabFrame = true; }
};
