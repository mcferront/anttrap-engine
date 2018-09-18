#include "EnginePch.h"

#include "RigidBodyComponent.h"
#include "CollisionComponent.h"
#include "DebugGraphics.h"

DefineComponentType(RigidBodyComponent, new RigidBodyComponentSerializer);

void RigidBodyComponent::Create(
    Id id,
    const Vector &tensor,
    float mass,
    bool useGravity
    )
{
    SetId( id );
    m_RigidBody.Create( this, tensor, mass, useGravity );
}

void RigidBodyComponent::Bind( void )
{
    CollisionComponent *pCollision = GetParent( )->GetComponent<CollisionComponent>();
    m_RigidBody.Bind( pCollision ? pCollision->GetCollisionObject() : NULL );
}

void RigidBodyComponent::Destroy( void )
{
    m_RigidBody.Destroy( );
}

void RigidBodyComponent::UseGravity( 
    bool useGravity
    )
{
    m_RigidBody.UseGravity( useGravity );
}

void RigidBodyComponent::Render( 
    const Vector &color
    )
{
    m_RigidBody.GetCollisionObject( )->Render( color );
}

void RigidBodyComponent::AddToScene( void )
{
    if ( false == GetParent()->IsInScene() ) return;
    m_RigidBody.AddToScene( );
}

void RigidBodyComponent::RemoveFromScene( void )
{
    m_RigidBody.RemoveFromScene( );
}

void RigidBodyComponent::ApplyWorldForce(
    const Vector &position, 
    const Vector &force
    )
{
    m_RigidBody.ApplyWorldForce( position, force );
}

void RigidBodyComponent::ResetPhysics( void )
{
    m_RigidBody.ResetPhysics( );
}

ISerializable *RigidBodyComponentSerializer::Deserialize(
    Serializer *pSerializer,
    ISerializable *pSerializable
    )
{
    if ( NULL == pSerializable ) pSerializable = new RigidBodyComponent; 

    RigidBodyComponent *pRigidBodyComponent = (RigidBodyComponent *) pSerializable;

    Id id = Id::Deserialize( pSerializer->GetInputStream() );

    Vector tensor;
    float mass;
    bool useGravity;

    pSerializer->GetInputStream()->Read( &tensor, sizeof(tensor) );
    pSerializer->GetInputStream()->Read( &mass, sizeof(mass) );
    pSerializer->GetInputStream()->Read( &useGravity, sizeof(useGravity) );

    pRigidBodyComponent->Create( id, tensor, mass, useGravity != 0 );

    return pSerializable;
}
