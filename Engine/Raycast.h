#pragma once

#include "EngineGlobal.h"
#include "CollisionObject.h"
#include "PhysicsObject.h"
#include "CollisionHandler.h"

struct RaycastResult
{
    Id     objectHit;
    Vector   point;
    Vector   normal;
    float    length;
    bool     isValid;
};

class Raycast : public CollisionObject
{
private:
    Vector m_Start;
    Vector m_End;
    Vector m_Direction;
    float  m_Length;

public:
    Raycast(
        const Vector &start,
        const Vector &direction,
        float length,
        const char *pLayer
        ) : CollisionObject( CollisionTypeRaycast )
    {
        m_Start     = start;
        m_End       = start + direction * length;
        m_Length    = length;
        m_Direction = direction;

        CollisionObject::SetCollisionLayerBitFlag( pLayer );
    }

    bool Cast( 
        RaycastResult *pResult 
        );

    const Vector *GetStart    ( void ) const { return &m_Start; }
    const Vector *GetEnd      ( void ) const { return &m_End; }
    const Vector *GetDirection( void ) const { return &m_Direction; }

    float GetLength( void ) const { return m_Length; }
};

class CollisionDesc;

class RaycastCollisionHandler : public ICollisionHandler
{
private:
    RaycastResult *m_pResult;

public:
    RaycastCollisionHandler(
        RaycastResult *pRaycast
        );

    virtual void Run(
        const CollisionObject *pObjectA,
        const CollisionObject *pObjectB
        );
};
