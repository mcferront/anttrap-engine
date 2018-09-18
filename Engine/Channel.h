#pragma once

#include "EngineGlobal.h"
#include "HashTable.h"
#include "ResourceMaps.h"
#include "MemoryPool.h"
#include "Threads.h"
#include "Identifiable.h"
#include "MemoryStreams.h"

class Channel : public Identifiable
{
private:
   struct ValidEventMap
   {
      IEventMap *pEventMap;
      int isValid;
   };

private:    
   typedef List<IMethodMap*>     MethodList;
   typedef List<IGetPropertyMap*>GetPropertyList;
   typedef List<ISetPropertyMap*>SetPropertyList;
   typedef List<ValidEventMap*>  EventList;

private:
   struct QueuedEvent
   {
      EventList *pListeners;
      MemoryStream stream;
   };

   typedef List<QueuedEvent*> QueuedEvents;

   class QueuedEventList
   {
   private:
      QueuedEvents m_QueuedEvents[ 2 ];
      uint32 m_QueueIndex;

   public:
      QueuedEventList( void )
      {
         m_QueueIndex = 0;
      }
      
      void Create( void )
      {
         m_QueuedEvents[ 0 ].Create( );
         m_QueuedEvents[ 1 ].Create( );
      }

      void Destroy( void )
      {
         m_QueuedEvents[ 1 ].Destroy( );
         m_QueuedEvents[ 0 ].Destroy( );
      }

      const QueuedEvents *Swap( void )
      {
         uint32 index = m_QueueIndex;
         m_QueueIndex ^= 1;

         m_QueuedEvents[ m_QueueIndex ].Clear( );

         return &m_QueuedEvents[ index ];
      }

      void Add(
         QueuedEvent *pQueuedEvent
      )
      {
         m_QueuedEvents[ m_QueueIndex ].Add( pQueuedEvent );
      }
   };

private:
   Lock m_Lock;

   HashTable<const char *, IMethodMap *>         m_MethodHash;
   HashTable<const char *, IGetPropertyMap *>    m_GetHash;
   HashTable<const char *, ISetPropertyMap *>    m_SetHash;
   HashTable<const char *, EventList *>          m_EventListeners;

   List<ValidEventMap*>    m_PendingListeners;
   MemoryPool<QueuedEvent> m_EventPool;
   QueuedEventList         m_QueuedEvents;

   Channel    *m_pBaseChannel;

   int m_LockListeners;

public:
   Channel( void ) {}
   virtual ~Channel( void ) {}

   void Create(
      Id id,
      Channel *pBaseChannel   
   );

   void CopyMethods(
      Channel *pCopyFrom
   );

   void Destroy( void );

   void Flush( void );

   bool HasEventListeners( void );

   Channel *GetBaseChannel( void ) const { return m_pBaseChannel; }

   virtual void ExecuteMethod(
      const char *pMethod, 
      const ArgList &list
   );

   virtual void SetProperty( 
      const char *pProperty, 
      const ArgList &list
   );
   
   virtual void GetProperty( 
      const char *pProperty, 
      ArgList &list
   );

   virtual void SendEvent(
      const char *pEvent,
      const ArgList &list
   );
  
   virtual void QueueEvent(
      const char *pEvent,
      const ArgList &list
   );

   virtual void AddEventListener(
      const IEventMap &eventMap
   );

   virtual void RemoveEventListener(
      const IEventMap &eventMap
   );

   virtual void AddMethod(
      const IMethodMap &methodMap
   );

   virtual void RemoveMethod(
      const IMethodMap &methodMap
   );

   virtual void AddGetProperty(
      const IGetPropertyMap &propertyMap
   );

   virtual void AddSetProperty(
      const ISetPropertyMap &propertyMap
   );

   virtual void RemoveGetProperty(
      const IGetPropertyMap &propertyMap
   );

   virtual void RemoveSetProperty(
      const ISetPropertyMap &propertyMap
   );

private:
   void ProcessPendingListeners( void );

   void RemoveEventMap(
      ValidEventMap *pMap
   );

   ValidEventMap *GetValidEventMap(
      const IEventMap &eventMap
   );
};


class TypeChannel : public Channel
{
public:
   virtual void ExecuteMethod(
      const char *pMethod, 
      const ArgList &list
   );

   virtual void SetProperty( 
      const char *pProperty, 
      const ArgList &list
   );
};