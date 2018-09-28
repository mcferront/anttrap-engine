#include "EnginePch.h"

#include "MipGenNode.h"
#include "GpuBuffer.h"

MipGenNode::MipGenNode(
    ResourceHandle target,
    ResourceHandle mipLevels[],
    int numMipLevels
)
{
    Identifiable::Create( Id::Create( ) );

    m_MipLevels.Create( );
    m_MipLevels.CopyFrom( mipLevels, numMipLevels );

    m_pDestLayouts = NULL;
    m_Target = target;
}

MipGenNode::~MipGenNode( void )
{
    m_MipLevels.Destroy( );

    m_Target = NullHandle;
    free( m_pDestLayouts );

    m_pDestLayouts = NULL;

    Identifiable::Destroy( );
}

void MipGenNode::GetRenderData(
    const Viewport &viewport
)
{
}

void MipGenNode::Render(
    const Viewport &viewport,
    GpuDevice::CommandList *pBatchCommandList
)
{
    uint32 numMips = m_MipLevels.GetSize( );

    GpuBuffer *pDestBuffer = GetResource( m_Target, GpuBuffer );

    ID3D12Resource *pDest = pDestBuffer->GetApiResource( );

    if ( NULL == m_pDestLayouts )
    {
        m_pDestLayouts = (byte *) malloc( sizeof( D3D12_PLACED_SUBRESOURCE_FOOTPRINT ) * numMips );
        GpuDevice::Instance( ).GetDevice( )->GetCopyableFootprints( &pDest->GetDesc( ), 0, numMips, 0, (D3D12_PLACED_SUBRESOURCE_FOOTPRINT *) m_pDestLayouts, NULL, NULL, NULL );
    }

    D3D12_PLACED_SUBRESOURCE_FOOTPRINT *pDestLayouts = (D3D12_PLACED_SUBRESOURCE_FOOTPRINT *) m_pDestLayouts;

    // copy each mip level over to the texture
    D3D12_TEXTURE_COPY_LOCATION dest = { };
    D3D12_TEXTURE_COPY_LOCATION source = { };

    GpuDevice::CommandList *pCommandList = pBatchCommandList;
    
    if ( NULL == pBatchCommandList )
        pCommandList = GpuDevice::Instance( ).AllocPerFrameGraphicsCommandList( );

    GetResource( m_Target, GpuBuffer )->TransitionTo( pCommandList, GpuBuffer::State::CopyDest );

    for ( uint32 i = 0; i < numMips; i++ )
    {
        GpuBuffer *pSourceBuffer = GetResource( m_MipLevels.Get( i ), GpuBuffer );
        pSourceBuffer->TransitionTo( pCommandList, GpuBuffer::State::CopySource );

        ID3D12Resource *pSource = pSourceBuffer->GetApiResource( );

        dest.pResource = pDest;
        dest.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
        dest.SubresourceIndex = i;

        // tell the GPU how to interpret this unknown buffer
        // by giving it the layout desc of the texture it contains
        source.pResource = pSource;
        source.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
        source.SubresourceIndex = 0;

        pCommandList->pList->CopyTextureRegion( &dest, 0, 0, 0, &source, NULL );
        pSourceBuffer->TransitionTo( pCommandList, GpuBuffer::State::PixelShaderResource );
    }

    pDestBuffer->TransitionTo( pCommandList, GpuBuffer::State::PixelShaderResource );

    if ( NULL == pBatchCommandList )
        GpuDevice::Instance( ).ExecuteCommandLists( &pCommandList, 1 );
}
