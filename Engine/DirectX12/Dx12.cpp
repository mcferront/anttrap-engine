#pragma once

#include "EnginePch.h"

#include "Dx12.h"
#include "Dx12Contexts.h"
#include "Dx12GpuBuffer.h"
#include "Log.h"
#include "SystemId.h"
#include "TextureAsset.h"
#include "GpuProfiler.h"

const GpuDevice::DepthStencilView     GpuDevice::DepthStencilView::Invalid; 
const GpuDevice::ConstantBufferView   GpuDevice::ConstantBufferView::Invalid; 
const GpuDevice::ShaderResourceView   GpuDevice::ShaderResourceView::Invalid; 
const GpuDevice::UnorderedAccessView  GpuDevice::UnorderedAccessView::Invalid; 
const GpuDevice::RenderTargetView     GpuDevice::RenderTargetView::Invalid; 

bool GpuDevice::CommandList::SetSamplePositions(
    const Vector2 *pPositions,
    uint32 numPositions
)
{
    if ( false == hasProgrammableSamplePositionsSupport )
    {
        static bool warned;

        if ( false == warned )
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
        Debug::Assert( Condition( numPositions <= maxPositions ), "Too many custom sample positions" );

        for ( uint32 i = 0; i < numPositions; i++ )
        {
            positions[i].X = Math::FloatToInt( pPositions[i].x );
            positions[i].Y = Math::FloatToInt( pPositions[i].y );
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
    GpuDevice::RenderTargetView *pRTV,
    bool clearColor,
    const Color *pColorValue
)
{
    if ( true == clearColor )
    {
        const float clearColor[] = { pColorValue->r / 255.0f, pColorValue->g / 255.0f, pColorValue->b / 255.0f, pColorValue->a / 255.0f };
        pCommandList->pList->ClearRenderTargetView( pRTV->view.cpuHandle, clearColor, 0, NULL );
    }
}

void GpuDevice::ClearDepthStencil(
    GpuDevice::CommandList *pCommandList,
    GpuDevice::DepthStencilView *pDSV,
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
        pCommandList->pList->ClearDepthStencilView( pDSV->view.cpuHandle, (D3D12_CLEAR_FLAGS) flags, depthValue, stencilValue, 0, NULL );
}

void GpuDevice::SetRenderTargets(
    GpuDevice::CommandList *pCommandList,
    GpuDevice::RenderTargetView *pRTVs[],
    uint32 numRenderTargets,
    GpuDevice::DepthStencilView *pDSV
)
{
    D3D12_CPU_DESCRIPTOR_HANDLE rtvHandles[RenderContexts::MaxRenderTargets];

    for ( uint32 i = 0; i < numRenderTargets; i++ )
        rtvHandles[i] = pRTVs[i]->view.cpuHandle;

    pCommandList->pList->OMSetRenderTargets( numRenderTargets,
        rtvHandles,
        FALSE,
        pDSV ? &pDSV->view.cpuHandle : NULL );
}

void GpuDevice::CopyResource(
    GpuDevice::CommandList *pList,
    GpuResource *pDest,
    GpuResource *pSource
)
{
    D3D12_RESOURCE_DESC desc = pDest->GetApiResource( )->GetDesc();
    D3D12_RESOURCE_DESC source_desc = pSource->GetApiResource( )->GetDesc();
    pList->pList->CopyResource( pDest->GetApiResource( ), pSource->GetApiResource( ) );
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
    m_pGraphicsFence = NULL;
    m_pComputeFence = NULL;
    m_NumGraphicsCommandLists = 0;
    m_NumComputeCommandLists = 0;
    m_FenceCount = 0;
    m_PresentInterval = 1;

    m_ProgrammableSamplePositionsSupport = false;

    for ( uint32 i = 0; i < FrameCount; i++ )
    {
        m_FrameResources[i].resources.Create( );
        m_FrameResources[i].numGraphicsCommandLists = 0;
        m_FrameResources[i].numComputeCommandLists = 0;
    }

    memset( m_pGraphicsPool, 0, sizeof( m_pGraphicsPool ) );
    memset( m_pComputePool, 0, sizeof( m_pComputePool ) );
    memset( m_pRenderTargets, 0, sizeof( m_pRenderTargets ) );

    do
    {
#if defined(_DEBUG)
        {
            ID3D12Debug1 *pDebugController;

            hr = D3D12GetDebugInterface( __uuidof(ID3D12Debug1), (void **) &pDebugController );
            Debug::Assert( Condition( SUCCEEDED( hr ) ), "Failed to D3D12GetDebugInterface (0x%08x)", hr );

            if ( SUCCEEDED( hr ) )
            {
                pDebugController->EnableDebugLayer( );
                //pDebugController->SetEnableGPUBasedValidation( TRUE );
                pDebugController->Release( );
            }
        }
#endif

        hr = CreateDXGIFactory1( __uuidof(IDXGIFactory4), (void **) &m_pFactory );
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
            hr = D3D12CreateDevice( pAdapter, D3D_FEATURE_LEVEL_12_0, __uuidof(ID3D12Device), (void **) &m_pDevice );

            pAdapter->Release( );
            pAdapter = NULL;

            BreakIf( SUCCEEDED( hr ) );

            m_pDevice = NULL;
        }

        BreakIf( NULL == m_pDevice );

        D3D12_COMMAND_QUEUE_DESC queueDesc = { };
        queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;

        queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
        hr = m_pDevice->CreateCommandQueue( &queueDesc, __uuidof(ID3D12CommandQueue), (void **) &m_pGraphicsQueue );
        Debug::Assert( Condition( SUCCEEDED( hr ) ), "Failed to CreateCommandQueue (0x%08x)", hr );
        BreakIf( FAILED( hr ) );

        queueDesc.Type = D3D12_COMMAND_LIST_TYPE_COMPUTE;
        hr = m_pDevice->CreateCommandQueue( &queueDesc, __uuidof(ID3D12CommandQueue), (void **) &m_pComputeQueue );
        Debug::Assert( Condition( SUCCEEDED( hr ) ), "Failed to CreateCommandQueue (0x%08x)", hr );
        BreakIf( FAILED( hr ) );

        // Create descriptor heaps.
        {
            m_RtvDesc.Create( D3D12_DESCRIPTOR_HEAP_TYPE_RTV, D3D12_DESCRIPTOR_HEAP_FLAG_NONE, 64 );
            m_DsvDesc.Create( D3D12_DESCRIPTOR_HEAP_TYPE_DSV, D3D12_DESCRIPTOR_HEAP_FLAG_NONE, 64 );
            m_ShaderDescHeap.Create( D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE, 1000000 );
        }

        hr = CreateSwapChain( hwnd, width, height, windowed );
        BreakIf( FAILED( hr ) );

        D3D12_FEATURE_DATA_D3D12_OPTIONS2 options;
        m_pDevice->CheckFeatureSupport( D3D12_FEATURE_D3D12_OPTIONS2, &options, sizeof( options ) );

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

        m_pMainThreadCommandList = CreateGraphicsCommandList( );
        m_pMainThreadCommandList->pList->Close();

        m_NumGraphicsCommandLists = MaxCommandLists;
        m_NumComputeCommandLists = MaxCommandLists;

        hr = m_pDevice->CreateFence( 0, D3D12_FENCE_FLAG_NONE, __uuidof(ID3D12Fence), (void **) &m_pGraphicsFence );
        Debug::Assert( Condition( SUCCEEDED( hr ) ), "Failed to CreateFence (0x%08x)", hr );
        BreakIf( FAILED( hr ) );

        hr = m_pDevice->CreateFence( 0, D3D12_FENCE_FLAG_NONE, __uuidof(ID3D12Fence), (void **) &m_pComputeFence );
        Debug::Assert( Condition( SUCCEEDED( hr ) ), "Failed to CreateFence (0x%08x)", hr );
        BreakIf( FAILED( hr ) );

        m_GraphicsFenceEvent = CreateEvent( nullptr, FALSE, FALSE, nullptr );
        m_ComputeFenceEvent = CreateEvent( nullptr, FALSE, FALSE, nullptr );


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
        m_FrameResources[i].resources.Destroy( );
    }

    if ( NULL != m_pFactory )
        m_pFactory->Release( );

    if ( NULL != m_pGraphicsFence )
        m_pGraphicsFence->Release( );

    if ( NULL != m_pComputeFence )
        m_pComputeFence->Release( );

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

    m_ShaderDescHeap.Destroy( );
    m_RtvDesc.Destroy( );
    m_DsvDesc.Destroy( );

    CloseHandle( m_GraphicsFenceEvent );
    CloseHandle( m_ComputeFenceEvent );

    m_pFactory = NULL;
    m_pDevice = NULL;
    m_pGraphicsQueue = NULL;
    m_pComputeQueue = NULL;
    m_pSwapChain = NULL;
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

    GpuBuffer *pBuffer = new GpuBuffer;
    pBuffer->Create( m_pRenderTargets[m_FrameIndex], GpuBuffer::State::Present );

    backBuffer.Bind( "BackBuffer", pBuffer );
    pBuffer->AddToScene( );

    m_BackBuffer = backBuffer;
}

void GpuDevice::AppShutdown( void )
{
    GpuBuffer *pBuffer = GetResource( m_BackBuffer, GpuBuffer );
    pBuffer->SwapApiResource( NULL, GpuResource::State::Unknown );

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

    CommandList *pCommandList = AllocPerFrameGraphicsCommandList( );

    GetResource( m_BackBuffer, GpuBuffer )->TransitionTo( pCommandList, GpuBuffer::State::Present );

    pCommandList->pList->OMSetRenderTargets( 1, &m_Rtvs[m_FrameIndex], FALSE, NULL );
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
    if ( FAILED( hr ) )
    {
        HRESULT reason_hr = m_pDevice->GetDeviceRemovedReason( );
        Debug::Assert( Condition( SUCCEEDED( hr ) ), "Failed to m_pSwapChain->Present (0x%08x)", hr );
    }

    m_PrevFrameIndex = m_FrameIndex;

    m_FrameIndex = (m_FrameIndex + 1) % FrameCount;

    GpuBuffer *pBuffer = GetResource( m_BackBuffer, GpuBuffer );
    pBuffer->SwapApiResource( m_pRenderTargets[m_FrameIndex], GpuBuffer::State::Present );
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
        uint32 i;

        // Create a RTV for each frame.
        for ( i = 0; i < FrameCount; i++ )
        {
            hr = m_pSwapChain->GetBuffer( i, __uuidof(ID3D12Resource), (void **) &m_pRenderTargets[i] );
            Debug::Assert( Condition( SUCCEEDED( hr ) ), "Failed to m_pSwapChain->GetBuffer %d (0x%08x)", i, hr );
            BreakIf( FAILED( hr ) );
        
            D3D12_GPU_DESCRIPTOR_HANDLE unused;
            m_RtvDesc.Alloc( &m_Rtvs[i], &unused );

            m_pDevice->CreateRenderTargetView( m_pRenderTargets[i], NULL, m_Rtvs[i] );
        }
        if ( FAILED( hr ) )
            return hr;
    }

    if ( m_BackBuffer != NullHandle )
    {
        GpuBuffer *pBuffer = GetResource( m_BackBuffer, GpuBuffer );
        pBuffer->SwapApiResource( m_pRenderTargets[m_FrameIndex], GpuBuffer::State::Present );
    }

    return S_OK;
}

