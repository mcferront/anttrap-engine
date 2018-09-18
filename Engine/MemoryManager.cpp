#include "EnginePch.h"

#include "MemoryManager.h"
#include "Threads.h"
#include "AllocSig.h"
#include "RegistryAsset.h"

//too be defined in platform specific code
void GetSystemUsage( 
   uint32 *pWorkingSet, 
   uint64 *pAllAvail 
);

MemoryManager &MemoryManager::Instance( void )
{
   static MemoryManager s_instance;
   return s_instance;
}

MemoryManager::MemoryManager( void ) :
   m_ThreadScopeCS( 500 )
{
   m_NumHeaps = 0;

#ifdef ENABLE_DEBUGMEMORY
   m_Heaps[ m_NumHeaps ].Create( "Debug", m_NumHeaps, 0, 1024 );
   ++m_NumHeaps;
#endif

   m_Heaps[ m_NumHeaps ].Create( "System", m_NumHeaps, 0, 1024 );
   m_pSystemHeap = &m_Heaps[ m_NumHeaps ];
   ++m_NumHeaps;
   
   m_NumThreads  = 0;
   m_Initialized = true;
}

MemoryManager::~MemoryManager( void )
{
   int i;

   for ( i = m_NumHeaps - 1; i >= 2; i-- )
      m_Heaps[ i ].Destroy( );
}

void MemoryManager::Destroy( void )
{
   m_Initialized = false;

   int i;

   for ( i = m_NumHeaps - 1; i >= 2; i-- )
   {
      m_Heaps[ i ].Destroy( );
   }

   m_NumHeaps = 2;
}

void MemoryManager::CreateHeap(
   const char *pName,
   int alignment,
   int size
)
{
   m_Heaps[ m_NumHeaps ].Create( pName, m_NumHeaps, alignment, size );

   ++m_NumHeaps;
}

MemoryHeap *MemoryManager::GetHeap( 
   const char *pName 
)
{
   int i;

   for ( i = 0; i < m_NumHeaps; i++ )
   {
      if ( 0 == strcmp(pName, m_Heaps[i].GetName()) )
      {
         return &m_Heaps[i];
      }
   }

   Debug::Assert( Condition(false), "Heap %s was not found", pName );
   return m_pSystemHeap;
}


MemoryHeap *MemoryManager::PushHeap(
   const char *pName
)
{
   return PushHeap( GetHeap(pName)->GetId() );
}
   
MemoryHeap *MemoryManager::PushHeap(
   int heapId
)
{
   ScopeStack *pScopeStack = GetThreadScope( );

   MemoryHeap *pHeap = &m_Heaps[ heapId ];
   pScopeStack->Push( pHeap );

   return pHeap;
}

void MemoryManager::PopHeap( void )
{
   ScopeStack *pScopeStack = GetThreadScope( );
   pScopeStack->Pop( );
}

MemoryHeap *MemoryManager::GetCurrentHeap( void )
{
   return GetThreadScope( )->Top( );
}

MemoryHeap *MemoryManager::GetHeap(
   const void *pMemory
)
{
   int i;

   for ( i = 0; i < m_NumHeaps; i++ )
   {
      if ( m_Heaps[i].Contains(pMemory) )
      {
         return &m_Heaps[i];
      }
   }

   return NULL;
}

#ifdef ENABLE_DEBUGMEMORY
void *MemoryManager::Alloc(
   size_t size, 
   const char *pFile, 
   int line
)
{
   return GetThreadScope( )->Top( )->Alloc( (uint32) size, pFile, line );
}

void *MemoryManager::Realloc(
   void *pMemory, 
   size_t size, 
   const char *pFile, 
   int line
)
{
   if ( NULL == pMemory )
   {
      return GetThreadScope( )->Top( )->Realloc( pMemory, size, pFile, line );
   }

   MemoryHeap *pHeap = GetHeap( pMemory );
   return pHeap->Realloc( pMemory, size, pFile, line );
}


void MemoryManager::Free( 
   void *pMemory, 
   const char *pFile, 
   int line
)
{
   if ( NULL == pMemory ) return;

   MemoryHeap *pHeap = GetHeap( pMemory );
   return pHeap->Free( pMemory, pFile, line );
}

#else
void *MemoryManager::Alloc(
   size_t size
)  
{
   return GetThreadScope( )->Top( )->Alloc( size );
}

