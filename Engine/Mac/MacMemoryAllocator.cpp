#include "EnginePch.h"

#include "MemoryAllocator.h"

#define Align(value, alignment) (((value) + ((alignment) - 1)) / (alignment) * (alignment))

void GetSystemUsage( 
   uint32 *pWorkingSet, 
   uint64 *pAllAvail 
)
{
   //mach maps virtual pages to physical pages
   mach_port_t host_port = mach_host_self();
   
   mach_msg_type_number_t host_size = sizeof(vm_statistics_data_t) / sizeof(integer_t);
   vm_size_t pagesize;
   vm_statistics_data_t vm_stat;
      
   //how big is the page size this mach maps
   host_page_size(host_port, &pagesize);
      
   //get virtual memory statistics for our host
   if (host_statistics(host_port, HOST_VM_INFO, (host_info_t)&vm_stat, &host_size) == KERN_SUCCESS)
   {
      //pages in use * the size of each page
      *pWorkingSet = (vm_stat.active_count + vm_stat.inactive_count + vm_stat.wire_count) * pagesize;
      //pages available to be allocated from
      *pAllAvail   = (*pWorkingSet) + vm_stat.free_count * pagesize;
   }
   else
   {
      *pWorkingSet = 0;
      *pAllAvail   = 0;
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
