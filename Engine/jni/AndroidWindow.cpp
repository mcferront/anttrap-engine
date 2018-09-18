#include "EnginePch.h"

#include "AndroidWindow.h"
#include "Viewport.h"

//for glCheckError
#include "GlShader.h"

DefineResourceType(Window, Resource, NULL);

void Window::Create(
   const SystemId &systemId,
   GLuint frameBuffer,
   GLuint renderBuffer,
   GLuint depthBuffer
)
{
   Identifiable::Create( systemId );

   //glCheckError( "Should be Clear" );

   ////glGenFramebuffers(1, &m_ViewFrameBuffer );
   ////glCheckError( "glGenFramebuffers" );

   //m_ViewFrameBuffer = 0;

   //glGenRenderbuffers(1, &m_ViewRenderBuffer);
   //glCheckError( "glGenRenderbuffers" );

   //glGenRenderbuffers(1, &m_DepthBuffer);
   //glCheckError( "glGenRenderbuffers" );

   //glBindFramebuffer(GL_FRAMEBUFFER, m_ViewFrameBuffer);
   //glCheckError( "glBindFramebuffer" );

   //int backingWidth  = Mode::Landscape.GetPhysicalWidth( );
   //int backingHeight = Mode::Landscape.GetPhysicalHeight( );

   //Debug::Print( Debug::TypeInfo, "Window Width = %d, Height = %d", backingWidth, backingHeight );

   ////color buffer
   //glBindRenderbuffer(GL_RENDERBUFFER, m_ViewRenderBuffer);
   //glCheckError( "glBindRenderbuffer" );
   //
   //glRenderbufferStorage(GL_RENDERBUFFER, GL_RGBA4, backingWidth, backingHeight);
   //glCheckError( "glRenderbufferStorage" );

   //glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, m_ViewRenderBuffer);
   //glCheckError( "glFramebufferRenderbuffer" );

   ////depth
   //glBindRenderbuffer(GL_RENDERBUFFER, m_DepthBuffer);
   //   
   //glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT16, backingWidth, backingHeight);
   //glCheckError( "glRenderbufferStorage" );

   //glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, m_DepthBuffer);
   //glCheckError( "glFramebufferRenderbuffer" );

   ////verify
   //GLenum status = glCheckFramebufferStatus( GL_FRAMEBUFFER );
   //if(status != GL_FRAMEBUFFER_COMPLETE) 
   //{
   //   Debug::Print( Debug::TypeError, "glError: failed to make complete framebuffer object 0x%04x", status );
   //}
}

void Window::Destroy( void )
{
   //glCheckError( "Should be Clear" );

   //glDeleteFramebuffers(1, &m_ViewFrameBuffer);
   //glCheckError( "glDeleteFramebuffers" );

   //m_ViewFrameBuffer = 0;

   //glDeleteRenderbuffers(1, &m_ViewRenderBuffer);
   //glCheckError( "glDeleteRenderbuffers" );

   //m_ViewRenderBuffer = 0;
   // 
   //if ( m_DepthBuffer ) 
   //{
   //    glDeleteRenderbuffers(1, &m_DepthBuffer);
   //    glCheckError( "glDeleteRenderbuffers" );

   //   m_DepthBuffer = 0;
   //}

   Resource::Destroy( );
}

void Window::BeginRender( 
   bool clear
)
{
   glCheckError( "Should be Clear" );

   //glBindFramebuffer( GL_FRAMEBUFFER, m_ViewFrameBuffer );
   //glCheckError( "glBindFramebuffer" );

   if ( true == clear )
   {
      glClearColor( 0.0f, 0.0f, 0.0f, 0.0f );
      glCheckError( "glClearColor" );

      //depth buffer is cleared with viewport 'MakeActive'
      //because each viewport is a render layer
      glClear( GL_COLOR_BUFFER_BIT );
      glCheckError( "clear buffer bit" );
   }
}

void Window::EndRender( void )
{
   //glCheckError( "Should be Clear" );

   //glBindRenderbuffer( GL_RENDERBUFFER, m_ViewRenderBuffer );
   //glCheckError( "glBindRenderbuffer" );

   Present( );
}

void Window::Present( void )
{
}
