#include "EnginePch.h"

#include "MaterialAsset.h"
#include "TextureAsset.h"
#include "Dx12Contexts.h"
#include "ShaderAsset.h"
#include "Renderer.h"
#include "Viewport.h"
#include "Dx12.h"
#include "LightComponent.h"
#include "RenderWorld.h"
#include "Log.h"

DefineResourceType( Material, Asset, new MaterialSerializer );

void Material::Destroy( void )
{
    delete m_pGraphicsMaterial;
    delete m_pComputeMaterial;

    Asset::Destroy( );
}

ComputeMaterial::~ComputeMaterial( void )
{
    delete[ ] m_pPassDatas;
}

void ComputeMaterial::PassData::CloneTo(
    ComputeMaterial::PassData *pPassData
) const
{
    pPassData->header = header;
    pPassData->shader = shader;
    pPassData->groupSizeTarget = groupSizeTarget;

    pPassData->psoDesc = psoDesc;
    pPassData->pName = StringRef( pName );

    pPassData->pFloat4s = new ComputeMaterial::PassData::Float4[ pPassData->header.numFloat4Names ];
    pPassData->pMatrix4s = new ComputeMaterial::PassData::Matrix4[ pPassData->header.numMatrix4Names ];
    pPassData->pBuffers = new PassData::Buffer[ pPassData->header.numBuffers ];

    for ( int c = 0; c < pPassData->header.numBuffers; c++ )
    {
        pPassData->pBuffers[ c ].pName = StringRef( pBuffers[ c ].pName );
        pPassData->pBuffers[ c ].header = pBuffers[ c ].header;
        pPassData->pBuffers[ c ].buffer = pBuffers[ c ].buffer;
    }

    for ( int c = 0; c < pPassData->header.numFloat4Names; c++ )
    {
        pPassData->pFloat4s[ c ].pName = StringRef( pFloat4s[ c ].pName );
        pPassData->pFloat4s[ c ].pRef = StringRef( pFloat4s[ c ].pRef );
        pPassData->pFloat4s[ c ].offset = pFloat4s[ c ].offset;
    }

    for ( int c = 0; c < pPassData->header.numMatrix4Names; c++ )
    {
        pPassData->pMatrix4s[ c ].pName = StringRef( pMatrix4s[ c ].pName );
        pPassData->pMatrix4s[ c ].pRef = StringRef( pMatrix4s[ c ].pRef );
        pPassData->pMatrix4s[ c ].offset = pMatrix4s[ c ].offset;
    }

    GraphicsMaterial::CreateConstantBuffer( &pPassData->constantBuffer, constantBuffer.desc.SizeInBytes );

    if ( NULL != pPassData->constantBuffer.pData )
    {
        memcpy( pPassData->constantBuffer.pData, constantBuffer.pData, sizeof( Vector ) * pPassData->header.totalFloat4s );
        pPassData->constantBuffer.pCBV = GpuDevice::Instance( ).CreateCbv( pPassData->constantBuffer.desc );     
    }
}

GraphicsMaterial::~GraphicsMaterial( void )
{
    delete[ ] m_pPassDatas;
}

