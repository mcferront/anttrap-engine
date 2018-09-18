#pragma once

#include "EngineGlobal.h"
#include "Component.h"
#include "CollisionObject.h"
#include "CollisionHandler.h"

class CollisionTrigger
{
public:
    typedef void (*TriggerCallback) (void *pUserData, Id id);

private:
    struct Callback
    {
        void *pUserData;
        TriggerCallback pEntered;
        TriggerCallback pExited;
    };

private:

    Callback         m_Callback;

    HashTable<Id, Id> m_HashA;
    HashTable<Id, Id> m_HashB;

    HashTable<Id, Id> *m_pExistingHash;
    HashTable<Id, Id> *m_pNewHash;

public:
    CollisionTrigger( void )
      {}

      void Create(
          Component *pComponent,
          void *pUserData,
          TriggerCallback pEntered,
          TriggerCallback pExited
          );


      void Destroy( void );

      void Run( 
          CollisionObject *pObject
          );

private:
    void OnCollisionCallback(
        const CollisionObject *pObject
        );

private:
    static void CollisionCallback(
        const CollisionObject *pObject, 
        nuint userData
        );
};

typedef void (*PhantomCollisionCallback)(const CollisionObject *pObject, nuint userData);

class PhantomCollisionHandler : public ICollisionHandler
{
private:
    PhantomCollisionCallback m_Callback;
    nuint m_UserData;

public:
    PhantomCollisionHandler(
        PhantomCollisionCallback,
        nuint userData
        );

    virtual void Run(
        const CollisionObject *pObjectA,
        const CollisionObject *pObjectB
        );
};
