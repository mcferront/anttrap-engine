   #pragma once

#include "EnginePch.h"

#include "Dx12.h"
#include "Dx12Contexts.h"
#include "Log.h"
#include "SystemId.h"
#include "TextureAsset.h"
#include "GpuProfiler.h"

GpuDevice::GpuHandle GpuDevice::GpuHandle::Invalid;

bool GpuDevice::CommandList::SetSamplePositions(
   const Vector2 *pPositions,
   uint32 numPositions
)
{
   if ( false == hasProgrammableSamplePositionsSupport )
   {
      static bool warned;

      if (false == warned)
         Debug::Print( Debug::TypeWarning, "SetSamplePositions is not supported on this device" );
      
      warned = true;
      return false;
   }

   if ( 0 == numPositions )
      pList1->SetSamplePositions( 0, 0, NULL );
   else
   {
      const byte maxPositions = 16;

      D3D12_SAMPLE_POSITION positions[maxPositions];
      Debug::Assert( Condition(numPositions <= maxPositions), "Too many custom sample positions" );

      for (uint32 i = 0; i < numPositions; i++)
      {
         positions[i].X = Math::FloatToInt(pPositions[i].x);
         positions[i].Y = Math::FloatToInt(pPositions[i].y);
      }

      pList1->SetSamplePositions( numPositions, 1, positions );
   }

   return true;
}

GpuDevice &GpuDevice::Instance( )
{
   static GpuDevice instance;
   return instance;
}

void GpuDevice::ClearRenderTarget(
   GpuDevice::CommandList *pCommandList,
   ImageBuffer *pRenderTarget,
   bool clearColor,
   const Color *pColorValue
)
{
   if ( true == clearColor )
   {
      D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = pRenderTarget->GetDescHeap( )->GetCPUDescriptorHandleForHeapStart( );

      const float clearColor[ ] = { pColorValue->r / 255.0f, pColorValue->g / 255.0f, pColorValue->b / 255.0f, pColorValue->a / 255.0f };
      pCommandList->pList->ClearRenderTargetView( rtvHandle, clearColor, 0, NULL );
   }
}

void GpuDevice::ClearDepthStencil(
   GpuDevice::CommandList *pCommandList,
   ImageBuffer *pDepthStencil,
   bool clearDepth,
   float depthValue,
   bool clearStencil,
   byte stencilValue
)
{
   uint32 flags = 0;

   if ( clearDepth ) flags |= D3D12_CLEAR_FLAG_DEPTH;
   if ( clearStencil ) flags |= D3D12_CLEAR_FLAG_STENCIL;

   if ( 0 != flags )
   {
      D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = pDepthStencil->GetDescHeap( )->GetCPUDescriptorHandleForHeapStart( );

      pCommandList->pList->ClearDepthStencilView( rtvHandle, (D3D12_CLEAR_FLAGS) flags, depthValue, stencilValue, 0, NULL );
   }
}

void GpuDevice::SetRenderTargets(
   GpuDevice::CommandList *pCommandList,
   ImageBuffer *pRenderTargets[],
   uint32 numRenderTargets,
   ImageBuffer *pDepthStencil
)
{
   D3D12_CPU_DESCRIPTOR_HANDLE rtvHandles[ RenderContexts::MaxRenderTargets ];
   D3D12_CPU_DESCRIPTOR_HANDLE depthHandle;

   for ( uint32 i = 0; i < numRenderTargets; i++ )
      rtvHandles[i] = pRenderTargets[i]->GetDescHeap( )->GetCPUDescriptorHandleForHeapStart( );
   
   if ( NULL != pDepthStencil )
      depthHandle = pDepthStencil->GetDescHeap( )->GetCPUDescriptorHandleForHeapStart( );;

   pCommandList->pList->OMSetRenderTargets( numRenderTargets,
                                            rtvHandles, 
                                            FALSE, 
                                            pDepthStencil ? &depthHandle : NULL );
}