void *MemoryManager::Realloc(
   void *pMemory, 
   size_t size
)
{
   if ( NULL == pMemory )
   {
      return GetThreadScope( )->Top( )->Realloc( pMemory, size );
   }
   
   MemoryHeap *pHeap = GetHeap( pMemory );
   return pHeap->Realloc( pMemory, size );
}

void MemoryManager::Free( 
   void *pMemory
)
{
   if ( NULL == pMemory ) return;

   MemoryHeap *pHeap = GetHeap( pMemory );
   return pHeap->Free( pMemory );
}
#endif

void MemoryManager::RegisterRemote( void )
{
   //GetChannel( )->AddMethod( MethodMap<MemoryManager>("GetStatistics", this, &MemoryManager::OnGetStatistics) );

   //tell the tools about us so they can call us and ask for statistics
   //ResourceHandle toolsSystem( "ToolsHub" );
   //GetResource( toolsSystem, Resource )->GetChannel( )->ExecuteMethod( "AddProxy", ArgList(GetId()) );
}

void MemoryManager::UnregisterRemote( void )
{
   //GetChannel( )->RemoveMethod( MethodMap<MemoryManager>("GetStatistics", this, &MemoryManager::OnGetStatistics) );
}

MemoryManager::ScopeStack *MemoryManager::GetThreadScope( void )
{
   //lock here so threadA isn't getting a scope while
   //threadb might be realloc'ing the threadscopes array
   CriticalSectionScope cs( m_ThreadScopeCS );

   int i;
   
   ThreadId id = Thread::GetCurrentThreadId( );

   for ( i = 0; i < m_NumThreads; i++ )
   {
      if ( id == m_ThreadScopes[ i ].threadId )
      {
         break;
      }
   }

   if ( i == m_NumThreads )
   {
      Debug::Assert( Condition(i < sizeof(m_ThreadScopes) / sizeof(ThreadScope)), "Too Many Threads for MemoryManager" );

      ++m_NumThreads;

      m_ThreadScopes[ i ].threadId        = id;
      m_ThreadScopes[ i ].scopeStack.next = 0;
      m_ThreadScopes[ i ].scopeStack.Push( m_pSystemHeap );
   }

   return &m_ThreadScopes[ i ].scopeStack;
}

void MemoryManager::OnGetStatistics(
   const char *pName,
   const ArgList &list
)
{
#ifdef ENABLE_DEBUGMEMORY
   int i;   

   MemoryScope memoryScope( "Debug" );

   List<AllocSig> allSigs;
   allSigs.Create( );

   //serialize the registry class and send it to the calling channel
   uint32 count = 0;

   MemoryStream stream;
   stream.Write( &count, sizeof(count) );

   char key[ 256 ];

   uint32 workingSet;
   uint64 allAvail;

   GetSystemUsage( &workingSet, &allAvail );

   stream.Write( &workingSet, sizeof(workingSet) );
   stream.Write( &allAvail,   sizeof(allAvail) );

   for ( i = 0; i < m_NumHeaps; i++ )
   {
      MemoryHeap *pHeap = &m_Heaps[ i ];
      pHeap->GetAllocSigs( &allSigs );

      uint32 c;
      
      for ( c = 0; c < allSigs.GetSize( ); c++ )
      {
         AllocSig *pSig = allSigs.GetPointer( c );
         String::Format( key, sizeof(key), "%s,%s,%d,%d,%d", pHeap->GetName( ), pSig->pFile, pSig->line, pSig->allocNumber, pSig->size );

         int length = strlen(key);
         stream.Write( &length, sizeof(length) );
         stream.Write( key, length );
      }

      count += allSigs.GetSize( );

      allSigs.Clear( );
   }


   allSigs.Destroy( );

   stream.Seek( 0, SeekBegin );
   stream.Write( &count, sizeof(count) );
   
   stream.Seek( 0, SeekEnd );

   MemoryStream inputStream( stream.GetBuffer( ), stream.GetAmountWritten( ), false );

   Id returnChannel;
   char *pReturnMethod;

   list.GetArg( 0, &returnChannel );
   list.GetArg( 1, &pReturnMethod );

   ResourceHandle returnHandle( returnChannel );

   returnHandle.GetChannel( )->ExecuteMethod( pReturnMethod, ArgList(&inputStream) );
#endif
}
