#pragma once

#include "EngineGlobal.h"
#include "Asset.h"
#include "GlShader.h"
#include "Renderer.h"

class IRasterizer;

class Material : public Asset
{
   friend class MaterialSerializer;

public:
   struct ParameterDesc
   {
      Cwid parameterId;
      Vector parameter;
   };

private:
   enum CullMode
   {
      CullNone,
      CullFront,
      CullBack,
   };

   enum WrapMode
   {
      Clamp,
      Wrap,
   };

   struct TextureDesc
   {
      ResourceHandle texture;
      WrapMode       wrapMode;
   };

public:
   DeclareResourceType(Material);

private:
   TextureDesc    *m_pTextures;
   ResourceHandle  m_Shader;
   CullMode        m_CullMode;
   Vector          m_Color;
   Vector          m_SelfIllumColor;
   int             m_NumTextures;
   bool            m_DepthTest;
   bool            m_DepthWrite;
   bool            m_AlphaBlend;
   bool            m_DepthMask;
   bool            m_Filter;
   
public:
   virtual void Create(
      const Cwid &systemId
   )
   {
      Asset::Create( systemId );
   }

   void Create(
      const Cwid &systemId,
      ResourceHandle texture,
      ResourceHandle shader,
      Vector color,
      bool depthTest,
      bool cull,
      bool alphaBlend,
      bool depthMask,
      bool filter
   );

   void Destroy( void );

   void Submit(
      const Transform &worldTransform,
      bool selected,
      bool skinned,
      IRasterizer *pRasterizer,
      int parameters = 0,
      const ParameterDesc *pDescs = NULL,
      const LightDescList *pLights = NULL
   ) const;

   void SetMatrices(
      const char *pName,
      const Matrix *pMatrices,
      int numMatrices
   );
   
   bool HasAlpha( void ) const { return true == m_AlphaBlend; }

   ResourceHandle GetTexture( void ) const { return m_NumTextures > 0 ? m_pTextures[0].texture : NullHandle; }
};

class MaterialSerializer : public ISerializer
{
public:
   virtual bool Serialize(
      Serializer *pSerializer,
      const ISerializable *pSerializable
   ) 
   { return false; }

   virtual ISerializable *Deserialize(
      Serializer *pSerializer,
      ISerializable *pSerializable
   );

   virtual const SerializableType &GetSerializableType( void ) const { return Material::StaticSerializableType( ); }

   virtual uint32 GetVersion( void ) const { return 1; }
};
