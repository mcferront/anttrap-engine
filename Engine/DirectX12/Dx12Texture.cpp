#include "EnginePch.h"

#include "Dx12Texture.h"
#include "Dx12.h"
#include "Log.h"

DefineResourceType( Texture, Asset, new TextureSerializer );

void Texture::CreateAsTarget( 
   Texture::Format::Type format,
   Texture::ViewType type,
   const Color &clearColor,
   uint32 width,
   uint32 height,
   byte sampleCount,
	byte mipLevels // = 1
)
{
   ID3D12Resource *pColor = NULL;
   ID3D12DescriptorHeap *pColorDescHeap = NULL;
   
   HRESULT hr;

   D3D12_HEAP_PROPERTIES heapProperties = { };
   heapProperties.Type = D3D12_HEAP_TYPE_DEFAULT;
   heapProperties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
   heapProperties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
   heapProperties.CreationNodeMask = 1;
   heapProperties.VisibleNodeMask = 1;

   // color
   D3D12_RESOURCE_DESC colorDesc = { };
   {
      colorDesc.MipLevels = mipLevels;
      colorDesc.Format = (DXGI_FORMAT) format;
      colorDesc.Width = width;
      colorDesc.Height = height;
      colorDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;
      
      // TODO: DX12 -we should know if it needs uav or not
      if ( sampleCount == 1 )
         colorDesc.Flags |= D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;

      colorDesc.DepthOrArraySize = 1;
      colorDesc.SampleDesc.Count = sampleCount;
      colorDesc.SampleDesc.Quality = sampleCount > 1 ? DXGI_STANDARD_MULTISAMPLE_QUALITY_PATTERN : 0;
      colorDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;

      D3D12_CLEAR_VALUE clearValue;
      clearValue.Format = colorDesc.Format;
      clearValue.Color[ 0 ] = clearColor.r / 255.0f;
      clearValue.Color[ 1 ] = clearColor.g / 255.0f;
      clearValue.Color[ 2 ] = clearColor.b / 255.0f;
      clearValue.Color[ 3 ] = clearColor.a / 255.0f;

      hr = GpuDevice::Instance( ).GetDevice( )->CreateCommittedResource(
         &heapProperties,
         D3D12_HEAP_FLAG_NONE,
         &colorDesc,
         (D3D12_RESOURCE_STATES) type,
         &clearValue,
         __uuidof( ID3D12Resource ),
         (void **) &pColor );
      
      Debug::Assert( Condition( SUCCEEDED( hr ) ), "Failed to CreateCommittedResource (0x%08x)", hr );
   
      
      D3D12_DESCRIPTOR_HEAP_DESC srvHeapDesc = { };
      srvHeapDesc.NumDescriptors = 1;
      srvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
      srvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
      
      hr = GpuDevice::Instance( ).GetDevice( )->CreateDescriptorHeap( &srvHeapDesc, __uuidof( ID3D12DescriptorHeap ), (void **) &pColorDescHeap );
      Debug::Assert( Condition( SUCCEEDED( hr ) ), "Failed to CreateDescriptorHeap (0x%08x)", hr );

      D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = pColorDescHeap->GetCPUDescriptorHandleForHeapStart( );
      GpuDevice::Instance( ).GetDevice( )->CreateRenderTargetView( pColor, NULL, rtvHandle );
   }

   m_NumMips = mipLevels;
   m_DSFormat = DXGI_FORMAT_UNKNOWN;
   m_RTFormat = format;
   m_UAVFormat = sampleCount == 1 ? format : DXGI_FORMAT_UNKNOWN;
   m_pDescHeap = pColorDescHeap;
   m_pResource = pColor;
   m_ViewType = type;
   m_HasCounter = false;
   m_CounterOffset = 0;
   m_StructuredBufferSize = 0;

   m_pResource->SetName( String::ToWideChar(GetId().ToString()) );
}