void GpuDevice::DestroySwapChain( void )
{
    uint32 i;

    for ( i = 0; i < FrameCount; i++ )
    {
        if ( NULL != m_pRenderTargets[i] )
            m_pRenderTargets[i]->Release( );

        m_pRenderTargets[i] = NULL;
    }

    if ( NULL != m_pSwapChain )
    {
        m_pSwapChain->SetFullscreenState( false, NULL );
        m_pSwapChain->Release( );

        m_pSwapChain = NULL;
    }
}

GpuDevice::CommandList *GpuDevice::AllocPerFrameGraphicsCommandList( void )
{
    HRESULT hr;
    int index;

    index = (int) AtomicDecrement( (uint32 *) &m_NumGraphicsCommandLists );
    Debug::Assert( Condition( index >= 0 ), "No Graphics CommandLists Remain" );

    CommandList *pList = m_pGraphicsPool[index];
    m_pGraphicsPool[index] = NULL;

    hr = pList->pAllocator->Reset( );
    Debug::Assert( Condition( SUCCEEDED( hr ) ), "Failed to m_pStartCommandList->pAllocator->Reset( ) (0x%08x)", hr );

    hr = pList->pList->Reset( pList->pAllocator, NULL );
    Debug::Assert( Condition( SUCCEEDED( hr ) ), "Failed to m_pStartCommandList->pList->Reset( ) (0x%08x)", hr );

    FrameResources *pFrame = &m_FrameResources[m_FrameIndex];

    index = (int) AtomicIncrement( (uint32 *) &pFrame->numGraphicsCommandLists ) - 1;
    pFrame->pGraphicsCommandLists[index] = pList;

    ID3D12DescriptorHeap *pHeaps[] = 
    {
        m_ShaderDescHeap.pHeap,
        //m_RtvDesc.pHeap,
        //m_DsvDesc.pHeap,
    };

    pList->pList->SetDescriptorHeaps( _countof(pHeaps), pHeaps );

    return pList;
}

