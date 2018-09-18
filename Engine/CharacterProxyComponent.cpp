#include "EnginePch.h"

#include "CharacterProxyComponent.h"
#include "CollisionComponent.h"

DefineComponentType(CharacterProxyComponent, NULL);

void CharacterProxyComponent::Create(
    Id id
    )
{
    SetId( id );
    m_CharacterProxy.Create( this );
}

void CharacterProxyComponent::Bind( void )
{
    CollisionComponent *pCollision = GetParent( )->GetComponent<CollisionComponent>();
    m_CharacterProxy.Bind( pCollision->GetCollisionObject() );
}

void CharacterProxyComponent::Destroy( void )
{
    m_CharacterProxy.Destroy( );
}

void CharacterProxyComponent::Render( 
    const Vector &color
    )
{
    m_CharacterProxy.GetCollisionObject( )->Render( color );
}

void CharacterProxyComponent::AddToScene( void )
{
    if ( false == GetParent()->IsInScene() ) return;

    m_CharacterProxy.AddToScene( );
}

void CharacterProxyComponent::RemoveFromScene( void )
{
    m_CharacterProxy.RemoveFromScene( );
}

void CharacterProxyComponent::ApplyMovement(
    const Vector &delta
    )
{
    m_CharacterProxy.ApplyMovement( delta );
}

void CharacterProxyComponent::UseGravity( 
    bool useGravity
    )
{
    m_CharacterProxy.UseGravity( useGravity );
}

