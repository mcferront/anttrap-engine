#include "EnginePch.h"

#include "MacWindow.h"
#include "Viewport.h"

DefineResourceType(Window, Resource, NULL);

void Window::Create(
   const SystemId &systemId,
   GLuint frameBuffer,
   GLuint renderBuffer,
   GLuint depthBuffer
)
{
   Identifiable::Create( systemId );

   //m_ViewFrameBuffer  = frameBuffer;
   //m_ViewRenderBuffer = renderBuffer;
   //m_DepthBuffer      = depthBuffer;
}

void Window::Destroy( void )
{
   //not needed our obj-c gl view sets it up for us
   //glDeleteFramebuffers(1, &m_ViewFrameBuffer);
   //m_ViewFrameBuffer = 0;

   //glDeleteRenderbuffers(1, &m_ViewRenderBuffer);
   //m_ViewRenderBuffer = 0;
    
   //if ( m_DepthBuffer ) 
   //{
   //    glDeleteRenderbuffers(1, &m_DepthBuffer);
   //    m_DepthBuffer = 0;
   //}

   Resource::Destroy( );
}

void Window::BeginRender( 
   bool clear
)
{
   //not needed our obj-c gl view sets it up for us
   //glBindFramebuffer( GL_FRAMEBUFFER, m_ViewFrameBuffer );

   if ( true == clear )
   {
      glClearColor( 0.0f, 0.0f, 0.0f, 0.0f );

      //depth buffer is cleared with viewport 'MakeActive'
      //because each viewport is a render layer
      glClear( GL_COLOR_BUFFER_BIT );
   }
}

void Window::EndRender( void )
{
   //not needed our obj-c gl view sets it up for us
   //glBindRenderbuffer( GL_RENDERBUFFER, m_ViewRenderBuffer );

   Present( );
}
