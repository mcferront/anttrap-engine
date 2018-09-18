#pragma once

#include "EngineGlobal.h"
#include "CollisionObject.h"
#include "ResourceWorld.h"
#include "List.h"
#include "ColliderAsset.h"

class CollisionMesh : public CollisionObject
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
    Transform   m_Transform;
    Transform   m_InvTransform;

    ResourceHandle m_Mesh;

public:
    CollisionMesh( void ) : 
      CollisionObject( CollisionTypeMesh )
      {}

      void Create(
          Component *pComponent,
          float restitution,
          float friction,
          ResourceHandle mesh
          );

      void Destroy( void );

      void Render(
          const Vector &color
          );

      void GetOverlappingTriangles(
          int *pIds,
          int size,
          int *pReturned,
          const Vector &position,
          float radius
          ) const;

      void GetOverlappingTriangles(
          int *pIds,
          int size,
          int *pReturned,
          const Vector &start,
          const Vector &direcxtion,
          float length
          ) const;

    const Collider::Triangle *GetTriangles( void ) const;

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
          m_Transform = transform;
          Math::Invert( &m_InvTransform, m_Transform );      
      }

      const Transform *GetInvTransform( void ) const { return &m_InvTransform; }
      const Transform *GetTransform( void ) const { return &m_Transform; }
};
