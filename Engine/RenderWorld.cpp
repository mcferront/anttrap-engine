#include "EnginePch.h"

#include "RenderWorld.h"
#include "RenderObject.h"
#include "Renderer.h"
#include "Window.h"
#include "Viewport.h"
#include "RenderTree.h"
#include "Node.h"
#include "CameraComponent.h"
#include "TaskWorld.h"

RenderWorld &RenderWorld::Instance( void )
{
   static RenderWorld s_instance;
   return s_instance;
}

void RenderWorld::AddWindow(
   const Window &window
   )
{
   MainThreadCheck;

   //require because the windows
   //will be used when Render is called
   ScopeLock lock( m_RenderSettingsLock );

   WindowDesc *pDesc = new WindowDesc;

   pDesc->window.Copy( window );
   pDesc->viewports.Create( );

   m_WindowHash.Add( pDesc->window.GetId( ).ToString( ), pDesc );
}

void RenderWorld::AddViewport(
   const Viewport &viewport,
   RenderTree *pRenderTree
   )
{
   MainThreadCheck;

   //require because the viewports
   //will be used when Render is called
   ScopeLock lock( m_RenderSettingsLock );

   ViewportDesc *pViewportDesc = new ViewportDesc;

   pViewportDesc->viewport.Copy( viewport );
   pViewportDesc->pRenderTree = pRenderTree;

   m_ViewportHash.Add( pViewportDesc->viewport.GetId( ).ToString( ), pViewportDesc );
}

void RenderWorld::BindViewportToWindow(
   Id windowId,
   Id viewportId,
   float order
   )
{
   MainThreadCheck;

   //require because the viewports and windows
   //will be used when Render is called
   ScopeLock lock( m_RenderSettingsLock );

   ViewportDesc *pViewportDesc;
   WindowDesc *pWindowDesc;

   if ( false == m_ViewportHash.Get( viewportId.ToString( ), &pViewportDesc ) ) return;
   if ( false == m_WindowHash.Get( windowId.ToString( ), &pWindowDesc ) ) return;

   ViewportOrderDesc orderDesc =
   {
       pViewportDesc,
       order
   };

   uint32 i, size = pWindowDesc->viewports.GetSize( );

   for ( i = 0; i < size; i++ )
      if ( pWindowDesc->viewports.GetPointer( i )->order > orderDesc.order )
         break;

   //i will be the end of the list
   //or at the correct insertion point
   pWindowDesc->viewports.Insert( orderDesc, i );
}

void RenderWorld::UnbindViewportsFromWindow(
   Id windowId
   )
{
   WindowDesc *pDesc;

   if ( true == m_WindowHash.Get( windowId.ToString( ), &pDesc ) )
      pDesc->viewports.Clear( );
}

void RenderWorld::UnbindViewportFromWindow(
   Id windowId,
   Id viewportId
   )
{
   WindowDesc *pDesc;

   if ( false == m_WindowHash.Get( windowId.ToString( ), &pDesc ) ) return;

   uint32 i, size = pDesc->viewports.GetSize( );

   for ( i = 0; i < size; i++ )
   {
      if ( pDesc->viewports.GetPointer( i )->pDesc->viewport.GetId( ) == viewportId )
      {
         pDesc->viewports.RemoveAt( i );
         break;
      }
   }
}

void RenderWorld::UpdateWindow(
   const Window &window
   )
{
   MainThreadCheck;

   //require because the viewports
   //will be used when Render is called
   ScopeLock lock( m_RenderSettingsLock );

   WindowDesc *pDesc;

   if ( true == m_WindowHash.Get( window.GetId( ).ToString( ), &pDesc ) )
   {
      pDesc->window.Copy( window );
   }
}

void RenderWorld::UpdateViewport(
   const Viewport &viewport
   )
{
   MainThreadCheck;

   //require because the viewports
   //will be used when Render is called
   ScopeLock lock( m_RenderSettingsLock );

   ViewportDesc *pDesc;

   if ( true == m_ViewportHash.Get( viewport.GetId( ).ToString( ), &pDesc ) )
   {
      pDesc->viewport.Copy( viewport );
   }
}