GpuDevice::CommandList *GpuDevice::AllocPerFrameComputeCommandList( void )
{
    HRESULT hr;
    int index;

    index = (int) AtomicDecrement( (uint32 *) &m_NumComputeCommandLists );
    Debug::Assert( Condition( index >= 0 ), "No Compute CommandLists Remain" );

    CommandList *pList = m_pComputePool[index];
    m_pComputePool[index] = NULL;

    hr = pList->pAllocator->Reset( );
    Debug::Assert( Condition( SUCCEEDED( hr ) ), "Failed to m_pStartCommandList->pAllocator->Reset( ) (0x%08x)", hr );

    hr = pList->pList->Reset( pList->pAllocator, NULL );
    Debug::Assert( Condition( SUCCEEDED( hr ) ), "Failed to m_pStartCommandList->pList->Reset( ) (0x%08x)", hr );

    FrameResources *pFrame = &m_FrameResources[m_FrameIndex];

    index = (int) AtomicIncrement( (uint32 *) &pFrame->numComputeCommandLists ) - 1;
    pFrame->pComputeCommandLists[index] = pList;

    ID3D12DescriptorHeap *pHeaps[] = 
    {
        m_ShaderDescHeap.pHeap,
        //m_RtvDesc.pHeap,
        //m_DsvDesc.pHeap,
    };

    pList->pList->SetDescriptorHeaps( _countof(pHeaps), pHeaps );

    return pList;
}

