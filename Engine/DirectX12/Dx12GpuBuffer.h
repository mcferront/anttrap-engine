#pragma once

#ifndef DIRECTX12
#error This should not be included
#endif

#include "EngineGlobal.h"

#include "Dx12.h"
#include "Dx12GpuResource.h"

class GpuBuffer : public GpuResource
{
public:
    DeclareResourceType(GpuBuffer);

    GpuBuffer( void ) 
    {
        m_pUAV = NULL;
        m_pSRV = NULL;
        m_pDSV = NULL;
        m_pRTV = NULL;
        m_Stride = 0;
        m_IsBuffer = false;
    };

    void Create(
        Heap::Type heapType, 
        State::Type state,
        Flags::Type flags,
        Format::Type format, 
        size_t width, 
        size_t height, 
        uint32 sampleCount,
        uint32 stride,
        uint32 mipLevels,
        bool isBuffer,
        const Color *pClearColor,
        const void *pInitialData = NULL
    );

    GpuDevice::UnorderedAccessView *GetUav( void );
    GpuDevice::RenderTargetView *GetRtv( void );
    GpuDevice::DepthStencilView *GetDsv( void );
    GpuDevice::ShaderResourceView *GetSrv( void );

    void *Map( void );
    void Unmap( void );

    virtual void RemoveFromScene( void );

    void BuildSrvDesc(
        D3D12_SHADER_RESOURCE_VIEW_DESC *pDesc
    );

    void BuildUavDesc(
        D3D12_UNORDERED_ACCESS_VIEW_DESC *pDesc 
    );

    void BuildRtvDesc(
        D3D12_RENDER_TARGET_VIEW_DESC *pDesc
    );

    void BuildDsvDesc(
        D3D12_DEPTH_STENCIL_VIEW_DESC *pDesc
    );

    size_t GetWidth( void ) const { return (size_t) m_ResourceDesc.Width; }
    size_t GetHeight( void ) const { return (size_t) m_ResourceDesc.Height; }
    uint32 GetSampleCount( void ) const { return m_ResourceDesc.SampleDesc.Count; }
    uint32 GetNumMips( void ) const { return m_ResourceDesc.MipLevels; }

protected:
    // TODO: get rid of the need for this call
    friend class GpuDevice;
    void Create(
        ID3D12Resource *pResource,
        State::Type state
    );
    virtual void SwapApiResource(
        ID3D12Resource *pResource,
        State::Type state
    );

private:
    void DestroyViews( void );

private:
    D3D12_RESOURCE_DESC m_ResourceDesc;

    uint32 m_Stride;
    bool m_IsBuffer;

    GpuDevice::UnorderedAccessView *m_pUAV;
    GpuDevice::ShaderResourceView *m_pSRV;
    GpuDevice::DepthStencilView *m_pDSV;
    GpuDevice::RenderTargetView *m_pRTV;

public:
    static ResourceHandle CreateBuffer(
        const Id &id,
        Flags::Type flags,
        State::Type state,
        uint32 size,
        uint32 stride,
        bool cpuReadback = false
    );

    static ResourceHandle CreateTexture(
        const Id &id,
        Format::Type format,
        Flags::Type flags,
        State::Type state,
        size_t width,
        size_t height,
        uint32 mipLevels = 1,
        const Color *pClearColor = NULL,
        uint32 sampleCount = 1
    );
};

