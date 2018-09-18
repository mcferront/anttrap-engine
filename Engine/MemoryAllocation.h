#pragma once

//methods for those core classes which can't include or directly reference the memory manager
//examples are List and MemoryPool
int GetCurrentMemoryHeapId( );
void PushMemoryHeap( int heapId );
void PopMemoryHeap( void );

extern "C"
{
   //wrap in C because we recompile some libs (like Ogg)
   //to call up into our functions
   void *CWPAlloc(size_t size);
   void *CWPRealloc(void *pData, size_t size);
   void *CWPCalloc(size_t num, size_t size);
   void  CWPFree(void *pData);
}

inline void *operator new(size_t size)
{
   return CWPAlloc( size );
}

inline void *operator new[](size_t size)
{
   return CWPAlloc( size );
}

inline void operator delete(void *pData)
{
   return CWPFree( pData );
}

inline void operator delete[](void *pData)
{
   return CWPFree( pData );
}


#ifdef ENABLE_DEBUGMEMORY

extern "C"
{
   //wrap in C because we recompile some libs (like Ogg)
   //to call up into our functions
   void *CWPDebugAlloc(size_t size, const char *pFile, int line);
   void *CWPDebugRealloc(void *pData, size_t size, const char *pFile, int line);
   void *CWPDebugCalloc(size_t num, size_t size, const char *pFile, int line);
   void  CWPDebugFree(void *pData, const char *pFile, int line);
}

inline void *operator new(size_t size, const char *pFile, int line)
{
   return CWPDebugAlloc( size, pFile, line );
}

inline void *operator new[](size_t size, const char *pFile, int line)
{
   return CWPDebugAlloc( size, pFile, line );
}

inline void operator delete(void *pData, const char *pFile, int line)
{
   return CWPDebugFree( pData, pFile, line );
}

inline void operator delete[](void *pData, const char *pFile, int line)
{
   return CWPDebugFree( pData, pFile, line );
}

#define REDEFINE_NEW                 new    (__FILE__, __LINE__)
#define REDEFINE_MALLOC(size)        CWPDebugAlloc( size, __FILE__, __LINE__ )
#define REDEFINE_REALLOC(data, size) CWPDebugRealloc( data, size, __FILE__, __LINE__ )
#define REDEFINE_CALLOC(num, size)   CWPDebugCalloc( num, size, __FILE__, __LINE__ )
#define REDEFINE_FREE(data)          CWPDebugFree ( data, __FILE__, __LINE__ )

#else

#define REDEFINE_NEW                 new
#define REDEFINE_MALLOC(size)        CWPAlloc( size )
#define REDEFINE_REALLOC(data, size) CWPRealloc( data, size )
#define REDEFINE_CALLOC(num, size)   CWPCalloc( num, size )
#define REDEFINE_FREE(data)          CWPFree ( data )

#endif

#define new          REDEFINE_NEW
#define malloc       REDEFINE_MALLOC
#define realloc      REDEFINE_REALLOC
#define calloc       REDEFINE_CALLOC
#define free         REDEFINE_FREE


