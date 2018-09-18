#pragma once

#include "EngineGlobal.h"
#include "Component.h"
#include "Sprite.h"

class SpriteComponent : public Component
{
public:
   DeclareComponentType(SpriteComponent);

private:
   Sprite    m_Renderable;

public:
   void Create(
      Id id,
      ResourceHandle material,
      const IdList &renderGroups,
      const Vector2 &size
   );

   virtual void AddToScene( void );
   virtual void RemoveFromScene( void );

   virtual void Destroy( void );

   void SetSize( 
      const Vector2 &size
   ) 
   {
      m_Renderable.SetSize(size); 
   }
   
   const Vector2 *GetSize( void ) const { return m_Renderable.GetSize(); }
   
   void SetAlpha( float alpha ) { m_Renderable.SetAlpha(alpha); }   
   float GetAlpha( void ) const { return m_Renderable.GetAlpha(); }
};

class SpriteComponentSerializer : public ISerializer
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

    virtual ISerializable *Instantiate() const { return new SpriteComponent; }

    virtual const SerializableType &GetSerializableType( void ) const { return SpriteComponent::StaticSerializableType( ); }

    virtual uint32 GetVersion( void ) const { return 1; }
};
