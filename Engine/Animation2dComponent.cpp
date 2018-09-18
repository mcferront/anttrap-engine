#include "EnginePch.h"

#include "Animation2dComponent.h"
#include "AnimatedTexture.h"
#include "List.h"

DefineComponentType(Animation2dComponent, NULL);

void Animation2dComponent::Create(
    Id outputTextureId,
    ResourceHandle identityFramemap,
    Channel *pChannel
    )
{
    m_Animation.Create( outputTextureId, identityFramemap, pChannel );
}

void Animation2dComponent::Destroy( void )
{
    m_Animation.Destroy( );
}

void Animation2dComponent::Play( 
    ResourceHandle framemapId,
    int playHowManyTimes // = -1
    )
{
    m_Animation.Play( framemapId, playHowManyTimes );
}

void Animation2dComponent::QueueNext(
    ResourceHandle framemapId,
    int playHowManyTimes // = -1
    )
{
    m_Animation.QueueNext( framemapId, playHowManyTimes  );
}

void Animation2dComponent::Stop( void )
{
    m_Animation.Stop( );
}

void Animation2dComponent::GetDeltaTransform(
    Transform *pTransform
    )
{
    m_Animation.GetDeltaTransform( pTransform );
}

void Animation2dComponent::AddToScene( void )
{
    if ( false == GetParent()->IsInScene() ) return;

    m_Animation.AddToScene( );
}

void Animation2dComponent::RemoveFromScene( void )
{
    m_Animation.RemoveFromScene( );
}
