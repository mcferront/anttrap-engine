#include "EnginePch.h"  

#include "MemoryAllocation.h"
#include "MemoryManager.h"

int GetCurrentMemoryHeapId( void )
{
   return MemoryManager::Instance( ).GetCurrentHeap( )->GetId( );
}

void PushMemoryHeap( int heapId )
{
   MemoryManager::Instance( ).PushHeap( heapId );
}

void PopMemoryHeap( void )
{
   MemoryManager::Instance( ).PopHeap( );
}

#ifdef ENABLE_DEBUGMEMORY

void *CWPAlloc(size_t size)
{
   return CWPDebugAlloc( size, "Unknown", -1 );
}

void *CWPRealloc(void *pData, size_t size)
{
   return CWPDebugRealloc( pData, size, "Unknown", -1 );
}

void *CWPCalloc(size_t num, size_t size)
{
   return CWPDebugCalloc( num, size, "Unknown", -1 );
}

void CWPFree(void *pData)
{
   CWPDebugFree( pData, "Unknown", -1 );
}

void *CWPDebugAlloc(size_t size, const char *pFile, int line)
{
   return MemoryManager::Instance( ).Alloc( size, pFile, line );
}

void *CWPDebugRealloc(void *pData, size_t size, const char *pFile, int line)
{
   return MemoryManager::Instance( ).Realloc( pData, size, pFile, line );
}

void *CWPDebugCalloc(size_t num, size_t size, const char *pFile, int line)
{
   void *pData = MemoryManager::Instance( ).Alloc( num * size, pFile, line );
   memset( pData, 0, num * size );

   return pData;
}

void CWPDebugFree(void *pData, const char *pFile, int line)
{
   MemoryManager::Instance( ).Free( pData, pFile, line );
}

#else

void *CWPAlloc(size_t size)
{
   return MemoryManager::Instance( ).Alloc( size );
}

void *CWPRealloc(void *pData, size_t size)
{
   return MemoryManager::Instance( ).Realloc( pData, size );
}

void *CWPCalloc(size_t num, size_t size)
{
   void *pData = MemoryManager::Instance( ).Alloc( num * size );
   memset( pData, 0, num * size );

   return pData;
}

void CWPFree(void *pData)
{
   MemoryManager::Instance( ).Free( pData );
}

#endif

//if we do a custom allocator (which doesn't use malloc) then put this in - but
//right now we use malloc, etc. in our allocators
//#ifndef WIN32
//   //malloc/free can't be overloaded on windows unless we rebuild the crt library
//   //so for now we'll let any crt mallocs go untracked

//   //but other platforms/linkers will allow this so we assume it works
//   //for anything other than windows
//   #undef free
//   #undef malloc
//   #undef realloc
//   #undef calloc

//extern "C" 
//{
//   void *malloc(size_t size)
//   {
//      return CWPAlloc( size );
//   }

//   void free(void *pData)
//   {
//      return CWPFree( pData );
//   }

//   void *realloc(
//     void *pData,
//     size_t size
//   )
//   {
//      return CWPRealloc( pData, size );
//   }

//   void *calloc(
//     size_t num,
//     size_t size
//   )
//   {
//      return CWPCalloc( num, size );
//   }
//}
//#endif
