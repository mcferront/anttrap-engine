#pragma once

#include "EngineGlobal.h"
#include "List.h"
#include "HashTable.h"
#include "SystemId.h"
#include "Viewport.h"
#include "Renderer.h"
#include "Window.h"

//each window can have muliple viewports associated with it
//each viewport contains a render tree to be rendered by the cameras
//each render tree contains a graph of nodes, each node is a custom renderer which holds a list of objects to render
//render trees can share nodes.. for example 'opaque' render node could exist in a main scene render tree and a security camera render tree
//nodes can share lists.. for example the 'main' list could exist in a default render node and a z prepass render node

class RenderObject;
class RenderTree;

class RenderWorld
{
private:
   struct ViewportDesc
   {
      Viewport viewport;
      RenderTree *pRenderTree;
      bool valid;
   };

   struct ViewportOrderDesc
   {
      ViewportDesc *pDesc;
      float order;
   };

   typedef List<ViewportOrderDesc> ViewportDescList;

   struct WindowDesc
   {
      Window window;
      ViewportDescList viewports;
   };

public:
   typedef HashTable<nuint, RenderObject*> RenderObjectHash;

private:
   static const uint32 MaxRenderObjectDescs = 64 * 1024;

   typedef HashTable<const char *, WindowDesc *>       WindowHash;
   typedef HashTable<const char *, ViewportDesc *>     ViewportHash;
   typedef HashTable<const char *, RenderObjectHash *> RenderGroupHash;
   typedef HashTable<const char *, RenderTree *>       RenderTreeHash;

public:
   static RenderWorld &Instance( void );

private:
   Lock             m_RenderSettingsLock;
   WindowHash       m_WindowHash;
   ViewportHash     m_ViewportHash;
   RenderGroupHash  m_RenderGroupHash;
   RenderTreeHash   m_RenderTreeHash;
   RenderObjectHash m_RenderObjectHash;
   RenderObjectDesc m_RenderObjectDescs[ MaxRenderObjectDescs ];
   RenderObjectHash m_FlushHash;
   uint32           m_UsedRenderObjectDescs;
   int              m_RenderGroupModification;
   bool             m_RenderGroupCacheDirty;

public:
   void Create( void )
   {
      m_WindowHash.Create( );
      m_ViewportHash.Create( );
      m_RenderGroupHash.Create( );
      m_RenderTreeHash.Create( );
      m_RenderObjectHash.Create( 128, 128, HashFunctions::NUIntHash, HashFunctions::NUIntCompare );
      m_FlushHash.Create( 128, 128, HashFunctions::NUIntHash, HashFunctions::NUIntCompare );
      m_RenderGroupCacheDirty = true;
      m_UsedRenderObjectDescs = 0;
      m_RenderGroupModification = 0;

#ifdef WIN32
      m_UseEvents = false;

      //event driven for video synching
      //AFTER SHIPPING CHANGE THIS SO WE DON'T NEED IFDEFS
      m_hExternalEvent = CreateEvent( NULL, FALSE, TRUE, NULL );
      m_hRendererEvent = CreateEvent( NULL, TRUE, TRUE, NULL );
#endif
   }

   void Destroy( void );

   RenderObjectDesc *AllocRenderObjectDesc( void )
   {
      Debug::Assert( Condition( m_UsedRenderObjectDescs < MaxRenderObjectDescs ), "RenderObjectDescs overflow" );
      return &m_RenderObjectDescs[ m_UsedRenderObjectDescs++ ];
   }

   void AddWindow(
      const Window &window
   );

   void AddViewport(
      const Viewport &viewport,
      RenderTree *pRenderTree
   );

   void BindViewportToWindow(
      Id windowId,
      Id viewportId,
      float order
   );

   void UnbindViewportsFromWindow(
      Id windowId
   );

   void UnbindViewportFromWindow(
      Id windowId,
      Id viewportId
   );

   void UpdateWindow(
      const Window &window
   );

   void UpdateViewport(
      const Viewport &viewport
   );

   const Viewport *GetViewport(
      Id viewportId
   ) const;

   const Window *GetWindow(
      Id windowId
   ) const;

   void RemoveWindow(
      Id windowId
   );

   void RemoveViewport(
      Id viewportId
   );

   void RemoveViewports( void );

   void AddRenderGroup(
      Id renderGroupId
   );

   void AddRenderTree(
      RenderTree *pTree
   );

   void AddObject(
      RenderObject *pObject
   );

   void RemoveObject(
      RenderObject *pObject
   );

   void GetRenderData( void );

   void GetRenderData(
      Id windowId
   );

   void FinishFrame( void );

   void Render( void );

   void Render(
      Id windowId
   );

   inline void AcquireLock( void ) { m_RenderSettingsLock.Acquire( ); }
   inline void Unlock( void ) { m_RenderSettingsLock.Release( ); }

   inline void RebuildRenderGroupCache( void ) { m_RenderGroupCacheDirty = true; }

   const RenderObjectHash *GetRenderGroup(
      Id group
   ) const
   {
      RenderObjectHash *pHash = NULL;
      m_RenderGroupHash.Get( group.ToString( ), &pHash );

      return pHash;
   }

   RenderTree *GetRenderTree(
      Id tree
   ) const
   {
      RenderTree *pTree = NULL;
      m_RenderTreeHash.Get( tree.ToString( ), &pTree );

      return pTree;
   }

   const int GetRenderGroupModification( void ) const { return m_RenderGroupModification; }

#ifdef WIN32
   HANDLE m_hRendererEvent;
   HANDLE m_hExternalEvent;
   bool m_UseEvents;

   inline void UseEvents( void ) { m_UseEvents = true; }

   inline void Go( void ) { SetEvent( m_hExternalEvent ); }
   inline void Wait( void ) { WaitForSingleObject( m_hRendererEvent, INFINITE ); }
#endif

private:
   void GetRenderData(
      WindowDesc *pDesc
   );

   void Render(
      WindowDesc *pDesc
   );

   void SetupRenderer( void );

   void AddObjectToRenderGroups(
      RenderObject *pObject
   );

   void RemoveObjectFromRenderGroups(
      RenderObject *pObject
   );

   void FreeRenderObjectDescs( )
   {
      for ( uint32 i = 0; i < m_UsedRenderObjectDescs; i++ )
         m_RenderObjectDescs[ i ].Reset( );

      m_UsedRenderObjectDescs = 0;
   }
};