GpuDevice::CommandList *GpuDevice::AllocThreadCommandList( void )
{
    MainThreadCheck;

    m_pMainThreadCommandList->pAllocator->Reset( );
    m_pMainThreadCommandList->pList->Reset( m_pMainThreadCommandList->pAllocator, NULL );

    ID3D12DescriptorHeap *pHeaps[] = 
    {
        m_ShaderDescHeap.pHeap,
        //m_RtvDesc.pHeap,
        //m_DsvDesc.pHeap,
    };

    m_pMainThreadCommandList->pList->SetDescriptorHeaps( _countof(pHeaps), pHeaps );

    return m_pMainThreadCommandList;
}

void GpuDevice::ExecuteCommandLists(
    CommandList *pCommandLists[],
    uint32 count,
    bool waitForCompletion //= false
)
{
    uint32 i;

    Debug::Assert( Condition( count <= 1024 ), "Too many for this temp function" );

    ID3D12CommandList *pLists[1024];

    if ( count > 0 )
    {
        ID3D12CommandQueue *pQueue;

        for ( i = 0; i < count; i++ )
        {
            pCommandLists[i]->pList->Close( );
            pLists[i] = pCommandLists[i]->pList;
        }

        if ( false == pCommandLists[0]->isCompute )
            pQueue = m_pGraphicsQueue;
        else
            pQueue = m_pComputeQueue;
    
        pQueue->ExecuteCommandLists( count, pLists );

        if ( true == waitForCompletion )
        {
            ID3D12Fence *pFence;

            m_pDevice->CreateFence( 0, D3D12_FENCE_FLAG_NONE, __uuidof( ID3D12Fence ), (void **) &pFence );
            
            HANDLE fenceEvent = CreateEvent( nullptr, FALSE, FALSE, nullptr );

            pQueue->Signal( pFence, 1 );

            if ( pFence->GetCompletedValue( ) < 1 )
            {
                pFence->SetEventOnCompletion( 1, fenceEvent );
                WaitForSingleObject( fenceEvent, INFINITE );
            }

            CloseHandle( fenceEvent );

            pFence->Release();
        }
    }
}

