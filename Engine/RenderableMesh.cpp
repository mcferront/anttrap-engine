#include "EnginePch.h"

#include "RenderableMesh.h"
#include "RegistryWorld.h"
#include "MeshAsset.h"
#include "RenderWorld.h"
#include "ResourceWorld.h"
#include "MaterialAsset.h"
#include "Viewport.h"
#include "Node.h"
#include "LightComponent.h"
#include "DebugGraphics.h"

void RenderableMesh::Create(
   Component *pComponent,
   const IdList &renderGroups,
   ResourceHandle model
   )
{
   m_pComponent = pComponent;

   m_Surfaces.Create( );
   m_OrderedBoneList.Create( );

   m_Model = model;
   m_PlaneToLocal = Math::IdentityMatrix( );
   m_PrevTransform = Math::IdentityTransform( );

   m_RenderGroups.Create( );
   m_RenderGroups.CopyFrom( renderGroups );

   m_RTransform = Math::IdentityTransform();
   m_PTransform = Math::IdentityTransform();
}

void RenderableMesh::Destroy( void )
{
   m_OrderedBoneList.Destroy( );

   for ( uint32 i = 0; i < m_Surfaces.GetSize( ); i++ )
      delete m_Surfaces.GetAt( i );

   m_Surfaces.Destroy( );
   m_RenderGroups.Destroy( );

   m_Model = NullHandle;
}

void RenderableMesh::Bind( void )
{
   FindBones( );
}

void RenderableMesh::AddMaterial(
   int surface,
   ResourceHandle material
   )
{
   while ( m_Surfaces.GetSize( ) <= (uint32) surface )
   {
      RenderableSurface *pSurface = new RenderableSurface( this, surface );
      m_Surfaces.Add( pSurface );
   }

   m_Surfaces.GetAt( surface )->SetMaterial( material );
}

bool RenderableMesh::IsVisible(
   const Frustum &frustum
   ) const
{
   if ( false == IsResourceLoaded( m_Model ) ) return false;

   const Mesh *pMesh = GetResource( m_Model, Mesh );

   Vector center = *pMesh->GetCenter( );
   Vector extents = *pMesh->GetExtents( );

   return BoxInFrustum( center, extents, GetComponent( )->GetParent( )->GetWorldTransform( ), frustum );
}

void RenderableMesh::AddToScene( void )
{
   RenderObject::AddToScene( );

   for ( uint32 i = 0; i < m_Surfaces.GetSize( ); i++ )
      m_Surfaces.GetAt( i )->AddToScene( );
}

void RenderableMesh::RemoveFromScene( void )
{
   RenderObject::RemoveFromScene( );

   for ( uint32 i = 0; i < m_Surfaces.GetSize( ); i++ )
      m_Surfaces.GetAt( i )->RemoveFromScene( );
}

int RenderableMesh::GetNumBones( void )
{
   int numBones = (int) m_OrderedBoneList.GetSize( );

   const Mesh *pMesh = GetResource( m_Model, Mesh );

   if ( NULL != pMesh->GetSkeleton( ) && pMesh->GetSkeleton( )->GetSkeleton( )->GetNumBones( ) != numBones )
      FindBones( );
   else
   {
      for ( int c = 0; c < numBones; c++ )
      {
         if ( m_OrderedBoneList.Get( c ) == NullHandle )
         {
            FindBones( );
            numBones = (int) m_OrderedBoneList.GetSize( );
            break;
         }
      }
   }

   return m_OrderedBoneList.GetSize( );
}

void RenderableMesh::Flush( void )
{
   m_PTransform = m_RTransform;
   GetWorldTransform( &m_RTransform );
}

Transform RenderableMesh::GetWorldBoneTransform(
   uint32 boneIndex
   ) const
{
   if ( IsResourceLoaded( m_OrderedBoneList.Get( boneIndex ) ) )
      return GetResource( m_OrderedBoneList.Get( boneIndex ), Node )->GetWorldTransform( );
   else
      return Math::IdentityMatrix( );
}

void RenderableMesh::FindBones( void )
{
   if ( false == IsResourceLoaded( m_Model ) ) return;

   const Mesh *pMesh = GetResource( m_Model, Mesh );

   m_OrderedBoneList.Clear( );
   m_Bones.Clear( );

   bool one_valid_bone_found = false;

   if ( NULL != pMesh->GetSkeleton( ) )
   {
      int numBones = pMesh->GetSkeleton( )->GetSkeleton( )->GetNumBones( );

      Node *pNode = m_pComponent->GetParent( )->GetParent( );
      if ( NULL == pNode ) pNode = m_pComponent->GetParent( );

      for ( int i = 0; i < numBones; i++ )
      {
         Node *pChild = pNode->FindChildByName( pMesh->GetSkeleton( )->GetBoneName( i ) );

         ResourceHandle childHandle;

         if ( NULL != pChild )
         {
            childHandle = pChild->GetHandle( );
            one_valid_bone_found = true;
         }
         else
            childHandle = NullHandle;

         m_Bones.Add( pMesh->GetSkeleton( )->GetBoneName( i ), childHandle );
         m_OrderedBoneList.Add( childHandle );
      }
   }

   // If no valid bones were found clear this out so 
   // he renders in the T-Pose
   if ( false == one_valid_bone_found )
   {
      m_OrderedBoneList.Clear( );
      m_Bones.Clear( );
   }
}

