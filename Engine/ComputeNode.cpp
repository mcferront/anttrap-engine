#include "EnginePch.h"

#include "ComputeNode.h"
#include "MaterialObject.h"
#include "Viewport.h"
#include "GpuProfiler.h"

ComputeNode::ComputeNode(
    const char *pPass,
    ResourceHandle computeMaterials[ ],
    bool useAsyncCompute, //= false
    PCALLBACK pComputeSettingsProc, //= NULL
    const void *pUserData //=NULL
)
{
    Identifiable::Create( Id::Create() );

    m_pPass = StringRef( pPass );

    m_ComputeObjects.Create( 4, 4 );
    m_ObjectsReady.Create( 4, 4 );

    m_UseAsyncCompute = useAsyncCompute;
    m_pComputeSettingsProc = pComputeSettingsProc;
    m_pUserData = pUserData;
    m_pGpuTimer = NULL;

    uint32 i = 0;

    while ( computeMaterials[ i ] != NullHandle )
        m_ComputeObjects.Add( new ComputeMaterialObject( computeMaterials[ i++ ] ) );
}

ComputeNode::~ComputeNode( void )
{
    GpuProfiler::Instance( ).FreeTimer( m_pGpuTimer );

    for ( uint32 i = 0; i < m_ComputeObjects.GetSize( ); i++ )
        delete m_ComputeObjects.GetAt( i );

    m_ComputeObjects.Destroy( );
    m_ObjectsReady.Destroy( );

    StringRel( m_pPass );

    Identifiable::Destroy( );
}

void ComputeNode::GetRenderData(
    const Viewport &viewport
)
{
    m_ObjectsReady.Clear( );

    for ( uint32 i = 0; i < m_ComputeObjects.GetSize( ); i++ )
    {
        ComputeMaterialObject *pComputeObject = m_ComputeObjects.GetAt( i );

        if ( pComputeObject->Prepare( ) )
            m_ObjectsReady.Add( pComputeObject );
    }
}

void ComputeNode::Render(
    const Viewport &viewport,
    GpuDevice::CommandList *pBatchCommandList
)
{
    GpuDevice::CommandList *pCommandList = pBatchCommandList;

    Debug::Assert( Condition( !(m_UseAsyncCompute && pBatchCommandList)), "ASync Compute cannot be used with batch command lists" );

    if ( NULL == pBatchCommandList )
    {
        if ( m_UseAsyncCompute )
            pCommandList = GpuDevice::Instance( ).AllocPerFrameComputeCommandList( );
        else
            pCommandList = GpuDevice::Instance( ).AllocPerFrameGraphicsCommandList( );
    }

    if ( m_pGpuTimer )
        m_pGpuTimer->Begin( pCommandList );

    for ( uint32 i = 0; i < m_ObjectsReady.GetSize( ); i++ )
    {
        ComputeMaterialObject *pComputeObject = m_ObjectsReady.GetAt( i );

        ComputeContext computeContext = pComputeObject->GetComputeContext( m_pPass );
        pComputeObject->SetComputeContext( computeContext, pCommandList );

        if ( NULL != m_pComputeSettingsProc )
            m_pComputeSettingsProc( pComputeObject, computeContext, m_pUserData );


        pComputeObject->Dispatch( computeContext, pCommandList );
    }

    if ( m_pGpuTimer )
        m_pGpuTimer->End( pCommandList );

    if ( NULL == pBatchCommandList )
        GpuDevice::Instance( ).ExecuteCommandLists( &pCommandList, 1 );
}

GpuTimer *ComputeNode::AddGpuTimer(
    const char *pName
)
{

    m_pGpuTimer = GpuProfiler::Instance( ).AllocTimer( pName );
    return m_pGpuTimer;
}
