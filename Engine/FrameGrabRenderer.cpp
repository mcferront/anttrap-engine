#include "EnginePch.h"

#include "FrameGrabRenderer.h"
#include "GpuBuffer.h"
#include "FileStreams.h"

FrameGrabRenderer::FrameGrabRenderer(
   ResourceHandle imageBuffer
)
{
   Identifiable::Create( Id::Create( ) );

   m_ImageSize = 0;
   m_Width = 0;
   m_Height = 0;
   m_ImageBuffer = imageBuffer;
   m_pCapturedImage = NULL;
   m_GrabFrame = 0;
}

FrameGrabRenderer::~FrameGrabRenderer( void )
{
   if ( NULL != m_pCapturedImage )
   {
      m_pCapturedImage->Destroy( );
      delete m_pCapturedImage;
   }

   m_ImageBuffer = NULL;

   Identifiable::Destroy( );
}

void FrameGrabRenderer::GetRenderData(
   const Viewport &viewport
)
{
}

void FrameGrabRenderer::Render(
   const Viewport &viewport,
    GpuDevice::CommandList *pBatchCommandList
)
{
   if ( false == m_GrabFrame )
      return;

   GpuBuffer *pSource = GetResource( m_ImageBuffer, GpuBuffer );

   // Create a staging texture
   if ( m_Width != pSource->GetWidth() || m_Height != pSource->GetHeight() )
   {
      m_Width = (int) pSource->GetWidth();
      m_Height = (int) pSource->GetHeight();

      m_ImageSize = Align(m_Width, 256) * m_Height * 4;

      if ( NULL != m_pCapturedImage )
         m_pCapturedImage->Destroy( );

      delete m_pCapturedImage;
      m_pCapturedImage = NULL;
   }

   if ( NULL == m_pCapturedImage )
   {
      m_pCapturedImage = new GpuBuffer;
      m_pCapturedImage->Create( GpuBuffer::Heap::Readback, GpuBuffer::State::CopyDest, GpuResource::Flags::None,
                                GpuResource::Format::Unknown, m_ImageSize, 1, 1, -1, 1, true, NULL );
   }

   GpuBuffer::State::Type state = pSource->GetState( );
   {
      GpuDevice::CommandList *pCommandList = pBatchCommandList;
      
      if ( NULL == pBatchCommandList )
          pCommandList = GpuDevice::Instance( ).AllocPerFrameGraphicsCommandList( );

      m_pCapturedImage->TransitionTo( pCommandList, GpuBuffer::State::CopyDest );
      pSource->TransitionTo( pCommandList, GpuBuffer::State::CopySource );

      GpuDevice::Instance( ).CopyResource( pCommandList, m_pCapturedImage, pSource );

      // Transition the resource to the next state
      pSource->TransitionTo( pCommandList, state );

      if ( NULL == pBatchCommandList )
          GpuDevice::Instance( ).ExecuteCommandLists( &pCommandList, 1 );
   }

   m_GrabFrame = false;
}

bool FrameGrabRenderer::SaveFile(
   const char *pFilename
)
{
   byte *pSourceData;

   if ( NULL == m_pCapturedImage )
      return false;

   pSourceData = (byte *) m_pCapturedImage->Map( );

   typedef struct _color
   {
      int b : 8;
      int g : 8;
      int r : 8;
      int a : 8;
   } color;

   byte *pData = (byte *) malloc( m_Width * m_Height * 3 );

   byte *pConverted = pData;
   {
      for ( int y = 0; y < m_Height; y++ )
      {
         for ( int x = 0; x < m_Width; x++ )
         {
             // TODO: DX12 we shouldn't assume D3D12_TEXTURE_DATA_PITCH_ALIGNMENT
             // instead we should call pHelper->GetDevice()->GetCopyableFootprints and look at the pitch layout
             color c = *(color *) &pSourceData[ y * Align(m_Width * 4, D3D12_TEXTURE_DATA_PITCH_ALIGNMENT) + (x * 4) ];

            pConverted[ 0 ] = (byte) c.r;
            pConverted[ 1 ] = (byte) c.g;
            pConverted[ 2 ] = (byte) c.b;

            pConverted += 3;
         }
      }
   }


   //needed for beyond compare to 
   //load the color data correctly
   byte pad[ 2 ];

   uint32 bmpSize = m_Width * m_Height * 3;
   uint32 headerSize = sizeof( BITMAPFILEHEADER ) + sizeof( BITMAPINFO ) + sizeof( pad );
   uint32 fileSize = headerSize + bmpSize;

   BITMAPFILEHEADER bmfh = { 0 };
   bmfh.bfType = 'MB';
   bmfh.bfOffBits = headerSize;
   bmfh.bfSize = fileSize;

   BITMAPINFO bmpInfo = { 0 };
   bmpInfo.bmiHeader.biSize = sizeof( bmpInfo.bmiHeader );
   bmpInfo.bmiHeader.biSizeImage = bmpSize;
   bmpInfo.bmiHeader.biWidth = m_Width;
   bmpInfo.bmiHeader.biHeight = -(int)  m_Height;
   bmpInfo.bmiHeader.biPlanes = 1;
   bmpInfo.bmiHeader.biBitCount = 3 * 8;
   bmpInfo.bmiHeader.biCompression = 0;

   FileOutputStream stream;
   if ( false == stream.Open( pFilename ) )
      return false;

   stream.Write( &bmfh, sizeof( bmfh ), NULL );
   stream.Write( &bmpInfo, sizeof( bmpInfo ), NULL );
   stream.Write( pad, sizeof( pad ), NULL );
   stream.Write( pData, bmpSize, NULL );

   free( pData );

   stream.Close( );

   return true;
}
