#include "EnginePch.h"

#include "PhysicsWorld.h"
#include "PhysicsObject.h"
#include "CollisionObject.h"
#include "CollisionHandler.h"
#include "ChannelSystem.h"
#include "Channel.h"

PhysicsWorld &PhysicsWorld::Instance( void )
{
    static PhysicsWorld s_instance;
    return s_instance;
}

void PhysicsWorld::Create(
    const Vector &gravity,
    float timeStep
    )
{
    m_pChannel = new Channel;
    m_pChannel->Create( Id("PhysicsWorld"), NULL );
    ChannelSystem::Instance( ).Add( m_pChannel );

    m_PhysicsObjects.Create( );
    m_CollisionLayers.Create( 16, 16, CollisionLayerHash, CollisionLayerCompare );

    m_Gravity  = gravity;
    m_TimeStep = timeStep;

    int i;

    for ( i = 0; i < 32; i++ )
    {
        m_EnabledLayers[ i ] = (uint32) 0xffffffff;
        m_LayersToBits[ i ] = Id::Empty;
    }

    m_CollisionHash.Create( 16, 16, CollisionKeyHash, CollisionKeyCompare );
    m_ActiveCollisions.Create( 16, 16, CollisionKeyHash, CollisionKeyCompare );
    m_NewCollisions.Create( );
    m_CollisionFunctionHash.Create( 16, 16, CollisionTypeHashFunc, CollisionCompareFunc );
    m_RequestedNotifications.Create( 16, 16, IdHash, IdCompare );

    m_Leftover = 0;
}

void PhysicsWorld::Destroy( void )
{
    ChannelSystem::Instance( ).Remove( m_pChannel );
    m_pChannel->Destroy( );
    delete m_pChannel;

    List<CollisionList *> list;
    list.Create( );

    {
        Enumerator<CollisionLayer, CollisionList *> e = m_CollisionLayers.GetEnumerator( );

        while ( e.EnumNext( ) )
        {
            list.Add( e.Data( ) );
        }
    }

    m_CollisionLayers.Clear( );
    m_PhysicsObjects.Destroy( );
    m_NewCollisions.Destroy( );
    m_ActiveCollisions.Destroy( );
    m_CollisionHash.Destroy( );
    m_CollisionFunctionHash.Destroy( );
    m_RequestedNotifications.Destroy( );

    int i, size = list.GetSize( );

    for ( i = 0; i < size; i++ )
    {
        list.GetAt( i )->Destroy( );
        delete list.GetAt( i );
    }

    list.Destroy( );
    m_CollisionLayers.Destroy( );
}

void PhysicsWorld::Tick(
    float deltaSeconds
    )
{
    static bool slow = false;

    if ( slow )
    {
        deltaSeconds = 0.001f;
    }

    float remaining = deltaSeconds + m_Leftover;

    BeginSimulation( (int) (deltaSeconds / m_TimeStep + .5f) );

    while ( remaining > m_TimeStep )
    {
        m_CollisionHash.Clear( );

        Update ( );
        Resolve( m_TimeStep );

        remaining -= m_TimeStep;
    }

    m_Leftover = remaining; 

    m_CollisionHash.Clear( );

    Update ( );
    Resolve( remaining );

    EndSimulation( );
}

void PhysicsWorld::AddObject(
    PhysicsObject *pObject
    )
{
    m_PhysicsObjects.AddUnique( pObject );
}

void PhysicsWorld::RemoveObject(
    PhysicsObject *pObject
    )
{
    m_PhysicsObjects.RemoveSorted( pObject );
}

void PhysicsWorld::AddObject(
    CollisionObject *pObject
    )
{
    CollisionList *pObjects;

    if ( false == m_CollisionLayers.Get(pObject->GetCollisionLayerBitFlag( ), &pObjects) )
    {
        pObjects = new CollisionList;
        pObjects->Create( );
        m_CollisionLayers.Add( pObject->GetCollisionLayerBitFlag( ), pObjects );
    }

    pObjects->AddUnique( pObject );
}

void PhysicsWorld::RemoveObject(
    CollisionObject *pObject
    )
{
    CollisionList *pObjects;

    if ( true == m_CollisionLayers.Get(pObject->GetCollisionLayerBitFlag( ), &pObjects) )
    {
        pObjects->Remove( pObject );
    }
}

void PhysicsWorld::RegisterCollisionFunction(
    CollisionType typeA,
    CollisionType typeB,
    CollisionFunction function
    )
{
    CollisionTypeKey key = { typeA, typeB };
    m_CollisionFunctionHash.Add( key, function );
}

