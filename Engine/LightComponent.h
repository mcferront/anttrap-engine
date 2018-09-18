#pragma once

#include "EngineGlobal.h"
#include "Component.h"
#include "Light.h"

class LightComponent : public Component
{
public:
   DeclareComponentType( LightComponent );

protected:
   Light m_Light;

public:
   virtual void Create(
      const IdList &renderGroups,
      const LightDesc &desc
      )
   {
      m_Light.Create( this, renderGroups, desc );
   }

   void Destroy( void );

   virtual void AddToScene( void );
   virtual void RemoveFromScene( void );
};

class DirectionalLightComponent : public LightComponent
{
public:
   DeclareComponentType( DirectionalLightComponent );

   void Create(
      const IdList &renderGroups,
      Vector color,
      float nits
      );
};

class AmbientLightComponent : public LightComponent
{
public:
   DeclareComponentType( AmbientLightComponent);

   void Create(
      const IdList &renderGroups,
      Vector color,
      float nits
      );
};

class SpotLightComponent : public LightComponent
{
public:
   DeclareComponentType( SpotLightComponent );

   void Create(
      const IdList &renderGroups,
      Vector color,
      float nits,
      float inner,
      float outer,
      float range
      );
};

class PointLightComponent : public LightComponent
{
public:
   DeclareComponentType( PointLightComponent );

   void Create(
      const IdList &renderGroups,
      Vector color,
      float nits,
      float range
      );

   void SetColor(
      const Vector &color
   );
};

class DirectionalLightComponentSerializer : public ISerializer
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

   virtual ISerializable *Instantiate( ) const { return new DirectionalLightComponent; }

   virtual const SerializableType &GetSerializableType( void ) const { return DirectionalLightComponent::StaticSerializableType( ); }

   virtual uint32 GetVersion( void ) const { return 1; }
};

class AmbientLightComponentSerializer : public ISerializer
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

   virtual ISerializable *Instantiate( ) const { return new AmbientLightComponent; }

   virtual const SerializableType &GetSerializableType( void ) const { return AmbientLightComponent::StaticSerializableType( ); }

   virtual uint32 GetVersion( void ) const { return 1; }
};

class SpotLightComponentSerializer : public ISerializer
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

   virtual ISerializable *Instantiate( ) const { return new SpotLightComponent; }

   virtual const SerializableType &GetSerializableType( void ) const { return SpotLightComponent::StaticSerializableType( ); }

   virtual uint32 GetVersion( void ) const { return 1; }
};

class PointLightComponentSerializer : public ISerializer
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

   virtual ISerializable *Instantiate( ) const { return new PointLightComponent; }

   virtual const SerializableType &GetSerializableType( void ) const { return PointLightComponent::StaticSerializableType( ); }

   virtual uint32 GetVersion( void ) const { return 1; }
};
