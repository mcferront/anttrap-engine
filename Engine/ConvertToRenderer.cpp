#include "EnginePch.h"

#include "ConvertToRenderer.h"
#include "Viewport.h"
#include "TextureAsset.h"
#include "GpuProfiler.h"

ConvertToRenderer::ConvertToRenderer(
    ResourceHandle image,
    ImageBuffer::ViewType convertTo
)
{
    Identifiable::Create( Id::Create( ) );

    m_Image = image;
    m_ConvertTo = convertTo;
}

ConvertToRenderer::~ConvertToRenderer( void )
{
    m_Image = NullHandle;
    Identifiable::Destroy( );
}

void ConvertToRenderer::GetRenderData(
    const Viewport &viewport
)
{}

void ConvertToRenderer::Render(
    const Viewport &viewport,
    GpuDevice::CommandList *pBatchCommandList
)
{
    ImageBuffer *pDest = GetResource( m_Image, ImageBuffer );

    if ( pDest->RequiresConvert( m_ConvertTo ) )
    {
        GpuDevice::CommandList *pCommandList = pBatchCommandList;

        if ( NULL == pBatchCommandList )
            pCommandList = GpuDevice::Instance( ).AllocGraphicsCommandList( );

        pDest->ConvertTo( m_ConvertTo, pCommandList );

        if ( NULL == pBatchCommandList )
            GpuDevice::Instance( ).ExecuteCommandLists( &pCommandList, 1 );
    }
}

BarrierRenderer::BarrierRenderer(
    ResourceHandle buffer,
    ImageBuffer::Barrier::Type type
)
{
    Identifiable::Create( Id::Create( ) );

    m_Buffer = buffer;
    m_Type = type;
}

BarrierRenderer::~BarrierRenderer( void )
{
    m_Buffer = NullHandle;

    Identifiable::Destroy( );
}

void BarrierRenderer::GetRenderData(
    const Viewport &viewport
)
{}

void BarrierRenderer::Render(
    const Viewport &viewport,
    GpuDevice::CommandList *pBatchCommandList
)
{
    GpuDevice::CommandList *pCommandList = pBatchCommandList;

    if ( NULL == pBatchCommandList )
        pCommandList = GpuDevice::Instance( ).AllocGraphicsCommandList( );

    ImageBuffer *pDest = GetResource( m_Buffer, ImageBuffer );
    pDest->Barrier( m_Type, pCommandList );

    if ( NULL == pBatchCommandList )
        GpuDevice::Instance( ).ExecuteCommandLists( &pCommandList, 1 );
}

GpuTimerNode::GpuTimerNode( 
    const char *pName
)
{
    Identifiable::Create( Id::Create( ) );

    m_pGpuTimer = GpuProfiler::Instance( ).AllocTimer( pName );

    m_Started = false;
}

GpuTimerNode::~GpuTimerNode( void )
{
    GpuProfiler::Instance( ).FreeTimer( m_pGpuTimer );

    Identifiable::Destroy( );
}

void GpuTimerNode::Render(
    const Viewport &viewport,
    GpuDevice::CommandList *pBatchCommandList
)
{
    GpuDevice::CommandList *pCommandList = pBatchCommandList;

    if ( NULL == pBatchCommandList )
        pCommandList = GpuDevice::Instance( ).AllocGraphicsCommandList( );

    if ( false == m_Started )
    {
        m_pGpuTimer->Begin( pCommandList );
        m_Started = true;
    }
    else
    {
        m_pGpuTimer->End( pCommandList );
        m_Started = false;
    }

    if ( NULL == pBatchCommandList )
        GpuDevice::Instance( ).ExecuteCommandLists( &pCommandList, 1 );
}