void Texture::CreateAsDepthStencil(
   uint32 width,
   uint32 height,
   byte sampleCount
   )
{
   ID3D12Resource *pDepthStencil = NULL;
   ID3D12DescriptorHeap *pDepthStencilDescHeap = NULL;

   HRESULT hr;
   
   D3D12_HEAP_PROPERTIES heapProperties = { };
   heapProperties.Type = D3D12_HEAP_TYPE_DEFAULT;
   heapProperties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
   heapProperties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
   heapProperties.CreationNodeMask = 1;
   heapProperties.VisibleNodeMask = 1;

   // depth
   D3D12_RESOURCE_DESC depthStencilDesc = { };
   {
      depthStencilDesc.MipLevels = 1;
      depthStencilDesc.Format = DXGI_FORMAT_R32_TYPELESS;
      depthStencilDesc.Width = width;
      depthStencilDesc.Height = height;
      depthStencilDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

      depthStencilDesc.DepthOrArraySize = 1;
      depthStencilDesc.SampleDesc.Count = sampleCount;
      depthStencilDesc.SampleDesc.Quality = sampleCount > 1 ? DXGI_STANDARD_MULTISAMPLE_QUALITY_PATTERN : 0;
      depthStencilDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;

      D3D12_CLEAR_VALUE clearValue = { };
      clearValue.Format = DXGI_FORMAT_D32_FLOAT;
      clearValue.DepthStencil.Depth = 0.0f;
      
      hr = GpuDevice::Instance( ).GetDevice( )->CreateCommittedResource(
         &heapProperties,
         D3D12_HEAP_FLAG_NONE,
         &depthStencilDesc,
         D3D12_RESOURCE_STATE_DEPTH_WRITE,
         &clearValue,
         __uuidof( ID3D12Resource ),
         (void **) &pDepthStencil );
      Debug::Assert( Condition( SUCCEEDED( hr ) ), "Failed to CreateCommittedResource (0x%08x)", hr );

      D3D12_DESCRIPTOR_HEAP_DESC srvHeapDesc = { };
      srvHeapDesc.NumDescriptors = 1;
      srvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
      srvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
      hr = GpuDevice::Instance( ).GetDevice( )->CreateDescriptorHeap( &srvHeapDesc, __uuidof( ID3D12DescriptorHeap ), (void **) &pDepthStencilDescHeap );
      Debug::Assert( Condition( SUCCEEDED( hr ) ), "Failed to CreateDescriptorHeap (0x%08x)", hr );
   }
    
   D3D12_DEPTH_STENCIL_VIEW_DESC depthViewDesc = { };
   {
      depthViewDesc.Format = DXGI_FORMAT_D32_FLOAT;
      depthViewDesc.ViewDimension = sampleCount > 1 ? D3D12_DSV_DIMENSION_TEXTURE2DMS : D3D12_DSV_DIMENSION_TEXTURE2D;
   }

   D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = pDepthStencilDescHeap->GetCPUDescriptorHandleForHeapStart( );
   GpuDevice::Instance( ).GetDevice( )->CreateDepthStencilView( pDepthStencil, &depthViewDesc, rtvHandle );

   m_NumMips = 1;
   m_DSFormat = depthViewDesc.Format;
   m_RTFormat = DXGI_FORMAT_R32_FLOAT;
   m_UAVFormat = DXGI_FORMAT_UNKNOWN;
   m_pDescHeap = pDepthStencilDescHeap;
   m_pResource = pDepthStencil;
   m_ViewType = DepthWriteResource;
   m_HasCounter = false;
   m_CounterOffset = 0;
   m_StructuredBufferSize = 0;

   m_pResource->SetName( String::ToWideChar(GetId().ToString()) );
}

void Texture::CreateAsStructuredBuffer(
   uint32 size,
   bool appendCounter //= false
)
{
   ID3D12Resource *pBuffer = NULL;
   ID3D12DescriptorHeap *pColorDescHeap = NULL;

   HRESULT hr;

   D3D12_HEAP_PROPERTIES heapProperties = { };
   heapProperties.Type = D3D12_HEAP_TYPE_DEFAULT;
   heapProperties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
   heapProperties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
   heapProperties.CreationNodeMask = 1;
   heapProperties.VisibleNodeMask = 1;

   uint32 counter_offset = 0;
   uint32 final_size = size;

   if ( appendCounter )
   {
      counter_offset = Align(size, D3D12_UAV_COUNTER_PLACEMENT_ALIGNMENT);
      final_size += (counter_offset - final_size) + 4;
   }

   D3D12_RESOURCE_DESC bufferDesc = { };
   {
      bufferDesc.MipLevels = 1;
      bufferDesc.Format = DXGI_FORMAT_UNKNOWN;
      bufferDesc.Width = final_size;
      bufferDesc.Height = 1;
      bufferDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
      bufferDesc.SampleDesc.Count = 1;
      bufferDesc.DepthOrArraySize = 1;
      bufferDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
      bufferDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;

      hr = GpuDevice::Instance( ).GetDevice( )->CreateCommittedResource(
         &heapProperties,
         D3D12_HEAP_FLAG_NONE,
         &bufferDesc,
         D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
         NULL,
         __uuidof( ID3D12Resource ),
         (void **) &pBuffer );

      Debug::Assert( Condition( SUCCEEDED( hr ) ), "Failed to CreateCommittedResource (0x%08x)", hr );
   }

   m_NumMips = 1;
   m_DSFormat = DXGI_FORMAT_UNKNOWN;
   m_RTFormat = DXGI_FORMAT_UNKNOWN;
   m_UAVFormat = DXGI_FORMAT_UNKNOWN;
   m_pDescHeap = NULL;
   m_pResource = pBuffer;
   m_ViewType = UavResource;
   m_HasCounter = appendCounter;
   m_CounterOffset = counter_offset;
   m_StructuredBufferSize = size;

   m_pResource->SetName( String::ToWideChar(GetId().ToString()) );
}

