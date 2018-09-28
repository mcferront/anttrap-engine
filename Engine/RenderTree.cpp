#include "EnginePch.h"

#include "RenderTree.h"
#include "Renderer.h"
#include "Viewport.h"

enum TOKENS
{
    BATCH_START = 1,
    BATCH_END,

    TOKEN_COUNT
};

void RenderTree::Destroy( void )
{
    if ( false == m_Created )
        return;

    m_Created = false;

    List<Renderer*> uniqueNodes; uniqueNodes.Create( );

    int i, size = m_Nodes.GetSize( );

    Renderer *pRenderer;

    for ( i = 0; i < size; i++ )
    {
        pRenderer = m_Nodes.GetAt( i );

        if ( (nuint) pRenderer > TOKEN_COUNT )
            uniqueNodes.AddUnique( pRenderer );
    }

    size = uniqueNodes.GetSize();

    for ( i = 0; i < size; i++ )
    {
        pRenderer = uniqueNodes.GetAt( i );
        
        pRenderer->Destroy( );
        delete pRenderer;
    }

    uniqueNodes.Destroy( );
    m_Nodes.Destroy( );
}

void RenderTree::BeginBatch( void )
{
    AddNode( (Renderer *) BATCH_START );
}

void RenderTree::EndBatch( void )
{
    AddNode( (Renderer *) BATCH_END );
}

void RenderTree::AddNode(
    Renderer *pNode
)
{
    m_Nodes.Add( pNode );
}

void RenderTree::GetRenderData(
    const Viewport &viewport
)
{
    Renderer *pRenderer;
    int i, size = m_Nodes.GetSize( );

    for ( i = 0; i < size; i++ )
    {
        pRenderer = m_Nodes.GetAt( i );
        
        if ( (nuint) pRenderer > TOKEN_COUNT )
            pRenderer->GetRenderData( viewport );
    }
}

void RenderTree::Render(
    const Viewport &viewport
)
{
    Renderer *pRenderer;
    int i, size = m_Nodes.GetSize( );

    GpuDevice::CommandList *pCommandList = NULL;

    for ( i = 0; i < size; i++ )
    {
        pRenderer = m_Nodes.GetAt( i );
        
        if ( (nuint) pRenderer == BATCH_START )
            pCommandList = GpuDevice::Instance( ).AllocPerFrameGraphicsCommandList( );
        else if ( (nuint) pRenderer == BATCH_END )
        {
            GpuDevice::Instance( ).ExecuteCommandLists( &pCommandList, 1 );
            pCommandList = NULL;
        }
        else
            pRenderer->Render( viewport, pCommandList );
    }
}
