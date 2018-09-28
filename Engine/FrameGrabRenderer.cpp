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

//#ifdef DIRECTX12
   //D3D12_HEAP_PROPERTIES heapProperties = { };
   //{
   //   heapProperties.Type = D3D12_HEAP_TYPE_READBACK;
   //   heapProperties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
   //   heapProperties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
   //   heapProperties.CreationNodeMask = 1;
   //   heapProperties.VisibleNodeMask = 1;
   //}

   //D3D12_RESOURCE_DESC desc = pSource->GetD3D12Resource( )->GetDesc( );

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
      //D3D12_RESOURCE_DESC bufferDesc = {};
      //{
      //   bufferDesc.Alignment = D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT;
      //   bufferDesc.DepthOrArraySize = 1;
      //   bufferDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
      //   bufferDesc.Flags = D3D12_RESOURCE_FLAG_NONE;
      //   bufferDesc.Format = DXGI_FORMAT_UNKNOWN;
      //   bufferDesc.Height = 1;
      //   bufferDesc.Width = m_ImageSize;
      //   bufferDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
      //   bufferDesc.MipLevels = 1;
      //   bufferDesc.SampleDesc.Count = 1;
      //   bufferDesc.SampleDesc.Quality = 0;
      //}

      m_pCapturedImage = new GpuBuffer;
      m_pCapturedImage->Create( GpuBuffer::Heap::Readback, GpuBuffer::State::CopyDest, GpuResource::Flags::None,
                                GpuResource::Format::Unknown, m_ImageSize, 1, 1, -1, 1, true, NULL );

      //HRESULT hr;

      //ID3D12Resource *pStaging;

      //hr = GpuDevice::Instance( ).GetDevice( )->CreateCommittedResource(
      //   &heapProperties,
      //   D3D12_HEAP_FLAG_NONE,
      //   &bufferDesc,
      //   D3D12_RESOURCE_STATE_COPY_DEST,
      //   nullptr,
      //   __uuidof( ID3D12Resource ),
      //   (void **) &pStaging );

      //if ( FAILED( hr ) )
      //   return;

      //m_pCapturedImage->Create( pStaging, ImageBuffer::ViewType::CopyDest );
   }

   GpuBuffer::State::Type state = pSource->GetState( );
   {
      GpuDevice::CommandList *pCommandList = pBatchCommandList;
      
      if ( NULL == pBatchCommandList )
          pCommandList = GpuDevice::Instance( ).AllocPerFrameGraphicsCommandList( );

      m_pCapturedImage->TransitionTo( pCommandList, GpuBuffer::State::CopyDest );
      pSource->TransitionTo( pCommandList, GpuBuffer::State::CopySource );

      GpuDevice::Instance( ).CopyResource( pCommandList, m_pCapturedImage, pSource );

      //// Get the copy target location
      //D3D12_PLACED_SUBRESOURCE_FOOTPRINT bufferFootprint = { 0 };
      //{
      //   bufferFootprint.Footprint.Width = (UINT) desc.Width;
      //   bufferFootprint.Footprint.Height = desc.Height;
      //   bufferFootprint.Footprint.Depth = 1;
      //   bufferFootprint.Footprint.RowPitch = (UINT) Align(desc.Width * 4, 256);
      //   bufferFootprint.Footprint.Format = desc.Format;
      //}

      //D3D12_TEXTURE_COPY_LOCATION copySource = { 0 };
      //{
      //   copySource.pResource = pSource->GetD3D12Resource( );
      //}

      //D3D12_TEXTURE_COPY_LOCATION copyDest = { 0 };
      //{
      //   copyDest.pResource = m_pCapturedImage->GetD3D12Resource( );
      //   copyDest.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
      //   copyDest.PlacedFootprint = bufferFootprint;
      //}

      //// Copy the texture
      //pCommandList->pList->CopyTextureRegion( &copyDest, 0, 0, 0, &copySource, nullptr );

      // Transition the resource to the next state
      pSource->TransitionTo( pCommandList, state );

      if ( NULL == pBatchCommandList )
          GpuDevice::Instance( ).ExecuteCommandLists( &pCommandList, 1 );
   }

   m_GrabFrame = false;
//#endif
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
            color c = *(color *) &pSourceData[ y * Align(m_Width * 4, 256) + (x * 4) ];

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
