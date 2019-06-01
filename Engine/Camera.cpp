#include "EnginePch.h"

#include "Camera.h"

void Camera::CreateAsPerspective(
   float fovX,
   float aspect,
   float nearZ,
   float farZ
)
{
   m_FovX = fovX;
   m_NearZ = nearZ;
   m_FarZ = farZ;
   m_Aspect = aspect;
   m_OrthoScale = 0.0f;
   m_Jitter = Math::ZeroVector( );

   float d = 1.0f;

   m_Width = Math::Tan( fovX / 2.0f );
   m_Width *= d;
   m_Width *= 2.0f;

   m_Height = m_Width / aspect;

   m_IsPerspective = true;
   m_WorldTransform = Math::IdentityTransform( );

   m_ViewDirty = true;
   m_ProjectionDirty = true;
   m_FrustumDirty = true;

   m_ViewportRect = Vector( 0, 0, 1, 1 );
   m_RenderedVP = Math::IdentityMatrix( );
   m_PrevVP = Math::IdentityMatrix( );
}

void Camera::CreateAsPerspectiveWH(
   float width,
   float height,
   float nearZ,
   float farZ
)
{
   m_FovX = 0.0f;
   m_NearZ = nearZ;
   m_FarZ = farZ;
   m_Aspect = width / height;
   m_OrthoScale = 0.0f;
   m_Jitter = Math::ZeroVector( );

   float d = 1.0f;

   m_Width = width;
   m_Height = height;

   m_IsPerspective = true;
   m_WorldTransform = Math::IdentityTransform( );

   m_ViewDirty = true;
   m_ProjectionDirty = true;
   m_FrustumDirty = true;

   m_ViewportRect = Vector( 0, 0, 1, 1 );
   m_RenderedVP = Math::IdentityMatrix( );
   m_PrevVP = Math::IdentityMatrix( );
}

void Camera::CreateAsOrtho(
   float scale,
   float width,
   float height,
   float nearZ,
   float farZ
)
{
   m_FovX = 1.0f;
   m_NearZ = nearZ;
   m_FarZ = farZ;
   m_OrthoScale = scale;
   m_Aspect = 0;
   m_Width = 0;
   m_Height = 0;
   m_Jitter = Math::ZeroVector( );

   m_IsPerspective = false;
   m_WorldTransform = Math::IdentityTransform( );

   m_ViewDirty = true;
   m_ProjectionDirty = true;
   m_FrustumDirty = true;

   m_ViewportRect = Vector( 0, 0, 1, 1 );
   m_RenderedVP = Math::IdentityMatrix( );
   m_PrevVP = Math::IdentityMatrix( );

   SetOrthoDimensions( (int) width, (int) height );
}

void Camera::Copy(
   const Camera &camera
)
{
   m_WorldTransform = camera.m_WorldTransform;
   m_ViewTransform = camera.m_ViewTransform;
   m_Projection = camera.m_Projection;
   m_InvProjection = camera.m_InvProjection;

   m_Jitter = camera.m_Jitter;
   m_FovX = camera.m_FovX;
   m_NearZ = camera.m_NearZ;
   m_FarZ = camera.m_FarZ;
   m_Width = camera.m_Width;
   m_Height = camera.m_Height;
   m_Aspect = camera.m_Aspect;
   m_OrthoScale = camera.m_OrthoScale;

   m_ViewportRect = camera.m_ViewportRect;

   m_Frustum = camera.m_Frustum;

   m_IsPerspective = camera.m_IsPerspective;
   m_ReverseDepthProjection = camera.m_ReverseDepthProjection;

   m_FrustumDirty = camera.m_FrustumDirty;
   m_ViewDirty = camera.m_ViewDirty;
   m_ProjectionDirty = camera.m_ProjectionDirty;

   m_PrevVP = camera.m_PrevVP;
   m_RenderedVP = camera.m_RenderedVP;
}