bool RenderableMesh::RenderableSurface::IsVisible(
   const Frustum &frustum
   ) const
{
   return m_pMesh->IsVisible( frustum );
}

void RenderableMesh::RenderableSurface::GetRenderData(
   RendererDesc *pDesc
   ) const
{
   ResourceHandle model = m_pMesh->GetModel( );

   if ( false == IsResourceLoaded( model ) ) return;
   if ( false == m_pMaterial->Prepare( ) ) return;
   if ( false == m_pMaterial->HasPass( pDesc->pPass ) ) return;

   int numBones = m_pMesh->GetNumBones( );
   
   Transform transform = m_pMesh->GetComponent( )->GetParent( )->GetWorldTransform( );

   Transform invWorld;
   Math::Invert( &invWorld, transform );

   RenderObjectDesc *pRODesc = RenderWorld::Instance( ).AllocRenderObjectDesc( );

   uint32 light_mask = 0;

   static const char *pLightMask = StringRef( "$RESSCALE_LIGHTMASK" );
   if ( m_pMaterial->GetPassData(pDesc->pPass)->GetFloat4MacroIndex(pLightMask) >= 0 )
   {
      const Mesh *pMesh = GetResource( model, Mesh );
      
      Vector center = *pMesh->GetCenter( );
      Vector extents = *pMesh->GetExtents( );

      uint32 i;

      for ( i = 0; i < pDesc->lightDescs.GetSize(); i++ )
      {
         const LightDesc *pLightDesc = pDesc->lightDescs.GetPointer(i);

         if ( pLightDesc->pLight->IsInRange(transform, invWorld, center, extents) )
            light_mask |= 1 << i;
      }

      pRODesc->buffer.Write( &pDesc->packedLights, sizeof(pDesc->packedLights) );
   }

   pRODesc->renderContext = m_pMaterial->GetRenderContext( pDesc->pPass, pDesc->viewport.GetContext(), GetResource(model, Mesh)->GetSurface(m_Index)->GetVertexContext() );
   pRODesc->pMaterial = m_pMaterial;
   pRODesc->pObject = this;
   pRODesc->argList = ArgList( model, transform, m_Index, light_mask, numBones, m_pMesh->m_PTransform );
   pRODesc->renderFunc = RenderableSurface::Render;

   if ( 0 != numBones )
   {
      Matrix *pMatrix = (Matrix *) pRODesc->buffer.Write( sizeof( Matrix ) * numBones );

      for ( int c = 0; c < numBones; c++ )
      {
         Transform t;

         Math::Multiply( &t, m_pMesh->GetWorldBoneTransform( c ), invWorld );
         pMatrix[ c ] = t.ToMatrix(true);
      }
   }

   pDesc->renderObjectDescs.Add( pRODesc );
}

