#include "EnginePch.h"

#include "CopyResourceRenderer.h"
#include "Viewport.h"
#include "MaterialAsset.h"
#include "TaskWorld.h"
#include "Quad.h"
#include "TextureAsset.h"

CopyResourceRenderer::CopyResourceRenderer(
   ResourceHandle destination,
   ResourceHandle source
   )
{
   Identifiable::Create( Id::Create() );

   m_Destination = destination;
   m_Source = source;
}

CopyResourceRenderer::~CopyResourceRenderer( void )
{
   m_Destination = NullHandle;
   m_Source = NullHandle;

   Identifiable::Destroy( );
}

void CopyResourceRenderer::GetRenderData(
   const Viewport &viewport
)
{}

void CopyResourceRenderer::Render(
   const Viewport &viewport,
    GpuDevice::CommandList *pBatchCommandList
   )
{
   GpuDevice::CommandList *pCommandList = pBatchCommandList;
   
   if ( NULL == pBatchCommandList )
       pCommandList = GpuDevice::Instance( ).AllocGraphicsCommandList( );

   ImageBuffer *pDest = GetResource( m_Destination, ImageBuffer );
   ImageBuffer *pSource = GetResource( m_Source, ImageBuffer );

   pDest->ConvertTo( ImageBuffer::CopyDest, pCommandList );
   pSource->ConvertTo( ImageBuffer::CopySource, pCommandList );

   GpuDevice::CopyResource( pCommandList, pDest, pSource );

   if ( NULL == pBatchCommandList )
       GpuDevice::Instance( ).ExecuteCommandLists( &pCommandList, 1 );
}
