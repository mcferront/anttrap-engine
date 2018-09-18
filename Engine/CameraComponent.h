#pragma once

#include "EngineGlobal.h"
#include "Component.h"
#include "Camera.h"

class CameraComponent : public Component
{
    friend class CameraComponentSerializer;

public:
    DeclareComponentType(CameraComponent);

private:
    Camera m_Camera;
    bool   m_Enabled;

public:
    void Create(
        Id id
        );

    void Destroy( void );

    virtual void AddToScene( void );
    virtual void RemoveFromScene( void );

    Camera *GetCamera( void );

    bool IsEnabled   ( void ) const { return m_Enabled; }
};

class CameraComponentSerializer : public ISerializer
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

    virtual ISerializable *Instantiate() const { return new CameraComponent; }

    virtual const SerializableType &GetSerializableType( void ) const { return CameraComponent::StaticSerializableType( ); }

    virtual uint32 GetVersion( void ) const { return 1; }
};
