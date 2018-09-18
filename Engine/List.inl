                                                      
template <typename t_item>
bool List<t_item>::Enumerator::EnumNext( void )
{
   return pList->EnumNext( this );
}

template <typename t_item>
List<t_item>::List( void )
{
   m_HeapId = GetCurrentMemoryHeapId( );
   m_pItems = NULL;

   m_WriteLock = 0;
}

template <typename t_item>
List<t_item>::~List( void )
{
   Debug::Assert( Condition(NULL == m_pItems), "List was not property destroyed" );
   Debug::Assert( Condition(0 == m_WriteLock), "List has been write locked, it cannot be modified" );
}

template <typename t_item>
void List<t_item>::Create( 
   uint32 startItems, //= 16
   uint32 growBy
)
{
   Debug::Assert( Condition(NULL == m_pItems), "List was already created" );
   Debug::Assert( Condition(0 == m_WriteLock), "List has been write locked, it cannot be modified" );

   m_GrowBy = growBy;
   m_MaxItems = startItems;
   
   m_NumItems = 0;                                                     

   PushMemoryHeap( m_HeapId );
      m_pItems = (t_item *) malloc( m_MaxItems * sizeof(t_item) );
   PopMemoryHeap( );
}
   
template <typename t_item>
void List<t_item>::Destroy( void )
{
   Debug::Assert( Condition(0 == m_WriteLock), "List has been write locked, it cannot be modified" );

   free( m_pItems );
   m_pItems = NULL;
}

template <typename t_item>
void List<t_item>::CopyFrom(
   const List<t_item> &copyFrom
)
{
   Debug::Assert( Condition(NULL != m_pItems), "List has not been created" );
   Debug::Assert( Condition(0 == m_WriteLock), "List has been write locked, it cannot be modified" );

   Clear( );

   uint32 i, size = copyFrom.GetSize( );

   for ( i = 0; i < size; i++ )
   {
      Add( copyFrom.GetAt(i) );
   }
}

template <typename t_item>
void List<t_item>::GrowTo(
   uint32 numItems
)
{
   Debug::Assert( Condition(0 == m_WriteLock), "List has been write locked, it cannot be modified" );

   if ( m_MaxItems < numItems )
   {
      m_MaxItems = numItems;
      m_pItems   = (t_item *) realloc( m_pItems, m_MaxItems * sizeof(t_item) );
   }
}

template <typename t_item>
void List<t_item>::CopyTo(
   t_item *pItems,
   uint32 numSlots
) const
{
   Debug::Assert( Condition(NULL != m_pItems), "List has not been created" );

   Debug::Assert( Condition(m_NumItems <= numSlots), "CopyTo array size isn't enough for list (%d and %d)", m_NumItems, numSlots );

   uint32 i, count = m_NumItems < numSlots ? m_NumItems : numSlots;

   for ( i = 0; i < count; i++ )
   {
      memcpy( &pItems[ i ], &m_pItems[ i ], sizeof(t_item) );
   }
}

template <typename t_item>
int List<t_item>::Add(
   const t_item &item
)
{
   Debug::Assert( Condition(NULL != m_pItems), "List has not been created" );
   Debug::Assert( Condition(0 == m_WriteLock), "List has been write locked, it cannot be modified" );

   if ( m_NumItems == m_MaxItems )
   {
      GrowTo( m_NumItems + m_GrowBy );
   }

   memcpy( &m_pItems[ m_NumItems ], &item, sizeof(t_item) );
   
   return m_NumItems++;
}

template <typename t_item>
void List<t_item>::Replace(
   uint32 index,
   const t_item &item
)
{
   Debug::Assert( Condition(NULL != m_pItems), "List has not been created" );
   Debug::Assert( Condition(0 == m_WriteLock), "List has been write locked, it cannot be modified" );

   Debug::Assert( Condition(index < m_NumItems), "Insert position %d, exceeds current items %d", index, m_NumItems );

   if ( index < m_NumItems )
   {
      memcpy( &m_pItems[ index ], &item, sizeof(t_item) );
   }
}

