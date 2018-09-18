#pragma once

#include "EngineGlobal.h"
#include "Resource.h"
#include "Viewport.h"

class Window : public Resource
{
public:
   DeclareResourceType(Window);

private:
   GLuint m_ViewRenderBuffer;
   GLuint m_ViewFrameBuffer;
   GLuint m_DepthBuffer;

public:
   virtual void Create(
      const SystemId &systemId,
      GLuint frameBuffer,
      GLuint renderBuffer,
      GLuint depthBuffer
   );

   virtual void Destroy( void );

   void BeginRender( 
      bool clear
   );
   
   void EndRender( void );

   bool CanRender( void ) { return true; }
   
   void Copy(
      const Window &copyFrom
   )
   {
      Identifiable::Create( copyFrom.GetId( ) );

      m_ViewRenderBuffer = copyFrom.m_ViewRenderBuffer;
      m_ViewFrameBuffer  = copyFrom.m_ViewFrameBuffer;
      m_DepthBuffer      = copyFrom.m_DepthBuffer;
   }
   
private:
   //must be defined in obj-c
   void Present  ( void );
};