void Camera::PushVPMatrix( )
{
   // copies rendered to prev
   // copies current to rendered
   m_PrevVP = m_RenderedVP;

   GetViewProjection( &m_RenderedVP );

   // what about jitter?
   // can we filter jitter out before writing velocity?
   //    we probably can in the post process shader
   //    it knows the jitter offset and can probalby subtract that from the velocity

   // should jitter be added in the shader, after proj calculations?
}

void Camera::GetWorldTransform(
   Transform *pTransform
) const
{
   *pTransform = m_WorldTransform;
}

void Camera::GetViewTransform(
   Transform *pTransform
) const
{
   if ( true == m_ViewDirty )
   {
      Math::Invert( &m_ViewTransform, m_WorldTransform );

      m_ViewDirty = false;
   }

   *pTransform = m_ViewTransform;
}

void Camera::GetViewProjection(
   Matrix *pVP
) const
{
   Matrix projection;
   Transform view;

   GetProjection( &projection );
   GetViewTransform( &view );

   Math::Multiply( pVP, view.ToMatrix( true ), projection );
}

void Camera::GetPrevVProjection(
   Matrix *pVP
) const
{
   *pVP = m_PrevVP;
}

void Camera::GetProjection(
   Matrix *pProjection
) const
{
   UpdateProjection( );

   *pProjection = m_Projection;
}

void Camera::GetReverseDepthProjection(
   Matrix *pProjection
) const
{
   UpdateProjection( );

   *pProjection = m_ReverseDepthProjection;
}

void Camera::GetInvProjection(
   Matrix *pInvProjection
) const
{
   UpdateProjection( );

   *pInvProjection = m_InvProjection;
}

void Camera::GetFrustum(
   Frustum *pFrustum
) const
{
   if ( true == m_FrustumDirty )
   {
      // Normals point inside the frustum

      Matrix transposed;
      Vector row[ 4 ];

      Transform view;
      GetViewTransform( &view );

      Matrix projection;
      GetProjection( &projection );

      Math::Multiply( &transposed, view.ToMatrix( true ), projection );

#if 0 //slow path for validation
      {
         Matrix invTransposed;
         Math::Invert( &invTransposed, transposed );
         Math::Invert( &invTransposed, invTransposed );
         Math::Transpose( &invTransposed, invTransposed );

         Frustum f;

         Math::TransformPlane( &f.leftPlane, Vector( 1, 0, 0, 1 ), invTransposed );
         Math::TransformPlane( &f.rightPlane, Vector( -1, 0, 0, 1 ), invTransposed );
         Math::TransformPlane( &f.topPlane, Vector( 0, -1, 0, 1 ), invTransposed );
         Math::TransformPlane( &f.bottomPlane, Vector( 0, 1, 0, 1 ), invTransposed );
         Math::TransformPlane( &f.nearPlane, Vector( 0, 0, 0, 1 ), invTransposed );
         Math::TransformPlane( &f.farPlane, Vector( 0, 0, -1, 1 ), invTransposed );

         Math::NormalizePlane( &f.leftPlane, f.leftPlane );
         Math::NormalizePlane( &f.rightPlane, f.rightPlane );
         Math::NormalizePlane( &f.topPlane, f.topPlane );
         Math::NormalizePlane( &f.bottomPlane, f.bottomPlane );
         Math::NormalizePlane( &f.nearPlane, f.nearPlane );
         Math::NormalizePlane( &f.farPlane, f.farPlane );
      }
#endif

      Math::Transpose( &transposed, transposed );

      transposed.GetRow( 0, &row[ 0 ] );
      transposed.GetRow( 1, &row[ 1 ] );
      transposed.GetRow( 2, &row[ 2 ] );
      transposed.GetRow( 3, &row[ 3 ] );

      m_Frustum.leftPlane = row[ 3 ] + row[ 0 ];
      Math::NormalizePlane( &m_Frustum.leftPlane, m_Frustum.leftPlane );

      m_Frustum.rightPlane = row[ 3 ] - row[ 0 ];
      Math::NormalizePlane( &m_Frustum.rightPlane, m_Frustum.rightPlane );

      m_Frustum.bottomPlane = row[ 3 ] + row[ 1 ];
      Math::NormalizePlane( &m_Frustum.bottomPlane, m_Frustum.bottomPlane );

      m_Frustum.topPlane = row[ 3 ] - row[ 1 ];
      Math::NormalizePlane( &m_Frustum.topPlane, m_Frustum.topPlane );

      Math::NormalizePlane( &m_Frustum.nearPlane, row[ 2 ] );

      m_Frustum.farPlane = row[ 3 ] - row[ 2 ];
      Math::NormalizePlane( &m_Frustum.farPlane, m_Frustum.farPlane );

      m_FrustumDirty = false;
   }

   *pFrustum = m_Frustum;
}