const Viewport *RenderWorld::GetViewport(
   Id viewportId
   ) const
{
   ViewportDesc *pDesc;

   if ( true == m_ViewportHash.Get( viewportId.ToString( ), &pDesc ) )
   {
      return &pDesc->viewport;
   }

   return NULL;
}

const Window *RenderWorld::GetWindow(
   Id windowId
   ) const
{
   WindowDesc *pDesc;

   if ( true == m_WindowHash.Get( windowId.ToString( ), &pDesc ) )
   {
      return &pDesc->window;
   }

   return NULL;
}

void RenderWorld::Destroy( void )
{
   MainThreadCheck;

   //require because we don't want to destroy
   //while the renderer is running
   ScopeLock lock( m_RenderSettingsLock );

#ifdef WIN32
   //event driven for video synching
   CloseHandle( m_hExternalEvent );
   CloseHandle( m_hRendererEvent );
#endif

   {
      Enumerator<const char *, RenderObjectHash*> e = m_RenderGroupHash.GetEnumerator( );

      while ( e.EnumNext( ) )
      {
         e.Data( )->Destroy( );
         delete e.Data( );
      }
   }

   {
      Enumerator<const char *, RenderTree*> e = m_RenderTreeHash.GetEnumerator( );

      while ( e.EnumNext( ) )
      {
         e.Data( )->Destroy( );
         delete e.Data( );
      }
   }


   List<Id> ids;
   ids.Create( );

   // Build a list of hardware windows and remove them
   {
      Enumerator<const char *, WindowDesc *> e = m_WindowHash.GetEnumerator( );

      while ( e.EnumNext( ) )
         ids.Add( e.Data( )->window.GetId( ) );
   }

   {
      List<Id>::Enumerator e = ids.GetEnumerator( );

      while ( e.EnumNext( ) )
         RemoveWindow( e.Data( ) );

      ids.Clear( );
   }

   // Build a list of viewports and remove them
   RemoveViewports( );

   ids.Destroy( );

   m_FlushHash.Destroy( );
   m_WindowHash.Destroy( );
   m_ViewportHash.Destroy( );
   m_RenderGroupHash.Destroy( );
   m_RenderTreeHash.Destroy( );
   m_RenderObjectHash.Destroy( );
}

void RenderWorld::RemoveWindow(
   Id windowId
   )
{
   MainThreadCheck;

   //require lock becaause these windows
   //will be used when Render is called
   ScopeLock lock( m_RenderSettingsLock );

   const char *pKey;
   WindowDesc *pDesc;

   m_WindowHash.Remove( windowId.ToString( ), &pKey, &pDesc );

   pDesc->viewports.Destroy( );
   delete pDesc;
}

void RenderWorld::RemoveViewport(
   Id viewportId
   )
{
   MainThreadCheck;

   //require lock becaause these viewports
   //will be used when Render is called
   ScopeLock lock( m_RenderSettingsLock );

   const char *pKey;
   ViewportDesc *pDesc = NULL;

   m_ViewportHash.Remove( viewportId.ToString( ), &pKey, &pDesc );

   //remove this viewport from any window
   //which references it
   Enumerator<const char *, WindowDesc *> e = m_WindowHash.GetEnumerator( );

   while ( e.EnumNext( ) )
   {
      WindowDesc *pWindowDesc = e.Data( );

      ViewportDescList *pViewports = &pWindowDesc->viewports;

      int i, size = pViewports->GetSize( );

      for ( i = 0; i < size; i++ )
      {
         const ViewportOrderDesc *pViewport = pViewports->GetPointer( i );

         if ( pViewport->pDesc == pDesc )
         {
            pViewports->RemoveAt( i );
            break;
         }
      }
   }

   delete pDesc;
}

