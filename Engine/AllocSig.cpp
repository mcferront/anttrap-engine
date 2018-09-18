#include "EnginePch.h"  

#include "AllocSig.h"
#include "List.h"
#include "MemoryPool.h"
#include "MemoryManager.h"

#ifdef ENABLE_DEBUGMEMORY

AllocSig g_Invalid =
{
   0,
   "Invalid",
   AllocSigs::Magic,
   -1,
   0,
   NULL,
   NULL
};

void AllocSigs::Create( void )
{
   m_AllocNum  = 0;
   m_MyAlloc   = false;
   m_pSigs     = NULL;
   m_pHeadSig  = NULL;

   m_ActiveAllocations = 0;
}

void AllocSigs::Destroy( void )
{
   if ( m_pSigs )
   {
      m_pSigs->Destroy( );
      delete m_pSigs;
   }
}

AllocSig *AllocSigs::Alloc( 
   size_t size, 
   const char *pFile,
   int line
)
{
   ++m_AllocNum;

   if ( true == m_MyAlloc ) 
   {
      //g_Invalid is shared across all the
      //untracked allocations - so continue
      //to incrememnt it and we can see
      //how many total untracked allocations there are
      ++g_Invalid.allocNumber;
      return &g_Invalid;
   }

   //prevent recursive alloc calls in here
   //if we need to allocate memory ourselves
   m_MyAlloc = true;   

   if ( NULL == m_pSigs )
   {
      MemoryScope scope( "Debug" );

      m_pSigs = new MemoryPool<AllocSig>;
      m_pSigs->Create( 1024, true );
   }

   AllocSig *pSig = m_pSigs->Alloc( );
   Debug::Assert( Condition(NULL != pSig), "Memory tracking has reached it's limit, further allocations will not be tracked" );

   if ( NULL == pSig ) 
   {
      m_MyAlloc = false;
      return &g_Invalid;
   }

   pSig->pPrev = NULL;
   pSig->pNext = m_pHeadSig;
   
   if ( NULL != m_pHeadSig ) m_pHeadSig->pPrev = pSig;

   m_pHeadSig  = pSig;

   pSig->magic = AllocSigs::Magic;
   pSig->line  = line;
   pSig->size  = size;
   pSig->pFile = pFile;
   pSig->allocNumber = m_AllocNum;
   
   ++m_ActiveAllocations;

   m_MyAlloc = false;

   return pSig;
}

void AllocSigs::Free(
   AllocSig *pSig
)
{
   if ( pSig != &g_Invalid )
   {
      Debug::Assert( Condition(pSig->magic == AllocSigs::Magic), "Invalid alloc sig" );

      if ( pSig->pPrev )
      {
         pSig->pPrev->pNext = pSig->pNext;

         if ( pSig->pNext )
         {
            pSig->pNext->pPrev = pSig->pPrev;
         }
      }
      else
      {
         Debug::Assert( Condition(m_pHeadSig == pSig), "Head wasn't current Sig, code bug!" );
         m_pHeadSig = pSig->pNext;

         if ( m_pHeadSig ) m_pHeadSig->pPrev = NULL;
      }

      --m_ActiveAllocations;
      m_pSigs->Free( pSig );
   }
}

#endif