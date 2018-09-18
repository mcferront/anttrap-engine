#pragma once

#include "EngineGlobal.h"
#include "AllocSig.h"
#include "ThreadLocks.h"
#include "List.h"
#include "MemoryAllocator.h"

class MemoryHeap
{
private:
   struct Header
   {
   #ifdef ENABLE_DEBUGMEMORY
      AllocSig *pAllocSig;
      nuint  pad0;
      nuint  pad1;
      nuint  pad2;
      nuint  pad3;
   #else
      nuint  pad0;
      nuint  pad1;
      nuint  pad2;
      nuint  pad3;
      nuint  pad4;
   #endif

      nuint  size;
      nuint  baseAddress;
      nuint  heapId;
   };

private:
   MemoryAllocator m_Allocator;

   int       m_Id;
   size_t    m_OriginalSize;
   Lock      m_Lock;

#ifdef ENABLE_DEBUGMEMORY
   AllocSigs m_AllocSigs;
#endif

   char m_Name[ MaxNameLength ];

public:
   void Create( 
      const char *pName,
      int id,
      int alignment,
      size_t numBytes 
   );

   void Destroy( void );

   bool Contains(
      const void *pMemory
   );

   inline const char *GetName( void ) const { return m_Name; }
   inline int GetId( void ) const { return m_Id; }

#ifdef ENABLE_DEBUGMEMORY
   void *Alloc  ( 
      size_t size, 
      const char *pFile, 
      int line 
   );
   
   void *Realloc( 
      void *pMemory, 
      size_t size, 
      const char *pFile, 
      int line
   );

   void Free( 
      void *pMemory,
      const char *pFile, 
      int line
   );

   void GetAllocSigs(
      List<AllocSig> *pList
   );
#else
   void *Alloc( 
      size_t size
   );
   
   void *Realloc( 
      void *pMemory, 
      size_t size 
   );

   void Free( 
      void *pMemory
   );
#endif
};