GpuDevice::ConstantBufferView *GpuDevice::CreateCbv(
    const D3D12_CONSTANT_BUFFER_VIEW_DESC &desc
)
{
    GpuDevice::ConstantBufferView *pCBV = new GpuDevice::ConstantBufferView;
    pCBV->view.pHeap = m_ShaderDescHeap.Alloc( &pCBV->view.cpuHandle, &pCBV->view.gpuHandle );

    m_pDevice->CreateConstantBufferView( &desc, pCBV->view.cpuHandle );

    return pCBV;
}

GpuDevice::ShaderResourceView *GpuDevice::CreateSrv(
    const D3D12_SHADER_RESOURCE_VIEW_DESC &desc,
    GpuResource *pResource
)
{
    return CreateSrvRange( &desc, &pResource, 1 );
}

GpuDevice::ShaderResourceView *GpuDevice::CreateSrvRange(
    const D3D12_SHADER_RESOURCE_VIEW_DESC *pDescs,
    GpuResource *pResources[],
    size_t numResources
)
{
    D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle;
    D3D12_GPU_DESCRIPTOR_HANDLE gpuHandle;

    const GpuDevice::DescHeap *pHeap = m_ShaderDescHeap.Alloc( &cpuHandle, &gpuHandle, numResources );

    GpuDevice::ShaderResourceView *pSRV = new GpuDevice::ShaderResourceView;
    pSRV->view.cpuHandle = cpuHandle;
    pSRV->view.gpuHandle = gpuHandle;
    pSRV->view.pHeap = pHeap;

    for (uint32 i = 0; i < numResources; i++)
    {
        CreateSrv( pDescs[i], pResources[i], cpuHandle );
        cpuHandle.ptr += pHeap->descHandleIncSize;
    }

    return pSRV;
}

void GpuDevice::CreateSrv(
    const D3D12_SHADER_RESOURCE_VIEW_DESC &desc,
    GpuResource *pResource,
    D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle
)
{
    m_pDevice->CreateShaderResourceView( pResource->GetApiResource(), &desc, cpuHandle );
}

GpuDevice::UnorderedAccessView *GpuDevice::CreateUav(
    const D3D12_UNORDERED_ACCESS_VIEW_DESC &desc,
    GpuResource *pResources
)
{
    return CreateUavRange( &desc, &pResources, 1 );
}

