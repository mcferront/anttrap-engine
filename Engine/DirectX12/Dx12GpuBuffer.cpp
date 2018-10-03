#include "EnginePch.h"

#include "Dx12GpuBuffer.h"

DefineResourceType( GpuBuffer, GpuResource, NULL );

//GpuBuffer::GpuBuffer(
//    GpuBuffer::Heap::Type heapType,
//    State::Type state,
//    size_t size,
//    bool isBuffer,
//    const void *pInitialData //= NULL
//) : GpuResource( state )
//{
//    Create( heapType, state, Flags::None, Format::Unknown, size, 1, 1, -1, 1, isBuffer, NULL, pInitialData );
//}
//
//GpuBuffer::GpuBuffer(
//    GpuBuffer::Heap::Type heapType,
//    State::Type state,
//    Flags::Type flags,
//    size_t size,
//    uint32 stride,
//    bool isBuffer,
//    const void *pInitialData // = NULL
//) : GpuResource( state )
//{
//    Create( heapType, state, flags, Format::Unknown, size, 1, 1, stride, 1, isBuffer, NULL, pInitialData );
//}
//
// GpuBuffer::GpuBuffer( 
//     Heap::Type heapType, 
//     State::Type state,
//     Flags::Type flags,
//     Format::Type format, 
//     size_t width, 
//     size_t height, 
//     uint32 mipLevels,
//     uint32 sampleCount,
//     bool isBuffer,
//     const Color *pClearColor,
//     const void *pInitialData //= NULL
// ) : GpuResource( state )
// {
//     Create( heapType, state, flags, format, width, height, sampleCount, -1, mipLevels, isBuffer, pClearColor, pInitialData );
// }
//
// GpuBuffer::GpuBuffer(
//     ID3D12Resource *pResource,
//     State::Type state
// ) : GpuResource( state )
// {
//     m_pUAV = NULL;
//     m_pSRV = NULL;
//     m_pDSV = NULL;
//     m_pRTV = NULL;
//
//     m_pResource = pResource;
//
//     if (m_pResource != NULL)
//         m_pResource->AddRef();
// }

 //GpuBuffer::~GpuBuffer( void )
 //{
 //    DestroyViews( );
 //}

void GpuBuffer::Create(
    Heap::Type heapType,
    State::Type state,
    Flags::Type flags,
    Format::Type format,
    uint32 width,
    uint32 height,
    uint32 sampleCount,
    uint32 stride,
    uint32 mipLevels,
    bool isBuffer,
    const Color *pClearColor,
    const void *pInitialData// = NULL
)
{
    GpuResource::Create( state );

    m_pUAV = NULL;
    m_pSRV = NULL;
    m_pDSV = NULL;
    m_pRTV = NULL;

    D3D12_HEAP_PROPERTIES heapProperties = { };
    {
        heapProperties.Type = (D3D12_HEAP_TYPE) heapType;
        heapProperties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
        heapProperties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
        heapProperties.CreationNodeMask = 1;
        heapProperties.VisibleNodeMask = 1;
    }

    D3D12_RESOURCE_DESC resourceDesc = { };
    {
        resourceDesc.MipLevels = mipLevels;
        resourceDesc.Format = (DXGI_FORMAT) format;
        resourceDesc.Width = width;
        resourceDesc.Height = height;
        resourceDesc.Flags = (D3D12_RESOURCE_FLAGS) flags;
        resourceDesc.SampleDesc.Count = sampleCount;
        resourceDesc.DepthOrArraySize = 1;

        if ( isBuffer )
        {
            resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
            resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
        }
        else
        {
            resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
            resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
        }
    }

    D3D12_CLEAR_VALUE clearValue;

    if ( NULL != pClearColor )
    {
        if ( flags & Flags::RenderTarget )
        {
            clearValue.Format = resourceDesc.Format;
            clearValue.Color[ 0 ] = pClearColor->r / 255.0f;
            clearValue.Color[ 1 ] = pClearColor->g / 255.0f;
            clearValue.Color[ 2 ] = pClearColor->b / 255.0f;
            clearValue.Color[ 3 ] = pClearColor->a / 255.0f;
        }
        else
        {
            clearValue.Format = DXGI_FORMAT_D32_FLOAT;
            clearValue.DepthStencil.Depth = pClearColor->r / 255.0f;
        }
    }

    HRESULT hr = GpuDevice::Instance( ).GetDevice( )->CreateCommittedResource(
        &heapProperties,
        D3D12_HEAP_FLAG_NONE,
        &resourceDesc,
        (D3D12_RESOURCE_STATES) state,
        pClearColor != NULL ? &clearValue : NULL,
        __uuidof( ID3D12Resource ),
        (void **) &m_pResource
    );
    Debug::Assert( Condition( SUCCEEDED( hr ) ), "CreateCommittedResource: 0x%08x", hr );

    m_Stride = stride;
    m_IsBuffer = isBuffer;

    if ( NULL != pInitialData )
    {
        if ( heapType == Heap::Upload )
        {
            void *pMappedData = this->Map( );

            memcpy( pMappedData, pInitialData, width * height );

            this->Unmap( );
        }
        else
        {
            GpuBuffer *pUpload = new GpuBuffer;

            pUpload->Create( Heap::Upload, GpuResource::State::GenericRead, GpuResource::Flags::None,
                GpuResource::Format::Unknown, width * height, 1, 1, 0, 1, true, NULL, pInitialData );
            {
                GpuDevice::CommandList *pCommandList = GpuDevice::Instance( ).AllocThreadCommandList( );

                this->TransitionTo( pCommandList, GpuResource::State::CopyDest );

                GpuDevice::Instance( ).CopyResource( pCommandList, this, pUpload );

                this->TransitionTo( pCommandList, state );

                GpuDevice::Instance( ).ExecuteCommandLists( &pCommandList, 1, true );
            }
            pUpload->Destroy( );

            delete pUpload;
        }
    }
}

