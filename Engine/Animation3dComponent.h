#pragma once

#include "EngineGlobal.h"
#include "Animation3dComposite.h"
#include "Component.h"
#include "SystemId.h"
#include "HashTable.h"
#include "ResourceWorld.h"

class Skeleton;
class Animation3d;

class AnimationComponent : public Component
{
public:
    DeclareComponentType( AnimationComponent );

private:
    Animation3dComposite   *m_pComposite;
    ResourceHandle          m_Animation;
    bool                    m_PlayAutomatically;

public:
    AnimationComponent( void )
    {}

    void Create(
        ResourceHandle animation,
        bool playAutomatically
        );

    void Destroy( void );

    void Bind( void );

    //-1 = infinite
    void Play(
        ResourceHandle animation,
        int playHowManyTimes = 1
        );

    //-1 = infinite
    void QueueNext(
        ResourceHandle animation,
        int playHowManyTimes = 1
        );

    void Stop( void );

    void GetDeltaTransform(
        Transform *pTransform
        );

    void PostTick( void );

    void AddToScene( void );
    void RemoveFromScene( void );

    const FullSkeleton *GetSkeleton( void );
};

class AnimationComponentSerializer : public ISerializer
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

    virtual ISerializable *Instantiate( ) const { return new AnimationComponent; }

    virtual const SerializableType &GetSerializableType( void ) const { return AnimationComponent::StaticSerializableType( ); }

    virtual uint32 GetVersion( void ) const { return 1; }
};
