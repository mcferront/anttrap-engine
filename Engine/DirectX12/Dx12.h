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
typedef Texture ImageBuffer;

class GpuDevice
{
public:
    static const int FrameCount = 2;

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
   struct GpuHandle
   {
      struct InternalRef
      {
         Id id;
         uint32 uavOffset;
         uint32 srvOffset;
         uint32 cbvOffset;
      };

      InternalRef *_internal;

      static GpuHandle Invalid;

      bool operator == (const GpuHandle &rhs) const
      {
         return _internal == rhs._internal;
      }

      bool operator != (const GpuHandle &rhs) const
      {
         return _internal != rhs._internal;
      }
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
            pList1->Release();

         pAllocator->Release( );
         pList->Release( );
      }

      bool SetSamplePositions(
         const Vector2 *pPositions,
         uint32 numPositions
      );

   public:
      ID3D12CommandAllocator *pAllocator;
      ID3D12GraphicsCommandList *pList;
      bool hasProgrammableSamplePositionsSupport;
      bool isCompute;

   private:
      ID3D12GraphicsCommandList1 *pList1;
   };

   struct FrameResources
   {
       CommandList *pGraphicsCommandLists[ MaxCommandLists ];
       CommandList *pComputeCommandLists[ MaxCommandLists ];

       int numGraphicsCommandLists;
       int numComputeCommandLists;

       List<ID3D12Resource*> resources;
   };

private:
   static const int MaxDescriptors = 1000000;

public:
   static GpuDevice &Instance( void );

   static void ClearRenderTarget(
      CommandList *pCommandList,
      ImageBuffer *pRenderTarget,
      bool clearColor,
      const Color *pColorValue
      );

   static void ClearDepthStencil(
      CommandList *pCommandList,
      ImageBuffer *pDepthStencil,
      bool clearDepth,
      float depthValue,
      bool clearStencil,
      byte stencilValue
   );

   static void SetRenderTargets(
      CommandList *pCommandList,
      ImageBuffer *pRenderTargets[],
      uint32 numRenderTargets,
      ImageBuffer *pDepthStencil
   );

   static void CopyResource(
      CommandList *pList,
      ImageBuffer *pDest,
      ImageBuffer *pSource
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

   CommandList *AllocGraphicsCommandList( void );
   CommandList *AllocComputeCommandList( void );

   void ExecuteCommandLists( 
      CommandList *pCommandLists[],
      uint32 count 
   );

   GpuHandle CreateGpuHandle(
      const Id &id
   );

   void SetGpuCbv(
      GpuHandle gpuHandle,
      const D3D12_CONSTANT_BUFFER_VIEW_DESC &desc
      );

   void SetGpuSrv(
      GpuHandle gpuHandle,
      ID3D12Resource *pResource,
      const D3D12_SHADER_RESOURCE_VIEW_DESC &desc
   );

   void SetGpuUav(
      GpuHandle gpuHandle,
      ID3D12Resource *pResource,
      const D3D12_UNORDERED_ACCESS_VIEW_DESC &desc,
      bool hasCounter = false
   );

   bool CheckSupport(
       GpuDevice::Support::Feature feature
   );

   void SetPresentInterval(
      uint32 interval
   )
   {
      m_PresentInterval = interval;
   }

   void ClearGpuHandle(
      GpuHandle handle
      )
   {
      MainThreadCheck;

      GpuHandle::InternalRef *pRef;

      if( NULL != handle._internal )
      {
         if ( m_ViewOffsets.Remove(handle._internal->id, NULL, &pRef) )
            m_FreeViewOffsets.Add( pRef );
      }
   }

   D3D12_GPU_DESCRIPTOR_HANDLE GetGpuSrvHandle(
      const GpuHandle &gpuHandle
      )
   {
      D3D12_GPU_DESCRIPTOR_HANDLE handle = m_GpuBaseHandle;
		Debug::Assert( Condition(gpuHandle._internal->srvOffset != -1), "Invalid SRV Handle" );
		
		handle.ptr += gpuHandle._internal->srvOffset;

      return handle;
   }
   
   D3D12_GPU_DESCRIPTOR_HANDLE GetGpuCbvHandle(
      const GpuHandle &gpuHandle
   )
   {
      D3D12_GPU_DESCRIPTOR_HANDLE handle = m_GpuBaseHandle;
		Debug::Assert( Condition(gpuHandle._internal->cbvOffset != -1), "Invalid CBV Handle" );
		
		handle.ptr += gpuHandle._internal->cbvOffset;

      return handle;
   }

   D3D12_GPU_DESCRIPTOR_HANDLE GetGpuUavHandle(
      const GpuHandle &gpuHandle
   )
   {
      D3D12_GPU_DESCRIPTOR_HANDLE handle = m_GpuBaseHandle;
      Debug::Assert( Condition(gpuHandle._internal->uavOffset != -1), "Invalid UAV Handle" );

		handle.ptr += gpuHandle._internal->uavOffset;

      return handle;
   }

   void AddPerFrameResource(
      ID3D12Resource *pResource
      )
   {
       static Lock _lock;

       ScopeLock lock( _lock );
           m_FrameResources[ m_FrameIndex ].resources.Add( pResource );
   }

   ID3D12Device *GetDevice( void ) { return m_pDevice; }
   ID3D12DescriptorHeap *GetDescHeap( void ) { return m_pDescHeap; }
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

   GpuHandle::InternalRef *CreateGpuRef(
      const Id &id
   );

   uint32 GetNextCpuOffset( void )
   {
      MainThreadCheck;

      Debug::Assert( Condition(m_NumDescriptors < MaxDescriptors), "Out of GPU Descriptors" );

      return m_DescHandleIncSize * m_NumDescriptors++;
   }

private:
   FrameResources m_FrameResources[ FrameCount ];

   IDXGIFactory4 *m_pFactory;
   ID3D12Device *m_pDevice;
   ID3D12CommandQueue *m_pGraphicsQueue;
   ID3D12CommandQueue *m_pComputeQueue;
   IDXGISwapChain3 *m_pSwapChain;
   ID3D12DescriptorHeap *m_pRTVDescHeap;
   ID3D12DescriptorHeap *m_pDescHeap;
   ID3D12Fence *m_pGraphicsFence;
   ID3D12Fence *m_pComputeFence;
   ID3D12Resource *m_pRenderTargets[ FrameCount ];

   CommandList *m_pGraphicsPool[ MaxCommandLists ];
   CommandList *m_pComputePool[ MaxCommandLists ];
   //CommandList *m_pGraphicsThreadLists[ MaxThreads ];
   //CommandList *m_pComputeThreadLists[ MaxThreads ];

   D3D12_CPU_DESCRIPTOR_HANDLE m_CpuBaseHandle;
   D3D12_GPU_DESCRIPTOR_HANDLE m_GpuBaseHandle;
   SwapChain m_SwapChain;
   
   HANDLE m_GraphicsFenceEvent;
   HANDLE m_ComputeFenceEvent;
   HashTable<Id, GpuHandle::InternalRef *> m_ViewOffsets;
   List<GpuHandle::InternalRef *> m_FreeViewOffsets;
   
   ResourceHandle m_BackBuffer;

   uint32 m_PresentInterval;
   uint32 m_NumDescriptors;
   uint32 m_DescHandleIncSize;
   uint32 m_FrameIndex;
   uint32 m_PrevFrameIndex;
   uint32 m_RtvDescSize;

   int m_NumGraphicsCommandLists;
   int m_NumComputeCommandLists;

   uint64 m_FenceCount;

   bool m_ProgrammableSamplePositionsSupport;
};
