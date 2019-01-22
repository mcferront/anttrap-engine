#include "EnginePch.h"

#include "Channel.h"
#include "ChannelSystem.h"
#include "MemoryStreams.h"
#include "ResourceMaps.h"
#include "ResourceWorld.h"

void Channel::Create(
   Id id,
   Channel *pBaseChannel
) 
{
   MainThreadCheck;

   SetId( id );

   m_pBaseChannel  = pBaseChannel;
   m_LockListeners = 0;

   m_MethodHash.Create( );
   m_GetHash.Create( );
   m_SetHash.Create( );
   m_EventListeners.Create( );

   m_PendingListeners.Create( );
   m_EventPool.Create( );
   m_QueuedEvents.Create( );
};

void Channel::CopyMethods(
   Channel *pCopyFrom
)
{
   MainThreadCheck;

   {
      Enumerator<const char *, ISetPropertyMap *> e = pCopyFrom->m_SetHash.GetEnumerator( );
      
      while ( e.EnumNext( ) )
      {
         ISetPropertyMap *pProperty = e.Data( );
         AddSetProperty( *pProperty );
      }
   }

   {
      Enumerator<const char *, IGetPropertyMap *> e = pCopyFrom->m_GetHash.GetEnumerator( );
      
      while ( e.EnumNext( ) )
      {
         IGetPropertyMap *pProperty = e.Data( );
         AddGetProperty( *pProperty );
      }
   }

   {
      Enumerator<const char *, IMethodMap *> e = pCopyFrom->m_MethodHash.GetEnumerator( );
      
      while ( e.EnumNext( ) )
      {
         IMethodMap *pMethod = e.Data( );
         AddMethod( *pMethod );
      }
   }

   {
      Enumerator<const char *, EventList *> e = pCopyFrom->m_EventListeners.GetEnumerator( );

      while ( e.EnumNext( ) )
      {
         EventList *pEventList = e.Data( );

         int i, size = pEventList->GetSize( );

         for ( i = 0; i < size; i++ )
         {
            AddEventListener( *pEventList->GetAt(i)->pEventMap );
         }
      }
   }

   //this does not copy queued event listeners
   //and queued events because Copy should not be called
   //within the event sending loops
}

void Channel::Destroy( void )
{
   MainThreadCheck;

   uint32 i;
   
   MethodList methods;
   GetPropertyList getProperties;
   SetPropertyList setProperties;
   List<EventList*> eventLists;

   methods.Create( );
   getProperties.Create( );
   setProperties.Create( );
   eventLists.Create( );

   //delete all the method maps
   {
      Enumerator<const char *, IMethodMap *> methodEnum   = m_MethodHash.GetEnumerator( );

      while ( methodEnum.EnumNext( ) )
      {
         methods.Add( methodEnum.Data( ) );
      }

      for ( i = 0; i < methods.GetSize( ); i++ )
      {
         delete methods.GetAt( i );
      }
   }

   //delete all the get/set property maps
   {
      Enumerator<const char *, IGetPropertyMap *> propertyEnum = m_GetHash.GetEnumerator( );
   
      while ( propertyEnum.EnumNext( ) )
      {
         getProperties.Add( propertyEnum.Data( ) );
      }

      for ( i = 0; i < getProperties.GetSize( ); i++ )
      {
         delete getProperties.GetAt( i );
      }
   }

   //delete all the get/set property maps
   {
      Enumerator<const char *, ISetPropertyMap *> propertyEnum = m_SetHash.GetEnumerator( );
   
      while ( propertyEnum.EnumNext( ) )
      {
         setProperties.Add( propertyEnum.Data( ) );
      }
      
      for ( i = 0; i < setProperties.GetSize( ); i++ )
      {
         delete setProperties.GetAt( i );
      }
   }

   //delete queued events
   const QueuedEvents *pEvents = m_QueuedEvents.Swap( );

   uint32 size = pEvents->GetSize( );

   for ( i = 0; i < size; i++ )
   {
      QueuedEvent *pEvent = pEvents->GetAt( i );
      pEvent->stream.Close( );

      m_EventPool.Free( pEvent );
   }


   //delete events and event lists
   {
      List<const char *> chars;
      chars.Create( );

      Enumerator<const char *, EventList *> listeners = m_EventListeners.GetEnumerator( );

      while ( listeners.EnumNext( ) )
      {
         chars.Add( listeners.Key( ) );
         eventLists.Add( listeners.Data( ) );
      }

      for ( i = 0; i < eventLists.GetSize( ); i++ )
      {
         uint32 c;

         EventList *pList = eventLists.GetAt( i );
      
         for ( c = 0; c < pList->GetSize( ); c++ )
         {
            ValidEventMap *pEventMap = pList->GetAt( c );

            delete pEventMap->pEventMap;
            delete pEventMap;
         }

         StringRel( chars.GetAt(i) );
         pList->Destroy( );
         delete pList;
      }

      chars.Destroy( );
   }

   methods.Destroy( );
   getProperties.Destroy( );
   setProperties.Destroy( );
   eventLists.Destroy( );

   m_QueuedEvents.Destroy( );
   m_MethodHash.Destroy( );
   m_GetHash.Destroy( );
   m_SetHash.Destroy( );
   m_EventListeners.Destroy( );

   m_PendingListeners.Destroy( );
   m_EventPool.Destroy( );
}

