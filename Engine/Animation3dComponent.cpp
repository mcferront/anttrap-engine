#include "EnginePch.h"

#include "Animation3dComponent.h"
#include "List.h"
#include "ResourceWorld.h"
#include "Node.h"

DefineComponentType(AnimationComponent, new AnimationComponentSerializer);

void AnimationComponent::Create(
    ResourceHandle animation,
    bool playAutomatically
    )
{
    m_Animation = animation;
    m_PlayAutomatically = playAutomatically;

    m_pComposite = new Animation3dComposite;
    m_pComposite->Create( );
}

void AnimationComponent::Destroy( void )
{
    m_pComposite->Destroy( );
    delete m_pComposite;

    m_pComposite = NULL;
    m_Animation  = NullHandle;

    Component::Destroy( );
}

void AnimationComponent::Bind( void )
{
    Component::Bind( );

    m_pComposite->Bind( GetParent()->GetChannel() );

    if ( true == m_PlayAutomatically )
        Play( m_Animation, -1 );
}

void AnimationComponent::Play( 
    ResourceHandle animation,
    int playHowManyTimes //= 1
    )
{
    m_pComposite->Play( animation, playHowManyTimes );
}

void AnimationComponent::QueueNext(
    ResourceHandle animation,
    int playHowManyTimes //= 1
    )
{
    m_pComposite->QueueNext( animation, playHowManyTimes );
}

void AnimationComponent::Stop( void )
{
    m_pComposite->Stop( );
}

void AnimationComponent::GetDeltaTransform(
    Transform *pTransform
    )
{
    m_pComposite->GetDeltaTransform( pTransform );
}

void AnimationComponent::PostTick( void )
{
    const FullSkeleton *pSkeleton = m_pComposite->GetSkeleton( );

    for ( int i = 0; i < pSkeleton->GetSkeleton( )->GetNumBones( ); i++ )
    {
        Node *pNode = GetParent( )->FindChildByName( pSkeleton->GetBoneName(i) );

        if ( NULL != pNode )
            pNode->SetLocalTransform( *pSkeleton->GetSkeleton( )->GetBone(i) );
        else
            Debug::Print( Debug::TypeInfo, "Bone %s not found\n", pSkeleton->GetBoneName(i) );
    }
}

void AnimationComponent::AddToScene( void )
{
    if ( false == GetParent()->IsInScene() ) return;

    Component::AddToScene( );
    m_pComposite->AddToScene( );
}

void AnimationComponent::RemoveFromScene( void )
{
    Component::RemoveFromScene( );

    m_pComposite->RemoveFromScene( );
}

const FullSkeleton *AnimationComponent::GetSkeleton( void )
{
    return m_pComposite->GetSkeleton( );
}

ISerializable *AnimationComponentSerializer::Deserialize(
    Serializer *pSerializer,
    ISerializable *pSerializable
    )
{
    if ( NULL == pSerializable ) pSerializable = new AnimationComponent; 

    AnimationComponent *pAnimationComponent = (AnimationComponent *) pSerializable;

    Id id = Id::Deserialize( pSerializer->GetInputStream() );
    Id animation = Id::Deserialize( pSerializer->GetInputStream() );

    byte play;
    pSerializer->GetInputStream()->Read( &play, sizeof(play) );

    pAnimationComponent->SetId( id );
    pAnimationComponent->Create( ResourceHandle(animation), play != 0 );

    return pSerializable;
}