void Texture::CreateAsCopyDest(
    uint32 size
)
{
    ID3D12Resource *pBuffer = NULL;
    ID3D12DescriptorHeap *pColorDescHeap = NULL;

    HRESULT hr;

    D3D12_HEAP_PROPERTIES heapProperties = { };
    heapProperties.Type = D3D12_HEAP_TYPE_READBACK;
    heapProperties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
    heapProperties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
    heapProperties.CreationNodeMask = 1;
    heapProperties.VisibleNodeMask = 1;

    D3D12_RESOURCE_DESC bufferDesc = { };
    {
        bufferDesc.MipLevels = 1;
        bufferDesc.Format = DXGI_FORMAT_UNKNOWN;
        bufferDesc.Width = size;
        bufferDesc.Height = 1;
        bufferDesc.Flags = D3D12_RESOURCE_FLAG_NONE;
        bufferDesc.SampleDesc.Count = 1;
        bufferDesc.DepthOrArraySize = 1;
        bufferDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
        bufferDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;

        hr = GpuDevice::Instance( ).GetDevice( )->CreateCommittedResource(
            &heapProperties,
            D3D12_HEAP_FLAG_NONE,
            &bufferDesc,
            D3D12_RESOURCE_STATE_COPY_DEST,
            NULL,
            __uuidof( ID3D12Resource ),
            (void **) &pBuffer );

        Debug::Assert( Condition( SUCCEEDED( hr ) ), "Failed to CreateCommittedResource (0x%08x)", hr );
    }

    m_NumMips = 1;
    m_DSFormat = DXGI_FORMAT_UNKNOWN;
    m_RTFormat = DXGI_FORMAT_UNKNOWN;
    m_UAVFormat = DXGI_FORMAT_UNKNOWN;
    m_pDescHeap = NULL;
    m_pResource = pBuffer;
    m_ViewType = CopyDest;
    m_HasCounter = false;
    m_CounterOffset = 0;
    m_StructuredBufferSize = size;

    m_pResource->SetName( String::ToWideChar(GetId().ToString()) );
}

void Texture::Bind( void )
{
   bool isBuffer = m_pResource->GetDesc( ).Dimension == D3D12_RESOURCE_DIMENSION_BUFFER;

   m_GpuHandle = GpuDevice::Instance( ).CreateGpuHandle( GetId() );

   D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = { };
   {
      srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
      srvDesc.Format = (DXGI_FORMAT) m_RTFormat;
      srvDesc.Texture2D.MipLevels = m_NumMips;

      if ( isBuffer )
      {
         srvDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
         srvDesc.Buffer.FirstElement = 0;
         srvDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;
         srvDesc.Buffer.NumElements = (UINT) (m_pResource->GetDesc( ).Width / sizeof(float));
         srvDesc.Buffer.StructureByteStride = sizeof(float);
      }
      else
      {
         srvDesc.ViewDimension = GetSampleCount( ) > 1 ? D3D12_SRV_DIMENSION_TEXTURE2DMS : D3D12_SRV_DIMENSION_TEXTURE2D;
      }

      // how to fix.. create gpu handle on bind
      // but then lazily create these when called (can be threaded)
      // just make sure the cpuoffsetinc in dx12 uses atomics
      GpuDevice::Instance( ).SetGpuSrv( m_GpuHandle, this->GetD3D12Resource(), srvDesc );
   }



   D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc = { };
   {
      uavDesc.Format = (DXGI_FORMAT) m_UAVFormat;

      if ( isBuffer )
      {
         uavDesc.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;
         uavDesc.Buffer.FirstElement = 0;
         uavDesc.Buffer.Flags = D3D12_BUFFER_UAV_FLAG_NONE;
         uavDesc.Buffer.NumElements = (UINT) m_StructuredBufferSize / sizeof(float);
         uavDesc.Buffer.StructureByteStride = sizeof(float);
         uavDesc.Buffer.CounterOffsetInBytes = m_CounterOffset;

         GpuDevice::Instance( ).SetGpuUav( m_GpuHandle, this->GetD3D12Resource(), uavDesc, m_HasCounter );
      }
      else if ( DXGI_FORMAT_UNKNOWN != m_UAVFormat )
      {
         uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;
         GpuDevice::Instance( ).SetGpuUav( m_GpuHandle, this->GetD3D12Resource(), uavDesc );
      }
   }

   Asset::Bind( );
}