void GraphicsMaterial::PassData::CloneTo(
    PassData *pPassData ) const
{
    pPassData->header = header;
    pPassData->shader = shader;

    pPassData->pFloat4s = NULL;
    pPassData->pMatrix4s = NULL;
    pPassData->pTextures = NULL;

    pPassData->pFloat4s = new GraphicsMaterial::PassData::Float4[ pPassData->header.numFloat4Names ];
    pPassData->pMatrix4s = new GraphicsMaterial::PassData::Matrix4[ pPassData->header.numMatrix4Names ];
    pPassData->pTextures = new GraphicsMaterial::PassData::Texture[ pPassData->header.numTextures ];

    for ( int c = 0; c < pPassData->header.numTextures; c++ )
    {
        pPassData->pTextures[ c ].pName = StringRef( pTextures[ c ].pName );
        pPassData->pTextures[ c ].texture = pTextures[ c ].texture;
        pPassData->pTextures[ c ].header = pTextures[ c ].header;
    }

    if ( pPassData->header.numTextures > 0 )
    {
        D3D12_SHADER_RESOURCE_VIEW_DESC descs[64];
        GpuResource *pResources[64];

        Debug::Assert( Condition(pPassData->header.numTextures <= _countof(descs)), "Need to increase temp array" );

        for (int i = 0; i < pPassData->header.numTextures; i++)
        {
            GetResource(pPassData->pTextures[i].texture, GpuBuffer)->BuildSrvDesc( &descs[i] );
            pResources[i] = GetResource(pPassData->pTextures[i].texture, GpuBuffer);
        }
    
        pPassData->pSRVs = GpuDevice::Instance().CreateSrvRange( descs, pResources, pPassData->header.numTextures );
    }

    for ( int c = 0; c < pPassData->header.numFloat4Names; c++ )
    {
        pPassData->pFloat4s[ c ].pName = StringRef( pFloat4s[ c ].pName );
        pPassData->pFloat4s[ c ].pRef = StringRef( pFloat4s[ c ].pRef );
        pPassData->pFloat4s[ c ].offset = pFloat4s[ c ].offset;
    }

    for ( int c = 0; c < pPassData->header.numMatrix4Names; c++ )
    {
        pPassData->pMatrix4s[ c ].pName = StringRef( pMatrix4s[ c ].pName );
        pPassData->pMatrix4s[ c ].pRef = StringRef( pMatrix4s[ c ].pRef );
        pPassData->pMatrix4s[ c ].offset = pMatrix4s[ c ].offset;
    }

    GraphicsMaterial::CreateConstantBuffer( &pPassData->constantBuffer, constantBuffer.desc.SizeInBytes );

    if ( NULL != pPassData->constantBuffer.pData )
    {
        memcpy( pPassData->constantBuffer.pData, constantBuffer.pData, sizeof( Matrix ) * pPassData->header.totalMatrix4s + sizeof( Vector ) * pPassData->header.totalFloat4s );
        pPassData->constantBuffer.pCBV = GpuDevice::Instance( ).CreateCbv( pPassData->constantBuffer.desc );     
    }

    pPassData->psoDesc = psoDesc;
    pPassData->pName = StringRef( pName );
}

bool GraphicsMaterial::CreateConstantBuffer(
    ConstantBuffer *pBuffer,
    uint32 size
)
{
    byte *pConstantData;
    ID3D12Resource *pConstantBuffer;
    HRESULT hr;

    pBuffer->pResource = NULL;
    pBuffer->pData = NULL;
    pBuffer->pCBV = NULL;
    pBuffer->desc.SizeInBytes = 0;

    if ( 0 == size )
        return true;

    // Create the constant buffer.
    D3D12_HEAP_PROPERTIES heapProperties = { };
    heapProperties.Type = D3D12_HEAP_TYPE_UPLOAD;
    heapProperties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
    heapProperties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
    heapProperties.CreationNodeMask = 1;
    heapProperties.VisibleNodeMask = 1;

    D3D12_RESOURCE_DESC resourceDesc = { };
    resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
    resourceDesc.Alignment = 0;
    resourceDesc.Width = Align( size, 256 );
    resourceDesc.Height = 1;
    resourceDesc.DepthOrArraySize = 1;
    resourceDesc.MipLevels = 1;
    resourceDesc.Format = DXGI_FORMAT_UNKNOWN;
    resourceDesc.SampleDesc.Count = 1;
    resourceDesc.SampleDesc.Quality = 0;
    resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
    resourceDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

    do
    {
        hr = GpuDevice::Instance( ).GetDevice( )->CreateCommittedResource(
            &heapProperties,
            D3D12_HEAP_FLAG_NONE,
            &resourceDesc,
            D3D12_RESOURCE_STATE_GENERIC_READ,
            NULL,
            __uuidof( ID3D12Resource ),
            (void **) &pConstantBuffer );

        //LOG( "CreateCommittedResource - ConstantBuffer" );
        Debug::Assert( Condition( SUCCEEDED( hr ) ), "Failed to CreateCommittedResource (ConstantBuffer) (0x%08x)", hr );
        BreakIf( FAILED( hr ) );

        // Initialize and map the constant buffers. We don't unmap this until the
        // app closes. Keeping things mapped for the lifetime of the resource is okay.
        D3D12_RANGE readRange = { };
        hr = pConstantBuffer->Map( 0, &readRange, (void **) &pConstantData );
        //LOG( "CreateCommittedResource - ConstantBuffer" );
        Debug::Assert( Condition( SUCCEEDED( hr ) ), "Failed to pConstantBuffer->Map (0x%08x)", hr );
        BreakIf( FAILED( hr ) );

        D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc = {};
        cbvDesc.BufferLocation = pConstantBuffer->GetGPUVirtualAddress( );
        cbvDesc.SizeInBytes = Align( size, 256 );	// CB size is required to be 256-byte aligned.

        pBuffer->pResource = pConstantBuffer;
        pBuffer->pData = pConstantData;
        pBuffer->desc = cbvDesc;

        pBuffer->pResource->SetName( String::ToWideChar(pBuffer->id.ToString()) );

        return true;

    } while ( 0 );

    return false;
}

