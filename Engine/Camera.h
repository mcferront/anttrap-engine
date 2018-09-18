#pragma once

#include "EngineGlobal.h"
#include "ISearchable.h"

class Camera
{
private:
   Matrix     m_PrevVP;
   Matrix     m_RenderedVP;
   Transform  m_WorldTransform;
   Vector     m_ViewportRect;
   Vector     m_Jitter;
   int        m_ClearFlags;

   float      m_NearZ;
   float      m_FarZ;
   float      m_FovX;
   float      m_OrthoScale;
   float      m_Aspect;
   float      m_Width;
   float      m_Height;

   mutable Matrix    m_Projection;
   mutable Matrix    m_ReverseDepthProjection;
   mutable Matrix    m_InvProjection;
   mutable Transform m_ViewTransform;
   mutable Frustum   m_Frustum;
   mutable bool      m_FrustumDirty;
   mutable bool      m_IsPerspective;
   mutable bool      m_ViewDirty;
   mutable bool      m_ProjectionDirty;

public:
   void CreateAsPerspective(
      float fovX,
      float aspectY,
      float nearZ,
      float farZ
   );

   void CreateAsPerspectiveWH(
      float width,
      float height,
      float nearZ,
      float farZ
   );

   void CreateAsOrtho(
      float scale,
      float width,
      float height,
      float nearZ,
      float farZ
   );

   void GetWorldTransform(
      Transform *pTransform
   ) const;

   void GetViewTransform(
      Transform *pTransform
   ) const;

   void GetViewProjection(
      Matrix *pVP
   ) const;

   void GetPrevVProjection(
      Matrix *pVP
   ) const;

   void GetProjection(
      Matrix *pProjection
   ) const;

   void GetReverseDepthProjection(
      Matrix *pProjection
   ) const;

   void GetInvProjection(
      Matrix *pInvProj
   ) const;

   void Copy(
      const Camera &camera
   );

   void PushVPMatrix( );

   void GetFrustum(
      Frustum *pFrustum
   ) const;

   void Project(
      Vector *pProjectedPosition,
      const Vector &position
   ) const;

   void Unproject(
      Vector *pUnprojected,
      const Vector &position
   ) const;

   void SetWorldTransform(
      const Transform &transform
   )
   {
      m_WorldTransform = transform;
      m_ViewDirty = true;
      m_FrustumDirty = true;
   }

   void SetJitter(
      const Vector2 &jitter
   )
   {
      m_Jitter = jitter;
      m_ProjectionDirty = true;
   }

   void GetJitter( 
      Vector2 *pJitter
   ) const 
   { 
      *pJitter = *(Vector2 *) &m_Jitter; 
   }

   void SetViewportRect(
      const Vector &rect
   )
   {
      m_ViewportRect = rect;
   }

   void GetViewportRect(
      Vector *pRect
   ) const
   {
      *pRect = m_ViewportRect;
   }

   void SetOrthoDimensions(
      int width,
      int height
   )
   {
      if ( false == m_IsPerspective )
      {
         m_Width = width  * m_OrthoScale;
         m_Height = height * m_OrthoScale;
         m_Aspect = m_Width / m_Height;

         m_ProjectionDirty = true;
      }
   }

   const Transform *GetWorldTransform( void ) const { return &m_WorldTransform; }

   bool  IsPerspective( void ) const { return m_IsPerspective; }
   float GetWidth( void ) const { return m_Width; }
   float GetHeight( void ) const { return m_Height; }
   float GetOrthoScale( void ) const { return m_OrthoScale; }
   float GetNearClip( void ) const { return m_NearZ; }
   float GetFarClip( void ) const { return m_FarZ; }
   float GetFovX( void ) const { return m_FovX; }
   float GetAspectRatio( void ) const { return m_Aspect; }
   int   GetClearFlags( void ) const { return m_ClearFlags; }

private:
   void UpdateProjection( void ) const;
};