void GpuDevice::CopyResource(
   GpuDevice::CommandList *pList,
   ImageBuffer *pDest,
   ImageBuffer *pSource
   )
{
   pList->pList->CopyResource( pDest->GetD3D12Resource( ), pSource->GetD3D12Resource( ) );
}

bool GpuDevice::Create(
   HWND hwnd,
   bool windowed,
   uint32 width,
   uint32 height
   )
{
   HRESULT hr;

   IDXGIAdapter1 *pAdapter = NULL;

   m_pDevice = NULL;
   m_pGraphicsQueue = NULL;
   m_pComputeQueue = NULL;
   m_pSwapChain = NULL;
   m_pRTVDescHeap = NULL;
   m_pDescHeap = NULL;
   m_pGraphicsFence = NULL;
   m_pComputeFence = NULL;
   m_NumGraphicsCommandLists = 0;
   m_NumComputeCommandLists = 0;
   m_NumDescriptors = 0;
   m_FenceCount = 0;
   m_PresentInterval = 1;

   m_ProgrammableSamplePositionsSupport = false;

   for ( uint32 i = 0; i < FrameCount; i++ )
   {
      m_FrameResources[ i ].resources.Create( );
      m_FrameResources[ i ].numGraphicsCommandLists = 0;
      m_FrameResources[ i ].numComputeCommandLists = 0;
   }

   memset( m_pGraphicsPool, 0, sizeof(m_pGraphicsPool) );
   memset( m_pComputePool, 0, sizeof(m_pComputePool) );
   memset( m_pRenderTargets, 0, sizeof( m_pRenderTargets ) );

   m_ViewOffsets.Create( 4096, 4096, IdHash, IdCompare );
   m_FreeViewOffsets.Create( 4096, 1024 );

   do
   {
#if defined(_DEBUG)
      {
         ID3D12Debug1 *pDebugController;

         hr = D3D12GetDebugInterface( __uuidof( ID3D12Debug1 ), (void **) &pDebugController );
         Debug::Assert( Condition( SUCCEEDED( hr ) ), "Failed to D3D12GetDebugInterface (0x%08x)", hr );

         if ( SUCCEEDED(hr) )
         {
            //pDebugController->SetEnableGPUBasedValidation( TRUE );
            pDebugController->EnableDebugLayer( );
            pDebugController->Release( );
         }
      }
#endif

      hr = CreateDXGIFactory1( __uuidof( IDXGIFactory4 ), (void **) &m_pFactory );
      Debug::Assert( Condition( SUCCEEDED( hr ) ), "Failed to CreateDXGIFactory1 (0x%08x)", hr );

      unsigned int i = 0;
      hr = E_FAIL;

      while ( DXGI_ERROR_NOT_FOUND != m_pFactory->EnumAdapters1( i++, &pAdapter ) )
      {
         DXGI_ADAPTER_DESC1 desc;
         pAdapter->GetDesc1( &desc );

         if ( (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE) != 0 )
            continue;

         //LOG( "D3D12CreateDevice..." );
         hr = D3D12CreateDevice( pAdapter, D3D_FEATURE_LEVEL_12_0, __uuidof( ID3D12Device ), (void **) &m_pDevice );

         pAdapter->Release( );
         pAdapter = NULL;

         BreakIf( SUCCEEDED( hr ) );

         m_pDevice = NULL;
      }

      BreakIf( NULL == m_pDevice );

      D3D12_COMMAND_QUEUE_DESC queueDesc = { };
      queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;

      queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
      hr = m_pDevice->CreateCommandQueue( &queueDesc, __uuidof( ID3D12CommandQueue ), (void **) &m_pGraphicsQueue );
      Debug::Assert( Condition( SUCCEEDED( hr ) ), "Failed to CreateCommandQueue (0x%08x)", hr );
      BreakIf( FAILED( hr ) );

      queueDesc.Type = D3D12_COMMAND_LIST_TYPE_COMPUTE;
      hr = m_pDevice->CreateCommandQueue( &queueDesc, __uuidof( ID3D12CommandQueue ), (void **) &m_pComputeQueue );
      Debug::Assert( Condition( SUCCEEDED( hr ) ), "Failed to CreateCommandQueue (0x%08x)", hr );
      BreakIf( FAILED( hr ) );

      // Create descriptor heaps.
      {
         // Describe and create a render target view (RTV) descriptor heap.
         D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = { };
         rtvHeapDesc.NumDescriptors = FrameCount;
         rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
         rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;

         //LOG( "CreateDescriptorHeap..." );
         hr = m_pDevice->CreateDescriptorHeap( &rtvHeapDesc, __uuidof( ID3D12DescriptorHeap ), (void **) &m_pRTVDescHeap );
         Debug::Assert( Condition( SUCCEEDED( hr ) ), "Failed to CreateDescriptorHeap (0x%08x)", hr );
         BreakIf( FAILED( hr ) );

         //LOG( "GetDescriptorHandleIncrementSize..." );
         m_RtvDescSize = m_pDevice->GetDescriptorHandleIncrementSize( D3D12_DESCRIPTOR_HEAP_TYPE_RTV );

         // Describe and create a shader resource view (SRV) heap for the texture.
         D3D12_DESCRIPTOR_HEAP_DESC srvHeapDesc = { };
         srvHeapDesc.NumDescriptors = MaxDescriptors;
         srvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
         srvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
         hr = m_pDevice->CreateDescriptorHeap( &srvHeapDesc, __uuidof(ID3D12DescriptorHeap), (void **) &m_pDescHeap );

         Debug::Assert( Condition(SUCCEEDED(hr)), "Failed to CreateDescriptorHeap (0x%08x)", hr );
         BreakIf( FAILED(hr) );

         m_pDescHeap->SetName( String::ToWideChar("Main") );
      }

      hr = CreateSwapChain( hwnd, width, height, windowed );
      BreakIf( FAILED( hr ) );

      D3D12_FEATURE_DATA_D3D12_OPTIONS2 options;
      m_pDevice->CheckFeatureSupport( D3D12_FEATURE_D3D12_OPTIONS2, &options, sizeof(options) );

      m_ProgrammableSamplePositionsSupport = 
         options.ProgrammableSamplePositionsTier == D3D12_PROGRAMMABLE_SAMPLE_POSITIONS_TIER_1 ||
         options.ProgrammableSamplePositionsTier == D3D12_PROGRAMMABLE_SAMPLE_POSITIONS_TIER_2;

      for ( i = 0; i < MaxCommandLists; i++ )
      {
         m_pGraphicsPool[i] = CreateGraphicsCommandList( );
         m_pGraphicsPool[i]->pList->Close( );

         m_pComputePool[i] = CreateComputeCommandList( );
         m_pComputePool[i]->pList->Close( );
      }

      m_NumGraphicsCommandLists = MaxCommandLists;
      m_NumComputeCommandLists = MaxCommandLists;
      
      hr = m_pDevice->CreateFence( 0, D3D12_FENCE_FLAG_NONE, __uuidof( ID3D12Fence ), (void **) &m_pGraphicsFence );
      Debug::Assert( Condition( SUCCEEDED( hr ) ), "Failed to CreateFence (0x%08x)", hr );
      BreakIf( FAILED( hr ) );

      hr = m_pDevice->CreateFence( 0, D3D12_FENCE_FLAG_NONE, __uuidof( ID3D12Fence ), (void **) &m_pComputeFence );
      Debug::Assert( Condition( SUCCEEDED( hr ) ), "Failed to CreateFence (0x%08x)", hr );
      BreakIf( FAILED( hr ) );

      m_GraphicsFenceEvent = CreateEvent( nullptr, FALSE, FALSE, nullptr );
      m_ComputeFenceEvent = CreateEvent( nullptr, FALSE, FALSE, nullptr );

      m_CpuBaseHandle = m_pDescHeap->GetCPUDescriptorHandleForHeapStart();
      m_GpuBaseHandle = m_pDescHeap->GetGPUDescriptorHandleForHeapStart();

      m_DescHandleIncSize = m_pDevice->GetDescriptorHandleIncrementSize( D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV );

   } while ( false );

   if ( NULL != pAdapter )
      pAdapter->Release( );

   if ( FAILED( hr ) )
      Destroy( );

   return SUCCEEDED( hr );
}

