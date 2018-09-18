                                                      
template <typename t_item>
Stack<t_item>::Stack( 
   uint32 startItems, //= 16
   uint32 growBy
)
{
   m_GrowBy = growBy;
   m_MaxItems = startItems;
   
   m_NumItems = 0;                                                     
   m_pItems = NULL;

   m_pItems = (t_item *) malloc( m_MaxItems * sizeof(t_item) );
}

template <typename t_item>
Stack<t_item>::~Stack( void )
{
   free( m_pItems );
}

template <typename t_item>
void Stack<t_item>::Copy(
   const Stack<t_item> &copyFrom
)
{
   Clear( );

   uint32 i, size = copyFrom.GetSize( );

   for ( i = 0; i < size; i++ )
   {
      Add( copyFrom.Get(i) );
   }
}

template <typename t_item>
int Stack<t_item>::Add(
   const t_item &item
)
{
   if ( m_NumItems == m_MaxItems )
   {
      m_MaxItems = m_NumItems + m_GrowBy;
      m_pItems   = (t_item *) realloc( m_pItems, m_MaxItems * sizeof(t_item) );
   }

   memcpy( &m_pItems[ m_NumItems ], &item, sizeof(t_item) );
   
   return m_NumItems++;
}

template <typename t_item>
void Stack<t_item>::Insert(
   const t_item &item,
   uint32 indexPosition
)
{
   Debug::Assert( Condition(indexPosition <= m_NumItems), "insert position %d, exceeds current items %d", indexPosition, m_NumItems );

   //always add it to the end so the stack will
   //grow correctly if needed
   Add( item );
   
   //resort it to the correct position
   if ( indexPosition < m_NumItems )
   {
      memcpy( &m_pItems[ indexPosition + 1 ], &m_pItems[ indexPosition ], (m_NumItems - indexPosition) * sizeof(t_item) );
      memcpy( &m_pItems[ indexPosition ], &item, sizeof(t_item) );
   }
}

template <typename t_item>
int Stack<t_item>::AddUnique(
   const t_item &item
)
{
   if ( InvalidIndex == GetIndex(item) )
   {
      return Add( item );
   }

   return -1;
}

template <typename t_item>
void Stack<t_item>::Remove(
   const t_item &item
)
{
   uint32 index = GetIndex( item );

   if ( InvalidIndex != index )
   {
      Remove( index );
   }
}

template <typename t_item>
void Stack<t_item>::Remove(
   uint32 index
)
{
   memcpy( &m_pItems[ index ], &m_pItems[ m_NumItems - 1 ], sizeof(t_item) );
   
   --m_NumItems;
}

template <typename t_item>
void Stack<t_item>::RemoveSorted(
   const t_item &item
)
{
   uint32 index = GetIndex( item );

   if ( InvalidIndex != index )
   {
      memcpy( &m_pItems[ index ], &m_pItems[ index + 1 ], (m_NumItems - index - 1) * sizeof(t_item) );      

      --m_NumItems;
   }
}

template <typename t_item>
void Stack<t_item>::RemoveSorted(
   uint32 index
)
{
   memcpy( &m_pItems[ index ], &m_pItems[ index + 1 ], (m_NumItems - index - 1) * sizeof(t_item) );      

   --m_NumItems;
}

template <typename t_item>
t_item Stack<t_item>::Get(
   uint32 index
) const
{
   return m_pItems[ index ];
}

template <typename t_item>
t_item *Stack<t_item>::GetPointer(
   uint32 index
) const
{
   return &m_pItems[ index ];
}

template <typename t_item>
uint32 Stack<t_item>::GetIndex(
   t_item item
) const
{
   uint32 i;
   
   for ( i = 0; i < m_NumItems; i++ )
   {
      if ( item == m_pItems[ i ] ) return i;
   }

   return InvalidIndex;
}

template <typename t_item>
Stack<t_item> &Stack<t_item>::operator = (const Stack<t_item> &rhs)
{
   Clear( );

   int i, size = rhs.GetSize( );

   for ( i = 0; i < size; i++ )
   {
      Add( rhs.Get(i) );
   }

   return *this;
}
