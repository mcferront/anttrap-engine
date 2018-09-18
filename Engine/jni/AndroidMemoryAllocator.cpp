#include "EnginePch.h"

#include "MemoryAllocator.h"

#define Align(value, alignment) (((value) + ((alignment) - 1)) / (alignment) * (alignment))

void GetSystemUsage( 
   uint32 *pWorkingSet, 
   uint64 *pAllAvail 
)
{
   *pWorkingSet = 0;
   *pAllAvail   = 0;
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
   return HeapAlloc( m_Heap, HEAP_NO_SERIALIZE, size );
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