void GpuDevice::Destroy( void )
{
   // must be signaled one more time
   // so it's internally released by the system
   // I don't understand why
   m_pGraphicsQueue->Signal( m_pGraphicsFence, m_FenceCount );
   m_pComputeQueue->Signal( m_pComputeFence, m_FenceCount );

   for ( uint32 i = 0; i < FrameCount; i++ )
   {
       FreePerFrameResources( i );
       m_FrameResources[ i ].resources.Destroy( );
   }

   if ( NULL != m_pFactory )
      m_pFactory->Release( );

   m_ViewOffsets.Destroy( );
   m_FreeViewOffsets.Destroy( );

   if ( NULL != m_pGraphicsFence )
      m_pGraphicsFence->Release( );

   if ( NULL != m_pComputeFence )
      m_pComputeFence->Release( );

   if ( NULL != m_pRTVDescHeap )
      m_pRTVDescHeap->Release( );

   if ( NULL != m_pDescHeap )
      m_pDescHeap->Release( );

   DestroySwapChain( );

   if ( NULL != m_pDevice )
      m_pDevice->Release( );

   for ( int i = 0; i < MaxCommandLists; i++ )
   {
      delete m_pGraphicsPool[i];
      m_pGraphicsPool[i] = NULL;

      delete m_pComputePool[i];
      m_pComputePool[i] = NULL;
   }

   if ( NULL != m_pGraphicsQueue )
      m_pGraphicsQueue->Release( );

   if ( NULL != m_pComputeQueue )
      m_pComputeQueue->Release( );

   CloseHandle( m_GraphicsFenceEvent );
   CloseHandle( m_ComputeFenceEvent );

   m_pFactory = NULL;
   m_pDevice = NULL;
   m_pGraphicsQueue = NULL;
   m_pComputeQueue = NULL;
   m_pSwapChain = NULL;
   m_pRTVDescHeap = NULL;
   m_pDescHeap = NULL;
   m_GraphicsFenceEvent = NULL;
   m_ComputeFenceEvent = NULL;
}

