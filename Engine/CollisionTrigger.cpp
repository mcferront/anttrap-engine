#include "EnginePch.h"

#include "CollisionTrigger.h"
#include "PhysicsWorld.h"
#include "DebugGraphics.h"
#include "Channel.h"

void CollisionTrigger::Create(
    Component *pComponent,
    void *pUserData,
    TriggerCallback pEntered,
    TriggerCallback pExited
    )
{
    m_HashA.Create( 16, 16, IdHash, IdCompare );
    m_HashB.Create( 16, 16, IdHash, IdCompare );

    m_pExistingHash = &m_HashA;
    m_pNewHash      = &m_HashB;

    m_Callback.pUserData = pUserData;
    m_Callback.pEntered = pEntered;
    m_Callback.pExited = pExited;
}

void CollisionTrigger::Destroy( void )
{
    m_HashA.Destroy( );
    m_HashB.Destroy( );
}

void CollisionTrigger::Run( 
    CollisionObject *pCollision
    )
{
    PhantomCollisionHandler handler( CollisionCallback, (nuint) this );
    PhysicsWorld::Instance( ).RunCollisionHandler( pCollision, &handler );

    {
        Enumerator<Id, Id> e = m_pExistingHash->GetEnumerator( );

        while ( e.EnumNext( ) )
        {
            if ( false == m_pNewHash->Contains(e.Data()) )
                m_Callback.pExited( m_Callback.pUserData, e.Data() );
        }
    }

    m_pExistingHash->Clear( );

    //swap hashtables
    HashTable<Id, Id> *pNewItems = m_pNewHash;

    m_pNewHash = m_pExistingHash;
    m_pExistingHash = pNewItems;
}

void CollisionTrigger::OnCollisionCallback(
    const CollisionObject *pObject
    )
{
    Id id = pObject->GetComponent()->GetParent()->GetId();

    if ( false == m_pExistingHash->Contains(id) )
        m_Callback.pEntered( m_Callback.pUserData, id );

    m_pNewHash->Add( id, id );
}

void CollisionTrigger::CollisionCallback(
    const CollisionObject *pObject, 
    nuint userData
    )
{
    CollisionTrigger *pMe = (CollisionTrigger *) userData;
    pMe->OnCollisionCallback( pObject );
}

PhantomCollisionHandler::PhantomCollisionHandler(
    PhantomCollisionCallback callback,
    nuint userData
    )
{
    m_Callback = callback;
    m_UserData = userData;
}

void PhantomCollisionHandler::Run(
    const CollisionObject *pObjectA,
    const CollisionObject *pObjectB
    )
{
    CollisionDesc desc;

    bool collision = PhysicsWorld::Instance( ).TestCollision( pObjectA, pObjectB, &desc );

    if ( true == collision )
    {
        m_Callback( pObjectB, m_UserData );
    }
}


