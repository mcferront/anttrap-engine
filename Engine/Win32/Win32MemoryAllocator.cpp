#include "EnginePch.h"

#include "MemoryAllocator.h"


void GetSystemUsage( 
   uint32 *pWorkingSet, 
   uint64 *pAllAvail 
)
{
   PROCESS_MEMORY_COUNTERS_EX pmc = { sizeof(PROCESS_MEMORY_COUNTERS_EX) };

   if ( GetProcessMemoryInfo( GetCurrentProcess( ), (PROCESS_MEMORY_COUNTERS*) &pmc, sizeof(pmc)) )
   {     
      MEMORYSTATUSEX mst = { sizeof(MEMORYSTATUSEX ) };

      GlobalMemoryStatusEx( &mst );

      //this gives us the equivilant of task manager's Memory Working Set - not Private Working Set
      //private working set would be just the memory we use, whereas Working Set (what we are reporting)
      //gives us the total memory (including shared dlls, etc.) used by us.
      //to get the private working set size you have to loop throgh all the pages
      //the code reference is here if it needs to be done: http://www.codeproject.com/KB/cpp/XPWSPrivate.aspx
      *pWorkingSet = pmc.WorkingSetSize;
      *pAllAvail   = mst.ullAvailVirtual;
   }
}
   
#ifndef GENERIC_MEMORYALLOCATOR
void MemoryAllocator::Create(
   size_t size,
   int alignment
)
{
   Debug::Assert( Condition(alignment == 0), "Currently no alignment is allowed for windows heap" );

   m_Alignment = alignment;

   m_Heap = HeapCreate( HEAP_NO_SERIALIZE, size, 0 );
}

void MemoryAllocator::Destroy( void )
{
   HeapDestroy( m_Heap );
}

void *MemoryAllocator::Alloc( 
   size_t size
)
{
#ifdef ENABLE_DEBUGMEMORY
   return HeapAlloc( m_Heap, HEAP_GENERATE_EXCEPTIONS | HEAP_NO_SERIALIZE, size );
#else
   return HeapAlloc( m_Heap, HEAP_NO_SERIALIZE, size );
#endif
}   

void *MemoryAllocator::Realloc( 
   void *pMemory, 
   size_t size 
)
{
   if ( NULL == pMemory ) return Alloc( size );

   return HeapReAlloc( m_Heap, HEAP_NO_SERIALIZE, pMemory, size );
}

void  MemoryAllocator::Free( 
   void *pMemory
)
{
   if ( NULL == pMemory ) return;

   HeapFree( m_Heap, HEAP_NO_SERIALIZE, pMemory );
}
#endif //#ifndef GENERIC_MEMORYALLOCATOR
