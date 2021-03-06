#pragma once

#include "EngineGlobal.h"
#include "Threads.h"
#include "List.h"
#include "HashTable.h"
#include "Resource.h"

#ifndef DIRECTX12
#error This should not be included
#endif

#define GRAPHICS_API_SZ "DirectX 12"

class Texture;

class GpuBuffer;
class GpuResource;

class GpuDevice
{
private:
    struct DescHeap
    {
        ID3D12DescriptorHeap *pHeap;

        size_t __declspec(align(32)) numDescriptors;
        size_t descHandleIncSize;

        D3D12_CPU_DESCRIPTOR_HANDLE cpuBaseHandle;
        D3D12_GPU_DESCRIPTOR_HANDLE gpuBaseHandle;

        size_t maxDescriptors;

        DescHeap( void )
        {
            pHeap = NULL;
            numDescriptors = 0;
        }

        void Create( const char *pName, D3D12_DESCRIPTOR_HEAP_TYPE type, D3D12_DESCRIPTOR_HEAP_FLAGS flags, uint32 _maxDescriptors )
        {
            maxDescriptors = _maxDescriptors;

            D3D12_DESCRIPTOR_HEAP_DESC desc = { };
            {
                desc.NumDescriptors = maxDescriptors;
                desc.Type = type;
                desc.Flags = flags;
            }

            HRESULT hr = GpuDevice::Instance( ).GetDevice( )->CreateDescriptorHeap( &desc, __uuidof(ID3D12DescriptorHeap), (void **) &pHeap );
            Debug::Assert( Condition( SUCCEEDED( hr ) ), "Failed to CreateDescriptorHeap (0x%08x)", hr );

            pHeap->SetName( String::ToWideChar(pName) );

            descHandleIncSize = GpuDevice::Instance( ).GetDevice( )->GetDescriptorHandleIncrementSize( type );
            
            cpuBaseHandle = pHeap->GetCPUDescriptorHandleForHeapStart( );
            gpuBaseHandle = pHeap->GetGPUDescriptorHandleForHeapStart( );
        }

        void Destroy( void )
        {
            pHeap->Release( );
        }

        const DescHeap *Alloc( D3D12_CPU_DESCRIPTOR_HANDLE *pCPUHandle, D3D12_GPU_DESCRIPTOR_HANDLE *pGPUHandle, size_t count = 1)
        {
            size_t slot = AtomicAdd( &numDescriptors, count ) - count;
            slot *= descHandleIncSize;

            Debug::Assert( Condition( numDescriptors <= maxDescriptors ), "Out of GPU Descriptors" );

            *pCPUHandle = D3D12_CPU_DESCRIPTOR_HANDLE{ slot + cpuBaseHandle.ptr };
            *pGPUHandle = D3D12_GPU_DESCRIPTOR_HANDLE{ slot + gpuBaseHandle.ptr };

            return this;
        }
    };
    
public:
    static const int FrameCount = 2;
    static const uint32 FrameConstantBufferSize = 2 * 1024 * 1024;

    struct Support
    {
        enum Feature
        {
            UavTypedLoad,
        };
    };

private:
    static const int MaxCommandLists = 512;

public:
    struct ViewHandle
    {
        ViewHandle() : pHeap(NULL) {}

        D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle;
        D3D12_GPU_DESCRIPTOR_HANDLE gpuHandle;

        const GpuDevice::DescHeap *pHeap;
    };

    struct DepthStencilView 
    {
        DepthStencilView() { *this = Invalid; }

        static const DepthStencilView Invalid;

        DXGI_FORMAT format;
        ViewHandle view;
    };

    struct ConstantBufferView
    {
        ConstantBufferView() { *this = Invalid; }

        static const ConstantBufferView Invalid;

        ViewHandle view;
    };

    struct ShaderResourceView
    {
        ShaderResourceView() { *this = Invalid; }

        static const ShaderResourceView Invalid;

        ViewHandle view;
    };

    struct UnorderedAccessView
    {
        UnorderedAccessView() { *this = Invalid; }

        static const UnorderedAccessView Invalid;

        ViewHandle view;
    };

    struct RenderTargetView
    {
        RenderTargetView() { *this = Invalid; }

        static const RenderTargetView Invalid;

        DXGI_FORMAT format;
        ViewHandle view;
    };

    struct ConstantBuffer
    {
        byte   *pData;
        size_t size;
    };

    struct SwapChain
    {
        HWND hWnd;
        int width;
        int height;
        HANDLE frameLatencyWaitableObject;
    };

    class CommandList
    {
        friend class GpuDevice;

    public:
        CommandList( void )
        {
            pList1 = NULL;
        }

        ~CommandList( void )
        {
            if ( NULL != pList1 )
                pList1->Release( );

            pAllocator->Release( );
            pList->Release( );
        }

        bool SetSamplePositions(
            const Vector2 *pPositions,
            uint32 numPositions
        );

    public:
        ID3D12CommandAllocator * pAllocator;
        ID3D12GraphicsCommandList *pList;
        bool hasProgrammableSamplePositionsSupport;
        bool isCompute;

    private:
        ID3D12GraphicsCommandList1 * pList1;
    };

    struct FrameResources
    {
        CommandList *pGraphicsCommandLists[MaxCommandLists];
        CommandList *pComputeCommandLists[MaxCommandLists];

        int numGraphicsCommandLists;
        int numComputeCommandLists;

        List<ID3D12Resource*> resources;

        ID3D12Resource *pConstantBuffer;
        byte *pConstantData;
        uint32 cbOffset;
    };

public:
    static GpuDevice &Instance( void );

