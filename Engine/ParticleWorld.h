#pragma once

#include "EngineGlobal.h"
#include "SystemId.h"
#include "HashTable.h"
#include "List.h"
#include "ResourceWorld.h"
#include "ParticleEmitter.h"

class ParticleWorld
{
public:
    static ParticleWorld &Instance( void );

private:
    HashTable<Id, ParticleEmitterDesc*> m_EmitterDescs;
    List<ParticleEmitter *> m_Emitters;

public:
    void Create( void );

    void CreateEmitter(
        Id name,
        ResourceHandle material,
        float delayTime,
        float minEmitVelocity,
        float maxEmitVelocity,
        float minActiveTime,
        float maxActiveTime,
        const Vector &emitArc,
        const Vector &startTintColor,
        const Vector &endTintColor,
        float startTintColorBlend,
        float endTintColorBlend,
        float particleStartWidth,
        float particleStartHeight,
        float particleEndWidth,
        float particleEndHeight,
        int tileWidth,
        int tileHeight,
        int minEmitCount,
        int maxEmitCount,
        float minCycleTime,
        float maxCycleTime,
        float minLifeTime,
        float maxLifeTime,
        float fadeInTime,
        float fadeOutTime,
        float gravity,
        float minRotationRate,
        float maxRotationRate,
        bool relativeToEmitter
    );

    void Destroy( void );

    nuint InstanceEmitter(
        const Id &name,
        const Transform &worldTransform
    );

    void EndEmitter(
       nuint emitter
    );
    void DestroyEmitter(
       nuint emitter
    );
    void DestroyAllEmitters( void );
    void UpdateEmitter(
       nuint emitter,
        const Transform &worldTransform
    );

    void Tick(
        float deltaSeconds
    );
};
