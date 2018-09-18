#pragma once

#include "EngineGlobal.h"
#include "Types.h"

template <typename t_item>
class List
{
public:
   class Enumerator
   {
      friend class List<t_item>;

   public:
      Enumerator(const List<t_item> *_pList)
      {
         pList = _pList;
         index = 0;
         pData = NULL;

         pList->WriteLock( true );
      }

      ~Enumerator( void )
      {
         pList->WriteLock( false );
      }

      void Reset( void )
      {
          index = 0;
          pData = NULL;
      }

      bool EnumNext( void );

      const t_item &Data( void ) { return *pData; }

      Enumerator& operator = (const Enumerator &rhs)
      {
         //remove current lock
         pList->WriteLock( false );

         pList  = rhs.pList;
         pData  = rhs.pData;
         index  = rhs.index;

         pList->WriteLock( true );
         
         return *this;
      }

   private:
      Enumerator( ) {}

   private:
      const List<t_item> *pList;

      t_item *pData;
      uint32  index;
   };

public:
   static const uint32 InvalidIndex = 0xffffffff;

protected:
   uint32 m_NumItems;
   uint32 m_GrowBy;
   uint32 m_MaxItems;
   int    m_HeapId;

   t_item *m_pItems;

   mutable uint32 m_WriteLock;

public:
   List( void );

   ~List( void );

   void Create(
      uint32 startItems = 16,
      uint32 growBy = 16
   );

   void Destroy( void );

   void CopyFrom(
      const List<t_item> &
   );

   void GrowTo(
      uint32 numItems
   );

   void CopyTo(
      t_item *pItems,
      uint32 numSlots
   ) const;

   int Add(
      const t_item &item
   );

   void Insert(
      const t_item &item,
      uint32 indexPosition
   );

   int AddUnique(
      const t_item &item
   );
   
   void Replace(
      uint32 index,
      const t_item &item
   );

   void Remove(
      const t_item &item
   );

   t_item RemoveAt(
      uint32 index
   );

   void RemoveSorted(
      const t_item &item
   );

   void RemoveSorted(
      uint32 index
   );

   t_item GetAt(
      uint32 index
   ) const;

   t_item *GetPointer(
      uint32 index
   ) const;

   uint32 GetIndex(
      t_item item
   ) const;
   
   List<t_item> & operator = (const List<t_item> &rhs);

   bool Contains(
      t_item item
   ) const
   {
      return InvalidIndex != GetIndex( item );
   }

   void Clear( void ) { m_NumItems = 0; }

   uint32 GetSize( void ) const { return m_NumItems; }

   Enumerator GetEnumerator( void ) const
   {
      Debug::Assert( Condition(NULL != m_pItems), "List has not been created" );
      return Enumerator(this);
   }

private:
   void WriteLock( bool lock ) const { lock ? ++m_WriteLock : --m_WriteLock; }

   bool EnumNext(
      Enumerator *pEnumerator
   ) const;
};

#include "List.inl"
