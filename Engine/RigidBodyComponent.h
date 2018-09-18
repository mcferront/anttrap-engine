#pragma once

#include "EngineGlobal.h"
#include "Component.h"
#include "RigidBody.h"

class RigidBodyComponent : public Component
{
public:
    DeclareComponentType( RigidBodyComponent );

private:
    RigidBody m_RigidBody;

public:
    RigidBodyComponent( void ) {}

    void Create(
        Id id,
        const Vector &tensor,
        float mass,
        bool useGravity
        );


    void Bind( void );

    void Render(
        const Vector &color
        );

    void AddToScene( void );
    void RemoveFromScene( void );

    void Destroy( void );

    void UseGravity(
        bool useGravity
        );

    void ApplyWorldForce(
        const Vector &position,
        const Vector &force
        );

    void ResetPhysics( void );

    void EndSimulation( void );
};

class RigidBodyComponentSerializer : public ISerializer
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

    virtual ISerializable *Instantiate( ) const { return new RigidBodyComponent; }

    virtual const SerializableType &GetSerializableType( void ) const { return RigidBodyComponent::StaticSerializableType( ); }

    virtual uint32 GetVersion( void ) const { return 1; }
};
