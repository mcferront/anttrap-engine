#include "EnginePch.h"

#include "TextReader.h"
#include "MemoryStreams.h"

const char *TextReader::NewlineInBlock( void )
{
   // block exists, looking for next line
   if ( NULL != m_pNext )
   {
      char *pNewline = strchr( m_pNext, '\n' );

      // if found, move up our next pointer psat it
      // and return the new line
      if ( pNewline )
      {
         char *pStart = m_pNext;

         *pNewline = 0;
         m_pNext = pNewline + 1;

         return pStart;
      }

      // if no newline found, move our remaining memory
      // to the head
      size_t nextSize = strlen(m_pNext);
      memmove( m_pHead, m_pNext, nextSize );
      m_pNext = m_pHead;
      m_pHead[nextSize] = NULL;
   }

   return NULL;
}

const char *TextReader::ReadLine( void )
{
   const char *pNewline = NewlineInBlock( );

   if ( NULL != pNewline )
      return pNewline;

   while ( true )
   {
      size_t nullStop = m_pNext ? strlen(m_pNext) : 0;

      // fill up our head block
      size_t actualRead = 0, sizeToRead = m_ReadSize - nullStop;

      if ( sizeToRead > 0 )
      {
         size_t headOffset = m_ReadSize - sizeToRead;

         sizeToRead = sizeToRead - 1;
         m_pStream->Read( m_pNext + headOffset, sizeToRead, &actualRead );
         
         m_pNext[headOffset + actualRead] = NULL;
      }

      // try and find the newline
      pNewline = NewlineInBlock( );

      if ( NULL != pNewline )
         return pNewline;

      // block is full and still no newline :-/
      
      // if there is nothing more to read
      if ( actualRead < sizeToRead )
      {
         char *pLastValue = m_pNext;
         m_pNext += strlen(m_pNext);

         return pLastValue;
      }

      // There is more to read
      // alloc more memory and try again
      m_ReadSize = m_ReadSize + BlockSize;
      m_pHead = (char *) realloc( m_pHead, m_ReadSize );
      m_pNext = m_pHead;
      m_pHead[nullStop] = NULL;
   }
}
