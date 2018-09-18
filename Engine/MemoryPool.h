#pragma once

#include "EngineGlobal.h"

template <typename t_item>
class MemoryPool
{
public:
   struct Block
   {
      t_item *pItems;
   };

protected:
   size_t   m_ItemsInBlock;
   size_t   m_NumBlocks;
   size_t   m_NextFreeItem;
   Block   *m_pBlocks;
   t_item **m_pItemPointers;
   int      m_HeapId;
   bool     m_CanGrow;

public:
   MemoryPool( void );
   ~MemoryPool( void );

   void Create(
      size_t numItems = 16,
      bool canGrow = true
   );

   void Destroy( void );

   void Reset( void );

   t_item *Alloc( void );

   void Free( t_item *pItem );

   size_t GetNumSlotsUsed( void ) const;
};

#include "MemoryPool.inl"