template <typename t_item>
void List<t_item>::Insert(
   const t_item &item,
   uint32 indexPosition
)
{
   Debug::Assert( Condition(NULL != m_pItems), "List has not been created" );
   Debug::Assert( Condition(0 == m_WriteLock), "List has been write locked, it cannot be modified" );

   Debug::Assert( Condition(indexPosition <= m_NumItems), "Insert position %d, exceeds current items %d", indexPosition, m_NumItems );

   //always add it to the end so the list will
   //grow correctly if needed
   uint32 count = m_NumItems;

   Add( item );
   
   //resort it to the correct position
   if ( indexPosition < count )
   {
      memmove( &m_pItems[ indexPosition + 1 ], &m_pItems[ indexPosition ], (count - indexPosition) * sizeof(t_item) );
      memmove( &m_pItems[ indexPosition ], &item, sizeof(t_item) );
   }
}

template <typename t_item>
int List<t_item>::AddUnique(
   const t_item &item
)
{
   Debug::Assert( Condition(NULL != m_pItems), "List has not been created" );
   Debug::Assert( Condition(0 == m_WriteLock), "List has been write locked, it cannot be modified" );

   if ( InvalidIndex == GetIndex(item) )
   {
      return Add( item );
   }

   return -1;
}

template <typename t_item>
void List<t_item>::Remove(
   const t_item &item
)
{
   Debug::Assert( Condition(NULL != m_pItems), "List has not been created" );
   Debug::Assert( Condition(0 == m_WriteLock), "List has been write locked, it cannot be modified" );

   uint32 index = GetIndex( item );

   if ( InvalidIndex != index )
   {
      RemoveAt( index );
   }
}

template <typename t_item>
t_item List<t_item>::RemoveAt(
   uint32 index
)
{
   Debug::Assert( Condition(NULL != m_pItems), "List has not been created" );
   Debug::Assert( Condition(0 == m_WriteLock), "List has been write locked, it cannot be modified" );

   t_item item = m_pItems[ index ];

   memcpy( &m_pItems[ index ], &m_pItems[ m_NumItems - 1 ], sizeof(t_item) );

   --m_NumItems;

   return item;
}

template <typename t_item>
void List<t_item>::RemoveSorted(
   const t_item &item
)
{
   Debug::Assert( Condition(NULL != m_pItems), "List has not been created" );
   Debug::Assert( Condition(0 == m_WriteLock), "List has been write locked, it cannot be modified" );

   uint32 index = GetIndex( item );

   if ( InvalidIndex != index )
   {
      memcpy( &m_pItems[ index ], &m_pItems[ index + 1 ], (m_NumItems - index - 1) * sizeof(t_item) );      

      --m_NumItems;
   }
}

template <typename t_item>
void List<t_item>::RemoveSorted(
   uint32 index
)
{
   Debug::Assert( Condition(NULL != m_pItems), "List has not been created" );
   Debug::Assert( Condition(0 == m_WriteLock), "List has been write locked, it cannot be modified" );

   memcpy( &m_pItems[ index ], &m_pItems[ index + 1 ], (m_NumItems - index - 1) * sizeof(t_item) );      

   --m_NumItems;
}

template <typename t_item>
t_item List<t_item>::GetAt(
   uint32 index
) const
{
   Debug::Assert( Condition(NULL != m_pItems), "List has not been created" );

   return m_pItems[ index ];
}

template <typename t_item>
t_item *List<t_item>::GetPointer(
   uint32 index
) const
{
   Debug::Assert( Condition(NULL != m_pItems), "List has not been created" );

   return &m_pItems[ index ];
}

template <typename t_item>
uint32 List<t_item>::GetIndex(
   t_item item
) const
{
   Debug::Assert( Condition(NULL != m_pItems), "List has not been created" );

   uint32 i;
   
   for ( i = 0; i < m_NumItems; i++ )
   {
      if ( item == m_pItems[ i ] ) return i;
   }

   return InvalidIndex;
}

template <typename t_item>
List<t_item> &List<t_item>::operator = (const List<t_item> &rhs)
{
   CopyFrom( rhs );

   return *this;
}


template <typename t_item>
bool List<t_item>::EnumNext(
   Enumerator *pEnumerator
) const
{
   Debug::Assert( Condition(NULL != m_pItems), "List has not been created" );

   if ( NULL == pEnumerator->pData )
   {
      pEnumerator->index = 0;
   }
   else
   {
      pEnumerator->index++;
   }

   if ( pEnumerator->index >= m_NumItems ) return false;

   pEnumerator->pData = &m_pItems[ pEnumerator->index ];
   return true;
}

