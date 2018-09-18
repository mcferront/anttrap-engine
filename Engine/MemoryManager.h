#pragma once

#include "EngineGlobal.h"
#include "ThreadLocks.h"
#include "MemoryHeap.h"
#include "ResourceMaps.h"

class MemoryManager
{
public:
   static MemoryManager &Instance( void );

private:
   struct ScopeStack
   {
      MemoryHeap *stack[ 128 ];
      int next;

      void Pop( void ) 
      { 
         Debug::Assert( Condition(next > 0), "ScopeStack is already 0" );      
         --next; 
      }

      void Push( MemoryHeap *pHeap ) 
      { 
         Debug::Assert( Condition(next < (sizeof(stack) / sizeof(MemoryHeap*))), "ScopeStack exceeded maximum value" );      
         stack[ next++ ] = pHeap; 
      }
      
      MemoryHeap *Top( void ) 
      { 
         return stack[ next - 1 ];
      }
   };

   struct ThreadScope
   {
      ThreadId   threadId;
      ScopeStack scopeStack;
   };

   CriticalSection m_ThreadScopeCS;

   ThreadScope  m_ThreadScopes[ 64 ];
   MemoryHeap  *m_pSystemHeap;
   MemoryHeap   m_Heaps[ 16 ];

   int  m_NumHeaps;
   int  m_NumThreads;
   bool m_Initialized;

public:
   MemoryManager( void );
   ~MemoryManager( void );

   void Destroy( void );

   void CreateHeap(
      const char *pName,
      int alignment,
      int size
   );

   MemoryHeap *GetHeap( 
      const char *pName 
   );

   MemoryHeap *PushHeap(
      const char *pName
   );

   MemoryHeap *PushHeap(
      int heapId
   );

   void PopHeap( void );

   MemoryHeap *GetHeap(
      const void *pMemory
   );

   MemoryHeap *GetCurrentHeap( void );

#ifdef ENABLE_DEBUGMEMORY
   void *Alloc  ( 
      size_t size, 
      const char *pFile, 
      int line 
   );
   
   void *Realloc( 
      void *pMemory, 
      size_t size, 
      const char *pFile, 
      int line
   );

   void Free( 
      void *pMemory, 
      const char *pFile, 
      int line
   );
#else
   void *Alloc( 
      size_t size
   );
   
   void *Realloc( 
      void *pMemory, 
      size_t size 
   );

   void Free( 
      void *pMemory
   );
#endif

   void RegisterRemote  ( void );
   void UnregisterRemote( void );

private:
   ScopeStack *GetThreadScope( void );

   void OnGetStatistics(
      const char *pName,
      const ArgList &list
   );
};

class MemoryScope
{
public:
   MemoryScope(
      const char *pHeapName
   )
   {
      MemoryManager::Instance( ).PushHeap( pHeapName );
   }

   MemoryScope(
      int heapId
   )
   {
      MemoryManager::Instance( ).PushHeap( heapId );
   }

   ~MemoryScope( void )
   {
      MemoryManager::Instance( ).PopHeap( );
   }
};