void Camera::Project(
   Vector *pProjectedPosition,
   const Vector &position
) const
{
   Transform view;
   GetViewTransform( &view );

   Matrix proj;
   GetProjection( &proj );

   Math::Multiply( &proj, view.ToMatrix( false ), proj );
   Math::TransformPosition( pProjectedPosition, position, proj );

   float invW;

   if ( 0 != pProjectedPosition->w )
      invW = 1.0f / pProjectedPosition->w;
   else
      invW = 0;

   pProjectedPosition->x *= invW;
   pProjectedPosition->y *= invW;
   pProjectedPosition->z *= invW;
   pProjectedPosition->w = 1.0f;
}

void Camera::Unproject(
   Vector *pUnprojected,
   const Vector &position
) const
{
   Vector result;

   float w = 1.0f;

   if ( true == IsPerspective( ) )
   {
      //we do  1/ for perspective correct w
      float nearClip = 1.0f / m_NearZ;
      float farClip = 1.0f / m_FarZ;

      w = ( farClip - nearClip ) * position.z + nearClip;
      w = 1.0f / w;
   }

   result = position * w;

   Transform view;
   GetViewTransform( &view );

   Matrix invProj;
   GetProjection( &invProj );

   Math::Multiply( &invProj, view.ToMatrix( false ), invProj );
   Math::Invert( &invProj, invProj );

   Math::Transform4( &result, result, invProj );

   *pUnprojected = result;
}

void Camera::UpdateProjection( void ) const
{
   if ( true == m_ProjectionDirty )
   {
      //Perspective WH maintains the fov as the window width changes, thus "scaling" the scene
      //which mimics the behavior of max/maya
      if ( true == m_IsPerspective )
      {
          if ( m_FovX == 0.0f )
          {
             Math::PerspectiveWH( &m_Projection, m_Width, m_Height, m_NearZ, m_FarZ );
             Math::PerspectiveWH( &m_ReverseDepthProjection, m_Width, m_Height, m_FarZ, m_NearZ );
          }
          else
          {
              Math::Perspective( &m_Projection, m_FovX, m_Aspect, m_NearZ, m_FarZ );
              Math::Perspective( &m_ReverseDepthProjection, m_FovX, m_Aspect, m_FarZ, m_NearZ );
          }
      }
      else
      {
         Math::Orthogonal( &m_Projection, m_Width, m_Height, m_NearZ, m_FarZ );
         Math::Orthogonal( &m_ReverseDepthProjection, m_Width, m_Height, m_FarZ, m_NearZ );
      }

      {
         Vector row;

         m_Projection.GetRow( 2, &row );
         row.x = -m_Jitter.x; row.y = -m_Jitter.y;
         m_Projection.SetRow( 2, row );

         m_ReverseDepthProjection.GetRow( 2, &row );
         row.x = -m_Jitter.x; row.y = -m_Jitter.y;
         m_ReverseDepthProjection.SetRow( 2, row );
      }

      Math::Invert( &m_InvProjection, m_Projection );

      m_ProjectionDirty = false;

      m_FrustumDirty = true;
   }
}
