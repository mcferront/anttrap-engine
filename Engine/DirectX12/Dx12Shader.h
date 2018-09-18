#pragma once

#include "EngineGlobal.h"
#include "Asset.h"
#include "Dx12.h"

#define SHADE_PER_SAMPLE

struct LightDesc;

class Shader : public Asset
{
    friend class ShaderSerializer;

private:
    D3D12_SHADER_BYTECODE m_VS;
    ID3DBlob *m_pVS;

    D3D12_SHADER_BYTECODE m_PS;
    ID3DBlob *m_pPS;

    D3D12_SHADER_BYTECODE m_CS;
    ID3DBlob *m_pCS;

public:
    DeclareResourceType( Shader );

public:
    void SetPSO( 
        D3D12_GRAPHICS_PIPELINE_STATE_DESC *pPSODesc 
        );

    void SetPSO( 
       D3D12_COMPUTE_PIPELINE_STATE_DESC *pPSODesc 
    );

    void Destroy( void );
};

class ShaderSerializer : public ISerializer
{
public:
    virtual bool Serialize(
        Serializer *pSerializer,
        const ISerializable *pSerializable
        )
    {
        return false;
    }

    virtual ISerializable *Deserialize(
        Serializer *pSerializer,
        ISerializable *pSerializable
        );

    virtual ISerializable *Instantiate( ) const { return new Shader; }

    virtual const SerializableType &GetSerializableType( void ) const { return Shader::StaticSerializableType( ); }

    virtual uint32 GetVersion( void ) const { return 1; }
};