ISerializable *MaterialSerializer::Deserialize(
    Serializer *pSerializer,
    ISerializable *pSerializable
)
{
    const byte Version = 2;

    struct Header
    {
        byte version;
        byte type;
    };

    Header header;

    if ( NULL == pSerializable ) pSerializable = new Material;

    Material *pMaterial = (Material *) pSerializable;
    pSerializer->GetInputStream( )->Read( &header, sizeof( header ), NULL );
    Debug::Assert( Condition( header.version == Version ), "Incorrect material version: %d, expecting: %d", header.version, Version );

    if ( header.type == 0 )
        return DeserializeGraphicsMaterial( pSerializer, pSerializable );
    else
        return DeserializeComputeMaterial( pSerializer, pSerializable );
}

ISerializable *MaterialSerializer::DeserializeComputeMaterial(
    Serializer *pSerializer,
    ISerializable *pSerializable
)
{
    struct Header
    {
        byte numPasses;
    };

    Header header;

    if ( NULL == pSerializable ) pSerializable = new Material;

    Material *pMaterial = (Material *) pSerializable;

    pSerializer->GetInputStream( )->Read( &header, sizeof( header ), NULL );

    ComputeMaterial::PassData *pPasses;
    if ( header.numPasses > 0 )
        pPasses = new ComputeMaterial::PassData[ header.numPasses ];
    else
        pPasses = NULL;

    for ( int i = 0; i < header.numPasses; i++ )
    {
        ComputeMaterial::PassData *pPass = &pPasses[ i ];

        pPass->pBuffers = NULL;
        pPass->pFloat4s = NULL;
        pPass->pMatrix4s = NULL;
        pPass->constantBuffer.pData = NULL;
        pPass->constantBuffer.pResource = NULL;

        pSerializer->GetInputStream( )->Read( &pPass->header, sizeof( pPass->header ) );

        pPass->pName = StringPool::Deserialize( pSerializer->GetInputStream( ) );
        pPass->shader = ResourceHandle( Id::Deserialize( pSerializer->GetInputStream( ) ) );

        pPass->pBuffers = new ComputeMaterial::PassData::Buffer[ pPass->header.numBuffers ];

        uint32 constantBufferSize =
            sizeof( Vector ) * pPass->header.totalFloat4s + sizeof( Matrix ) * pPass->header.totalMatrix4s;

        if ( pPass->header.numFloat4Names )
            pPass->pFloat4s = new ComputeMaterial::PassData::Float4[ pPass->header.numFloat4Names ];

        if ( pPass->header.numMatrix4Names )
            pPass->pMatrix4s = new ComputeMaterial::PassData::Matrix4[ pPass->header.numMatrix4Names ];

        for ( int c = 0; c < pPass->header.numBuffers; c++ )
        {
            pSerializer->GetInputStream( )->Read( &pPass->pBuffers[ c ].header, sizeof( pPass->pBuffers[ c ].header ) );

            pPass->pBuffers[ c ].pName = StringPool::Deserialize( pSerializer->GetInputStream( ) );
            pPass->pBuffers[ c ].buffer = ResourceHandle( Id::Deserialize( pSerializer->GetInputStream( ) ) );
        }

        bool result = GraphicsMaterial::CreateConstantBuffer( &pPass->constantBuffer, constantBufferSize );
        BreakIf( false == result );

        int float4ValueOffset = 0;
        int matrix4ValueOffset = 0;

        for ( int c = 0; c < pPass->header.numFloat4Names; c++ )
        {
            const char *pName = StringPool::Deserialize( pSerializer->GetInputStream( ) );
            const char *pRef = StringPool::Deserialize( pSerializer->GetInputStream( ) );

            int amount;
            pSerializer->GetInputStream( )->Read( &amount, sizeof( amount ) );

            pPass->pFloat4s[ c ].offset = float4ValueOffset;
            float4ValueOffset += amount * sizeof( Vector );

            pPass->pFloat4s[ c ].pName = pName;
            pPass->pFloat4s[ c ].pRef = pRef;
            pSerializer->GetInputStream( )->Read( pPass->constantBuffer.pData + pPass->pFloat4s[ c ].offset, sizeof( Vector ) );
        }

        for ( int c = 0; c < pPass->header.numMatrix4Names; c++ )
        {
            const char *pName = StringPool::Deserialize( pSerializer->GetInputStream( ) );
            const char *pRef = StringPool::Deserialize( pSerializer->GetInputStream( ) );

            int amount;
            pSerializer->GetInputStream( )->Read( &amount, sizeof( amount ) );

            pPass->pMatrix4s[ c ].offset = matrix4ValueOffset + float4ValueOffset;
            matrix4ValueOffset += amount * sizeof( Matrix );

            pPass->pMatrix4s[ c ].pName = pName;
            pPass->pMatrix4s[ c ].pRef = pRef;
            pSerializer->GetInputStream( )->Read( pPass->constantBuffer.pData + pPass->pMatrix4s[ c ].offset, sizeof( Matrix ) );
        }

        const char *pGroupSizeTarget = StringPool::Deserialize( pSerializer->GetInputStream( ) );

        if ( NULL != *pGroupSizeTarget )
            pPass->groupSizeTarget = ResourceHandle( pGroupSizeTarget );
        else
            pPass->groupSizeTarget = NullHandle;


        const int MaxBuffers = 16;

        int descIndex = 0;
        int samplerIndex = 0;
        int rootParamIndex = 0;

        D3D12_DESCRIPTOR_RANGE descRange[ MaxBuffers + 1 ]; // +1 for constant buffer
        D3D12_ROOT_PARAMETER rootParameters[ MaxBuffers + 2 ]; // +2 for each constant buffer
        D3D12_STATIC_SAMPLER_DESC samplers[ MaxBuffers ];

        // constant buffer
        if ( constantBufferSize > 0 )
        {
            rootParameters[ rootParamIndex ].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
            rootParameters[ rootParamIndex ].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
            rootParameters[ rootParamIndex ].Descriptor.ShaderRegister = 0;
            rootParameters[ rootParamIndex ].Descriptor.RegisterSpace = 0;
            ++rootParamIndex;
        }

        // buffers
        int srvReg = 0;
        int uavReg = 0;
        int samplerShaderRegister = 0;
        for ( int c = 0; c < pPass->header.numBuffers; c++ )
        {
            int baseReg;

            if ( pPass->pBuffers[ c ].header.type == ComputeMaterial::PassData::Buffer::UAV )
            {
                descRange[ descIndex ].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_UAV;
                baseReg = uavReg++;
            }
            else if ( pPass->pBuffers[ c ].header.type == ComputeMaterial::PassData::Buffer::SRV )
            {
                descRange[ descIndex ].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
                baseReg = srvReg++;
            }
            else
                Debug::Assert( Condition( false ), "Unrecognized compute buffer type: %d", pPass->pBuffers[ c ].header.type );

            descRange[ descIndex ].NumDescriptors = 1;
            descRange[ descIndex ].BaseShaderRegister = baseReg;
            descRange[ descIndex ].RegisterSpace = 0;
            descRange[ descIndex ].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
            ++descIndex;

            rootParameters[ rootParamIndex ].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
            rootParameters[ rootParamIndex ].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
            rootParameters[ rootParamIndex ].DescriptorTable.NumDescriptorRanges = 1;
            rootParameters[ rootParamIndex ].DescriptorTable.pDescriptorRanges = &descRange[ descIndex - 1 ];
            ++rootParamIndex;

            if ( pPass->pBuffers[ c ].header.hasSampler )
            {
                D3D12_FILTER filter;

                //TODO: DX12 better converter
                switch ( pPass->pBuffers[ c ].header.filter )
                {
                case 1:
                    filter = D3D12_FILTER_MIN_MAG_MIP_POINT;
                    break;
                case 2:
                    filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
                    break;

                case 9:
                    filter = D3D12_FILTER_COMPARISON_MIN_MAG_LINEAR_MIP_POINT;
                    break;

                case 10:
                    filter = D3D12_FILTER_COMPARISON_MIN_MAG_MIP_POINT;
                    break;
                }

                samplers[ samplerIndex ].Filter = filter;

                //TODO: DX12 better converter
                samplers[ samplerIndex ].AddressU = (D3D12_TEXTURE_ADDRESS_MODE) pPass->pBuffers[ c ].header.address;
                samplers[ samplerIndex ].AddressV = (D3D12_TEXTURE_ADDRESS_MODE) pPass->pBuffers[ c ].header.address;
                samplers[ samplerIndex ].AddressW = (D3D12_TEXTURE_ADDRESS_MODE) pPass->pBuffers[ c ].header.address;
                samplers[ samplerIndex ].MipLODBias = 0;
                samplers[ samplerIndex ].MaxAnisotropy = 0;
                samplers[ samplerIndex ].ComparisonFunc = D3D12_COMPARISON_FUNC_LESS;
                samplers[ samplerIndex ].BorderColor = D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK;
                samplers[ samplerIndex ].MinLOD = 0.0f;
                samplers[ samplerIndex ].MaxLOD = D3D12_FLOAT32_MAX;
                samplers[ samplerIndex ].ShaderRegister = samplerShaderRegister++;
                samplers[ samplerIndex ].RegisterSpace = 0;
                samplers[ samplerIndex ].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
                ++samplerIndex;
            }
        }

        // store the descriptor tables in the root signature
        D3D12_ROOT_SIGNATURE_DESC rootSignatureDesc = {};
        rootSignatureDesc.NumParameters = rootParamIndex;
        rootSignatureDesc.pParameters = rootParameters;
        rootSignatureDesc.NumStaticSamplers = samplerIndex;
        rootSignatureDesc.pStaticSamplers = samplers;
        rootSignatureDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

        ID3D12RootSignature *pRootSignature = RenderContexts::RegisterRootSignature( rootSignatureDesc );

        D3D12_COMPUTE_PIPELINE_STATE_DESC psoDesc = { };
        psoDesc.pRootSignature = pRootSignature;

        pPass->psoDesc = psoDesc;

        Shader *pShader = GetResource( pPass->shader, Shader );
        pShader->SetPSO( &pPass->psoDesc );
    }

    ComputeMaterial *pCompute = new ComputeMaterial;
    pCompute->m_NumPasses = header.numPasses;
    pCompute->m_pPassDatas = pPasses;

    pMaterial->m_pGraphicsMaterial = NULL;
    pMaterial->m_pComputeMaterial = pCompute;

    return pSerializable;
}