void GpuDevice::ResetDevice( void )
{
   // TODO: DX12
}

bool GpuDevice::HasDevice( void )
{
   // TODO: DX12
   return true;
}

void GpuDevice::AppReady( void )
{
   ResourceHandle backBuffer( Id::Create( ) );

   ImageBuffer *pBuffer = new ImageBuffer;
   pBuffer->Create( m_pRenderTargets[ m_FrameIndex ], ImageBuffer::ViewType::Present );

   backBuffer.Bind("BackBuffer", pBuffer);
   pBuffer->AddToScene( );

   m_BackBuffer = backBuffer;
}

void GpuDevice::AppShutdown( void )
{
   ImageBuffer *pBuffer = GetResource(m_BackBuffer, ImageBuffer);

   pBuffer->Create( NULL, ImageBuffer::ViewType::Unknown ); //clear RT from it, it will be released in ::Destroy
   pBuffer->RemoveFromScene( );
   pBuffer->Destroy( );

   delete pBuffer;

   m_BackBuffer = NullHandle;
}

void GpuDevice::BeginRender( void )
{
}

void GpuDevice::EndRender( void )
{
   MainThreadCheck;

   CommandList *pCommandList = AllocGraphicsCommandList( );

   GetResource( m_BackBuffer, ImageBuffer )->ConvertTo( ImageBuffer::Present, pCommandList );

   D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = m_pRTVDescHeap->GetCPUDescriptorHandleForHeapStart( );
   rtvHandle.ptr += m_FrameIndex * m_RtvDescSize;

   pCommandList->pList->OMSetRenderTargets( 1, &rtvHandle, FALSE, NULL );
   ExecuteCommandLists( &pCommandList, 1 );

   ++m_FenceCount;
   m_pGraphicsQueue->Signal( m_pGraphicsFence, m_FenceCount );
   m_pComputeQueue->Signal( m_pComputeFence, m_FenceCount );
}

