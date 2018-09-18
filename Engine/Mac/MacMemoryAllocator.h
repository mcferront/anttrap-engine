#include "EngineGlobal.h"

class MemoryAllocator
{
private:
   HANDLE m_Heap;
   int    m_Alignment;

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