GpuDevice::UnorderedAccessView *GpuBuffer::GetUav( void )
{
    if ( m_pUAV )
        return m_pUAV;

    m_pUAV = new GpuDevice::UnorderedAccessView;

    D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc = { };
    {
        Debug::Assert( Condition( m_pResource->GetDesc( ).Flags & GpuResource::Flags::UnorderedAccess ), "UnorderedAccessView was not one of the flags" );

        uavDesc.Format = m_pResource->GetDesc( ).Format;

        if ( m_IsBuffer )
        {
            uavDesc.Buffer.FirstElement = 0;
            uavDesc.Buffer.Flags = D3D12_BUFFER_UAV_FLAG_NONE;
            uavDesc.Buffer.NumElements = (UINT) m_pResource->GetDesc( ).Width / m_Stride;
            uavDesc.Buffer.StructureByteStride = m_Stride;
            uavDesc.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;
        }
        else if ( DXGI_FORMAT_UNKNOWN != uavDesc.Format )
            uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;
    }

    m_pUAV->desc = uavDesc;

    GpuDevice::Instance( ).CreateUav( m_pUAV, m_pResource, false );

    return m_pUAV;
}

GpuDevice::ShaderResourceView *GpuBuffer::GetSrv( void )
{
    if ( m_pSRV )
        return m_pSRV;

    m_pSRV = new GpuDevice::ShaderResourceView;

    D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = { };
    {
        srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
        srvDesc.Texture2D.MipLevels = this->GetNumMips( );

        if ( m_pResource->GetDesc( ).Format == DXGI_FORMAT_R32_TYPELESS )
            srvDesc.Format = DXGI_FORMAT_R32_FLOAT;
        else
            srvDesc.Format = m_pResource->GetDesc( ).Format;

        if ( m_IsBuffer )
        {
            srvDesc.Buffer.FirstElement = 0;
            srvDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;
            srvDesc.Buffer.NumElements = (UINT) m_pResource->GetDesc( ).Width / m_Stride;
            srvDesc.Buffer.StructureByteStride = m_Stride;
            srvDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
        }
        else
            srvDesc.ViewDimension = GetSampleCount( ) > 1 ? D3D12_SRV_DIMENSION_TEXTURE2DMS : D3D12_SRV_DIMENSION_TEXTURE2D;
    }

    m_pSRV->desc = srvDesc;

    GpuDevice::Instance( ).CreateSrv( m_pSRV, m_pResource );

    return m_pSRV;
}

GpuDevice::RenderTargetView *GpuBuffer::GetRtv( void )
{
    if ( m_pRTV )
        return m_pRTV;

    m_pRTV = new GpuDevice::RenderTargetView;

    D3D12_RENDER_TARGET_VIEW_DESC rtvDesc = { };
    rtvDesc.Format = m_pResource->GetDesc( ).Format;
    rtvDesc.ViewDimension = GetSampleCount( ) > 1 ? D3D12_RTV_DIMENSION_TEXTURE2DMS : D3D12_RTV_DIMENSION_TEXTURE2D;

    m_pRTV->desc = rtvDesc;

    GpuDevice::Instance( ).CreateRtv( m_pRTV, m_pResource );

    return m_pRTV;
}