void GpuDevice::FinishFrame( void )
{
   if ( m_FenceCount == 0 )
      return;

   HRESULT hr;

   if ( m_pGraphicsFence->GetCompletedValue( ) < m_FenceCount )
   {
      hr = m_pGraphicsFence->SetEventOnCompletion( m_FenceCount, m_GraphicsFenceEvent );
      Debug::Assert( Condition( SUCCEEDED( hr ) ), "Failed to SetEventOnCompletion (0x%08x)", hr );

      WaitForSingleObject( m_GraphicsFenceEvent, INFINITE );
   }
   
   if ( m_pComputeFence->GetCompletedValue( ) < m_FenceCount )
   {
      hr = m_pComputeFence->SetEventOnCompletion( m_FenceCount, m_ComputeFenceEvent );
      Debug::Assert( Condition( SUCCEEDED( hr ) ), "Failed to SetEventOnCompletion (0x%08x)", hr );

      WaitForSingleObject( m_ComputeFenceEvent, INFINITE );
   }

   //wait for previous present
   WaitForSwapChain( ); 

   //swap chain presented so we are 100% sure prev frame's resources are clear
   FreePerFrameResources( m_PrevFrameIndex );
   GpuProfiler::Instance( ).ResolveTimers( m_PrevFrameIndex );

   // Present the current frame.
   hr = m_pSwapChain->Present( m_PresentInterval, 0 );
   if ( FAILED(hr) )
   {
      HRESULT reason_hr = m_pDevice->GetDeviceRemovedReason( );
      Debug::Assert( Condition( SUCCEEDED( hr ) ), "Failed to m_pSwapChain->Present (0x%08x)", hr );
   }

   m_PrevFrameIndex = m_FrameIndex;

   m_FrameIndex = ( m_FrameIndex + 1 ) % FrameCount;

   ImageBuffer *pBuffer = GetResource(m_BackBuffer, ImageBuffer);
   pBuffer->Create( m_pRenderTargets[ m_FrameIndex ], ImageBuffer::ViewType::Present );
}

void GpuDevice::WaitForSwapChain( void )
{
   WaitForSingleObjectEx( m_SwapChain.frameLatencyWaitableObject, INFINITE, true );
}

HRESULT GpuDevice::CreateSwapChain(
   HWND hWnd,
   int width, 
   int height, 
   bool windowed 
)
{
   HRESULT hr;

   // Describe and create the swap chain.
   DXGI_SWAP_CHAIN_DESC swapChainDesc = { };
   swapChainDesc.BufferCount = FrameCount;
   swapChainDesc.BufferDesc.Width = width;
   swapChainDesc.BufferDesc.Height = height;
   swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
   swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
   swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
   swapChainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_FRAME_LATENCY_WAITABLE_OBJECT;
   swapChainDesc.OutputWindow = hWnd;
   swapChainDesc.SampleDesc.Count = 1;
   swapChainDesc.Windowed = true == windowed;

   //LOG( "CreateSwapChain..." );
   hr = m_pFactory->CreateSwapChain(
      m_pGraphicsQueue, // Swap chain needs the queue so that it can force a flush on it.
      &swapChainDesc,
      (IDXGISwapChain **) &m_pSwapChain
   );
   if ( FAILED( hr ) )
      return hr;

   m_SwapChain.hWnd = hWnd;
   m_SwapChain.width = width;
   m_SwapChain.height = height;
   m_SwapChain.frameLatencyWaitableObject = m_pSwapChain->GetFrameLatencyWaitableObject( );

   hr = m_pFactory->MakeWindowAssociation( hWnd, DXGI_MWA_NO_ALT_ENTER );
   
   m_FrameIndex = m_pSwapChain->GetCurrentBackBufferIndex( );
   m_PrevFrameIndex = m_FrameIndex;


   // Create frame resources.
   {
      D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = m_pRTVDescHeap->GetCPUDescriptorHandleForHeapStart( );

      uint32 i;

      // Create a RTV for each frame.
      for ( i = 0; i < FrameCount; i++ )
      {
         hr = m_pSwapChain->GetBuffer( i, __uuidof( ID3D12Resource ), (void **) &m_pRenderTargets[ i ] );
         Debug::Assert( Condition( SUCCEEDED( hr ) ), "Failed to m_pSwapChain->GetBuffer %d (0x%08x)", i, hr );
         BreakIf( FAILED( hr ) );

         m_pDevice->CreateRenderTargetView( m_pRenderTargets[ i ], NULL, rtvHandle );

         rtvHandle.ptr += m_RtvDescSize;
      }
      if ( FAILED( hr ) )
         return hr;
   }

   if ( m_BackBuffer != NullHandle )
   {
      ImageBuffer *pBuffer = GetResource(m_BackBuffer, ImageBuffer);
      pBuffer->Create( m_pRenderTargets[ m_FrameIndex ], ImageBuffer::ViewType::Present );
   }

   return S_OK;
}

