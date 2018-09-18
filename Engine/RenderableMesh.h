#pragma once

#include "EngineGlobal.h"
#include "RenderObject.h"
#include "ResourceMaps.h"
#include "ResourceWorld.h"
#include "Skeleton.h"
#include "List.h"
#include "MaterialObject.h"

class Viewport;
class Entity;

class RenderableMesh : public RenderObject
{
   class RenderableSurface : public RenderObject
   {
   private:
      RenderableMesh *m_pMesh;
      GraphicsMaterialObject *m_pMaterial;
      uint32 m_Index;

   public:
      RenderableSurface(
         RenderableMesh *pMesh,
         uint32 index
         )
      {
         m_pMesh = pMesh;
         m_pMaterial = NULL;
         m_Index = index;
      }

      ~RenderableSurface( void )
      {
         delete m_pMaterial;
      }

      virtual bool IsVisible(
         const Frustum &frustum
         ) const;

      virtual void GetRenderData(
         RendererDesc *pDesc
         ) const;

      virtual void GetRenderGroups(
         IdList *pList ) const
      {
         pList->CopyFrom( m_pMesh->m_RenderGroups );
      }

      void SetMaterial(
         ResourceHandle material
      )
      {
         if ( m_pMaterial )
            delete m_pMaterial;

         m_pMaterial = new GraphicsMaterialObject( material );;
      }

      virtual void GetWorldTransform(
         Transform *pWorldTransform
      ) const
      {
         *pWorldTransform = m_pMesh->GetComponent( )->GetParent( )->GetWorldTransform( );
      }

      virtual int GetRenderType( void ) const { return RenderObject::Type::Mesh; }

   private:
      static void Render(
         const RenderDesc &desc,
         RenderStats *pStats
         );
   };

private:
   typedef List<RenderableSurface *> SurfaceList;

private:
   Component     *m_pComponent;
   Vector         m_ModColor;
   IdList         m_RenderGroups;
   Transform      m_RTransform;
   Transform      m_PTransform;

   ResourceHandleHash m_Bones;
   ResourceHandleList m_OrderedBoneList;

   ResourceHandle      m_Model;
   SurfaceList         m_Surfaces;

   mutable Matrix    m_PlaneToLocal;
   mutable Transform m_PrevTransform;
   mutable bool      m_SkeletonDirty;

public:
   void Create(
      Component *pComponent,
      const IdList &renderGroups,
      ResourceHandle model
      );

   void Destroy( void );

   void Bind( void );

   void AddMaterial(
      int surface,
      ResourceHandle material
      );

   bool IsVisible(
      const Frustum &frustum
      ) const;

   void AddToScene( void );
   void RemoveFromScene( void );

   int GetNumBones( void );

   virtual bool NeedsFlush( void ) const { return true; }
   virtual void Flush( void );

   Transform GetWorldBoneTransform(
      uint32 boneIndex
      ) const;

   uint32 GetNumMaterials( void ) const { return m_Surfaces.GetSize( ); }

   ResourceHandle GetModel( void ) const { return m_Model; }

   Component *GetComponent( void ) const { return m_pComponent; }

   virtual void GetWorldTransform(
      Transform *pWorldTransform
   ) const
   {
      *pWorldTransform = GetComponent( )->GetParent( )->GetWorldTransform( );
   }

   virtual int GetRenderType( void ) const { return RenderObject::Type::Mesh; }

   virtual void GetRenderGroups(
      IdList *pList ) const
   {
      pList->CopyFrom( m_RenderGroups );
   }

   virtual void GetRenderData(
      RendererDesc *pDesc
   ) const
   {
   }

private:
   void FindBones( void );
};
