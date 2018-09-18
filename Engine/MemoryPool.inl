                                                      
template <typename t_item>
MemoryPool<t_item>::MemoryPool( void )
{
   m_HeapId = GetCurrentMemoryHeapId( );
   m_pItemPointers = NULL;
}

template <typename t_item>
MemoryPool<t_item>::~MemoryPool( void )
{
   Debug::Assert( Condition(NULL == m_pItemPointers), "MemoryPool was not property destroyed" );   
}

template <typename t_item>
void MemoryPool<t_item>::Create( 
   size_t numItems, // = 16
   bool canGrow // = true
)
{
   Debug::Assert( Condition(NULL == m_pItemPointers), "MemoryPool was already created" );

   size_t i;

   m_CanGrow      = canGrow;
   m_NumBlocks    = 1;
   m_NextFreeItem = 0;
   m_ItemsInBlock = numItems;

   PushMemoryHeap( m_HeapId );

      m_pBlocks = (Block *) malloc( sizeof(Block) * 1 );

      //block allocate pointers to the items
      m_pBlocks[ 0 ].pItems = new t_item[ m_ItemsInBlock * m_NumBlocks ];

      //assign pointers to these items for quick alloc and free reference
      m_pItemPointers = (t_item **) malloc( sizeof(t_item*) * m_ItemsInBlock * m_NumBlocks );
   
      for ( i = 0; i < m_ItemsInBlock * m_NumBlocks; i++ )
      {
         m_pItemPointers[ i ] = &m_pBlocks[ 0 ].pItems[ i ];
      }

   PopMemoryHeap( );
}

template <typename t_item>
void MemoryPool<t_item>::Destroy( void )
{
   Debug::Assert( Condition(NULL != m_pItemPointers), "MemoryPool has not been created" );

   size_t i;

   for ( i = 0; i < m_NumBlocks; i++ )
   {
      delete [] m_pBlocks[ i ].pItems;
   }

   free( m_pBlocks );
   free( m_pItemPointers );

   m_pItemPointers = NULL;
}

template <typename t_item>
void MemoryPool<t_item>::Reset( void )
{
   Debug::Assert( Condition(NULL != m_pItemPointers), "MemoryPool has not been created" );

   size_t i, c, index = 0;
   
   m_NextFreeItem = 0;

   for ( i = 0; i < m_NumBlocks; i++ )
   {
      for ( c = 0; c < m_ItemsInBlock; c++ )
      {
         m_pItemPointers[ index++ ] = &m_pBlocks[ i ].pItems[ c ];
      }
   }
}

template <typename t_item>
t_item *MemoryPool<t_item>::Alloc( void )
{
   Debug::Assert( Condition(NULL != m_pItemPointers), "MemoryPool has not been created" );

   //grow if there are no more free items
   if ( m_NextFreeItem == m_ItemsInBlock * m_NumBlocks )
   {
      if ( false == m_CanGrow ) return NULL;

      PushMemoryHeap( m_HeapId );

         size_t i;
         size_t blockIndex = m_NumBlocks;
         size_t newBlockSize = m_NumBlocks + 1;

         //realloc the blocks which point to lists of items
         m_pBlocks = (Block *) realloc( m_pBlocks, sizeof(Block) * newBlockSize );
         
         //alloc the list of items for our new block
         m_pBlocks[ blockIndex ].pItems = new t_item[ m_ItemsInBlock ];

         //realloc the pointers which point to all the items across the blocks
         m_pItemPointers = (t_item **) realloc( m_pItemPointers, sizeof(t_item*) * m_ItemsInBlock * newBlockSize );

         //point the new alloc'd data to the new items in the blocksd
         size_t start = m_ItemsInBlock * blockIndex;
         size_t end   = start + m_ItemsInBlock;
         size_t itemIndex = 0;

         for ( i = start; i < end; i++ )
         {
            m_pItemPointers[ i ] = &m_pBlocks[ blockIndex ].pItems[ itemIndex++ ];
         }

         ++m_NumBlocks;

      PopMemoryHeap( );
   }

   return m_pItemPointers[ m_NextFreeItem++ ];
}

template <typename t_item>
void MemoryPool<t_item>::Free(
   t_item *pData
)
{
   Debug::Assert( Condition(NULL != m_pItemPointers), "MemoryPool has not been created" );

   m_pItemPointers[ --m_NextFreeItem ] = pData;
}

template <typename t_item>
size_t MemoryPool<t_item>::GetNumSlotsUsed( void ) const
{
   return m_NextFreeItem;
}
