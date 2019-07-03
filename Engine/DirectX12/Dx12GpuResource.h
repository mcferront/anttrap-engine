#pragma once

#ifndef DIRECTX12
#error This should not be included
#endif

#include "EngineGlobal.h"

#include "Dx12.h"

class GpuResource : public Resource
{
    friend class GpuDevice;

public:
    DeclareResourceType( GpuResource );

    struct Barrier
    {
        enum Type
        {
            Uav,
        };
    };

    struct State
    {
        enum Type
        {
            Unknown = -1,

            DepthWriteResource = D3D12_RESOURCE_STATE_DEPTH_WRITE,
            DepthReadResource = D3D12_RESOURCE_STATE_DEPTH_READ,
            RenderTarget = D3D12_RESOURCE_STATE_RENDER_TARGET,
            CopySource = D3D12_RESOURCE_STATE_COPY_SOURCE,
            CopyDest = D3D12_RESOURCE_STATE_COPY_DEST,
            PixelShaderResource = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
            ShaderResource = D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE | D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
            UnorderedAccess = D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
            Present = D3D12_RESOURCE_STATE_PRESENT,
            IndirectArg = D3D12_RESOURCE_STATE_INDIRECT_ARGUMENT,
            VertexAndConstantBuffer = D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER,
            GenericRead = D3D12_RESOURCE_STATE_GENERIC_READ,
            IndexBuffer = D3D12_RESOURCE_STATE_INDEX_BUFFER,
        };
    };

    struct Format
    {
        enum Type
        {
            Unknown = DXGI_FORMAT_UNKNOWN,
            OpaqueBuffer = DXGI_FORMAT_R16G16B16A16_FLOAT,
            TransparentBuffer = DXGI_FORMAT_R16G16B16A16_FLOAT,
            SpecularBuffer = DXGI_FORMAT_R16G16B16A16_FLOAT,
            MatProperties = DXGI_FORMAT_R16G16B16A16_FLOAT,
            NormalBuffer = DXGI_FORMAT_R16G16_FLOAT,
            LightMaskBuffer = DXGI_FORMAT_R32_UINT,
            HDR = DXGI_FORMAT_R16G16B16A16_FLOAT,
            ShadowMap = DXGI_FORMAT_R32_FLOAT,
            LDR = DXGI_FORMAT_R8G8B8A8_UNORM,
            LDR_SRGB = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB,
            SSAO = DXGI_FORMAT_R16G16_FLOAT,
            SSR = DXGI_FORMAT_R16G16_UINT,
            COC = DXGI_FORMAT_R8_UNORM,
            LinearZ = DXGI_FORMAT_R16_FLOAT,
            Depth = DXGI_FORMAT_R32_TYPELESS,
            Float = DXGI_FORMAT_R32_FLOAT,
            DepthFloat = DXGI_FORMAT_D32_FLOAT,
        };
    };

    struct Heap
    {
        enum Type
        {
            Default = D3D12_HEAP_TYPE_DEFAULT,
            Upload = D3D12_HEAP_TYPE_UPLOAD,
            Readback = D3D12_HEAP_TYPE_READBACK,
            Custom = D3D12_HEAP_TYPE_CUSTOM,
        };
    };

    struct Flags
    {
        enum Type
        {
            None = D3D12_RESOURCE_FLAG_NONE,
            RenderTarget = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET,
            DepthStencil = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL,
            UnorderedAccess = D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS,
            DenyShaderResource = D3D12_RESOURCE_FLAG_DENY_SHADER_RESOURCE,
            CrossAdapter = D3D12_RESOURCE_FLAG_ALLOW_CROSS_ADAPTER,
            SimultaneousAccess = D3D12_RESOURCE_FLAG_ALLOW_SIMULTANEOUS_ACCESS,
        };
    };

    GpuResource( void );

    virtual ~GpuResource( void )
    {
        Debug::Assert( Condition( NULL == m_pResource ), "GpuResource not destroyed" );
    }

    virtual void Destroy( void )
    {
        if ( NULL != m_pResource )
            m_pResource->Release( );

        m_pResource = NULL;

        Resource::Destroy( );
    }

    virtual void AddToScene( void );
    virtual void RemoveFromScene( void );

    void TransitionTo(
        GpuDevice::CommandList *,
        State::Type state
    );

    ID3D12Resource *GetApiResource( ) { return m_pResource; }
    State::Type GetState( void ) const { return m_State; }

protected:
    void Create( State::Type state ) { m_State = state; }

    virtual void SwapApiResource(
        ID3D12Resource *pResource,
        State::Type state
    );

private:
    State::Type m_State;
    Lock m_Lock;

protected:
    ID3D12Resource *m_pResource;
};
