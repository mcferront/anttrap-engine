#include "EnginePch.h"

#include "RigidBody.h"
#include "PhysicsWorld.h"
#include "CollisionHandler.h"
#include "DebugGraphics.h"
#include "Component.h"

void RigidBody::Create(
    Component *pComponent,
    const Vector &tensor,
    float mass,
    bool useGravity
    )
{
    PhysicsObject::Create( pComponent );

    m_pCollision = NULL;

    m_LinearForce  = Math::ZeroVector( );
    m_AngularForce = Math::ZeroVector( );

    m_LinearVelocity  = Math::ZeroVector( );
    m_AngularVelocity = Math::ZeroVector( );

    m_Mass = mass;
    m_InvMass = mass ? 1.0f / mass : 0;

    Vector localTensor = tensor;
    localTensor.w = 1.0f;

    Math::Scale ( &m_Tensor, localTensor );
    Math::Invert( &m_InvTensor, m_Tensor );

    m_UseGravity = useGravity;
}

void RigidBody::Destroy( void )
{
}

void RigidBody::Bind(
    CollisionObject *pCollision
)
{
    m_pCollision = pCollision;
    
    if (NULL != m_pCollision)
        m_pCollision->SetSurfaceVelocity( m_LinearVelocity, m_AngularVelocity );
}

void RigidBody::ApplyLocalForce(
    const Vector &position,
    const Vector &force
    )
{
    Vector worldForce;
    Vector worldPosition;

    Math::Rotate( &worldForce, force, GetComponent()->GetParent()->GetWorldTransform() );
    Math::TransformPosition( &worldPosition, position, GetComponent()->GetParent()->GetWorldTransform() );

    ApplyWorldForce( worldPosition, worldForce );
}

void RigidBody::ApplyWorldForce(
    const Vector &position,
    const Vector &force
    )
{
    Vector translation;
    GetComponent()->GetParent()->GetWorldTransform().GetTranslation( &translation );

    Vector pointToCg = position - translation;
    Math::Normalize( &pointToCg, pointToCg );

    Vector torque;
    Math::CrossProduct( &torque, pointToCg, force );

    m_AngularForce += torque;
    m_LinearForce  += force;
}

void RigidBody::ApplyLocalTorque(
    const Vector &torque
    )
{
    Vector worldTorque;
    Math::Rotate( &worldTorque, torque, GetComponent()->GetParent()->GetWorldTransform() );
}

void RigidBody::ApplyWorldTorque(
    const Vector &torque
    )
{
    m_AngularForce += torque;
}

void RigidBody::ResetPhysics( void )
{
    m_AngularForce    = Math::ZeroVector( );
    m_LinearForce     = Math::ZeroVector( );
    m_AngularVelocity = Math::ZeroVector( );
    m_LinearVelocity  = Math::ZeroVector( );

    if (NULL != m_pCollision)
        m_pCollision->SetSurfaceVelocity( m_LinearVelocity, m_AngularVelocity );
}

void RigidBody::BeginSimulation( 
    uint32 intervals
    )
{
    m_SimulationTransform = GetComponent()->GetParent()->GetWorldTransform(); 
    
    if ( true == m_UseGravity )
    {
        Vector translation;

        if (NULL != m_pCollision)
            m_pCollision->SetTransform( m_SimulationTransform );

        m_SimulationTransform.GetTranslation( &translation );
        ApplyWorldForce( translation, *PhysicsWorld::Instance( ).GetGravity( ) * m_Mass );
    }
}

void RigidBody::Update( void )
{
    RigidBodyCollisionHandler handler( this );

    if (NULL != m_pCollision)
        PhysicsWorld::Instance( ).RunCollisionHandler( m_pCollision, &handler );
}

void RigidBody::Resolve(
    float deltaTime
    )
{
    if ( m_Mass <= 0.0f ) return;

    Quaternion rotation;

    //linear
    {
        Vector acceleration = m_LinearForce * m_InvMass;
        m_LinearVelocity += acceleration * deltaTime;
    }

    //angular
    {
        Vector torque;
        Math::Rotate( &torque, m_AngularForce, m_InvTensor );

        m_AngularVelocity += torque * deltaTime;

        rotation.Set( m_SimulationTransform );

        Quaternion momentRotation( m_AngularVelocity.x, m_AngularVelocity.y, m_AngularVelocity.z, 0.0f );

        rotation += (momentRotation * rotation) * (0.5f * deltaTime);      
        Math::Normalize( &rotation, rotation );
    }

    Vector translation;
    m_SimulationTransform.GetTranslation( &translation );

    translation += m_LinearVelocity * deltaTime;

    m_SimulationTransform.SetTranslation( translation );
    m_SimulationTransform.SetOrientation( rotation );

    if (NULL != m_pCollision)
    {
        m_pCollision->SetSurfaceVelocity( m_LinearVelocity, m_AngularVelocity );
        m_pCollision->SetTransform( m_SimulationTransform );
    }
}

void RigidBody::EndSimulation( void )
{
    m_LinearForce  = Math::ZeroVector( );
    m_AngularForce = Math::ZeroVector( );

    GetComponent()->GetParent()->SetWorldTransform( m_SimulationTransform );
}

RigidBodyCollisionHandler::RigidBodyCollisionHandler(
    RigidBody *pRigidBody
    )
{
    m_pRigidBody = pRigidBody;   
}

void RigidBodyCollisionHandler::Run(
    const CollisionObject *pObjectA,
    const CollisionObject *pObjectB
    )
{
    CollisionDesc desc;

    bool collision = PhysicsWorld::Instance( ).TestCollision( pObjectA, pObjectB, &desc );

    if ( true == collision )
    {
        Respond( desc ); 
    }
}

void RigidBodyCollisionHandler::Respond(
    const CollisionDesc &desc
    )
{
    for ( int i = 0; i < desc.count; i++ )
    {
        const CollisionDesc::Desc *pDesc = &desc.desc[ i ];
        //DebugGraphics::Instance( ).RenderLine( Math::IdentityTransform, pDesc->intersection, pDesc->intersection + pDesc->normal * 100, Vector(1, 0, 0, 1) );

        Vector netForce, forces;

        Vector myTotalVelocity, theirTotalVelocity;

        Vector intersectionA = pDesc->intersection - desc.surfaceA.position;
        Vector intersectionB = pDesc->intersection - desc.surfaceB.position;

        Math::CrossProduct( &myTotalVelocity, desc.surfaceA.angular, intersectionA );
        myTotalVelocity += desc.surfaceA.linear;

        Math::CrossProduct( &theirTotalVelocity, desc.surfaceB.angular, intersectionB );
        theirTotalVelocity += desc.surfaceB.linear;

        Vector relativeVelocity = myTotalVelocity - theirTotalVelocity;

        float spring = desc.surfaceB.spring * pDesc->penetration;

        //calculate value to remove all forces
        //not directly related to the collision normal
        float velocityOnNormal = Math::DotProduct( relativeVelocity, pDesc->normal );

        if ( velocityOnNormal > 0 )
            spring *= 0.01f;

        forces = pDesc->normal * velocityOnNormal;
        forces = relativeVelocity - forces;
        forces = forces * desc.surfaceB.friction;
        forces = forces * m_pRigidBody->GetMass( );

        //remove all forces not related to the collision normal
        netForce  = pDesc->normal * spring;
        netForce  = netForce - forces;

        netForce = netForce * (1.0f / desc.count);

        m_pRigidBody->ApplyWorldForce( pDesc->intersection, netForce );
    }    
}