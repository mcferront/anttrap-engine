#include "EnginePch.h"

#include "MemoryAllocator.h"

#ifdef GENERIC_MEMORYALLOCATOR

#undef malloc
#undef realloc
#undef free

void MemoryAllocator::Create(
   size_t size,
   int alignment
)
{
   Debug::Assert( Condition(alignment == 0), "Currently no alignment is allowed for windows heap" );

   m_Alignment = alignment;
}

void MemoryAllocator::Destroy( void )
{
}

void *MemoryAllocator::Alloc( 
   size_t size
)
{
   return malloc( size );
}

void *MemoryAllocator::Realloc( 
   void *pMemory, 
   size_t size 
)
{
   if ( NULL == pMemory ) return Alloc( size );

   return realloc( pMemory, size );
}

void  MemoryAllocator::Free( 
   void *pMemory
)
{
   free( pMemory );

}
#endif