void PhysicsWorld::RunCollisionHandler( 
    CollisionObject *pCollisionObject,
    ICollisionHandler *pCollisionHandler
    )
{
    uint32 i, size;
    Enumerator<CollisionLayer, CollisionList *> e = m_CollisionLayers.GetEnumerator( );

    while ( e.EnumNext( ) )
    {
        if ( true == AreLayersEnabled(pCollisionObject->GetCollisionLayerBitFlag( ), e.Key()) )
        {
            CollisionList *pList = e.Data( );

            size = pList->GetSize( );

            for ( i = 0; i < size; i++ )
            {
                CollisionObject *pCollider = pList->GetAt( i );

                if ( pCollisionObject != pCollider )
                {
                    pCollisionHandler->Run( pCollisionObject, pCollider );
                }
            }
        }
    }
}

bool PhysicsWorld::TestCollision(
    const CollisionObject *pObjectA,
    const CollisionObject *pObjectB,
    CollisionDesc *pDesc,
    bool useCachedResults //= true
    )
{
    CollisionFunction function;

    bool exists = false;

    if ( true == useCachedResults )
    {
        exists = GetCollision( pObjectA, pObjectB, pDesc );
    }
    else
    {
        UnregisterCollision( pObjectA, pObjectB );
    }

    if ( false == exists )
    {
        CollisionTypeKey key = { pObjectA->GetCollisionType(), pObjectB->GetCollisionType() };

        if ( true == m_CollisionFunctionHash.Get(key, &function) )
        {  
            function( pObjectA, pObjectB, pDesc );
            RegisterCollision( pObjectA, pObjectB, *pDesc );
        }
        else
        {
            Debug::Print( Debug::TypeWarning, "Could not find collision function for key: %d.%d\n", key.typeA, key.typeB );
        }
    }

    return pDesc->collision;
}

void PhysicsWorld::DisableCollisionLayers(
    CollisionLayer layerA,
    CollisionLayer layerB
    )
{
    uint32 index;

    index = GetLayerIndex( layerA );
    m_EnabledLayers[ index ] &= ~layerB;

    index = GetLayerIndex( layerB );
    m_EnabledLayers[ index ] &= ~layerA;
}

void PhysicsWorld::EnableCollisionLayers(
    CollisionLayer layerA,
    CollisionLayer layerB
    )
{
    uint32 index;

    index = GetLayerIndex( layerA );
    m_EnabledLayers[ index ] |= layerB;

    index = GetLayerIndex( layerB );
    m_EnabledLayers[ index ] |= layerA;
}

void PhysicsWorld::AddNotificationRequest(
    Id id
    )
{
    MainThreadCheck;

    int refCount = 0;

    if ( true == m_RequestedNotifications.Get(id, &refCount) )
    {
        m_RequestedNotifications.Remove( id );
    }

    ++refCount;

    m_RequestedNotifications.Add( id, refCount );
}

void PhysicsWorld::RemoveNotificationRequest(
    Id id
    )
{
    MainThreadCheck;

    int refCount = 0;

    if ( true == m_RequestedNotifications.Get(id, &refCount) )
    {
        m_RequestedNotifications.Remove( id );
    }

    --refCount;

    if ( refCount > 0 )
    {
        m_RequestedNotifications.Add( id, refCount );
    }
}

uint32 PhysicsWorld::LayerToBitFlag(
    Id id
    )
{
    MainThreadCheck;

    int i;

    for ( i = 0; i < sizeof(m_LayersToBits) / sizeof(Id); i++ )
    {
        if (m_LayersToBits[i] == id)
            return 1 << i;

        if (m_LayersToBits[i] == Id::Empty)
        {
            m_LayersToBits[i] = id;
            return 1 << i;
        }
    }

    Debug::Print( Debug::TypeWarning, "Collision Layers limit reached, cannot add collision layer %s\n", id.ToString() );
    return 0;
}

void PhysicsWorld::BeginSimulation( 
    uint32 intervals
    )
{
    m_NewCollisions.Clear( );

    uint32 i, size;

    size = m_PhysicsObjects.GetSize( );

    for ( i = 0; i < size; i++ )
    {
        m_PhysicsObjects.GetAt( i )->BeginSimulation( intervals );
    }
}