void Texture::ConvertTo( 
   Texture::ViewType viewType,
   GpuDevice::CommandList *pCommandList
   )
{
   ScopeLock lock( m_ConvertLock );

   if ( viewType != m_ViewType )
   {
      D3D12_RESOURCE_TRANSITION_BARRIER source = { };
      source.pResource = m_pResource;
      source.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
      source.StateBefore = (D3D12_RESOURCE_STATES) m_ViewType;
      source.StateAfter = (D3D12_RESOURCE_STATES) viewType;

      D3D12_RESOURCE_BARRIER barrier = { };
      barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
      barrier.Transition = source;

      pCommandList->pList->ResourceBarrier( 1, &barrier );

      m_ViewType = viewType;
   }
}

void Texture::Barrier(
   Barrier::Type type,
   GpuDevice::CommandList *pCommandList
)
{
   if ( type == Barrier::Uav )
   {
      D3D12_RESOURCE_UAV_BARRIER source = { };
      source.pResource = m_pResource;

      D3D12_RESOURCE_BARRIER barrier = { };
      barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_UAV;
      barrier.UAV = source;

      pCommandList->pList->ResourceBarrier( 1, &barrier );
   }
   else
      Debug::Assert( Condition(false), "Unrecognized barrier type: %d", type );
}

