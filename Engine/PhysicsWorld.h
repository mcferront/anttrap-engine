#pragma once

#include "EngineGlobal.h"
#include "List.h"
#include "HashTable.h"
#include "SystemId.h"

class PhysicsObject;
class CollisionObject;
class ICollisionHandler;
class Channel;

class CollisionDesc
{
public:
    CollisionDesc( void )
    {
        collision = false;
        count = 0;
    }
    struct Desc
    {
        Vector intersection;
        Vector normal;
        float penetration;
    };

    struct SurfaceDesc
    {
        Vector position;
        Vector linear;
        Vector angular;
        float spring;
        float friction;
    };

    int count;

    Desc desc[ 8 ];
    SurfaceDesc surfaceA;
    SurfaceDesc surfaceB;

    const CollisionObject *pObjectA;
    const CollisionObject *pObjectB;

    bool collision;

    void Add(const Vector &normal, const Vector &intersection, float penetration)
    {
        if ( count < sizeof(desc) / sizeof(Desc) )
        {
            desc[ count ].intersection = intersection;
            desc[ count ].normal       = normal;
            desc[ count ].penetration  = penetration;

            ++count;
        }
    }

    void Swap( void )
    {
        for ( int i = 0; i < count; i ++ )
        {
            desc[ i ].normal = - desc[ i ].normal;
        }

        SurfaceDesc s = surfaceA;

        surfaceA = surfaceB;
        surfaceB = s;

        const CollisionObject *pO = pObjectA;
        pObjectA = pObjectB;
        pObjectB = pO;
    }
};

typedef void (*CollisionFunction) (const CollisionObject *, const CollisionObject *, CollisionDesc *pDesc);

enum CollisionType
{
   CollisionTypeMesh          = 1 << 0,
   CollisionTypeRaycast       = 1 << 2,
   CollisionTypeHeightField   = 1 << 3,
   CollisionTypePhantom       = 1 << 4,
   CollisionTypeMesh2d        = 1 << 5,
   CollisionTypePhantom2d     = 1 << 6,
   CollisionTypeSphere2d      = 1 << 7,
   CollisionTypeSphere        = 1 << 8,
   CollisionTypePlane         = 1 << 9,
};

typedef uint32 CollisionLayer;

class PhysicsWorld
{
private:
    struct CollisionKey 
    {
        const CollisionObject *pObjectA;
        const CollisionObject *pObjectB;
    };

    struct CollisionTypeKey
    {
        CollisionType typeA;
        CollisionType typeB;
    };

    struct ActiveCollisionDesc
    {
        Id objA;
        Id objB;
    };

private:  
    static inline uint32 CollisionLayerHash( 
        CollisionLayer layer
        )
    {
        return HashFunctions::NUIntHash( layer );
    }

    static inline bool CollisionLayerCompare(
        CollisionLayer layer1,
        CollisionLayer layer2
        )
    {
        return layer1 == layer2;
    }

    static inline uint32 CollisionKeyHash( 
        CollisionKey key
        )
    {
        return HashFunctions::NUIntHash( (nuint) key.pObjectA | (nuint) key.pObjectB );
    }

    static inline bool CollisionKeyCompare(
        CollisionKey key1,
        CollisionKey key2
        )
    {
        return (key1.pObjectA == key2.pObjectA && key1.pObjectB == key2.pObjectB) ||
            (key1.pObjectA == key2.pObjectB && key1.pObjectB == key2.pObjectA);
    }

    static inline uint32 CollisionTypeHashFunc( 
        CollisionTypeKey key
        )
    {
        return HashFunctions::NUIntHash( key.typeA | key.typeB );
    }

    static inline bool CollisionCompareFunc(
        CollisionTypeKey key1,
        CollisionTypeKey key2
        )
    {
        return key1.typeA == key2.typeA &&
            key1.typeB == key2.typeB;
    }

private:
    typedef List<PhysicsObject*>     PhysicsList;
    typedef List<CollisionObject*>   CollisionList;
    typedef List<CollisionKey>       CollisionKeyList;
    typedef HashTable<CollisionKey, CollisionDesc> CollisionHash;
    typedef HashTable<CollisionKey, ActiveCollisionDesc> ActiveCollisionHash;
    typedef HashTable<CollisionLayer, CollisionList*> CollisionListHash;
    typedef HashTable<CollisionTypeKey, CollisionFunction> CollisionFunctionHash;

public:
    static PhysicsWorld &Instance( void );

private:
    PhysicsList           m_PhysicsObjects;
    CollisionListHash     m_CollisionLayers;
    CollisionHash         m_CollisionHash;
    CollisionFunctionHash m_CollisionFunctionHash;

    //for sending out start / end collision events
    ActiveCollisionHash   m_ActiveCollisions;
    CollisionKeyList      m_NewCollisions;
    HashTable<Id, int> m_RequestedNotifications;

    Vector   m_Gravity;
    float    m_TimeStep;
    float    m_Leftover;
    Channel *m_pChannel;
    uint32   m_EnabledLayers[ 32 ];
    Id     m_LayersToBits[ 32 ];

public:
    PhysicsWorld( void )
    {}

    void Create(
        const Vector &gravity,
        float timeStep
        );

    void Destroy( void );

    void Tick(
        float deltaSeconds
        );

    void AddObject(
        PhysicsObject *pPhysicsObject
        );

    void RemoveObject(
        PhysicsObject *pPhysicsObject
        );

    void AddObject(
        CollisionObject *pCollisionObject
        );

    void RemoveObject(
        CollisionObject *pCollisionObject
        );

    void RegisterCollisionFunction(
        CollisionType typeA,
        CollisionType typeB,
        CollisionFunction function
        );

    void RunCollisionHandler( 
        CollisionObject *pCollisionObject,
        ICollisionHandler *pCollisionHandler
        );

    bool TestCollision(
        const CollisionObject *pObjectA,
        const CollisionObject *pObjectB,
        CollisionDesc *pDesc,
        bool useCachedResults = true
        );

    void DisableCollisionLayers(
        CollisionLayer layerA,
        CollisionLayer layerB
        );

    void EnableCollisionLayers(
        CollisionLayer layerA,
        CollisionLayer layerB
        );

    void AddNotificationRequest(
        Id id
        );

    void RemoveNotificationRequest(
        Id id
        );

    CollisionLayer LayerToBitFlag(
        Id id
        );

    void SetGravity(
        const Vector &gravity
        )
    {
        m_Gravity = gravity;
    }

    const Vector *GetGravity( void ) { return &m_Gravity; }

    Channel *GetChannel( void ) const { return m_pChannel; }

private:
    virtual void BeginSimulation( 
        uint32 intervals
        );

    void EndSimulation  ( void );
    void Update         ( void );

    void Resolve(
        float timeStep
        );

    bool AreLayersEnabled(
        CollisionLayer collisionLayerA,
        CollisionLayer collisionLayerB
        ) const;

    uint32 GetLayerIndex(
        CollisionLayer layer
        ) const;

    void RegisterCollision(
        const CollisionObject *pObjectA,
        const CollisionObject *pObjectB,
        const CollisionDesc &desc
        );

    void UnregisterCollision(
        const CollisionObject *pObjectA,
        const CollisionObject *pObjectB
        );

    bool GetCollision(
        const CollisionObject *pObjectA,
        const CollisionObject *pObjectB,
        CollisionDesc *pDesc
        );
};