    static void ClearRenderTarget(
        CommandList *pCommandList,
        GpuDevice::RenderTargetView *pRTV,
        bool clearColor,
        const Color *pColorValue
    );

    static void ClearDepthStencil(
        CommandList *pCommandList,
        GpuDevice::DepthStencilView *pDSV,
        bool clearDepth,
        float depthValue,
        bool clearStencil,
        byte stencilValue
    );

    static void SetRenderTargets(
        CommandList *pCommandList,
        GpuDevice::RenderTargetView *pRTVs[],
        uint32 numRenderTargets,
        GpuDevice::DepthStencilView *pDSV
    );

    static void CopyResource(
        CommandList *pList,
        GpuResource *pDest,
        GpuResource *pSource
    );

public:
    bool Create(
        HWND hwnd,
        bool windowed,
        uint32 width,
        uint32 height
    );

    void Destroy( void );

    void ResetDevice( void );

    bool HasDevice( void );

    void AppReady( void );

    void AppShutdown( void );

    void BeginRender( void );

    void EndRender( void );

    void FinishFrame( void );

    void Fullscreen( void );

    void WaitForSwapChain( void );

    HRESULT CreateSwapChain(
        HWND hWnd,
        int width,
        int height,
        bool windowed
    );

    void DestroySwapChain( void );

    CommandList *AllocPerFrameGraphicsCommandList( void );
    CommandList *AllocPerFrameComputeCommandList( void );
    CommandList *AllocThreadCommandList( void );

    void ExecuteCommandLists(
        CommandList *pCommandLists[],
        uint32 count,
        bool waitForCompletion = false
    );

    D3D12_GPU_VIRTUAL_ADDRESS UpdateCbv(
        const ConstantBuffer &cb
    );

    //ConstantBufferView *CreateCbv( void );

    ShaderResourceView *CreateSrv(
        const D3D12_SHADER_RESOURCE_VIEW_DESC &desc,
        GpuResource *pResource
    );

    void CreateSrv(
        const D3D12_SHADER_RESOURCE_VIEW_DESC &desc,
        GpuResource *pResource,
        D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle
    );

    UnorderedAccessView *CreateUav(
        const D3D12_UNORDERED_ACCESS_VIEW_DESC &desc,
        GpuResource *pResource
    );

    void CreateUav(
        const D3D12_UNORDERED_ACCESS_VIEW_DESC &desc,
        GpuResource *pResource,
        D3D12_CPU_DESCRIPTOR_HANDLE cpuhandle
    );

    RenderTargetView *CreateRtv(
        const D3D12_RENDER_TARGET_VIEW_DESC &desc,
        GpuResource *pResource
    );

    DepthStencilView *CreateDsv(
        const D3D12_DEPTH_STENCIL_VIEW_DESC &desc,
        GpuResource *pResource
    );

    void AllocShaderDescRange(
        ViewHandle *pHandles,
        uint32 count
    );

    void DestroySrv(
        ShaderResourceView *pSRV
    );

    void DestroyUav(
        UnorderedAccessView *pUAV
    );

    void DestroyRtv(
        RenderTargetView *pRTV
    );

    void DestroyDsv(
        DepthStencilView *pDSV
    );

    void FreeViewHandles(
        ViewHandle *pHandles
    );

    bool CheckSupport(
        Support::Feature feature
    );

    void SetPresentInterval(
        uint32 interval
    )
    {
        m_PresentInterval = interval;
    }

    void AddPerFrameResource(
        ID3D12Resource *pResource
    )
    {
        static Lock _lock;

        ScopeLock lock( _lock );
        m_FrameResources[m_FrameIndex].resources.Add( pResource );
    }

    ID3D12Device *GetDevice( void ) { return m_pDevice; }
    ID3D12CommandQueue *GetGraphicsQueue( void ) { return m_pGraphicsQueue; }
    ID3D12CommandQueue *GetComputeQueue( void ) { return m_pComputeQueue; }
    const SwapChain *GetSwapChain( void ) const { return &m_SwapChain; }
    uint32 GetFrameIndex( void ) const { return m_FrameIndex; }

private:
    CommandList *CreateGraphicsCommandList( void );
    CommandList *CreateComputeCommandList( void );

    void FreePerFrameResources(
        int index
    );

private:
    FrameResources m_FrameResources[FrameCount];
    DescHeap m_ShaderDescHeap;
    DescHeap m_UploadDescHeap;
    DescHeap m_RtvDesc;
    DescHeap m_DsvDesc;

    IDXGIFactory4 *m_pFactory;
    ID3D12Device *m_pDevice;
    ID3D12CommandQueue *m_pGraphicsQueue;
    ID3D12CommandQueue *m_pComputeQueue;
    IDXGISwapChain3 *m_pSwapChain;
    ID3D12Fence *m_pGraphicsFence;
    ID3D12Fence *m_pComputeFence;
    ID3D12Resource *m_pRenderTargets[FrameCount];
    D3D12_CPU_DESCRIPTOR_HANDLE m_Rtvs[FrameCount];

    CommandList *m_pGraphicsPool[MaxCommandLists];
    CommandList *m_pComputePool[MaxCommandLists];

    CommandList *m_pMainThreadCommandList;

    SwapChain m_SwapChain;

    HANDLE m_GraphicsFenceEvent;
    HANDLE m_ComputeFenceEvent;
    
    ResourceHandle m_BackBuffer;

    uint32 m_PresentInterval;
    
    uint32 m_FrameIndex;
    uint32 m_PrevFrameIndex;
    
    int m_NumGraphicsCommandLists;
    int m_NumComputeCommandLists;

    uint64 m_FenceCount;

    bool m_ProgrammableSamplePositionsSupport;
};
