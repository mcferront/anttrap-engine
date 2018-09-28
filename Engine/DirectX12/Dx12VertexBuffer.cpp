#include "EnginePch.h"

#include "VertexBuffer.h"
#include "ShaderAsset.h"
#include "RenderContexts.h"
#include "GpuBuffer.h"

void VertexBuffer::Create( void )
{
    m_pVertexBuffer = NULL;
    m_pIndexBuffer = NULL;

    memset( &m_VertexBufferView, 0, sizeof( m_VertexBufferView ) );
    memset( &m_IndexBufferView, 0, sizeof( m_IndexBufferView ) );

    m_VertexElementDesc.pElementDescs = m_ElementDescs;
    m_VertexElementDesc.numElementDescs = 0;
    m_VertexElementDesc.topology = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
    m_IndexCount = 0;
}

void VertexBuffer::Destroy( void )
{
    if ( m_pVertexBuffer )
        m_pVertexBuffer->Destroy();

    delete m_pVertexBuffer;
    
    if ( m_pIndexBuffer )
        m_pIndexBuffer->Destroy();

    delete m_pIndexBuffer;
}

void VertexBuffer::SetVertices(
    const void *pVertices,
    uint32 size
)
{
    m_pVertexBuffer = new GpuBuffer;
    m_pVertexBuffer->Create( GpuResource::Heap::Default, GpuResource::State::VertexAndConstantBuffer, GpuResource::Flags::None, 
                                GpuResource::Format::Unknown, size, 1, 1, -1, 1, true, NULL, pVertices );

    m_VertexBufferView.BufferLocation = m_pVertexBuffer->GetApiResource()->GetGPUVirtualAddress( );
    m_VertexBufferView.SizeInBytes = size;
}

void VertexBuffer::SetIndices(
    const void *pIndices,
    uint32 size,
    uint32 type,
    uint32 count
)
{
    m_pIndexBuffer = new GpuBuffer;
    m_pIndexBuffer->Create( GpuResource::Heap::Default, GpuResource::State::IndexBuffer, GpuResource::Flags::None, 
                                GpuResource::Format::Unknown, size, 1, 1, -1, 1, true, NULL, pIndices );

    m_IndexBufferView.BufferLocation = m_pIndexBuffer->GetApiResource()->GetGPUVirtualAddress( );
    m_IndexBufferView.Format = (type == StreamDecl::UShort) ? DXGI_FORMAT_R16_UINT : DXGI_FORMAT_R32_UINT;
    m_IndexBufferView.SizeInBytes = size;

    m_IndexCount = count;
}

void VertexBuffer::AddAttributes(
    int usage,
    StreamDecl decl
)
{
    int stride = 0;
    DXGI_FORMAT d3dFormat;

    switch ( decl.dataType )
    {
    case StreamDecl::Float:
        stride = 4 * decl.numElements;

        if ( decl.numElements == 1 )
            d3dFormat = DXGI_FORMAT_R32_FLOAT;
        else if ( decl.numElements == 2 )
            d3dFormat = DXGI_FORMAT_R32G32_FLOAT;
        else if ( decl.numElements == 3 )
            d3dFormat = DXGI_FORMAT_R32G32B32_FLOAT;
        else if ( decl.numElements == 4 )
            d3dFormat = DXGI_FORMAT_R32G32B32A32_FLOAT;
        else
            Debug::Assert( Condition( false ), "Invalid number of vertex elements" );

        break;

    case StreamDecl::UShort:
        stride = 2 * decl.numElements;
        Debug::Assert( Condition( decl.numElements == 4 ), "Only 4 ushort usage is supported" );
        d3dFormat = DXGI_FORMAT_R16G16B16A16_UINT;
        break;
    case StreamDecl::Byte:
        stride = 1 * decl.numElements;
        Debug::Assert( Condition( decl.numElements == 4 ), "Only 4 byte usage is supported" );
        d3dFormat = DXGI_FORMAT_R8G8B8A8_UINT;
        break;

    case StreamDecl::Color:
        stride = 1 * decl.numElements;
        Debug::Assert( Condition( decl.numElements == 4 ), "Only 4 byte usage is supported" );
        d3dFormat = DXGI_FORMAT_R8G8B8A8_UINT;
        break;

    default:
        Debug::Assert( Condition( false ), "Unspecified dataType type: %d", decl.dataType );
        break;
    }

    const char *pUsage;

    BYTE usageIndex;

    switch ( usage )
    {
    case StreamDecl::Positions: pUsage = "POSITION";     usageIndex = 0; break;
    case StreamDecl::Normals: pUsage = "NORMAL";       usageIndex = 0; break;
    case StreamDecl::Binormals: pUsage = "BINORMAL";     usageIndex = 0; break;
    case StreamDecl::Tangents: pUsage = "TANGENT";      usageIndex = 0; break;
    case StreamDecl::Colors: pUsage = "COLOR";        usageIndex = 0; break;
    case StreamDecl::UV0s: pUsage = "TEXCOORD";     usageIndex = 0; break;
    case StreamDecl::UV1s: pUsage = "TEXCOORD";     usageIndex = 1; break;
    case StreamDecl::BoneIndices: pUsage = "BLENDINDICES"; usageIndex = 0; break;
    case StreamDecl::BoneWeights: pUsage = "BLENDWEIGHT";  usageIndex = 0; break;

    default:
        Debug::Assert( Condition( false ), "Unspecified Usage type: %d", usage );
        break;
    }

    D3D12_INPUT_ELEMENT_DESC declaration = { pUsage, usageIndex, d3dFormat, 0, m_VertexBufferView.StrideInBytes, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };

    m_ElementDescs[m_VertexElementDesc.numElementDescs++] = declaration;

    m_VertexBufferView.StrideInBytes += stride;
}

void VertexBuffer::EndAttributes( void )
{
    m_VertexContext = RenderContexts::RegisterVertexContext( m_VertexElementDesc );
}

void VertexBuffer::Set(
    GpuDevice::CommandList *pCommandList
) const
{
    pCommandList->pList->IASetPrimitiveTopology( D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST );
    pCommandList->pList->IASetVertexBuffers( 0, 1, &m_VertexBufferView );
    pCommandList->pList->IASetIndexBuffer( &m_IndexBufferView );
}

void VertexBuffer::Submit(
    GpuDevice::CommandList *pCommandList,
    RenderStats *pStats
) const
{
    Set( pCommandList );

    pCommandList->pList->DrawIndexedInstanced( m_IndexCount, 1, 0, 0, 0 );

    pStats->num_verts = m_VertexBufferView.SizeInBytes / m_VertexBufferView.StrideInBytes;
    pStats->num_faces = m_IndexCount / 3;
}
