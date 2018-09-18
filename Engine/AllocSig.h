#pragma once

#ifdef ENABLE_DEBUGMEMORY

#include "EngineGlobal.h"

//alloc sigs currently don't create alloc sigs for themselves
//so the alloc sig pooled memory won't show up in the profiling

struct AllocSig
{
   size_t size;
   const char *pFile;
   uint32 magic;
   int line;
   uint32 allocNumber;
   AllocSig *pNext;
   AllocSig *pPrev;
};

template<class t_item> class MemoryPool;

class AllocSigs
{
public:
   static const uint32 Magic = 0x12345678;

private:
   MemoryPool<AllocSig>        *m_pSigs;
   AllocSig *m_pHeadSig;

   uint32 m_ActiveAllocations;
   uint32 m_AllocNum;
   bool   m_MyAlloc;

public:
   void Create( void );
   void Destroy( void );

   AllocSig *Alloc( 
      size_t size, 
      const char *pFile, 
      int line
   );

   void Free(
      AllocSig *pAllocSig
   );

   const uint32    GetNumActiveAllocations( void ) const { return m_ActiveAllocations; }
   const AllocSig *GetHeadActiveAllocation( void ) const { return m_pHeadSig; }
};

#endif