GpuDevice::UnorderedAccessView *GpuDevice::CreateUavRange(
    const D3D12_UNORDERED_ACCESS_VIEW_DESC *pDescs,
    GpuResource *pResources[],
    size_t numResources
)
{
    D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle;
    D3D12_GPU_DESCRIPTOR_HANDLE gpuHandle;

    const GpuDevice::DescHeap *pHeap = m_ShaderDescHeap.Alloc( &cpuHandle, &gpuHandle, numResources );

    GpuDevice::UnorderedAccessView *pUAV = new GpuDevice::UnorderedAccessView;
    pUAV->view.cpuHandle = cpuHandle;
    pUAV->view.gpuHandle = gpuHandle;
    pUAV->view.pHeap = pHeap;

    for (uint32 i = 0; i < numResources; i++)
    {
        CreateUav( pDescs[i], pResources[i], cpuHandle );
        cpuHandle.ptr += pHeap->descHandleIncSize;
    }

    return pUAV;
}

void GpuDevice::CreateUav(
    const D3D12_UNORDERED_ACCESS_VIEW_DESC &desc,
    GpuResource *pResource,
    D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle
)
{
    m_pDevice->CreateUnorderedAccessView( pResource->GetApiResource(), NULL, &desc, cpuHandle );
}

GpuDevice::RenderTargetView *GpuDevice::CreateRtv(
    const D3D12_RENDER_TARGET_VIEW_DESC &desc,
    GpuResource *pResource
)
{
    RenderTargetView *pRTV = new RenderTargetView;
    pRTV->view.pHeap = m_RtvDesc.Alloc( &pRTV->view.cpuHandle, &pRTV->view.gpuHandle );
    pRTV->format = desc.Format;

    m_pDevice->CreateRenderTargetView( pResource->GetApiResource(), &desc, pRTV->view.cpuHandle );

    return pRTV;
}

GpuDevice::DepthStencilView *GpuDevice::CreateDsv(
    const D3D12_DEPTH_STENCIL_VIEW_DESC &desc,
    GpuResource *pResource
)
{
    DepthStencilView *pDSV = new DepthStencilView;
    pDSV->view.pHeap = m_DsvDesc.Alloc( &pDSV->view.cpuHandle, &pDSV->view.gpuHandle );
    pDSV->format = desc.Format;

    m_pDevice->CreateDepthStencilView( pResource->GetApiResource(), &desc, pDSV->view.cpuHandle );

    return pDSV;
}

void GpuDevice::AllocShaderDescRange(
    GpuDevice::ViewHandle *pHandles,
    uint32 count
)
{
    pHandles->pHeap = m_ShaderDescHeap.Alloc( &pHandles->cpuHandle, &pHandles->gpuHandle, count );
}

void GpuDevice::DestroyCbv(
    ConstantBufferView *pCBV
)
{
    //TODO: DX12 should be able to determine the offset
    // from the handle - heap->basehandle and the range
    // and the heap can keep a free list

    delete pCBV;
}

void GpuDevice::DestroySrv(
    ShaderResourceView *pSRV
)
{
    //TODO: DX12 should be able to determine the offset
    // from the handle - heap->basehandle and the range
    // and the heap can keep a free list

    delete pSRV;
}

void GpuDevice::DestroyUav(
    UnorderedAccessView *pUAV
)
{
    //TODO: DX12 should be able to determine the offset
    // from the handle - heap->basehandle and the range
    // and the heap can keep a free list

    delete pUAV;
}

void GpuDevice::DestroyRtv(
    RenderTargetView *pRTV
)
{
    //TODO: DX12 should be able to determine the offset
    // from the handle - heap->basehandle and the range
    // and the heap can keep a free list

    delete pRTV;
}

void GpuDevice::DestroyDsv(
    DepthStencilView *pDSV
)
{
    //TODO: DX12 should be able to determine the offset
    // from the handle - heap->basehandle and the range
    // and the heap can keep a free list

    delete pDSV;
}

