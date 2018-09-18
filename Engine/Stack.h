#pragma once

#include "EngineGlobal.h"
#include "Types.h"
#include "List.h"

template <typename t_item>
class Stack
{
protected:
   List<t_item> m_Items;

public:
   Stack( ) { }
   ~Stack( void ) { }

   void Create( void ) 
   {
      m_Items.Create( );
   }

   void Destroy( void )
   {
      m_Items.Destroy( );
   }

   void Push(const t_item &item) { m_Items.Add(item); }
   t_item Pop()
   {
      t_item retval = m_Items.GetAt(m_Items.GetSize() - 1);
      m_Items.RemoveAt(m_Items.GetSize() - 1);
      return retval;
   }
   t_item Top() { return m_Items.GetAt(m_Items.GetSize() - 1); }

   void Clear( void ) { m_Items.Clear(); }

   uint32 GetSize( void ) const { return m_Items.GetSize(); }
};