GpuDevice::DepthStencilView *GpuBuffer::GetDsv( void )
{
    if ( m_pDSV )
        return m_pDSV;

    m_pDSV = new GpuDevice::DepthStencilView;

    D3D12_DEPTH_STENCIL_VIEW_DESC depthViewDesc = { };
    {
        if ( m_pResource->GetDesc( ).Format == DXGI_FORMAT_R32_TYPELESS )
            depthViewDesc.Format = DXGI_FORMAT_D32_FLOAT;
        else
            depthViewDesc.Format = m_pResource->GetDesc( ).Format;

        depthViewDesc.ViewDimension = GetSampleCount( ) > 1 ? D3D12_DSV_DIMENSION_TEXTURE2DMS : D3D12_DSV_DIMENSION_TEXTURE2D;
    }

    m_pDSV->desc = depthViewDesc;

    GpuDevice::Instance( ).CreateDsv( m_pDSV, m_pResource );

    return m_pDSV;
}

void *GpuBuffer::Map( void )
{
    void *pData;

    D3D12_RANGE range = { 0, 0 };

    HRESULT hr = m_pResource->Map( 0, &range, (void **) &pData );
    Debug::Assert( Condition( SUCCEEDED( hr ) ), "Map: 0x%08x", hr );

    return pData;
}

void GpuBuffer::Unmap( void )
{
    m_pResource->Unmap( 0, NULL );
}

void GpuBuffer::RemoveFromScene( void )
{
    Resource::RemoveFromScene( );
    DestroyViews( );
}

 void GpuBuffer::Create(
     ID3D12Resource *pResource,
     State::Type state
 )
 {
     GpuResource::Create( state );

     m_pUAV = NULL;
     m_pSRV = NULL;
     m_pDSV = NULL;
     m_pRTV = NULL;

     m_pResource = pResource;

     if (m_pResource != NULL)
         m_pResource->AddRef();
 }

void GpuBuffer::SwapApiResource(
    ID3D12Resource *pResource,
    State::Type state
)
{
    DestroyViews( );

    GpuResource::SwapApiResource( pResource, state );
}


void GpuBuffer::DestroyViews( void )
{
    GpuDevice::Instance( ).DestroyDsv( m_pDSV );
    GpuDevice::Instance( ).DestroyRtv( m_pRTV );
    GpuDevice::Instance( ).DestroyUav( m_pUAV );
    GpuDevice::Instance( ).DestroySrv( m_pSRV );

    free( m_pDSV );
    free( m_pRTV );
    free( m_pUAV );
    free( m_pSRV );

    m_pDSV = NULL;
    m_pRTV = NULL;
    m_pUAV = NULL;
    m_pSRV = NULL;
}

ResourceHandle GpuBuffer::CreateBuffer(
    const Id &id,
    Flags::Type flags,
    State::Type state,
    uint32 size,
    uint32 stride,
    bool cpuReadback // = false
)
{
    GpuBuffer *pBuffer = new GpuBuffer;
    pBuffer->Create( cpuReadback ? GpuResource::Heap::Readback : GpuResource::Heap::Default,
        state, flags, GpuResource::Format::Unknown, size, 1, 1, stride, 1, true, NULL );

    ResourceHandle handle( id );

    handle.Bind( NULL, pBuffer );
    pBuffer->AddToScene( );

    return handle;
}

ResourceHandle GpuBuffer::CreateTexture(
    const Id &id,
    GpuResource::Format::Type format,
    GpuResource::Flags::Type flags,
    GpuResource::State::Type state,
    size_t width,
    size_t height,
    uint32 mipLevels, // = 1
    const Color *pClearColor, // = NULL
    uint32 sampleCount // = 1
)
{
    GpuBuffer *pBuffer = new GpuBuffer;
    pBuffer->Create( GpuResource::Heap::Default, state, flags, format, width, height, sampleCount, -1, mipLevels, false, pClearColor );

    ResourceHandle handle( id );

    handle.Bind( NULL, pBuffer );
    pBuffer->AddToScene( );

    return handle;
}