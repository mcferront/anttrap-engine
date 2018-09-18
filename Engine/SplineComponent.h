#pragma once

#include "EngineGlobal.h"
#include "Component.h"
#include "ResourceWorld.h"

class SplineComponent : public Component
{
public:
    DeclareComponentType(SplineComponent);

private:
    ResourceHandle m_Spline;

public:
    void Create(
        Id id,
        ResourceHandle spline
        );

    void Destroy( void );

    virtual void EditorRender( void );

    Vector GetClosestPosition(
        const Vector &position
        );

    Quaternion GetClosestRotation(
        const Vector &position
        );

    Transform GetClosestTransform(
        const Vector &position
        );

    float GetClosestParam(
        const Vector &position
        );

    Transform GetTransform(
        float p
        );

    float GetLength( void ) const;
};

class SplineComponentSerializer : public ISerializer
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

    virtual ISerializable *Instantiate() const { return new SplineComponent; }

    virtual const SerializableType &GetSerializableType( void ) const { return SplineComponent::StaticSerializableType( ); }

    virtual uint32 GetVersion( void ) const { return 1; }
};
