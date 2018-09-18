#pragma once

#include "EngineGlobal.h"
#include "List.h"
#include "Resource.h"
#include "Camera.h"
#include "GraphicsApi.h"
#include "TextureAsset.h"
#include "RenderContexts.h"

class GraphicsMaterialObject;

class Viewport : public Identifiable
{
public:
   enum ClearFlags
   {
      ClearFlagsNone = 0x0,
      ClearFlagsColor = 0x1,
      ClearFlagsDepth = 0x2,

      ClearFlagsAll = 0xffffff
   };

private:
   struct ScissorRect
   {
      int left;
      int right;
      int top;
      int bottom;
   };

   struct ViewportRect
   {
      float left;
      float top;
      float width;
      float height;
      float nearZ;
      float farZ;
   };

private:
   int             m_Width;
   int             m_Height;
   Camera          m_Camera;
   ScissorRect     m_Scissor;
   ViewportRect    m_Viewport;
   ResourceHandle  m_hCamera;
   ResourceHandle  m_hRenderTargets[ RenderContexts::MaxRenderTargets ];
   ResourceHandle  m_hDepthStencil;
   ViewportContext m_ViewportContext;
   uint32          m_NumRenderTargets;
   Vector2         m_RenderScale;
   ClearFlags		 m_ClearFlags;
   Color           m_ClearColor;
   bool            m_IsScissored;
   bool            m_Orthographic;

public:
   Viewport( void )
   {
      m_ViewportContext = -1;
   }

   void Create(
      Id id,
      int width,
      int height,
      ResourceHandle camera,
      ResourceHandle renderTargets[ ],
      uint32 numTargets,
      ResourceHandle depthStencil
   );

   void Create(
      int width,
      int height,
      ResourceHandle camera,
      ResourceHandle renderTargets[ ],
      uint32 numTargets,
      ResourceHandle depthStencil
   );

   void Create(
      const Viewport &viewport
   );

   void Destroy( void )
   {
      m_hCamera = NullHandle;
      m_hDepthStencil = NullHandle;

      for ( uint32 i = 0; i < m_NumRenderTargets; i++ )
         m_hRenderTargets[ i ] = NullHandle;
   }

   void SetRenderScale(
      const Vector2 &scale
   )
   {
      m_RenderScale = scale;
   }

   void SetRenderTarget(
      uint32 index,
      ResourceHandle renderTarget
   )
   {
      m_hRenderTargets[ index ] = renderTarget;
   }

   void SetDepthStencil(
      ResourceHandle depthStencilTarget
   )
   {
      m_hDepthStencil = depthStencilTarget;
   }

   void SetClearColor(
      const Color &color
   )
   {
      m_ClearColor = color;
   }

   void SetClearFlags(
      ClearFlags flags
   )
   {
      m_ClearFlags = flags;
   }

   void Copy(
      const Viewport &copyFrom
   );

   bool ShouldRender( void );
   bool MakeActive( void );

   void Set(
      GpuDevice::CommandList *pCommandList
   );

   ResourceHandle GetRenderTarget(
      uint32 index
   ) const;

   ResourceHandle GetDepthStencil( void ) const;

   void SetCamera(
      ResourceHandle camera
   );

   void SetScissor(
      int left,
      int top,
      int right,
      int bottom
   );

   void VirtualToProjected(
      float *pX,
      float *pY,
      int x,
      int y
   ) const;

   void PhysicalToProjected(
      float *pX,
      float *pY,
      int x,
      int y
   ) const;

   float GetProjectedWidth(
      uint32 pixels
   ) const;

   float GetProjectedHeight(
      uint32 pixels
   ) const;


   const void GetRenderScale(
      Vector2 *pScale
   ) const
   {
      *pScale = m_RenderScale;
   }

   int GetWidth( void ) const { return m_Width; }
   int GetHeight( void ) const { return m_Height; }

   ViewportContext GetContext( void ) const { return m_ViewportContext; }

   uint32 GetNumRenderTargets( void ) const { return m_NumRenderTargets; }

   const Camera *GetCamera( void ) const { return &m_Camera; }
};

typedef List<Viewport> ViewportList;

Vector2 GetMaterialSize(
   const GraphicsMaterialObject *pMaterial
);

Vector2 GetMaterialUV(
   const GraphicsMaterialObject *pMaterial
);

Vector2 VirtualSizeToProjected(
   const Viewport &viewport,
   const GraphicsMaterialObject *pMaterial,
   int x,
   int y
);

Vector2 VirtualSizeToProjected(
   const Viewport &viewport,
   const GraphicsMaterialObject *pMaterial
);

Vector2 VirtualSizeToProjected(
   const Viewport &viewport,
   int x,
   int y
);

Vector2 VirtualPosToProjected(
   const Viewport &viewport,
   int x,
   int y
);

void PhysicalToProjected(
   float *pX,
   float *pY,
   int x,
   int y,
   int cameraWidth,
   int cameraHeight,
   int viewportLeft,
   int viewportTop,
   int viewportWidth,
   int viewportHeight
);
