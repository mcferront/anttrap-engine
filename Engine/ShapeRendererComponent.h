#pragma once

#include "EngineGlobal.h"
#include "Component.h"
#include "RenderableShape.h"
#include "ResourceWorld.h"

class ShapeRendererComponent : public Component
{
friend class ShapeRendererComponentSerializer;

public:
   DeclareComponentType(ShapeRendererComponent);

private:
    RenderableShape m_Shape;

public:
    void Create(
        Id id,
        ResourceHandle material,
        const IdList &renderGroups,
        const Triangle *pTriangles,
        int numTriangles,
        const Line *pLines,
        int numLines
    );

    virtual void Bind( void );

    virtual void Destroy( void );

    virtual void AddToScene( void );
    virtual void RemoveFromScene( void );
};

class ShapeRendererComponentSerializer : public ISerializer
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

   virtual ISerializable *Instantiate() const { return new ShapeRendererComponent; }

   virtual const SerializableType &GetSerializableType( void ) const { return ShapeRendererComponent::StaticSerializableType( ); }

   virtual uint32 GetVersion( void ) const { return 1; }
};
