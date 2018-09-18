#include "EnginePch.h"

#include "Dx12Shader.h"
#include "TextureAsset.h"
#include "Log.h"
#include "Dx12.h"
#include "Geometry.h"

DefineResourceType(Shader, Asset, new ShaderSerializer);

void Shader::Destroy( )
{
    if ( NULL != m_pVS )
        m_pVS->Release( );

    m_pVS = NULL;

    Asset::Destroy( );
}

void Shader::SetPSO( 
    D3D12_GRAPHICS_PIPELINE_STATE_DESC *pPSODesc 
    )
{
    pPSODesc->VS = m_VS;
    pPSODesc->PS = m_PS;
}

void Shader::SetPSO( 
   D3D12_COMPUTE_PIPELINE_STATE_DESC *pPSODesc 
)
{
   pPSODesc->CS = m_CS;
}

ID3DBlob *CompileShader( Serializer *pSerializer, const char *pVersion )
{
    ID3DBlob *pBlob = NULL;
    HRESULT hr;

    const char *pEntry = StringPool::Deserialize( pSerializer->GetInputStream() );

    if ( NULL != *pEntry )
    {
        const char *pSourceName = StringPool::Deserialize( pSerializer->GetInputStream( ) );

        uint32 size;
        pSerializer->GetInputStream( )->Read( &size, sizeof( size ) );

        byte *pByteBlob = (byte *) malloc( size );
        pSerializer->GetInputStream()->Read( pByteBlob, size );

        hr = D3DCreateBlob( size, &pBlob );

        if ( SUCCEEDED(hr) )
            memcpy( pBlob->GetBufferPointer(), pByteBlob, size );
        else
            Debug::Assert( Condition(SUCCEEDED(hr)), "Failed to create shader blob of size: %d for shader: %s", size, pSourceName );

        StringRel( pSourceName );
    }

    StringRel( pEntry );

    return pBlob;
}

ISerializable *ShaderSerializer::Deserialize(
    Serializer *pSerializer,
    ISerializable *pSerializable
    )
{
    const uint32 Version = 3;

    struct Header
    {
        unsigned int version;
    };

    Header header;

    if ( NULL == pSerializable ) pSerializable = new Shader;

    Shader *pShader = (Shader *) pSerializable;

    pSerializer->GetInputStream( )->Read( &header, sizeof(header), NULL );
    Debug::Assert( Condition(header.version == Version), "Incorrect shader version: %d, expecting: %d", header.version, Version );

    ID3DBlob *pVSBlob = CompileShader( pSerializer, "vs_5_0" );

    if ( NULL != pVSBlob )
    {
        pShader->m_VS.pShaderBytecode = pVSBlob->GetBufferPointer( );
        pShader->m_VS.BytecodeLength = pVSBlob->GetBufferSize( );
        pShader->m_pVS = pVSBlob;
    }
    else
    {
        pShader->m_VS.pShaderBytecode = NULL;
        pShader->m_VS.BytecodeLength = 0;
        pShader->m_pVS = NULL;
    }

    ID3DBlob *pPSBlob = CompileShader( pSerializer, "ps_5_0" );

    if ( NULL != pPSBlob )
    {
        pShader->m_PS.pShaderBytecode = pPSBlob->GetBufferPointer( );
        pShader->m_PS.BytecodeLength = pPSBlob->GetBufferSize( );
        pShader->m_pPS = pPSBlob;
    }
    else
    {
        pShader->m_PS.pShaderBytecode = NULL;
        pShader->m_PS.BytecodeLength = 0;
        pShader->m_pPS = NULL;
    }

    ID3DBlob *pCSBlob = CompileShader( pSerializer, "cs_5_0" );

    if ( NULL != pCSBlob )
    {
        pShader->m_CS.pShaderBytecode = pCSBlob->GetBufferPointer( );
        pShader->m_CS.BytecodeLength = pCSBlob->GetBufferSize( );
        pShader->m_pCS = pCSBlob;
    }
    else
    {
        pShader->m_CS.pShaderBytecode = NULL;
        pShader->m_CS.BytecodeLength = 0;
        pShader->m_pCS = NULL;
    }

    return pSerializable;
}

