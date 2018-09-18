#pragma once

#include "EngineGlobal.h"
#include "MemoryPool.h"
#include "List.h"
#include "ResourceWorld.h"
#include "RenderObject.h"

class GraphicsMaterialObject;

struct ParticleEmitterDesc
{
    Id name;
    VertexContext vertexContext;
    GraphicsMaterialObject *pMaterial;
    float delayTime;
    float minEmitVelocity;
    float maxEmitVelocity;
    float minActiveTime;
    float maxActiveTime;
    Vector emitArc;
    uint32 startTintColor;
    uint32 endTintColor;
    float startTintColorBlend;
    float endTintColorBlend;
    float particleStartWidth;
    float particleStartHeight;
    float particleEndWidth;
    float particleEndHeight;
    int tileWidth;
    int tileHeight;
    int minEmitCount;
    int maxEmitCount;
    float minCycleTime;
    float maxCycleTime;
    float minLifeTime;
    float maxLifeTime;
    float fadeInTime;
    float fadeOutTime;
    float gravity;
    float minRotationRate;
    float maxRotationRate;
    bool relativeToEmitter;
};

struct ParticleTileDesc
{
    int tileWidth;
    int tileHeight;
    float particleStartWidth;
    float particleStartHeight;
    float particleEndWidth;
    float particleEndHeight;
    float startColorBlend;
    float endColorBlend;
};

class Particle
{
public:
    Vector m_Velocity;
    Vector m_Position;
    float  m_Alpha;
    float  m_Time;
    float  m_Lifetime;
    float  m_RotationRate;
    float  m_FadeInTime;
    float  m_FadeOutTime;

public:
    void Create(
        const Vector &startPosition,
        const Vector &velocity,
        float lifeTime,
        float rotationRate,
        float fadeInTime,
        float fadeOutTime
    );

    bool Update(
        float deltaSeconds,
        float gravity
    );

    const Vector *GetPosition    ( void ) const { return &m_Position; }
    const float   GetTime        ( void ) const { return m_Time; }
    const float   GetLifetime    ( void ) const { return m_Lifetime; }
    const float   GetAlpha        ( void ) const { return m_Alpha; }
};

class ParticleEmitter : public RenderObject
{
private:
    ParticleEmitterDesc  m_Desc;
    ParticleTileDesc     m_TileDesc;
    MemoryPool<Particle> m_Pool;
    List<Particle *>     m_Particles;
    Transform            m_WorldTransform;

    Component *m_pComponent;

    float m_Time;
    float m_ActiveTime;
    float m_CycleTime;
    float m_NextCycleTime;
    bool  m_GoForever;

public:
    void Create(
        const Transform &worldTransform,
        const ParticleEmitterDesc &
    );

    void Destroy( void );

    bool Update(
        float deltaSeconds
    );

    void End( void );
    void GetRenderData(
        RendererDesc *pDesc
    ) const;

    void SetWorldTransform(
       const Transform &transform
    )
    {
       m_WorldTransform = transform;
    }
    Id GetName( void ) const { return m_Desc.name; }

    virtual bool IsVisible( 
        const Frustum &frustum
    ) const { return true; }

    virtual void GetRenderGroups( 
        IdList *pGroups
        ) const
    {
        pGroups->Add( Id("Default") );
    }

    virtual int GetRenderType( void ) const { return RenderObject::Type::Particle; }

    virtual void GetWorldTransform(
       Transform *pTransform
       ) const
    {
       *pTransform = m_WorldTransform;
    }

private:
    void EmitParticles( 
        List<Particle*> *pAddTo 
    );

public:
    static void Render( 
       const RenderDesc &desc,
       RenderStats *pStats
    );
};
