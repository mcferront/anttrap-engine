#include "EnginePch.h"

#include "ParticleWorld.h"
#include "ResourceWorld.h"
#include "MaterialObject.h"

ParticleWorld &ParticleWorld::Instance( void )
{
   static ParticleWorld s_instance;
   return s_instance;
}

void ParticleWorld::Create( void )
{
    m_EmitterDescs.Create( 16, 16, IdHash, IdCompare );
    m_Emitters.Create( );
}

void ParticleWorld::CreateEmitter(
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
)
{
    ParticleEmitterDesc *pDesc;
    
    if ( false == m_EmitterDescs.Get(name, &pDesc) )
    {
        pDesc = new ParticleEmitterDesc;
        m_EmitterDescs.Add( name, pDesc );
    }

    pDesc->name = name;
    pDesc->pMaterial = new GraphicsMaterialObject( material );
    pDesc->delayTime = delayTime;
    pDesc->minEmitVelocity = minEmitVelocity;
    pDesc->maxEmitVelocity = maxEmitVelocity;
    pDesc->minActiveTime = minActiveTime;
    pDesc->maxActiveTime = maxActiveTime;
    pDesc->emitArc = emitArc;
    pDesc->particleStartWidth = particleStartWidth;
    pDesc->particleStartHeight = particleStartHeight;
    pDesc->particleEndWidth = particleEndWidth;
    pDesc->particleEndHeight = particleEndHeight;
    pDesc->tileWidth = tileWidth;
    pDesc->tileHeight = tileHeight;
    pDesc->minEmitCount = minEmitCount;
    pDesc->maxEmitCount = maxEmitCount;
    pDesc->minCycleTime = minCycleTime;
    pDesc->maxCycleTime = maxCycleTime;
    pDesc->minLifeTime = minLifeTime;
    pDesc->maxLifeTime = maxLifeTime;
    pDesc->fadeInTime = fadeInTime;
    pDesc->fadeOutTime = fadeOutTime;
    pDesc->gravity = gravity;
    pDesc->minRotationRate = minRotationRate;
    pDesc->maxRotationRate = maxRotationRate;
    pDesc->relativeToEmitter = relativeToEmitter;
    pDesc->startTintColorBlend = startTintColorBlend;
    pDesc->endTintColorBlend = endTintColorBlend;

    pDesc->startTintColor = 0xff000000 | 
                           (Math::FloatToInt(startTintColor.x) << 16) |
                           (Math::FloatToInt(startTintColor.y) <<  8) |
                           (Math::FloatToInt(startTintColor.z) <<  0);
    pDesc->endTintColor = 0xff000000 | 
                           (Math::FloatToInt(endTintColor.x) << 16) |
                           (Math::FloatToInt(endTintColor.y) <<  8) |
                           (Math::FloatToInt(endTintColor.z) <<  0);

    for ( uint32 i = 0; i < m_Emitters.GetSize(); i++ )
    {
        ParticleEmitter *pEmitter = m_Emitters.GetAt( i );

        if (pEmitter->GetName() == pDesc->name) 
        {
            Transform t;
            pEmitter->GetWorldTransform( &t );

            pEmitter->Destroy( );
            pEmitter->Create( t, *pDesc );
        }
    }
}
void ParticleWorld::EndEmitter(
    nuint emitter
)
{
   if (0 == emitter) return;
   for ( uint32 i = 0; i < m_Emitters.GetSize(); i++ )
   {
      ParticleEmitter *pEmitter = m_Emitters.GetAt( i );
      if ( emitter == (nuint) pEmitter )
      {
         pEmitter->End( );
         break;
      }
   }
}
void ParticleWorld::DestroyEmitter(
    nuint emitter
)
{
   for ( uint32 i = 0; i < m_Emitters.GetSize(); i++ )
   {
      ParticleEmitter *pEmitter = m_Emitters.GetAt( i );
      if ( emitter == (nuint) pEmitter )
      {
         m_Emitters.RemoveAt( i );
         pEmitter->Destroy( );
         delete pEmitter;
         break;
      }
   }
}
void ParticleWorld::DestroyAllEmitters( void )
{
    {
        List<ParticleEmitter *>::Enumerator e = m_Emitters.GetEnumerator();
        while ( e.EnumNext() )
        {
            e.Data( )->Destroy( );
            delete e.Data( );
        }
    }
    m_Emitters.Clear();
}
void ParticleWorld::UpdateEmitter(
   nuint emitter,
   const Transform &worldTransform
)
{
   if (0 == emitter) return;
   for ( uint32 i = 0; i < m_Emitters.GetSize(); i++ )
   {
      ParticleEmitter *pEmitter = m_Emitters.GetAt( i );
      if ( emitter == (nuint) pEmitter )
      {
         pEmitter->SetWorldTransform( worldTransform );
         break;
        }
    }
}

void ParticleWorld::Destroy( void )
{
    {
        List<ParticleEmitter *>::Enumerator e = m_Emitters.GetEnumerator();

        while ( e.EnumNext() )
        {
            e.Data( )->Destroy( );
            delete e.Data( );
        }
    }

    {
        Enumerator<Id, ParticleEmitterDesc*> e = m_EmitterDescs.GetEnumerator();

        while ( e.EnumNext() )
        {
            delete e.Data( );
        }
    }

    m_EmitterDescs.Destroy();
    m_Emitters.Destroy( );
}

nuint ParticleWorld::InstanceEmitter(
    const Id &name,
    const Transform &worldTransform
)
{
    ParticleEmitterDesc *pDesc;
    ParticleEmitter *pEmitter = NULL;

    if ( true == m_EmitterDescs.Get(name, &pDesc) )
    {
        pEmitter = new ParticleEmitter;
        pEmitter->Create( worldTransform, *pDesc );
        m_Emitters.Add( pEmitter );
    }
    else
    {
        Debug::Print( Debug::TypeWarning, "Emitter %s was not found\n", name.ToString() );
    }

    return (nuint) pEmitter;
}

void ParticleWorld::Tick(
    float deltaSeconds
)
{
    for ( uint32 i = 0; i < m_Emitters.GetSize(); i++ )
    {
        ParticleEmitter *pEmitter = m_Emitters.GetAt( i );

        bool result = pEmitter->Update( deltaSeconds );
        if ( false == result ) 
        {
            m_Emitters.RemoveAt( i-- );
            pEmitter->Destroy( );
            delete pEmitter;
        }
    }
}