void RenderWorld::RemoveViewports( void )
{
   MainThreadCheck;

   //require lock becaause these viewports
   //will be used when Render is called
   ScopeLock lock( m_RenderSettingsLock );


   List<Id> ids;
   ids.Create( );

   {
      Enumerator<const char *, ViewportDesc *> e = m_ViewportHash.GetEnumerator( );

      while ( e.EnumNext( ) )
         ids.Add( e.Data( )->viewport.GetId( ) );
   }

   {
      List<Id>::Enumerator e = ids.GetEnumerator( );

      while ( e.EnumNext( ) )
         RemoveViewport( e.Data( ) );

      ids.Clear( );
   }

   ids.Destroy( );
}

void RenderWorld::AddRenderGroup(
   Id renderGroupId
   )
{
   MainThreadCheck;

   RenderObjectHash *pHash;

   if ( false == m_RenderGroupHash.Get( renderGroupId.ToString( ), &pHash ) )
   {
      pHash = new RenderObjectHash( );
      pHash->Create( 128, 128, HashFunctions::NUIntHash, HashFunctions::NUIntCompare );

      m_RenderGroupHash.Add( renderGroupId.ToString( ), pHash );
   }
}

void RenderWorld::AddRenderTree(
   RenderTree *pTree
   )
{
   MainThreadCheck;

   m_RenderTreeHash.Add( pTree->GetId( ).ToString( ), pTree );
}

void RenderWorld::AddObject(
   RenderObject *pObject
   )
{
   MainThreadCheck;

   //no lock required because render results are cached
   //and this object won't make it in the cache
   if ( false == m_RenderObjectHash.Contains((nuint) pObject) )
   {
      m_RenderObjectHash.Add( (nuint) pObject, pObject );
      AddObjectToRenderGroups( pObject );
   }
}

void RenderWorld::RemoveObject(
   RenderObject *pObject
   )
{
   MainThreadCheck;

   //no lock required because render results are cached
   //and this object won't make it in the cache
   m_RenderObjectHash.Remove( (nuint) pObject );
   RemoveObjectFromRenderGroups( pObject );
}

void RenderWorld::GetRenderData( void )
{
   MainThreadCheck;

   // some objects need to prepare themselves before render
   // aka 'flush'
   //
   {
      Enumerator<nuint, RenderObject *> e = m_FlushHash.GetEnumerator( );

      while ( e.EnumNext() )
         e.Data( )->Flush( );
   }

   ScopeLock lock( m_RenderSettingsLock );

   SetupRenderer( );

   {
      Enumerator<const char *, WindowDesc *> e = m_WindowHash.GetEnumerator( );

      while ( e.EnumNext( ) )
         GetRenderData( e.Data( ) );
   }
}

void RenderWorld::GetRenderData(
   Id windowId
   )
{
   MainThreadCheck;

   ScopeLock lock( m_RenderSettingsLock );

   WindowDesc *pWindowDesc;

   if ( m_WindowHash.Get( windowId.ToString( ), &pWindowDesc ) )
   {
      SetupRenderer( );
      GetRenderData( pWindowDesc );
   }
}

void RenderWorld::FinishFrame( void )
{
   GpuDevice::Instance( ).FinishFrame( );
}

void RenderWorld::Render( void )
{
   //not required to be on the main thread
   ScopeLock lock( m_RenderSettingsLock );

#ifdef WIN32
   if ( true == m_UseEvents )
   {
      if ( WAIT_TIMEOUT == WaitForSingleObject( m_hExternalEvent, 100 ) )
      {
         m_UseEvents = false;
      }
   }

   ResetEvent( m_hRendererEvent );
#endif

   Enumerator<const char *, WindowDesc *> e = m_WindowHash.GetEnumerator( );

   while ( e.EnumNext( ) )
      Render( e.Data( ) );

#ifdef WIN32
   SetEvent( m_hRendererEvent );
#endif

   FreeRenderObjectDescs( );
}

