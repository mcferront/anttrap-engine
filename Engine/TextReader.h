#pragma once

#include "EngineGlobal.h"
#include "IOStreams.h"

class MemoryStream;

class TextReader
{
private:
   MemoryStream *m_pStream;
   
   uint32 BlockSize = 1024;
   uint32 m_ReadSize;

   char *m_pHead;
   char *m_pNext;

public:
   TextReader(
      MemoryStream *pStream
   )
   {
      m_pStream = pStream;
      m_pNext = NULL;
      m_pHead = NULL;
      m_ReadSize = 0;
   }

   ~TextReader( void )
   {
      free( m_pHead );
   }

   const char *ReadLine( void );

private:
   const char *NewlineInBlock( void );

};