ISerializable *TextureSerializer::Deserialize(
   Serializer *pSerializer,
   ISerializable *pSerializable
   )
{
   const uint32 Version = 1;

   struct Header
   {
      uint32 version;
      uint32 desiredWidth, desiredHeight;
      uint32 actualWidth, actualHeight;
      uint32 size, format, type, compressed;
      uint32 pixelFormat, mipLevels;
   };

   Header header;

   if ( NULL == pSerializable ) pSerializable = Instantiate( );

   Texture *pTexture = (Texture *) pSerializable;

   pSerializer->GetInputStream( )->Read( &header, sizeof( header ), NULL );
   Debug::Assert( Condition( header.version == Version ), "Incorrect texture version: %d, expecting: %d", header.version, Version );

   pSerializer->GetInputStream( )->Seek( header.mipLevels * sizeof( int ), SeekCurrent );

   DXGI_FORMAT format;

   //TODO: DX12 - real format support
   switch ( header.format )
   {
      case 21:
         format = DXGI_FORMAT_R8G8B8A8_UNORM;
         break;

      case MAKEFOURCC('D', 'X', 'T', '1'):
         format = DXGI_FORMAT_BC1_UNORM_SRGB;
         break;
      
      case MAKEFOURCC('D', 'X', 'T', '2'):
      case MAKEFOURCC('D', 'X', 'T', '3'):
         format = DXGI_FORMAT_BC2_UNORM_SRGB;
         break;

      case MAKEFOURCC('D', 'X', 'T', '4'):
         format = DXGI_FORMAT_BC3_UNORM_SRGB;
         break;
      case MAKEFOURCC('D', 'X', 'T', '5'):
         format = DXGI_FORMAT_BC3_UNORM;
         break;

      default:
         Debug::Assert( Condition(false), "Unsupported texture format" );
   }

   D3D12_RESOURCE_DESC textureDesc = { };
   textureDesc.MipLevels = header.mipLevels;
   textureDesc.Format = format;
   textureDesc.Width = header.actualWidth;
   textureDesc.Height = header.actualHeight;
   textureDesc.Flags = D3D12_RESOURCE_FLAG_NONE;
   textureDesc.DepthOrArraySize = 1;
   textureDesc.SampleDesc.Count = 1;
   textureDesc.SampleDesc.Quality = 0;
   textureDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;

   D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = { };
   srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
   srvDesc.Format = textureDesc.Format;
   srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
   srvDesc.Texture2D.MipLevels = header.mipLevels;

   pTexture->Create( );
   pTexture->m_ViewType = Texture::PixelShaderResource;
   pTexture->m_SRVDesc = srvDesc;
   pTexture->m_DSFormat = DXGI_FORMAT_UNKNOWN;
   pTexture->m_RTFormat = srvDesc.Format;
   pTexture->m_UAVFormat = DXGI_FORMAT_UNKNOWN;

   pTexture->m_NumMips = header.mipLevels;

   D3D12_HEAP_PROPERTIES heapProperties = { };
   heapProperties.Type = D3D12_HEAP_TYPE_DEFAULT;
   heapProperties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
   heapProperties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
   heapProperties.CreationNodeMask = 1;
   heapProperties.VisibleNodeMask = 1;

   ID3D12Resource *pResource = NULL;
   ID3D12Resource *pTextureUploadHeap = NULL;
   GpuDevice::CommandList *pCommandList = NULL;
   ID3D12Fence *pFence = NULL;
   uint32 *pNumRows = NULL;
   uint64 *pRowSizeInBytes = NULL;
   D3D12_PLACED_SUBRESOURCE_FOOTPRINT *pDestLayouts = NULL;

   HRESULT hr;

   do
   {
      pCommandList = new GpuDevice::CommandList;

      // command list to copy it up to the GPU
      hr = GpuDevice::Instance( ).GetDevice( )->CreateCommandAllocator( D3D12_COMMAND_LIST_TYPE_DIRECT, __uuidof( ID3D12CommandAllocator ), (void **) &pCommandList->pAllocator );
      Debug::Assert( Condition( SUCCEEDED( hr ) ), "Failed to m_pDevice->CreateCommandAllocator (0x%08x)", hr );

      hr = GpuDevice::Instance( ).GetDevice( )->CreateCommandList( 0, D3D12_COMMAND_LIST_TYPE_DIRECT, pCommandList->pAllocator, NULL, __uuidof( ID3D12GraphicsCommandList ), (void **) &pCommandList->pList );
      Debug::Assert( Condition( SUCCEEDED( hr ) ), "Failed to CreateCommandList (0x%08x)", hr );

      // Allocate memory on the GPU side
      hr = GpuDevice::Instance( ).GetDevice( )->CreateCommittedResource(
         &heapProperties,
         D3D12_HEAP_FLAG_NONE,
         &textureDesc,
         D3D12_RESOURCE_STATE_COPY_DEST,
         NULL,
         __uuidof( ID3D12Resource ),
         (void **) &pResource );

      //LOG( "CreateCommittedResource" );
      Debug::Assert( Condition( SUCCEEDED( hr ) ), "Failed to CreateCommittedResource (0x%08x)", hr );
      BreakIf( FAILED( hr ) );

      // Ask the device for the layout of the data so we know how to copy it
      uint64 requiredSize;
      pNumRows = (uint32 *) malloc( sizeof( uint32 ) * header.mipLevels );
      pRowSizeInBytes = (uint64 *) malloc( sizeof( uint64 ) * header.mipLevels );
      pDestLayouts = (D3D12_PLACED_SUBRESOURCE_FOOTPRINT *) malloc( sizeof( D3D12_PLACED_SUBRESOURCE_FOOTPRINT ) * header.mipLevels );

      GpuDevice::Instance( ).GetDevice( )->GetCopyableFootprints( &pResource->GetDesc( ), 0, header.mipLevels, 0, pDestLayouts, pNumRows, pRowSizeInBytes, &requiredSize );

      heapProperties.Type = D3D12_HEAP_TYPE_UPLOAD;

      D3D12_RESOURCE_DESC resourceDesc = { };
      resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
      resourceDesc.Alignment = 0;
      resourceDesc.Width = requiredSize;
      resourceDesc.Height = 1;
      resourceDesc.DepthOrArraySize = 1;
      resourceDesc.MipLevels = 1;
      resourceDesc.Format = DXGI_FORMAT_UNKNOWN;
      resourceDesc.SampleDesc.Count = 1;
      resourceDesc.SampleDesc.Quality = 0;
      resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
      resourceDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

      // Create the GPU upload buffer
      hr = GpuDevice::Instance( ).GetDevice( )->CreateCommittedResource(
         &heapProperties,
         D3D12_HEAP_FLAG_NONE,
         &resourceDesc,
         D3D12_RESOURCE_STATE_GENERIC_READ,
         NULL,
         __uuidof( ID3D12Resource ),
         (void **) &pTextureUploadHeap );
      //LOG( "CreateCommittedResource" );
      Debug::Assert( Condition( SUCCEEDED( hr ) ), "Failed to CreateCommittedResource (0x%08x)", hr );
      BreakIf( FAILED( hr ) );

      byte *pHead;
      hr = pTextureUploadHeap->Map( 0, NULL, (void **) &pHead );
      //LOG( "pTextureUploadHeap->Map" );
      Debug::Assert( Condition( SUCCEEDED( hr ) ), "Failed to pTextureUploadHeap->Map (0x%08x)", hr );
      BreakIf( FAILED( hr ) );

      // Copy the data to the upload buffer
      for ( uint32 i = 0; i < header.mipLevels; i++ )
      {
         uint32 c;

         uint32 numRows = pNumRows[ i ];
         uint64 rowSizeInBytes = pRowSizeInBytes[ i ];
         
         byte *pData = pHead + pDestLayouts[ i ].Offset;

         for ( c = 0; c < numRows; c++ )
         {
            pSerializer->GetInputStream( )->Read( pData, (uint32) rowSizeInBytes, NULL );
            pData += pDestLayouts[ i ].Footprint.RowPitch;
         }
      }

      pTextureUploadHeap->Unmap( 0, NULL );

      // copy each mip level over to the texture
      for ( uint32 i = 0; i < header.mipLevels; i++ )
      {
         D3D12_TEXTURE_COPY_LOCATION dest = { };
         dest.pResource = pResource;
         dest.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
         dest.SubresourceIndex = i;

         // tell the GPU how to interpret this unknown buffer
         // by giving it the layout desc of the texture it contains
         D3D12_TEXTURE_COPY_LOCATION source = { };
         source.pResource = pTextureUploadHeap;
         source.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
         source.PlacedFootprint = pDestLayouts[ i ];

         pCommandList->pList->CopyTextureRegion( &dest, 0, 0, 0, &source, NULL );
      }

      // Ask the GPU to transition the texture memory type from copy to a shader resource
      D3D12_RESOURCE_TRANSITION_BARRIER transition = { };
      transition.pResource = pResource;
      transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
      transition.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST;
      transition.StateAfter = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;

      D3D12_RESOURCE_BARRIER barrier = { };
      barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
      barrier.Transition = transition;

      pCommandList->pList->ResourceBarrier( 1, &barrier );

      hr = pCommandList->pList->Close( );
      //LOG( "pCommandList->Close()" );
      Debug::Assert( Condition( SUCCEEDED( hr ) ), "Failed to pCommandList->Close() (0x%08x)", hr );
      BreakIf( FAILED( hr ) );

      ID3D12CommandList* ppCommandLists[ ] = { pCommandList->pList };
      GpuDevice::Instance( ).GetGraphicsQueue( )->ExecuteCommandLists( _countof( ppCommandLists ), ppCommandLists );

      hr = GpuDevice::Instance( ).GetDevice( )->CreateFence( 0, D3D12_FENCE_FLAG_NONE, __uuidof( ID3D12Fence ), (void **) &pFence );
      //LOG( "CreateFence" );
      Debug::Assert( Condition( SUCCEEDED( hr ) ), "Failed to CreateFence (0x%08x)", hr );
      BreakIf( FAILED( hr ) );

      HANDLE fenceEvent = CreateEvent( nullptr, FALSE, FALSE, nullptr );

      GpuDevice::Instance( ).GetGraphicsQueue( )->Signal( pFence, 1 );

      if ( pFence->GetCompletedValue( ) < 1 )
      {
         hr = pFence->SetEventOnCompletion( 1, fenceEvent );
         Debug::Assert( Condition( SUCCEEDED( hr ) ), "Failed to SetEventOnCompletion (0x%08x)", hr );
         BreakIf( FAILED( hr ) );

         WaitForSingleObject( fenceEvent, INFINITE );
      }

      CloseHandle( fenceEvent );

   } while ( false );

   free( pNumRows );
   free( pRowSizeInBytes );
   free( pDestLayouts );

   delete pCommandList;

   if ( NULL != pTextureUploadHeap )
      pTextureUploadHeap->Release( );

   if ( NULL != pFence )
      pFence->Release( );

   if ( SUCCEEDED( hr ) )
      pTexture->m_pResource = pResource;
   else
   {
      if ( NULL != pResource )
         pResource->Release( );
   }

   return pTexture;
}