void GpuDevice::DestroySwapChain( void )
{
   uint32 i;

   for ( i = 0; i < FrameCount; i++ )
   {
      if ( NULL != m_pRenderTargets[ i ] )
         m_pRenderTargets[ i ]->Release( );

      m_pRenderTargets[ i ] = NULL;
   }

   if ( NULL != m_pSwapChain )
   {
      m_pSwapChain->SetFullscreenState( false, NULL );
      m_pSwapChain->Release( );

      m_pSwapChain = NULL;
   }
}

GpuDevice::CommandList *GpuDevice::AllocGraphicsCommandList( void )
{
   HRESULT hr;
   int index;

   index = (int) AtomicDecrement( (uint32 *) &m_NumGraphicsCommandLists );
   Debug::Assert( Condition(index >= 0), "No Graphics CommandLists Remain" );

   CommandList *pList = m_pGraphicsPool[ index ];
   m_pGraphicsPool[ index ] = NULL;

   hr = pList->pAllocator->Reset( );
   Debug::Assert( Condition( SUCCEEDED( hr ) ), "Failed to m_pStartCommandList->pAllocator->Reset( ) (0x%08x)", hr );

   hr = pList->pList->Reset( pList->pAllocator, NULL );
   Debug::Assert( Condition( SUCCEEDED( hr ) ), "Failed to m_pStartCommandList->pList->Reset( ) (0x%08x)", hr );

   FrameResources *pFrame = &m_FrameResources[ m_FrameIndex ];

   index = (int) AtomicIncrement( (uint32 *) &pFrame->numGraphicsCommandLists ) - 1;
   pFrame->pGraphicsCommandLists[ index ] = pList;

   return pList;
}

GpuDevice::CommandList *GpuDevice::AllocComputeCommandList( void )
{
    HRESULT hr;
    int index;

    index = (int) AtomicDecrement( (uint32 *) &m_NumComputeCommandLists );
    Debug::Assert( Condition(index >= 0), "No Compute CommandLists Remain" );

    CommandList *pList = m_pComputePool[ index ];
    m_pComputePool[ index ] = NULL;

    hr = pList->pAllocator->Reset( );
    Debug::Assert( Condition( SUCCEEDED( hr ) ), "Failed to m_pStartCommandList->pAllocator->Reset( ) (0x%08x)", hr );

    hr = pList->pList->Reset( pList->pAllocator, NULL );
    Debug::Assert( Condition( SUCCEEDED( hr ) ), "Failed to m_pStartCommandList->pList->Reset( ) (0x%08x)", hr );
    
    FrameResources *pFrame = &m_FrameResources[ m_FrameIndex ];

    index = (int) AtomicIncrement( (uint32 *) &pFrame->numComputeCommandLists ) - 1;
    pFrame->pComputeCommandLists[ index ] = pList;

    return pList;
}