void RenderWorld::Render(
   Id windowId
   )
{
   //not required to be on the main thread
   ScopeLock lock( m_RenderSettingsLock );

#ifdef WIN32
   if ( true == m_UseEvents )
   {
      if ( WAIT_TIMEOUT == WaitForSingleObject( m_hExternalEvent, 100 ) )
      {
         m_UseEvents = false;
      }
   }

   ResetEvent( m_hRendererEvent );
#endif

   WindowDesc *pWindowDesc;

   if ( m_WindowHash.Get( windowId.ToString( ), &pWindowDesc ) )
      Render( pWindowDesc );

#ifdef WIN32
   SetEvent( m_hRendererEvent );
#endif
}


void RenderWorld::GetRenderData(
   WindowDesc *pWindowDesc
   )
{
   if ( true == pWindowDesc->window.CanRender( ) )
   {
      ViewportDescList *pViewports = &pWindowDesc->viewports;

      int i, size = pViewports->GetSize( );

      for ( i = 0; i < size; i++ )
      {
         ViewportDesc *pViewport = pViewports->GetPointer( i )->pDesc;
         pViewport->valid = pViewport->viewport.ShouldRender( );

         if ( true == pViewport->valid )
            pViewport->pRenderTree->GetRenderData( pViewport->viewport );
      }
   }
}

void RenderWorld::Render(
   WindowDesc *pWindowDesc
   )
{
   if ( true == pWindowDesc->window.CanRender( ) )
   {
      pWindowDesc->window.BeginRender( );

      ViewportDescList *pViewports = &pWindowDesc->viewports;

      int i, size = pViewports->GetSize( );

      for ( i = 0; i < size; i++ )
      {
         ViewportDesc *pViewport = pViewports->GetPointer( i )->pDesc;
         if ( false == pViewport->valid ) continue;

         if ( true == pViewport->viewport.MakeActive( ) )
            pViewport->pRenderTree->Render( pViewport->viewport );
      }
   
      pWindowDesc->window.EndRender( true );
   }
}

void RenderWorld::SetupRenderer( void )
{
   if ( true == m_RenderGroupCacheDirty )
   {
      m_FlushHash.Clear();

      // Clear last frames render details
      {
         Enumerator<const char *, RenderObjectHash*> e = m_RenderGroupHash.GetEnumerator( );

         while ( e.EnumNext( ) )
            e.Data( )->Clear( );
      }

      Enumerator<nuint, RenderObject*> e = m_RenderObjectHash.GetEnumerator( );

      while ( e.EnumNext( ) )
         AddObjectToRenderGroups( e.Data( ) );

      m_RenderGroupCacheDirty = false;
   }
}

void RenderWorld::AddObjectToRenderGroups(
   RenderObject *pObject
   )
{
   RenderObjectHash *pHash = NULL;

   IdList renderGroups; renderGroups.Create( );
   pObject->GetRenderGroups( &renderGroups );

   {
      IdList::Enumerator e = renderGroups.GetEnumerator( );

      while ( e.EnumNext( ) )
      {
         if ( true == m_RenderGroupHash.Get( e.Data( ).ToString( ), &pHash ) )
         {
            if ( false == pHash->Contains( (nuint) pObject ) )
               pHash->Add( (nuint) pObject, pObject );
         }
      }
   }

   renderGroups.Destroy( );

   if ( pObject->NeedsFlush() )
      m_FlushHash.Add((nuint) pObject, pObject);

   ++m_RenderGroupModification; 
}

void RenderWorld::RemoveObjectFromRenderGroups(
   RenderObject *pObject
   )
{
   m_FlushHash.Remove( (nuint) pObject );

   Enumerator<const char *, RenderObjectHash*> e = m_RenderGroupHash.GetEnumerator( );

   while ( e.EnumNext( ) )
      e.Data( )->Remove( (nuint) pObject );

   ++m_RenderGroupModification; 
}
