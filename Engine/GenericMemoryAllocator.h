#include "EngineGlobal.h"

#define GENERIC_MEMORYALLOCATOR

class MemoryAllocator
{
private:
   int m_Alignment;

public:
   void Create ( 
      size_t size,
      int alignment
   );
   void Destroy( void );

   void *Alloc( 
      size_t size
   );
   
   void *Realloc( 
      void *pMemory, 
      size_t size 
   );
   void  Free( 
      void *pMemory
   );
};