void GpuDevice::ExecuteCommandLists( 
   CommandList *pCommandLists[],
   uint32 count 
   )
{
   uint32 i;

   Debug::Assert( Condition(count <= 1024), "Too many for this temp function" );
   
   ID3D12CommandList *pLists[ 1024 ];

   if ( count > 0 )
   {
      for ( i = 0; i < count; i++ )
      {
         pCommandLists[ i ]->pList->Close( );
         pLists[ i ] = pCommandLists[ i ]->pList;
      }

      if ( false == pCommandLists[ 0 ]->isCompute )
         m_pGraphicsQueue->ExecuteCommandLists( count, pLists );
      else
         m_pComputeQueue->ExecuteCommandLists( count, pLists );
   }
}

GpuDevice::GpuHandle GpuDevice::CreateGpuHandle(
   const Id &id
)
{
   GpuHandle::InternalRef *pRef = CreateGpuRef( id );

   GpuHandle gpuHandle;
   gpuHandle._internal = pRef;

   return gpuHandle;
}

void GpuDevice::SetGpuCbv(
   GpuHandle gpuHandle,
   const D3D12_CONSTANT_BUFFER_VIEW_DESC &desc
   )
{
   GpuHandle::InternalRef *pRef = gpuHandle._internal;

   D3D12_CPU_DESCRIPTOR_HANDLE handle = m_CpuBaseHandle;
   
   if ( -1 == pRef->cbvOffset )
      pRef->cbvOffset = GetNextCpuOffset( );

   handle.ptr += pRef->cbvOffset;

   m_pDevice->CreateConstantBufferView( &desc, handle );
}

void GpuDevice::SetGpuSrv(
   GpuHandle gpuHandle,
   ID3D12Resource *pResource,
   const D3D12_SHADER_RESOURCE_VIEW_DESC &desc
   )
{
   GpuHandle::InternalRef *pRef = gpuHandle._internal;

   D3D12_CPU_DESCRIPTOR_HANDLE handle = m_CpuBaseHandle;

   if ( -1 == pRef->srvOffset )
      pRef->srvOffset = GetNextCpuOffset( );

   handle.ptr += pRef->srvOffset;

   m_pDevice->CreateShaderResourceView( pResource, &desc, handle );
}

void GpuDevice::SetGpuUav(
   GpuHandle gpuHandle,
   ID3D12Resource *pResource,
   const D3D12_UNORDERED_ACCESS_VIEW_DESC &desc,
   bool hasCounter
)
{
   GpuHandle::InternalRef *pRef = gpuHandle._internal;

   D3D12_CPU_DESCRIPTOR_HANDLE handle = m_CpuBaseHandle;

   if ( -1 == pRef->uavOffset )
      pRef->uavOffset = GetNextCpuOffset( );

   handle.ptr += pRef->uavOffset;

   m_pDevice->CreateUnorderedAccessView( pResource, hasCounter ? pResource : NULL, &desc, handle );
}

bool GpuDevice::CheckSupport(GpuDevice::Support::Feature feature)
{
    switch (feature)
    {
        case Support::Feature::UavTypedLoad : 
        {
            D3D12_FEATURE_DATA_FORMAT_SUPPORT support = { DXGI_FORMAT_R11G11B10_FLOAT, D3D12_FORMAT_SUPPORT1_NONE, D3D12_FORMAT_SUPPORT2_NONE };
            m_pDevice->CheckFeatureSupport( D3D12_FEATURE_FORMAT_SUPPORT, &support, sizeof(support) );
            return ( support.Support2 & D3D12_FORMAT_SUPPORT2_UAV_TYPED_LOAD ) != 0;
        }
        default : return false;
    }
}

