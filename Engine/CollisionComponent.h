#pragma once

#include "EngineGlobal.h"
#include "CollisionObject.h"
#include "Component.h"
#include "SystemId.h"

class CollisionTrigger;

class CollisionComponent : public Component
{
public:
    DeclareComponentType( CollisionComponent );

private:
    CollisionObject  *m_pCollision;
    CollisionTrigger *m_pTrigger;

public:
    CollisionComponent( void ) {}

    void Create(
        Id id,
        CollisionObject *pCollision
        );

    void AddTrigger( void );

    void Destroy( void );

    void Render(
        const Vector &color
        );

    virtual void AddToScene( void );
    virtual void Bind( void );

    virtual void RemoveFromScene( void );

    virtual void Tick(
        float deltaSeconds
        );

    virtual void PostTick( void );
    virtual void Final( void );

    CollisionObject *GetCollisionObject( void ) { return m_pCollision; }

public:
    static void OnTriggerEntered(
        void *pData,
        Id o
        );

    static void OnTriggerExited(
        void *pData,
        Id o
        );
};

class CollisionComponentSerializer : public ISerializer
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

    virtual ISerializable *Instantiate( ) const { return new CollisionComponent; }

    virtual const SerializableType &GetSerializableType( void ) const { return CollisionComponent::StaticSerializableType( ); }

    virtual uint32 GetVersion( void ) const { return 1; }
};
