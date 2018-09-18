#pragma once

#include "EngineGlobal.h"
#include "CollisionObject.h"
#include "List.h"

class CollisionSphere : public CollisionObject
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

    Vector m_Position;
    float  m_Radius;

public:
    CollisionSphere( void ) : 
      CollisionObject( CollisionTypeSphere )
      {}

      void Create(
          Component *pComponent,
          float restituion,
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

      float GetRadius( void ) const { return m_Radius; }

      void SetTransform(
          const Transform &transform
          )
      {
          transform.GetTranslation( &m_Position );
          m_Radius = transform.GetScale( ).y;
      }

      void GetPosition(
          Vector *pPosition
          ) const
      {
          *pPosition = m_Position;
      }

      const Vector *GetPosition( void ) const { return &m_Position; }
};
