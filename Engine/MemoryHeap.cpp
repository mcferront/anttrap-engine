#include "EnginePch.h"

#include "MemoryHeap.h"
#include "MemoryPool.h"
#include "RegistryAsset.h"

#define GetHeader(address) ((Header *)(((char*)(address)) - sizeof(Header)))

#ifdef ENABLE_DEBUGMEMORY
   #ifdef _DEBUG
      #define DEBUGMEMORY_CLEAR
   #endif
#endif

void MemoryHeap::Create( 
   const char *pName,
   int id,
   int alignment,
   size_t numBytes 
)
{
   m_Id  = id;

   m_OriginalSize = numBytes;

   String::Copy( m_Name, pName, sizeof(m_Name) );

   m_Allocator.Create( m_OriginalSize, alignment );

#ifdef ENABLE_DEBUGMEMORY
   m_AllocSigs.Create( );
#endif
}

void MemoryHeap::Destroy( void )
{

#ifdef ENABLE_DEBUGMEMORY
   {
      Debug::Print( Debug::TypeInfo, "Heap %s Destroying\n", m_Name );
      
      const AllocSig *pHeadSig = m_AllocSigs.GetHeadActiveAllocation( );

      while ( pHeadSig )
      {
         Debug::Print( Debug::TypeInfo, "%s (%d): %d bytes still allocated\n", pHeadSig->pFile, pHeadSig->line, pHeadSig->size );
         pHeadSig = pHeadSig->pNext;
      }
   }

   m_AllocSigs.Destroy( );
#endif

   m_Allocator.Destroy( );
}

bool MemoryHeap::Contains(
   const void *pMemory
)
{
   Header *pHeader = (Header *) ((char *)pMemory - sizeof(Header));
   return pHeader->heapId == m_Id;   
}

#ifdef ENABLE_DEBUGMEMORY
void *MemoryHeap::Alloc( 
   size_t size, 
   const char *pFile, 
   int line
)
{
   size_t fullSize = size + sizeof(Header);

   char *pHeapAddress;
   AllocSig *pSig;

   {
      ScopeLock lock( m_Lock );  
      pHeapAddress = (char *) m_Allocator.Alloc( fullSize );
      Debug::Assert( Condition(NULL != pHeapAddress), "Heap % is out of memory: requested: %u", m_Name, fullSize );

      pSig = m_AllocSigs.Alloc( size, pFile, line );
   }

   char *pMemory = pHeapAddress + sizeof(Header);   
   
   Header *pHeader = GetHeader(pMemory);
   
   pHeader->pAllocSig   = pSig;
   pHeader->size        = (nuint) size;
   pHeader->baseAddress = (nuint) pHeapAddress;
   pHeader->heapId      = m_Id;
   pHeader->pad0        = 0;

#ifdef DEBUGMEMORY_CLEAR
   memset( pMemory, 0, size );
#endif
   return pMemory;
}

void *MemoryHeap::Realloc( 
   void *pMemory, 
   size_t size, 
   const char *pFile, 
   int line
)
{
   if ( NULL == pMemory )
   {
      return Alloc( size, pFile, line );
   }

   Header *pHeader = GetHeader(pMemory);

#ifdef DEBUGMEMORY_CLEAR
   size_t oldSize  = pHeader->size;
#endif
   size_t fullSize = size + sizeof(Header);

   char *pHeapAddress;
   AllocSig *pSig;

   {
      ScopeLock lock( m_Lock );  

      m_AllocSigs.Free( pHeader->pAllocSig );

      pHeapAddress = (char *) m_Allocator.Realloc( (void *) pHeader->baseAddress, fullSize );
      Debug::Assert( Condition(NULL != pHeapAddress), "Heap % is out of memory: requested: %u", m_Name, fullSize );

      pSig = m_AllocSigs.Alloc( size, pFile, line );
   }


   pMemory = pHeapAddress + sizeof(Header);
   pHeader = GetHeader(pMemory);

   pHeader->pAllocSig   = pSig;
   pHeader->size        = (nuint) size;
   pHeader->baseAddress = (nuint) pHeapAddress;
   pHeader->heapId      = m_Id;
   pHeader->pad0        = 0;

#ifdef DEBUGMEMORY_CLEAR
   if ( size > oldSize )
   {
      memset( ((char *)pMemory) + oldSize, 0, size - oldSize );
   }
#endif

   return pMemory;
}

void MemoryHeap::Free( 
   void *pMemory,
   const char *pFile,
   int line
)
{
   if ( NULL == pMemory ) return;

   Header *pHeader = GetHeader(pMemory);

   ScopeLock lock( m_Lock );

   m_AllocSigs.Free( pHeader->pAllocSig );

   m_Allocator.Free( (void *) pHeader->baseAddress );
}

void MemoryHeap::GetAllocSigs(
   List<AllocSig> *pList
)
{
   ScopeLock lock( m_Lock );

   //size the list ahead of time so it doesn't try and allocate
   //memory out of this heap as we're adding too it
   uint32 slotsUsed = m_AllocSigs.GetNumActiveAllocations( );
   pList->GrowTo( slotsUsed + 1 );

   const AllocSig *pHeadSig = m_AllocSigs.GetHeadActiveAllocation( );

   uint32 debugCount = 0;

   while ( pHeadSig )
   {
      pList->Add( *pHeadSig );
      pHeadSig = pHeadSig->pNext;
   
      ++debugCount;
   }

   Debug::Assert( Condition(debugCount == slotsUsed), "Reported active allocations did not equal real active allocations" );
}
#else
void *MemoryHeap::Alloc(
   size_t size 
)
{
   size_t fullSize = size + sizeof(Header);

   char *pHeapAddress;
   
   {
      ScopeLock lock( m_Lock );
      pHeapAddress = (char *) m_Allocator.Alloc( fullSize );
      Debug::Assert( Condition(NULL != pHeapAddress), "Heap % is out of memory: requested: %u", m_Name, fullSize );
   }

   char *pMemory = pHeapAddress + sizeof(Header);   
   
   Header *pHeader = GetHeader(pMemory);
   
   pHeader->baseAddress = (nuint) pHeapAddress;
   pHeader->heapId      = m_Id;
   pHeader->size        = size;

   return pMemory;
}

void *MemoryHeap::Realloc(
   void *pMemory, 
   size_t size
   )
{
   if ( NULL == pMemory )
   {
      return Alloc( size );
   }

   Header *pHeader = GetHeader(pMemory);
   
   size_t fullSize = size + sizeof(Header);

   char *pHeapAddress;

   {
      ScopeLock lock( m_Lock );
      pHeapAddress = (char *) m_Allocator.Realloc( (void *) pHeader->baseAddress, fullSize );
      Debug::Assert( Condition(NULL != pHeapAddress), "Heap % is out of memory: requested: %u", m_Name, fullSize );
   }

   pMemory = pHeapAddress + sizeof(Header);      
   pHeader = GetHeader(pMemory);

   pHeader->baseAddress = (nuint) pHeapAddress;
   pHeader->heapId      = m_Id;
   pHeader->size        = size;

   return pMemory;
}

void MemoryHeap::Free( 
   void *pMemory
)
{
   if ( NULL == pMemory ) return;

   Header *pHeader = GetHeader(pMemory);

   ScopeLock lock( m_Lock );
   m_Allocator.Free( (void *) pHeader->baseAddress );
}
#endif
