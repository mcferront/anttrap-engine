#pragma once

#include "EngineGlobal.h"
#include "CollisionObject.h"

class CollisionPlane : public CollisionObject
{
private:
    struct SurfaceDesc
    {
        Vector  linearVelocity;
        Vector  angularVelocity;
        float   spring;
        float   friction;
    };

private:
    SurfaceDesc m_SurfaceDesc;
    Vector m_WorldPlane;

public:
    CollisionPlane( void ) : 
      CollisionObject( CollisionTypePlane )
      {}

      void Create(
          Component *pComponent,
          float restitution,
          float friction
          );

      void Destroy( void );

      void SetSurfaceDesc(
          float spring,
          float friction
          )
      {
          m_SurfaceDesc.spring   = spring;
          m_SurfaceDesc.friction = friction;
      }

      void SetSurfaceVelocity(
          const Vector &linearVelocity,
          const Vector &angularVelocity
          )
      {
          m_SurfaceDesc.linearVelocity = linearVelocity;
          m_SurfaceDesc.angularVelocity= angularVelocity;
      }


      const Vector *GetLinearVelocity ( void )  const { return &m_SurfaceDesc.linearVelocity; }
      const Vector *GetAngularVelocity( void )  const { return &m_SurfaceDesc.angularVelocity; }
      float GetSpring( void )                   const { return m_SurfaceDesc.spring; }
      float GetFriction( void )                 const { return m_SurfaceDesc.friction; }

      void SetTransform(
          const Transform &transform
          )
      {
          m_WorldPlane = transform.GetUp( );
          m_WorldPlane.w = Math::DotProduct(m_WorldPlane, transform.GetUp());
      }

      void GetPlane( 
          Vector *pPlane
          ) const
      { 
          *pPlane = m_WorldPlane;
      }
};
