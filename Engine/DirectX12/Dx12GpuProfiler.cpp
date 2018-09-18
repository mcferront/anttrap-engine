#include "EnginePch.h"

#include "Dx12GpuProfiler.h"
#include "Dx12Texture.h"

GpuProfiler &GpuProfiler::Instance( void )
{
    static GpuProfiler s_instance;
    return s_instance;
}

void GpuProfiler::Create( void )
{
    D3D12_QUERY_HEAP_DESC heapDesc = {};
    {
        heapDesc.Count = GpuProfiler::MaxTimers * 2 * GpuDevice::FrameCount;
        heapDesc.NodeMask = 0;
        heapDesc.Type = D3D12_QUERY_HEAP_TYPE_TIMESTAMP;
        
        GpuDevice::Instance( ).GetDevice( )->CreateQueryHeap( &heapDesc, __uuidof(ID3D12QueryHeap*), (void **) &m_pHeap );

        for (int i = 0; i < GpuDevice::FrameCount; i++)
        {
            m_ReadBuffers[ i ] = ResourceHandle( Id::Create() );
            
            ImageBuffer *pBuffer = new ImageBuffer;
            m_ReadBuffers[ i ].Bind(NULL, pBuffer);

            pBuffer->CreateAsCopyDest( GpuProfiler::MaxTimers * 2 * sizeof(uint64) );
            pBuffer->AddToScene();
        }

        memset( m_pTimers, 0, sizeof(m_pTimers) );
    }
}

void GpuProfiler::Destroy( void )
{
    for (int i = 0; i < GpuDevice::FrameCount; i++)
    {
        ImageBuffer *pBuffer = GetResource( m_ReadBuffers[i], ImageBuffer );
        pBuffer->RemoveFromScene();
        pBuffer->Destroy( );
        
        delete pBuffer;
        m_ReadBuffers[i] = NullHandle;
    }

    m_pHeap->Release( );
}

GpuTimer *GpuProfiler::AllocTimer( 
    const char *pName
)
{
    MainThreadCheck;
    
    int i;
    
    for (i = 0; i < GpuProfiler::MaxTimers; i++ )
    {
        if ( m_pTimers[ i ] == NULL )
            break;
    }

    Debug::Assert( Condition(i < GpuProfiler::MaxTimers), "No free GpuTimers available" );

    GpuTimer *pTimer = new GpuTimer( pName, i * 2, m_pHeap, m_ReadBuffers );

    m_pTimers[ i ] = pTimer;

    return pTimer;
}

void GpuProfiler::FreeTimer( 
    GpuTimer *pTimer 
)
{
    if ( pTimer )
    {
        MainThreadCheck;

        m_pTimers[ pTimer->slot / 2 ] = NULL;

        StringRel(pTimer->pName);

        delete pTimer;
    }
}

void GpuProfiler::ResolveTimers( 
    int frameIndex 
)
{
    MainThreadCheck;

    D3D12_RANGE range = { 0 };
    uint64 *pSourceData;
    UINT64 gfxFrequency, computeFrequency;

    GpuDevice::Instance( ).GetGraphicsQueue( )->GetTimestampFrequency( &gfxFrequency );
    GpuDevice::Instance( ).GetComputeQueue( )->GetTimestampFrequency( &computeFrequency );

    ImageBuffer *pBuffer = GetResource( m_ReadBuffers[ frameIndex ], ImageBuffer );

    pBuffer->GetD3D12Resource( )->Map( 0, &range, (void **) &pSourceData );

    for (int i = 0; i < GpuProfiler::MaxTimers; i++ )
    {
        if ( m_pTimers[ i ] != NULL )
            m_pTimers[ i ]->Resolve( gfxFrequency, computeFrequency, pSourceData );
    }

    pBuffer->GetD3D12Resource( )->Unmap( 0, NULL );
}
    
GpuTimer::GpuTimer( 
    const char *_pName,
    uint32 _slot,
    ID3D12QueryHeap *_pHeap,
    ResourceHandle _readBuffers[ GpuDevice::FrameCount ]
)
{
    MainThreadCheck;

    slot = _slot;
    pName = StringRef(_pName);
    pHeap = _pHeap;
    isCompute = false;
    startTime = 0;
    endTime = 0;
    duration = 0;

    for (int i = 0; i < GpuDevice::FrameCount; i++)
        readBuffers[i] = _readBuffers[i];
}

GpuTimer::~GpuTimer( void )
{
    for (int i = 0; i < GpuDevice::FrameCount; i++)
        readBuffers[i] = NullHandle;
}

void GpuTimer::Begin(
    GpuDevice::CommandList *pCommandList
)
{
    pCommandList->pList->EndQuery( pHeap, D3D12_QUERY_TYPE_TIMESTAMP, slot );
}

void GpuTimer::End(
    GpuDevice::CommandList *pCommandList
)
{
    int frameIndex = GpuDevice::Instance( ).GetFrameIndex();

    isCompute = pCommandList->isCompute;

    pCommandList->pList->EndQuery( pHeap, D3D12_QUERY_TYPE_TIMESTAMP, slot + 1 );
    pCommandList->pList->ResolveQueryData( pHeap, D3D12_QUERY_TYPE_TIMESTAMP, slot, 2, GetResource(readBuffers[ frameIndex ], ImageBuffer)->GetD3D12Resource( ), slot * sizeof(uint64) );
}

void GpuTimer::Resolve( 
    uint64 gfxFrequency,
    uint64 computeFrequency,
    uint64 *pSourceData
)
{
    startTime = *(pSourceData + slot);
    endTime = *(pSourceData + (slot + 1));

    if ( endTime > startTime )
    {
        uint64 delta = endTime - startTime;

        double frequency = isCompute ? (double) computeFrequency : (double) gfxFrequency;

        const float K = .1f;
        duration = (duration * (1 - K)) + ((float) ((delta / frequency) * 1000.0) * K);
    }
}