void RenderableMesh::RenderableSurface::Render(
   const RenderDesc &desc,
   RenderStats *pStats
   )
{
   ResourceHandle model;
   Transform transform;
   Transform prevTransform;
   uint32 surfaceIndex;
   uint32 light_mask;
   int numBones;

   desc.pDesc->argList.GetArg( 0, &model );
   desc.pDesc->argList.GetArg( 1, &transform );
   desc.pDesc->argList.GetArg( 2, &surfaceIndex );
   desc.pDesc->argList.GetArg( 3, &light_mask );
   desc.pDesc->argList.GetArg( 4, &numBones );
   desc.pDesc->argList.GetArg( 5, &prevTransform );

   if ( false == IsResourceLoaded( model ) )
      return;

   const Mesh *pMesh = GetResource( model, Mesh );

   const Skeleton *pSkeleton = pMesh->GetInvSkeleton( );

   const Surface *pSurface = pMesh->GetSurface( surfaceIndex );

   GraphicsMaterialObject *pMaterial = desc.pDesc->pMaterial;

   GraphicsMaterialObject::Pass *pPass = pMaterial->GetPass( desc.pDesc->renderContext );

   int index = 0;

   Transform viewTransform;
   desc.pViewport->GetCamera( )->GetViewTransform( &viewTransform );

   Transform cameraWorldTransform;
   desc.pViewport->GetCamera( )->GetWorldTransform( &cameraWorldTransform );

   Matrix projection;
   desc.pViewport->GetCamera( )->GetReverseDepthProjection( &projection );

   Vector cameraParams;
   cameraParams.x = desc.pViewport->GetCamera( )->GetNearClip( );
   cameraParams.y = desc.pViewport->GetCamera( )->GetFarClip( );
   
   static const char *pResScaleLightMask = StringRef( "$RESSCALE_LIGHTMASK" );
   static const char *pWorld = StringRef( "$WORLD_MATRIX" );
   static const char *pView = StringRef( "$VIEW_MATRIX" );
   static const char *pProjection = StringRef( "$PROJECTION_MATRIX" );
   static const char *pVP = StringRef( "$VP_MATRIX" );
   static const char *pCameraWorld = StringRef( "$CAMERA_WORLD_MATRIX" );
   static const char *pCameraParams = StringRef( "$CAMERA_PARAMS" );
   static const char *pPWVP = StringRef( "$PWVP_MATRIX" );

   pPass->GetData()->SetFloat4x4s( pWorld, &transform.ToMatrix( true ), 1 );
   pPass->GetData()->SetFloat4x4s( pView, &viewTransform.ToMatrix( true ), 1 );
   pPass->GetData()->SetFloat4x4s( pProjection, &projection, 1 );
   pPass->GetData()->SetFloat4x4s( pCameraWorld, &cameraWorldTransform.ToMatrix( true ), 1 );
   pPass->GetData()->SetFloat4s( pCameraParams, &cameraParams, 1 );

   Vector resScaleLightMask;
   desc.pViewport->GetRenderScale( (Vector2 *) &resScaleLightMask );
   resScaleLightMask.z = *(float *) (uint32 *) &light_mask;
   pPass->GetData()->SetFloat4s( pResScaleLightMask, &resScaleLightMask, 1 );
   
   Matrix pwvp;
   desc.pViewport->GetCamera( )->GetPrevVProjection( &pwvp );

   Math::Multiply( &pwvp, prevTransform.ToMatrix(true), pwvp );
   pPass->GetData()->SetFloat4x4s( pPWVP, &pwvp, 1 );

   Matrix vp;
   Math::Multiply( &vp, viewTransform.ToMatrix(true), projection );
   pPass->GetData()->SetFloat4x4s( pVP, &vp, 1 );

   int res_scale_light_mask_index = pPass->GetData()->GetFloat4MacroIndex(pResScaleLightMask);

   if ( res_scale_light_mask_index >= 0 )
   {
      static const char *pLightsDir = StringRef( "$LIGHT_DIR" );
      static const char *pLightsColor = StringRef( "$LIGHT_COLOR" );
      static const char *pLightsPosType = StringRef( "$LIGHT_POS_TYPE" );
      static const char *pLightsAtten = StringRef( "$LIGHT_ATTEN" );
      static const char *pLightAmbient = StringRef( "$LIGHT_AMBIENT" );
      static const char *pShadowProj = StringRef( "$SHADOW_PROJECTION_MATRIX" );
      static const char *pShadowView = StringRef( "$SHADOW_VIEW_MATRIX" );

      PackedLights *pLights = (PackedLights *) desc.pDesc->buffer.Read( index++ );
      
      if ( light_mask != 0 )
      {
         pPass->GetData()->SetFloat4s( pLightsDir, pLights->light_dir, sizeof(pLights->light_dir) / sizeof(pLights->light_dir[0]) );
         pPass->GetData()->SetFloat4s( pLightsColor, pLights->light_color, sizeof(pLights->light_color) / sizeof(pLights->light_color[0]) );
         pPass->GetData()->SetFloat4s( pLightsAtten, pLights->light_atten, sizeof(pLights->light_atten) / sizeof(pLights->light_atten[0]) );
         pPass->GetData()->SetFloat4s( pLightsPosType, pLights->light_pos_type, sizeof(pLights->light_pos_type) / sizeof(pLights->light_pos_type[0]) );
      }

      pPass->GetData()->SetFloat4s( pLightAmbient, &pLights->ambient, 1 );
      pPass->GetData()->SetFloat4x4s( pShadowProj, &pLights->shadowProjMatrix, 1 );
      pPass->GetData()->SetFloat4x4s( pShadowView, &pLights->shadowViewMatrix, 1 );
   }

   if ( NULL != pSkeleton )
   {
      Matrix *pMatrices;
      if ( numBones ) pMatrices = (Matrix *) desc.pDesc->buffer.Read( index++ );

      Matrix matrices[ 256 ];
      pSurface->ComputeSkin( matrices, transform, pMesh->GetInvSkeleton( )->GetBones( ), pMatrices, numBones );

      static const char *pSkin = StringRef( "$SKIN" );
      pPass->GetData()->SetFloat4x4s( pSkin, matrices, numBones );
   }

   pMaterial->SetRenderData( pPass, desc.pCommandList );
   
   pSurface->Render( desc.pCommandList, pStats );
}
