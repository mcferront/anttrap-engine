#include "EnginePch.h"

#include "ConvertToRenderer.h"
#include "Viewport.h"
#include "TextureAsset.h"
#include "GpuProfiler.h"

ConvertToRenderer::ConvertToRenderer(
    ResourceHandle gpuResource,
    GpuResource::State::Type convertTo
)
{
    Identifiable::Create( Id::Create( ) );

    m_Resource = gpuResource;
    m_ConvertTo = convertTo;
}

ConvertToRenderer::~ConvertToRenderer( void )
{
    m_Resource = NullHandle;
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
    GpuResource *pResource = GetResource( m_Resource, GpuResource );

    if ( pResource->GetState( ) != m_ConvertTo )
    {
        GpuDevice::CommandList *pCommandList = pBatchCommandList;

        if ( NULL == pBatchCommandList )
            pCommandList = GpuDevice::Instance( ).AllocPerFrameGraphicsCommandList( );

        pResource->TransitionTo( pCommandList, m_ConvertTo );

        if ( NULL == pBatchCommandList )
            GpuDevice::Instance( ).ExecuteCommandLists( &pCommandList, 1 );
    }
}

BarrierRenderer::BarrierRenderer(
    ResourceHandle buffer,
    GpuResource::Barrier::Type barrier
)
{
    Identifiable::Create( Id::Create( ) );

    m_Buffer = buffer;
    m_Barrier = barrier;
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
        pCommandList = GpuDevice::Instance( ).AllocPerFrameGraphicsCommandList( );

    if ( m_Barrier == GpuResource::Barrier::Uav )
    {
        D3D12_RESOURCE_UAV_BARRIER source = { };
        source.pResource = GetResource(m_Buffer, GpuResource)->GetApiResource();
    
        D3D12_RESOURCE_BARRIER barrier = { };
        barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_UAV;
        barrier.UAV = source;
    
        pCommandList->pList->ResourceBarrier( 1, &barrier );
    }
    else
        Debug::Assert( Condition(false), "Unrecognized barrier type: %d", m_Barrier );

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
        pCommandList = GpuDevice::Instance( ).AllocPerFrameGraphicsCommandList( );

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