void GpuDevice::FreeViewHandles(
    ViewHandle *pHandles
)
{
    //TODO: DX12 should be able to determine the offset
    // from the handle - heap->basehandle and the range
    // and the heap can keep a free list

    pHandles->pHeap = NULL;
}

bool GpuDevice::CheckSupport( GpuDevice::Support::Feature feature )
{
    switch ( feature )
    {
    case Support::Feature::UavTypedLoad:
    {
        D3D12_FEATURE_DATA_FORMAT_SUPPORT support = { DXGI_FORMAT_R11G11B10_FLOAT, D3D12_FORMAT_SUPPORT1_NONE, D3D12_FORMAT_SUPPORT2_NONE };
        m_pDevice->CheckFeatureSupport( D3D12_FEATURE_FORMAT_SUPPORT, &support, sizeof( support ) );
        return (support.Support2 & D3D12_FORMAT_SUPPORT2_UAV_TYPED_LOAD) != 0;
    }
    default: return false;
    }
}

GpuDevice::CommandList *GpuDevice::CreateGraphicsCommandList( void )
{
    HRESULT hr;

    CommandList *pCommandList = new CommandList;

    hr = m_pDevice->CreateCommandAllocator( D3D12_COMMAND_LIST_TYPE_DIRECT, __uuidof(ID3D12CommandAllocator), (void **) &pCommandList->pAllocator );
    Debug::Assert( Condition( SUCCEEDED( hr ) ), "Failed to m_pDevice->CreateCommandAllocator (0x%08x)", hr );

    hr = m_pDevice->CreateCommandList( 0, D3D12_COMMAND_LIST_TYPE_DIRECT, pCommandList->pAllocator, NULL, __uuidof(ID3D12GraphicsCommandList), (void **) &pCommandList->pList );
    Debug::Assert( Condition( SUCCEEDED( hr ) ), "Failed to CreateCommandList (0x%08x)", hr );

    hr = pCommandList->pList->QueryInterface( __uuidof(ID3D12GraphicsCommandList1), (void **) &pCommandList->pList1 );
    if ( FAILED( hr ) )
        pCommandList->pList1 = NULL;

    pCommandList->hasProgrammableSamplePositionsSupport = m_ProgrammableSamplePositionsSupport;
    pCommandList->isCompute = false;

    return pCommandList;
}

GpuDevice::CommandList *GpuDevice::CreateComputeCommandList( void )
{
    HRESULT hr;

    CommandList *pCommandList = new CommandList;

    hr = m_pDevice->CreateCommandAllocator( D3D12_COMMAND_LIST_TYPE_COMPUTE, __uuidof(ID3D12CommandAllocator), (void **) &pCommandList->pAllocator );
    Debug::Assert( Condition( SUCCEEDED( hr ) ), "Failed to m_pDevice->CreateCommandAllocator (0x%08x)", hr );

    hr = m_pDevice->CreateCommandList( 0, D3D12_COMMAND_LIST_TYPE_COMPUTE, pCommandList->pAllocator, NULL, __uuidof(ID3D12GraphicsCommandList), (void **) &pCommandList->pList );
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
    FrameResources *pFrame = &m_FrameResources[index];

    for ( int i = 0; i < pFrame->numGraphicsCommandLists; i++ )
        m_pGraphicsPool[m_NumGraphicsCommandLists++] = pFrame->pGraphicsCommandLists[i];

    for ( int i = 0; i < pFrame->numComputeCommandLists; i++ )
        m_pComputePool[m_NumComputeCommandLists++] = pFrame->pComputeCommandLists[i];

    pFrame->numGraphicsCommandLists = 0;
    pFrame->numComputeCommandLists = 0;

    // Guaranteed everything is done in our previous frame
    // because the swap chain has complete
    List<ID3D12Resource*>::Enumerator e = pFrame->resources.GetEnumerator( );

    while ( e.EnumNext( ) )
        e.Data( )->Release( );

    pFrame->resources.Clear( );
}