void Channel::Flush( void )
{
   MainThreadCheck;

   ScopeLock lock( m_Lock );

   Debug::Assert( Condition(0 == m_LockListeners), "Flush should never be called from within an event" );

   ++m_LockListeners;

   const QueuedEvents *pEvents = m_QueuedEvents.Swap( );

   uint32 i, c, size = pEvents->GetSize( );

   for ( i = 0; i < size; i++ )
   {
      QueuedEvent *pEvent = pEvents->GetAt( i );

      MemoryStream stream( pEvent->stream.GetBuffer( ), pEvent->stream.GetAmountWritten( ), false );
      SerializedArgList serializedList;

      Serializer serializer( stream.GetInputStream() );
      serializer.Deserialize( &serializedList.m_ArgList );

      for ( c = 0; c < pEvent->pListeners->GetSize( ); c++ )
      {
         ValidEventMap *pValidMap = pEvent->pListeners->GetAt( c );

         if ( pValidMap->isValid )
         {
            pValidMap->pEventMap->Event( this, serializedList.m_ArgList );
         }
      }

      pEvent->stream.Close( );
      serializedList.Destroy( );
      
      m_EventPool.Free( pEvent );
   }

   --m_LockListeners;

   ProcessPendingListeners( );
}

bool Channel::HasEventListeners( void )
{
   Enumerator<const char *, EventList *> e = m_EventListeners.GetEnumerator( );

   while ( e.EnumNext( ) )
   {
      int size = e.Data( )->GetSize( );
      if ( 0 != size ) return true;
   }

   return false;
}

void Channel::ExecuteMethod( 
   const char *pName, 
   const ArgList &list
)
{
   MainThreadCheck;

   IMethodMap *pMethodMap;

   if ( m_MethodHash.Get(pName, &pMethodMap) )
   {
      pMethodMap->Method( pName, list );
   }

   if ( m_MethodHash.Get("", &pMethodMap) )
   {
      pMethodMap->Method( pName, list );
   }
}

void Channel::SetProperty( 
   const char *pName, 
   const ArgList &list
)
{
   MainThreadCheck;

   ISetPropertyMap *pPropertyMap;

   if ( m_SetHash.Get(pName, &pPropertyMap) )
   {
      pPropertyMap->Property( list );
   }
}

void Channel::GetProperty( 
   const char *pName, 
   ArgList &list
)
{
   MainThreadCheck;

   IGetPropertyMap *pPropertyMap;

   if ( m_GetHash.Get(pName, &pPropertyMap) )
   {
      pPropertyMap->Property( list );
   }
}

void Channel::SendEvent(
   const char *pEvent,
   const ArgList &list
)
{
   MainThreadCheck;

   uint32 i;
   EventList *pEventList;

   ++m_LockListeners;

   if ( m_EventListeners.Get(pEvent, &pEventList) )
   {
      for ( i = 0; i < pEventList->GetSize( ); i++ )
      {
         ValidEventMap *pValidMap = pEventList->GetAt( i );

         if ( pValidMap->isValid )
         {
            pValidMap->pEventMap->Event( this, list );
         }
      }
   }

   //propogate event down to all the
   //listeners of our base channels
   if ( m_pBaseChannel )
   {
      m_pBaseChannel->SendEvent( pEvent, list );
   }

   --m_LockListeners;

   if ( 0 == m_LockListeners )
   {
      ProcessPendingListeners( );
   }
}

void Channel::QueueEvent(
   const char *pEvent,
   const ArgList &list
)
{
   ScopeLock lock( m_Lock );

   EventList *pEventList;

   QueuedEvent *pQueuedEvent = m_EventPool.Alloc( );
   
   pEvent = StringRef(pEvent);

   if ( m_EventListeners.Get(pEvent, &pEventList) )
   {
      pQueuedEvent->pListeners = pEventList;
      
      Serializer serializer( pQueuedEvent->stream.GetOutputStream() );
      serializer.Serialize( &list );

      m_QueuedEvents.Add( pQueuedEvent );
   }

   //propogate event down to all the
   //listeners of our base channels
   if ( m_pBaseChannel )
   {
      m_pBaseChannel->QueueEvent( pEvent, list );
   }
}

void Channel::AddEventListener(
   const IEventMap &eventMap
)
{
   MainThreadCheck;

   ScopeLock lock( m_Lock );

   Debug::Assert( Condition(NULL == GetValidEventMap(eventMap)), "Event Listener is being added twice: %s", eventMap.GetName() );

   EventList *pEventList;
 
   IEventMap *pEventMap = eventMap.Clone( );
 
   if ( false == m_EventListeners.Get(pEventMap->GetName(), &pEventList) )
   {
      pEventList = new EventList;
      pEventList->Create( );

      m_EventListeners.Add( StringRef(pEventMap->GetName()), pEventList );
   }


   ValidEventMap *pValidMap = new ValidEventMap;
   pValidMap->pEventMap = pEventMap;
   pValidMap->isValid   = 1;

   pEventList->Add( pValidMap );
}

