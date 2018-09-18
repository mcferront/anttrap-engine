#pragma once

#include "EngineGlobal.h"
#include "IOStreams.h"

class MemoryStream : public IInputStream, public IOutputStream
{
private:
   uint32 m_Position;
   uint32 m_BufferSize;
   uint32 m_Size;
   
   byte *m_pBuffer;
   const byte *m_pConstBuffer;

   bool  m_OwnsMemory;

public:
   MemoryStream( void )
   {
      m_Size    = 0;
      m_Position= 0;
      m_BufferSize = 0;
      m_pBuffer = NULL;
      m_pConstBuffer = NULL;
      m_OwnsMemory = true;
   }

   MemoryStream(
      const void *pBuffer,
      uint32 size
   );

   MemoryStream(
      void *pBuffer,
      uint32 size,
      bool ownsMemory
   );

   ~MemoryStream( void )
   {
      Close( );
   }

   void Close( void );

   virtual void Read(
      void *pBuffer,
      uint32 size,
      uint32 *pBytesRead = NULL
   );

   void Copy(
      IInputStream *pStream,
      uint32 amountToCopy
   );

   virtual uint32 Seek(
      int amount,
      SeekOrigin origin
   );
   
   virtual void Write(
      const void *pBuffer,
      uint32 size,
      uint32 *pBytesWritten = NULL
   );

   void *Write(
      uint32 size
   );
   
   uint32 GetCurrentPosition( void ) const { return m_Position; }
   uint32 GetAmountWritten( void ) const { return m_Size; }
   byte *GetBuffer( void ) const { return (byte *) m_pBuffer; }
   const byte *GetConstBuffer( void ) const { return (byte *) m_pConstBuffer; }

   IInputStream  *GetInputStream ( void ) { return this; }
   IOutputStream *GetOutputStream( void ) { return this; }

private:
   void Realloc(
      uint32 newSize
   );
};