ISerializable *MaterialSerializer::DeserializeGraphicsMaterial(
    Serializer *pSerializer,
    ISerializable *pSerializable
)
{
    struct Header
    {
        byte numPasses;
    };

    Header header;

    if ( NULL == pSerializable ) pSerializable = new Material;

    Material *pMaterial = (Material *) pSerializable;

    pSerializer->GetInputStream( )->Read( &header, sizeof( header ), NULL );

    GraphicsMaterial::PassData *pPasses;
    if ( header.numPasses > 0 )
        pPasses = new GraphicsMaterial::PassData[ header.numPasses ];
    else
        pPasses = NULL;

    for ( int i = 0; i < header.numPasses; i++ )
    {
        GraphicsMaterial::PassData *pPass = &pPasses[ i ];

        pPass->pFloat4s = NULL;
        pPass->pMatrix4s = NULL;
        pPass->pTextures = NULL;
        pPass->constantBuffer.pData = NULL;
        pPass->constantBuffer.pResource = NULL;

        pSerializer->GetInputStream( )->Read( &pPass->header, sizeof( pPass->header ) );

        pPass->pName = StringPool::Deserialize( pSerializer->GetInputStream( ) );
        pPass->shader = ResourceHandle( Id::Deserialize( pSerializer->GetInputStream( ) ) );

        uint32 constantBufferSize =
            sizeof( Vector ) * pPass->header.totalFloat4s +
            sizeof( Matrix ) * pPass->header.totalMatrix4s;

        if ( pPass->header.numFloat4Names )
            pPass->pFloat4s = new GraphicsMaterial::PassData::Float4[ pPass->header.numFloat4Names ];

        if ( pPass->header.numMatrix4Names )
            pPass->pMatrix4s = new GraphicsMaterial::PassData::Matrix4[ pPass->header.numMatrix4Names ];

        if ( pPass->header.numTextures )
            pPass->pTextures = new GraphicsMaterial::PassData::Texture[ pPass->header.numTextures ];

        for ( int c = 0; c < pPass->header.numTextures; c++ )
        {
            pPass->pTextures[ c ].pName = StringPool::Deserialize( pSerializer->GetInputStream( ) );
            pPass->pTextures[ c ].texture = ResourceHandle( Id::Deserialize( pSerializer->GetInputStream( ) ) );
            pSerializer->GetInputStream( )->Read( &pPass->pTextures[ c ].header, sizeof( pPass->pTextures[ c ].header ) );
        }

        bool result = GraphicsMaterial::CreateConstantBuffer( &pPass->constantBuffer, constantBufferSize );
        BreakIf( false == result );

        int float4ValueOffset = 0;

        for ( int c = 0; c < pPass->header.numFloat4Names; c++ )
        {
            const char *pName = StringPool::Deserialize( pSerializer->GetInputStream( ) );
            const char *pRef = StringPool::Deserialize( pSerializer->GetInputStream( ) );

            int amount;
            pSerializer->GetInputStream( )->Read( &amount, sizeof( amount ) );

            pPass->pFloat4s[ c ].offset = float4ValueOffset;
            float4ValueOffset += amount * sizeof( Vector );

            pPass->pFloat4s[ c ].pName = pName;
            pPass->pFloat4s[ c ].pRef = pRef;
            pSerializer->GetInputStream( )->Read( pPass->constantBuffer.pData + pPass->pFloat4s[ c ].offset, sizeof( Vector ) );
        }

        int matrix4ValueOffset = 0;

        for ( int c = 0; c < pPass->header.numMatrix4Names; c++ )
        {
            const char *pName = StringPool::Deserialize( pSerializer->GetInputStream( ) );
            const char *pRef = StringPool::Deserialize( pSerializer->GetInputStream( ) );

            int amount;
            pSerializer->GetInputStream( )->Read( &amount, sizeof( amount ) );

            pPass->pMatrix4s[ c ].offset = matrix4ValueOffset + float4ValueOffset;
            matrix4ValueOffset += amount * sizeof( Matrix );

            pPass->pMatrix4s[ c ].pName = pName;
            pPass->pMatrix4s[ c ].pRef = pRef;
            pSerializer->GetInputStream( )->Read( pPass->constantBuffer.pData + pPass->pMatrix4s[ c ].offset, sizeof( Matrix ) );
        }


        const int MaxTextures = 16;

        int samplerIndex = 0;
        int rootParamIndex = 0;

        D3D12_DESCRIPTOR_RANGE descRange = {};
        D3D12_ROOT_PARAMETER rootParameters[ 2 ] = {};
        D3D12_STATIC_SAMPLER_DESC samplers[ MaxTextures ] = {};
        
        if ( constantBufferSize > 0 )
        {
            // vertex/pixel shader constants
            rootParameters[ rootParamIndex ].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
            rootParameters[ rootParamIndex ].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
            rootParameters[ rootParamIndex ].Descriptor.ShaderRegister = 0;
            rootParameters[ rootParamIndex ].Descriptor.RegisterSpace = 0;
            ++rootParamIndex;
        }

        if ( pPass->header.numTextures )
        {
            descRange.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
            descRange.NumDescriptors = pPass->header.numTextures;
            descRange.BaseShaderRegister = 0;
            descRange.RegisterSpace = 0;
            descRange.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

            rootParameters[ rootParamIndex ].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
            rootParameters[ rootParamIndex ].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
            rootParameters[ rootParamIndex ].DescriptorTable.NumDescriptorRanges = 1;
            rootParameters[ rootParamIndex ].DescriptorTable.pDescriptorRanges = &descRange;
            ++rootParamIndex;

            // textures for pixel shader
            for ( int c = 0; c < pPass->header.numTextures; c++ )
            {
                D3D12_FILTER filter;

                //TODO: DX12 better converter
                switch ( pPass->pTextures[ c ].header.filter )
                {
                case 1:
                    filter = D3D12_FILTER_MIN_MAG_MIP_POINT;
                    break;
                case 2:
                    filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
                    break;

                case 9:
                    filter = D3D12_FILTER_COMPARISON_MIN_MAG_LINEAR_MIP_POINT;
                    break;

                case 10:
                    filter = D3D12_FILTER_COMPARISON_MIN_MAG_MIP_POINT;
                    break;
                }

                samplers[ samplerIndex ].Filter = filter;

                //TODO: DX12 better converter
                samplers[ samplerIndex ].AddressU = (D3D12_TEXTURE_ADDRESS_MODE) pPass->pTextures[ c ].header.address;
                samplers[ samplerIndex ].AddressV = (D3D12_TEXTURE_ADDRESS_MODE) pPass->pTextures[ c ].header.address;
                samplers[ samplerIndex ].AddressW = (D3D12_TEXTURE_ADDRESS_MODE) pPass->pTextures[ c ].header.address;
                samplers[ samplerIndex ].MipLODBias = 0;
                samplers[ samplerIndex ].MaxAnisotropy = 0;
                samplers[ samplerIndex ].ComparisonFunc = D3D12_COMPARISON_FUNC_LESS;
                samplers[ samplerIndex ].BorderColor = D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK;
                samplers[ samplerIndex ].MinLOD = 0.0f;
                samplers[ samplerIndex ].MaxLOD = D3D12_FLOAT32_MAX;
                samplers[ samplerIndex ].ShaderRegister = c;
                samplers[ samplerIndex ].RegisterSpace = 0;
                samplers[ samplerIndex ].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
                ++samplerIndex;
            }
        }

        // store the descriptor tables in the root signature
        D3D12_ROOT_SIGNATURE_DESC rootSignatureDesc = {};
        rootSignatureDesc.NumParameters = rootParamIndex;
        rootSignatureDesc.pParameters = rootParameters;
        rootSignatureDesc.NumStaticSamplers = samplerIndex;
        rootSignatureDesc.pStaticSamplers = samplers;
        rootSignatureDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

        ID3D12RootSignature *pRootSignature = RenderContexts::RegisterRootSignature( rootSignatureDesc );

        D3D12_RASTERIZER_DESC rasterDesc = { };
        rasterDesc.FillMode = D3D12_FILL_MODE_SOLID;
        rasterDesc.CullMode = (D3D12_CULL_MODE) pPass->header.cullMode; //1 = none, 2 = front, 3 = back
        rasterDesc.FrontCounterClockwise = FALSE;
        rasterDesc.DepthBias = 0;
        rasterDesc.DepthBiasClamp = 0;
        rasterDesc.SlopeScaledDepthBias = 0;
        rasterDesc.DepthClipEnable = TRUE;
        rasterDesc.MultisampleEnable = FALSE;
        rasterDesc.AntialiasedLineEnable = FALSE;
        rasterDesc.ForcedSampleCount = 0;
        rasterDesc.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;

        // Describe and create the graphics pipeline state object (PSO).
        D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = { };
        psoDesc.pRootSignature = pRootSignature;
        psoDesc.RasterizerState = rasterDesc;
        psoDesc.BlendState.AlphaToCoverageEnable = FALSE;
        psoDesc.BlendState.IndependentBlendEnable = FALSE;

        psoDesc.BlendState.RenderTarget[ 0 ].BlendEnable = pPass->header.blendEnable;
        psoDesc.BlendState.RenderTarget[ 0 ].BlendOp = D3D12_BLEND_OP_ADD;
        psoDesc.BlendState.RenderTarget[ 0 ].SrcBlend = (D3D12_BLEND) pPass->header.sourceBlend;
        psoDesc.BlendState.RenderTarget[ 0 ].DestBlend = (D3D12_BLEND) pPass->header.destBlend;
        psoDesc.BlendState.RenderTarget[ 0 ].SrcBlendAlpha = (D3D12_BLEND) pPass->header.sourceBlend;
        psoDesc.BlendState.RenderTarget[ 0 ].DestBlendAlpha = (D3D12_BLEND) pPass->header.destBlend;
        psoDesc.BlendState.RenderTarget[ 0 ].BlendOpAlpha = D3D12_BLEND_OP_ADD;
        psoDesc.BlendState.RenderTarget[ 0 ].LogicOpEnable = FALSE;
        psoDesc.BlendState.RenderTarget[ 0 ].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

        psoDesc.DepthStencilState.DepthEnable = pPass->header.depthTest;
        psoDesc.DepthStencilState.DepthFunc = (D3D12_COMPARISON_FUNC) pPass->header.depthFunc;
        psoDesc.DepthStencilState.DepthWriteMask = (D3D12_DEPTH_WRITE_MASK) pPass->header.depthWrite;
        psoDesc.DepthStencilState.StencilEnable = FALSE;
        psoDesc.SampleMask = UINT_MAX;
        psoDesc.SampleDesc.Count = 1;

        pPass->psoDesc = psoDesc;

        Shader *pShader = GetResource( pPass->shader, Shader );
        pShader->SetPSO( &pPass->psoDesc );
    }

    GraphicsMaterial *pGraphics = new GraphicsMaterial;
    pGraphics->m_NumPasses = header.numPasses;
    pGraphics->m_pPassDatas = pPasses;

    pMaterial->m_pGraphicsMaterial = pGraphics;
    pMaterial->m_pComputeMaterial = NULL;

    return pSerializable;
}