void PhysicsWorld::EndSimulation( void )
{
    uint32 i, size;

    size = m_PhysicsObjects.GetSize( );

    for ( i = 0; i < size; i++ )
    {
        m_PhysicsObjects.GetAt( i )->EndSimulation( );
    }

    size = m_NewCollisions.GetSize( );

    //send out new collisions
    ActiveCollisionDesc desc;

    for ( i = 0; i < size; i++ )
    {
        CollisionKey key = m_NewCollisions.GetAt( i );

        m_ActiveCollisions.Get( key, &desc );

        if ( true == m_RequestedNotifications.Contains(desc.objA) )
        {
            m_pChannel->SendEvent( "CollisionStarted", ArgList(desc.objA, desc.objB) );
        }
        if ( true == m_RequestedNotifications.Contains(desc.objB) )
        {
            m_pChannel->SendEvent( "CollisionStarted", ArgList(desc.objB, desc.objA) );
        }
    }

    {
        //send out collisions which no longer exist
        Enumerator<CollisionKey, ActiveCollisionDesc> e = m_ActiveCollisions.GetEnumerator( );

        while ( e.EnumNext( ) )
        {
            CollisionDesc desc;

            m_CollisionHash.Get( e.Key(), &desc );

            if ( false == desc.collision )
            {
                if ( true == m_RequestedNotifications.Contains(e.Data( ).objA) )
                {
                    m_pChannel->SendEvent( "CollisionEnded", ArgList(e.Data( ).objA, e.Data( ).objB) );
                }
                if ( true == m_RequestedNotifications.Contains(e.Data( ).objB) )
                { 
                    m_pChannel->SendEvent( "CollisionEnded", ArgList(e.Data( ).objB, e.Data( ).objA) );
                }
            }
        }
    }

    //rebuild active collisions based on the latest data
    m_ActiveCollisions.Clear( );

    {
        Enumerator<CollisionKey, CollisionDesc> e = m_CollisionHash.GetEnumerator( );

        while ( e.EnumNext( ) )
        {
            if ( true == e.Data( ).collision )
            {
                ActiveCollisionDesc desc = { e.Data( ).pObjectA->GetComponent()->GetId( ), e.Data( ).pObjectB->GetComponent()->GetId( ) };
                m_ActiveCollisions.Add( e.Key( ), desc );
            }
        }
    }

    m_CollisionHash.Clear( );
    m_NewCollisions.Clear( );
}

void PhysicsWorld::Update( void )
{
    uint32 i, size;

    size = m_PhysicsObjects.GetSize( );

    for ( i = 0; i < size; i++ )
    {
        m_PhysicsObjects.GetAt( i )->Update( );
    }
}

void PhysicsWorld::Resolve(
    float timeStep
    )
{
    uint32 i, size;

    size = m_PhysicsObjects.GetSize( );

    for ( i = 0; i < size; i++ )
    {
        m_PhysicsObjects.GetAt( i )->Resolve( timeStep );
    }

    Enumerator<CollisionKey, CollisionDesc> e = m_CollisionHash.GetEnumerator( );

    ActiveCollisionDesc desc;

    while ( e.EnumNext( ) )
    {
        if ( true == e.Data( ).collision && false == m_ActiveCollisions.Contains(e.Key()) )
        {
            desc.objA = e.Data( ).pObjectA->GetComponent()->GetId( );
            desc.objB = e.Data( ).pObjectB->GetComponent()->GetId( );

            m_ActiveCollisions.Add( e.Key( ), desc );
            m_NewCollisions.Add( e.Key( ) );
        }
    }
}

bool PhysicsWorld::AreLayersEnabled(
    const CollisionLayer collisionLayerA,
    const CollisionLayer collisionLayerB
    ) const
{
    uint32 index = GetLayerIndex( collisionLayerA );

    return 0 != (m_EnabledLayers[ index ] & collisionLayerB);
}

uint32 PhysicsWorld::GetLayerIndex(
    CollisionLayer layer
    ) const
{
    uint32 index = 0;

    while ( 0 == (layer & (0x1 << index++)) ) {}

    return index;
}

void PhysicsWorld::RegisterCollision(
    const CollisionObject *pObjectA,
    const CollisionObject *pObjectB,
    const CollisionDesc &desc
    )
{
    CollisionKey key = { pObjectA, pObjectB };
    m_CollisionHash.Add( key, desc );
}

void PhysicsWorld::UnregisterCollision(
    const CollisionObject *pObjectA,
    const CollisionObject *pObjectB
    )
{
    CollisionKey key = { pObjectA, pObjectB };
    m_CollisionHash.Remove( key );
}

bool PhysicsWorld::GetCollision(
    const CollisionObject *pObjectA,
    const CollisionObject *pObjectB,
    CollisionDesc *pDesc
    )
{
    CollisionKey key = { pObjectA, pObjectB };
    bool result = m_CollisionHash.Get( key, pDesc );

    if ( true == result )
    {
        if ( pObjectA == pDesc->pObjectB )
        {
            pDesc->Swap( );
        }
    }

    return result;
}


