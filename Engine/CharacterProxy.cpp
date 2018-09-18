#include "EnginePch.h"

#include "CharacterProxy.h"
#include "PhysicsWorld.h"
#include "CollisionHandler.h"
#include "DebugGraphics.h"

void CharacterProxy::Create(
    Component *pComponent
    )
{
    PhysicsObject::Create( pComponent );

    m_pCollision = NULL;

    m_UseGravity  = true;
    m_LinearVelocity = Math::ZeroVector();

    m_CollisionCorrection = Math::ZeroVector();

    m_MovementRequest  = Math::ZeroVector();
    m_Gravity          = Math::ZeroVector();
    m_CollisionNormal  = Math::ZeroVector();
    m_Transform        = Math::IdentityTransform();
}

void CharacterProxy::Destroy( void )
{
}

void CharacterProxy::Bind(
    CollisionObject *pCollision
)
{
    m_pCollision = pCollision;
    
    if (NULL != m_pCollision)
    {
        m_pCollision->SetSurfaceVelocity( m_LinearVelocity, Math::ZeroVector() );
        m_pCollision->SetTransform( m_Transform );            
    }
}

void CharacterProxy::ApplyMovement( 
    const Vector &delta 
    )
{
    m_MovementRequest += delta;
}

void CharacterProxy::BeginSimulation( 
    uint32 intervals
    )
{
    if ( true == m_UseGravity )
    {
        Vector gravity = *PhysicsWorld::Instance( ).GetGravity( );
        m_Gravity += Vector(gravity.x, gravity.y, gravity.z);
    }

    m_MovementRequest = m_MovementRequest * (1.0f / intervals);

    m_CollisionNormal = Math::ZeroVector();
}

void CharacterProxy::Update( void )
{
    //do nothing in update, allow character's movement
    //to play out in resolve and then check for collisions
    //doing this lets the character's movement penetrate objects more
    //before it's pushed back - so the world feels more interactive
}

void CharacterProxy::Resolve(
    float deltaTime
    )
{
    Vector position;

    //move according to our collision, gravity and external forces
    m_Transform.GetTranslation( &position );

    Vector origin = position;

    position += m_Gravity * deltaTime;
    position += m_MovementRequest;

    //should always be 0 because don't resolve our collision
    //until the character has performed their movement
    position += m_CollisionCorrection;
    m_CollisionCorrection = Math::ZeroVector2();

    m_Transform.SetTranslation( position ); 

    m_pCollision->SetTransform( m_Transform );

    //now that we've moved based on player input, run our collision 
    //to see if we should adjust from other objects' final resting place
    CharacterProxyCollisionHandler handler( this );
    PhysicsWorld::Instance( ).RunCollisionHandler( m_pCollision, &handler );

    //now that we've moved, run collision our collision response
    //using only the collision forces to push us back
    position += m_CollisionCorrection;

    m_Transform.SetTranslation( position ); 
    m_pCollision->SetTransform( m_Transform );

    if ( Math::DotProduct(m_Gravity, m_CollisionCorrection) < 0 )
    {
        m_Gravity = Math::ZeroVector();
    }

    m_pCollision->SetSurfaceVelocity( position - origin, Math::ZeroVector() );

    m_CollisionCorrection = Math::ZeroVector();
}

void CharacterProxy::EndSimulation( void )
{
    m_MovementRequest = Math::ZeroVector();
    m_CollisionCorrection   = Math::ZeroVector();

    Math::Normalize( &m_CollisionNormal, m_CollisionNormal );
}

CharacterProxyCollisionHandler::CharacterProxyCollisionHandler(
    CharacterProxy *pCharacterProxy
    )
{
    m_pCharacterProxy = pCharacterProxy;   
}

void CharacterProxyCollisionHandler::Run(
    const CollisionObject *pObjectA,
    const CollisionObject *pObjectB
    )
{
    CollisionDesc desc;

    bool collision = PhysicsWorld::Instance( ).TestCollision( pObjectA, pObjectB, &desc, true );

    if ( true == collision )
    {
        Respond( desc ); 
    }
}

void CharacterProxyCollisionHandler::Respond(
    const CollisionDesc &desc
    )
{
    Vector movement = Math::ZeroVector();

    for ( int i = 0; i < desc.count; i++ )
    {
        const CollisionDesc::Desc *pDesc = &desc.desc[ i ];
        movement += pDesc->normal * pDesc->penetration * (1.0f / desc.count);

        //DebugGraphics::Instance( ).RenderLine( Math::IdentityTransform, pDesc->intersection, pDesc->intersection + pDesc->normal * 10 * pDesc->penetration, Vector(1, 0, 0, 1) );
    }   

    m_pCharacterProxy->ApplyCollisionCorrection( movement );
}


