#include "EnginePch.h"

#include "CollisionComponent.h"
#include "CollisionSphere.h"
#include "CollisionPlane.h"
#include "CollisionMesh.h"
#include "CollisionTrigger.h"

DefineComponentType(CollisionComponent, new CollisionComponentSerializer);

void CollisionComponent::Create(
    Id id,
    CollisionObject *pObject
    )
{
    SetId( id );
    m_pCollision = pObject;
    m_pTrigger = NULL;
}

void CollisionComponent::AddTrigger( void )
{
    m_pTrigger = new CollisionTrigger;
    m_pTrigger->Create( this, this, CollisionComponent::OnTriggerEntered, CollisionComponent::OnTriggerExited );
}

void CollisionComponent::Destroy( void )
{
    if (NULL != m_pTrigger)
    {
        m_pTrigger->Destroy( );
        delete m_pTrigger;
    }

    m_pCollision->Destroy( );
    delete m_pCollision;
}

void CollisionComponent::Render( 
    const Vector &color
    )
{
    m_pCollision->Render( color );
}

void CollisionComponent::AddToScene( void )
{
    if ( false == GetParent()->IsInScene() ) return;

    Component::AddToScene( );

    if (NULL == m_pTrigger)
        m_pCollision->AddToScene( );
}

void CollisionComponent::Bind( void )
{
    Component::Bind( );

    Transform t = GetParent()->GetWorldTransform( );
    m_pCollision->SetTransform( t );
}

void CollisionComponent::RemoveFromScene( void )
{
    if (NULL == m_pTrigger)
        m_pCollision->RemoveFromScene( );

    Component::RemoveFromScene( );
}

void CollisionComponent::Tick( 
    float deltaSeconds 
    )
{
    Component::Tick( deltaSeconds ); 
    
    //Render( Vector(1,1,1,1) );

    Transform t = GetParent()->GetWorldTransform( );
    m_pCollision->SetTransform( t );
}

void CollisionComponent::PostTick( void )
{
    Component::PostTick(); 
    
    //Render( Vector(1,1,1,1) );

    Transform t = GetParent()->GetWorldTransform( );
    m_pCollision->SetTransform( t );
}

void CollisionComponent::Final( void )
{
    Component::Final();

    if (NULL != m_pTrigger)
        m_pTrigger->Run( m_pCollision );
}

void CollisionComponent::OnTriggerEntered(
    void *pData, Id o
)
{
    CollisionComponent *pC = (CollisionComponent *) pData;
    //Debug::Print( Debug::TypeInfo, "Object %s entered trigger %s", o.ToString(), pC->GetParent()->GetName() ); 
    pC->GetParent()->GetChannel()->SendEvent( "TriggerEntered", ArgList(o) );
    
    ResourceHandle other(o);

    if (IsResourceLoaded(other))
        GetResource(other, Resource)->GetChannel()->SendEvent( "TriggerEntered", ArgList(pC->GetParent()->GetId()) );
}

void CollisionComponent::OnTriggerExited(
    void *pData, Id o
)
{
    CollisionComponent *pC = (CollisionComponent *) pData;
    //Debug::Print( Debug::TypeInfo, "Object %s exited trigger %s", o.ToString(), pC->GetParent()->GetName() ); 
    pC->GetParent()->GetChannel()->SendEvent( "TriggerExited", ArgList(o) );

    ResourceHandle other(o);

    if (IsResourceLoaded(other))
        GetResource(other, Resource)->GetChannel()->SendEvent( "TriggerExited", ArgList(pC->GetParent()->GetId()) );
}

ISerializable *CollisionComponentSerializer::Deserialize(
    Serializer *pSerializer,
    ISerializable *pSerializable
    )
{
    if ( NULL == pSerializable ) pSerializable = new CollisionComponent; 

    CollisionComponent *pCollisionComponent = (CollisionComponent *) pSerializable;

    Id id = Id::Deserialize( pSerializer->GetInputStream() );
    
    int type;
    pSerializer->GetInputStream()->Read( &type, sizeof(type) );

    float restitution, friction;

    pSerializer->GetInputStream()->Read( &restitution, sizeof(restitution) );
    pSerializer->GetInputStream()->Read( &friction, sizeof(friction) );

    if ( 999 == type )
    {
        byte trigger;

        Id collider = Id::Deserialize( pSerializer->GetInputStream() );
        pSerializer->GetInputStream()->Read( &trigger, sizeof(byte) );

        CollisionMesh *pMesh = new CollisionMesh;
        pMesh->Create( pCollisionComponent, restitution, friction, collider );

        pCollisionComponent->Create( id, pMesh );

        if (0 != trigger)
            pCollisionComponent->AddTrigger( );
    }
    else if ( 0 == type ) //sphere
    {
        byte trigger;

        pSerializer->GetInputStream()->Read( &trigger, sizeof(byte) );

        CollisionSphere *pSphere = new CollisionSphere;
        pSphere->Create( pCollisionComponent, restitution, friction );

        pCollisionComponent->Create( id, pSphere );

        if (0 != trigger)
            pCollisionComponent->AddTrigger( );
    }
    else if ( 1 == type ) //plane
    {
        byte trigger;
        pSerializer->GetInputStream()->Read( &trigger, sizeof(byte) );

        CollisionPlane *pPlane = new CollisionPlane;
        pPlane->Create( pCollisionComponent, restitution, friction );

        pCollisionComponent->Create( id, pPlane );

        if (0 != trigger)
            pCollisionComponent->AddTrigger( );
    }
    else
        Debug::Assert( Condition(false), "Unrecognized collision type: %d", type );

    return pSerializable;
}
