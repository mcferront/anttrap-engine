#pragma once

#include "EngineGlobal.h"
#include "Asset.h"
#include "Dx9.h"

struct LightDesc;

class Shader : public Asset
{
public:
    DeclareResourceType( Shader );

    IDirect3DVertexShader9      *m_pVertexShader;
    IDirect3DPixelShader9       *m_pPixelShader;
    //   IDirect3DVertexShader9      *m_pSkinShader;
    //   IDirect3DPixelShader9       *m_pPixelSelShader;
    //
    HashTable<const char *, int>  m_VertexRegisters;
    HashTable<const char *, int>  m_PixelRegisters;
    //   HashTable<const char *, int>  m_PixelSelRegisters;
    //   HashTable<const char *, int> *m_pActivePixelRegisters;
    //   int m_Requires;
    //
    //   const char *m_pType;
    //
    public:
       void Create( void );
    
       void Destroy( void );

       void MakeActive( void );
    
       void SetTransforms(
          const char *pName,
          const Transform *pTransforms,
          int numTransforms
       );
    
       void SetMatrices(
          const char *pName,
          const Matrix *pMatrices,
          int numMatrices
       );
    
       void SetTexture(
          const char *pName,
          ResourceHandle texture,
          D3DTEXTUREADDRESS wrap,
          D3DTEXTUREFILTERTYPE filter
       );
    
       void SetVectors(
          const char *pName,
          const Vector *pVectors,
          int numVectors
       );
    //
    //   void SetLight(
    //      int lightIndex,
    //      const LightDesc &desc
    //   );
    //
    //   int GetStreamRequirements( void ) { return m_Requires; }
    //
    private:
       int GetVertexRegister(
          const char *pName
       );
    
       int GetPixelRegister(
          const char *pName
       );
    //
    //   void CreateDx9Shaders ( void );
    //   void DestroyDx9Shaders( void );
    //
    //   void OnDeviceLost(
    //      const Channel *pSender,
    //      const char *pName,
    //      const ArgList &list
    //   );
    //
    //   void OnDeviceRestored(
    //      const Channel *pSender,
    //      const char *pName,
    //      const ArgList &list
    //   );
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
