#pragma once

#include "EngineGlobal.h"
#include "Asset.h"
#include "Dx12.h"

class Texture : public Asset
{
    friend class TextureSerializer;

public:
    struct Format
    {
        enum Type
        {
            OpaqueBuffer = DXGI_FORMAT_R11G11B10_FLOAT,
            TransparentBuffer = DXGI_FORMAT_R16G16B16A16_FLOAT,
            SpecularBuffer = DXGI_FORMAT_R11G11B10_FLOAT,
            MatProperties = DXGI_FORMAT_R16G16B16A16_FLOAT,
            NormalBuffer = DXGI_FORMAT_R16G16_FLOAT,
            LightMaskBuffer = DXGI_FORMAT_R32_UINT,
            HDR = DXGI_FORMAT_R11G11B10_FLOAT,
            ShadowMap = DXGI_FORMAT_R32_FLOAT,
            LDR = DXGI_FORMAT_R8G8B8A8_UNORM,
            SSAO = DXGI_FORMAT_R16G16_FLOAT,
            SSR = DXGI_FORMAT_R16G16_UINT,
            COC = DXGI_FORMAT_R8_UNORM,
            LinearZ = DXGI_FORMAT_R16_FLOAT,
        };
    };

    struct Barrier
    {
        enum Type
        {
            Uav = D3D12_RESOURCE_BARRIER_TYPE_UAV,
        };
    };

    enum ViewType
    {
        Unknown = -1,

        DepthWriteResource = D3D12_RESOURCE_STATE_DEPTH_WRITE,
        DepthReadResource = D3D12_RESOURCE_STATE_DEPTH_READ,
        RenderTarget = D3D12_RESOURCE_STATE_RENDER_TARGET,
        CopySource = D3D12_RESOURCE_STATE_COPY_SOURCE,
        CopyDest = D3D12_RESOURCE_STATE_COPY_DEST,
        PixelShaderResource = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
        ShaderResource = D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE | D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
        UavResource = D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
        Present = D3D12_RESOURCE_STATE_PRESENT,
        IndirectArg = D3D12_RESOURCE_STATE_INDIRECT_ARGUMENT,
    };

private:
    ID3D12Resource * m_pResource;
    ID3D12DescriptorHeap *m_pDescHeap;
    D3D12_SHADER_RESOURCE_VIEW_DESC m_SRVDesc;
    uint32  m_DSFormat;
    uint32  m_RTFormat;
    uint32  m_UAVFormat;

    GpuDevice::GpuHandle m_GpuHandle;

    Lock     m_ConvertLock;
    ViewType m_ViewType;
    uint32   m_NumMips;
    uint32   m_CounterOffset;
    uint32   m_StructuredBufferSize;
    bool     m_HasCounter;

public:
    Texture(void)
    {
        m_pResource = NULL;
    }

    virtual void Create(void)
    {
        m_GpuHandle = GpuDevice::GpuHandle::Invalid;

        m_pDescHeap = NULL;
    }

    void Create(
        ID3D12Resource *pResource,
        ViewType viewType
    )
    {
        if (NULL != m_pResource)
            m_pResource->Release();

        m_pResource = pResource;
        m_pDescHeap = NULL;
        m_GpuHandle = GpuDevice::GpuHandle::Invalid;
        m_ViewType = viewType;

        if (m_pResource != NULL)
            m_pResource->AddRef();
    }

    void CreateAsTarget(
        Format::Type format,
        ViewType type,
        const Color &clearColor,
        uint32 width,
        uint32 height,
        byte sampleCount,
        byte mipLevel = 1
    );

    void CreateAsDepthStencil(
        uint32 width,
        uint32 height,
        byte sampleCount
    );

    void CreateAsStructuredBuffer(
        uint32 size,
        bool appendCounter = false
    );

    void CreateAsCopyDest(
        uint32 size
    );

    byte GetSampleCount(void) const
    {
        return m_pResource->GetDesc().SampleDesc.Count;
    }

    virtual void Destroy(void)
    {
        if (m_GpuHandle != GpuDevice::GpuHandle::Invalid)
            GpuDevice::Instance().ClearGpuHandle(m_GpuHandle);

        m_GpuHandle = GpuDevice::GpuHandle::Invalid;

        if (NULL != m_pResource)
            m_pResource->Release();

        if (NULL != m_pDescHeap)
            m_pDescHeap->Release();

        Asset::Destroy();
    }

    virtual void Bind(void);

    void ConvertTo(
        ViewType viewType,
        GpuDevice::CommandList *pCommandList
    );

    void Barrier(
        Barrier::Type type,
        GpuDevice::CommandList *pCommandList
    );

    bool RequiresConvert(ViewType viewType) const { return m_ViewType != viewType; }

    int GetDesiredWidth(void) const { return GetActualWidth(); }
    int GetDesiredHeight(void) const { return GetActualHeight(); }

    int GetActualWidth(void) const { return (int) m_pResource->GetDesc().Width; }
    int GetActualHeight(void) const { return (int) m_pResource->GetDesc().Height; }

    ID3D12Resource *GetD3D12Resource() const { return m_pResource; }
    ID3D12DescriptorHeap *GetDescHeap() const { return m_pDescHeap; }

    ViewType GetViewType(void) const { return m_ViewType; }

    uint32 GetDepthStencilFormat(void) const { return m_DSFormat; }
    uint32 GetRenderTargetFormat(void) const { return m_RTFormat; }

    GpuDevice::GpuHandle GetGpuHandle(void) const { return m_GpuHandle; }

    DeclareResourceType(Texture);
};


class TextureSerializer : public ISerializer
{
public:
    virtual bool Serialize(
        Serializer *pSerializer,
        const ISerializable *pSerializable
    ) {
        return false;
    }

    virtual ISerializable *Deserialize(
        Serializer *pSerializer,
        ISerializable *pSerializable
    );

    virtual ISerializable *Instantiate() const { return new Texture; }

    virtual const SerializableType &GetSerializableType(void) const { return Texture::StaticSerializableType(); }

    virtual uint32 GetVersion(void) const { return 2; }
};