GpuDevice::CommandList *GpuDevice::CreateGraphicsCommandList( void )
{
   HRESULT hr;

   CommandList *pCommandList = new CommandList;

   hr = m_pDevice->CreateCommandAllocator( D3D12_COMMAND_LIST_TYPE_DIRECT, __uuidof( ID3D12CommandAllocator ), (void **) &pCommandList->pAllocator );
   Debug::Assert( Condition( SUCCEEDED( hr ) ), "Failed to m_pDevice->CreateCommandAllocator (0x%08x)", hr );

   hr = m_pDevice->CreateCommandList( 0, D3D12_COMMAND_LIST_TYPE_DIRECT, pCommandList->pAllocator, NULL, __uuidof( ID3D12GraphicsCommandList ), (void **) &pCommandList->pList );
   Debug::Assert( Condition( SUCCEEDED( hr ) ), "Failed to CreateCommandList (0x%08x)", hr );

   hr = pCommandList->pList->QueryInterface( __uuidof(ID3D12GraphicsCommandList1), (void **) &pCommandList->pList1 );
   if ( FAILED(hr) )
      pCommandList->pList1 = NULL;
   
   pCommandList->hasProgrammableSamplePositionsSupport = m_ProgrammableSamplePositionsSupport;
   pCommandList->isCompute = false;

   return pCommandList;
}

GpuDevice::CommandList *GpuDevice::CreateComputeCommandList( void )
{
   HRESULT hr;

   CommandList *pCommandList = new CommandList;

   hr = m_pDevice->CreateCommandAllocator( D3D12_COMMAND_LIST_TYPE_COMPUTE, __uuidof( ID3D12CommandAllocator ), (void **) &pCommandList->pAllocator );
   Debug::Assert( Condition( SUCCEEDED( hr ) ), "Failed to m_pDevice->CreateCommandAllocator (0x%08x)", hr );

   hr = m_pDevice->CreateCommandList( 0, D3D12_COMMAND_LIST_TYPE_COMPUTE, pCommandList->pAllocator, NULL, __uuidof( ID3D12GraphicsCommandList ), (void **) &pCommandList->pList );
   Debug::Assert( Condition( SUCCEEDED( hr ) ), "Failed to CreateCommandList (0x%08x)", hr );

   pCommandList->pList1 = NULL;
   pCommandList->hasProgrammableSamplePositionsSupport = 0;
   pCommandList->isCompute = true;

   return pCommandList;
}

void GpuDevice::FreePerFrameResources( 
    int index
)
{
    FrameResources *pFrame = &m_FrameResources[ index ];

    for ( int i = 0; i < pFrame->numGraphicsCommandLists; i++ )
        m_pGraphicsPool[ m_NumGraphicsCommandLists++ ] = pFrame->pGraphicsCommandLists[ i ];

    for ( int i = 0; i < pFrame->numComputeCommandLists; i++ )
        m_pComputePool[ m_NumComputeCommandLists++ ] = pFrame->pComputeCommandLists[ i ];

    pFrame->numGraphicsCommandLists = 0;
    pFrame->numComputeCommandLists = 0;

    // Guaranteed everything is done in our previous frame
    // because the swap chain has complete
    List<ID3D12Resource*>::Enumerator e = pFrame->resources.GetEnumerator( );

    while ( e.EnumNext( ) )
        e.Data( )->Release( );

    pFrame->resources.Clear( );
}

GpuDevice::GpuHandle::InternalRef *GpuDevice::CreateGpuRef(
   const Id &id
)
{
   MainThreadCheck;

   GpuHandle::InternalRef *pRef;

   if ( false == m_ViewOffsets.Get(id, &pRef) )
   {
      uint32 freeOffsets = m_FreeViewOffsets.GetSize();

      if ( freeOffsets > 0 )
      {
         pRef = m_FreeViewOffsets.GetAt( freeOffsets - 1 );
         m_FreeViewOffsets.RemoveAt( (uint32) (freeOffsets - 1) );
      }
      else
      {
         pRef = new GpuHandle::InternalRef;
         pRef->cbvOffset = -1;
         pRef->srvOffset = -1;
         pRef->uavOffset = -1;
      }

      pRef->id = id;
      m_ViewOffsets.Add( id, pRef );
   }

   return pRef;
}