void Channel::RemoveEventListener(
   const IEventMap &eventMap
)
{
   MainThreadCheck;

   ScopeLock lock( m_Lock );

   ValidEventMap *pMap = GetValidEventMap( eventMap );
   
   if ( pMap )
   {
      pMap->isValid = 0;
      m_PendingListeners.Add( pMap );
   }
}

void Channel::AddMethod(
   const IMethodMap &methodMap
)
{
   MainThreadCheck;

   IMethodMap *pMap = methodMap.Clone( );

   Debug::Assert( Condition(false == m_GetHash.Contains(pMap->GetName())), "Method Map %s already exists", pMap->GetName() );

   m_MethodHash.Add( pMap->GetName( ), pMap );
}

void Channel::RemoveMethod(
   const IMethodMap &methodMap
)
{
   MainThreadCheck;

   IMethodMap *pMap;

   if ( m_MethodHash.Get(methodMap.GetName( ), &pMap) )
   {
      m_MethodHash.Remove( methodMap.GetName() );
      delete pMap;
   }
}

void Channel::AddGetProperty(
   const IGetPropertyMap &propertyMap
)
{
   MainThreadCheck;

   IGetPropertyMap *pMap = propertyMap.Clone( );

   Debug::Assert( Condition(false == m_GetHash.Contains(pMap->GetName())), "GetProperty Map %s already exists", pMap->GetName() );

   m_GetHash.Add( pMap->GetName( ), pMap );
}

void Channel::AddSetProperty(
   const ISetPropertyMap &propertyMap
)
{
   MainThreadCheck;

   ISetPropertyMap *pMap = propertyMap.Clone( );

   Debug::Assert( Condition(false == m_SetHash.Contains(pMap->GetName())), "SetProperty Map %s already exists", pMap->GetName() );

   m_SetHash.Add( pMap->GetName( ), pMap );
}

void Channel::RemoveGetProperty(
   const IGetPropertyMap &getPropertyMap
)
{
   MainThreadCheck;

   IGetPropertyMap *pMap;

   if ( m_GetHash.Get(getPropertyMap.GetName( ), &pMap) )
   {
      m_GetHash.Remove( getPropertyMap.GetName() );
      delete pMap;
   }
}

void Channel::RemoveSetProperty(
   const ISetPropertyMap &setPropertyMap
)
{
   MainThreadCheck;

   ISetPropertyMap *pMap;

   if ( m_SetHash.Get(setPropertyMap.GetName( ), &pMap) )
   {
      m_SetHash.Remove( setPropertyMap.GetName() );
      delete pMap;
   }
}

void Channel::ProcessPendingListeners( void )
{
   MainThreadCheck;

   ScopeLock lock( m_Lock );

   Debug::Assert( Condition(0 == m_LockListeners), "This can only be called when events are not processing" );

   int i, size = m_PendingListeners.GetSize( );

   for ( i = 0; i < size; i++ )
   {
      ValidEventMap *pMap = m_PendingListeners.GetAt( i );
      RemoveEventMap( pMap );
   }

   m_PendingListeners.Clear( );
}

void Channel::RemoveEventMap(
   ValidEventMap *pMap
)
{
   EventList *pEventList;

   if ( true == m_EventListeners.Get(pMap->pEventMap->GetName(), &pEventList) )
   {
      uint32 i;

      for ( i = 0; i < pEventList->GetSize( ); i++ )
      {
         ValidEventMap *pEventMap = pEventList->GetAt( i );
         
         //safe to do a pointer compare because this function
         //is only called internally from queued for delete event maps
         if ( pMap == pEventMap )
         {
            pEventList->RemoveAt( i );

            delete pEventMap->pEventMap;
            delete pEventMap;
            break;
         }
      }
   }
}

Channel::ValidEventMap *Channel::GetValidEventMap(
   const IEventMap &eventMap
)
{
   EventList *pEventList;

   if ( true == m_EventListeners.Get(eventMap.GetName(), &pEventList) )
   {
      uint32 i;

      for ( i = 0; i < pEventList->GetSize( ); i++ )
      {
         ValidEventMap *pEventMap = pEventList->GetAt( i );
         
         if ( pEventMap->isValid && eventMap == *pEventMap->pEventMap )
         {
            return pEventMap;
         }
      }
   }
   
   return NULL;
}


void TypeChannel::ExecuteMethod( 
   const char *pName, 
   const ArgList &list
)
{
   MainThreadCheck;
   ChannelSystem::Instance( ).ExecuteTypeMethod( pName, list, this );
}

void TypeChannel::SetProperty( 
   const char *pName, 
   const ArgList &list
)
{
   MainThreadCheck;
   ChannelSystem::Instance( ).SetTypeProperty( pName, list, this );
}
