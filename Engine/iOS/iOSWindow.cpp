#include "EnginePch.h"

#include "iOSWindow.h"
#include "Viewport.h"

DefineResourceType(Window, Resource, NULL);

void Window::Create(
   const Cwid &cwid,
   GLuint frameBuffer,
   GLuint renderBuffer,
   GLuint depthBuffer
)
{
   Identifiable::Create( cwid );

   m_ViewFrameBuffer  = frameBuffer;
   m_ViewRenderBuffer = renderBuffer;
   m_DepthBuffer      = depthBuffer;
}

void Window::Destroy( void )
{
   glDeleteFramebuffers(1, &m_ViewFrameBuffer);
   m_ViewFrameBuffer = 0;

   glDeleteRenderbuffers(1, &m_ViewRenderBuffer);
   m_ViewRenderBuffer = 0;
    
   if ( m_DepthBuffer ) 
   {
       glDeleteRenderbuffers(1, &m_DepthBuffer);
       m_DepthBuffer = 0;
   }

   Resource::Destroy( );
}

void Window::BeginRender( 
   bool clear
)
{
   glBindFramebuffer( GL_FRAMEBUFFER, m_ViewFrameBuffer );

   //if ( true == clear )
   {
      glClearColor( 0.0f, 0.0f, 0.0f, 0.0f );
      glClear( GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT );
   }
}

void Window::EndRender(
   bool present
)
{
   glBindRenderbuffer( GL_RENDERBUFFER, m_ViewRenderBuffer );

   if (true == present)
      Present( );
}

void Window::GetDimensions(
   int *pWidth,
   int *pHeight
) const
{
   *pWidth = Mode::Landscape.GetPhysicalWidth();
   *pHeight= Mode::Landscape.GetPhysicalHeight();
}