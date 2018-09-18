#include "EnginePch.h"

#include "MemoryStreams.h"

MemoryStream::MemoryStream(
   void *pData,
   uint32 size,
   bool ownsMemory
)
{
   m_pBuffer  = (byte *) pData;
   m_pConstBuffer = m_pBuffer;
   m_Position = 0;
   m_Size      = size;
   m_BufferSize= size;
   
   m_OwnsMemory = true == ownsMemory;
}

MemoryStream::MemoryStream(
   const void *pData,
   uint32 size
)
{
   m_pConstBuffer = (const byte *) pData;
   m_pBuffer = NULL;
   m_Position = 0;
   m_Size      = size;
   m_BufferSize= size;
   m_OwnsMemory = false;
}

void MemoryStream::Close( void )
{
   if ( true == m_OwnsMemory )
      free( m_pBuffer );

   m_pConstBuffer = NULL;
   m_pBuffer  = NULL;
   m_Position = 0;
   m_BufferSize= 0;
   m_Size      = 0;
}

void MemoryStream::Read(
   void *pBuffer,
   uint32 size,
   uint32 *pBytesRead// = NULL
)
{
   uint32 maxSize = size >= m_Size - m_Position ? m_Size - m_Position : size;

   memcpy( pBuffer, m_pConstBuffer + m_Position, maxSize );

   if (NULL != pBytesRead) *pBytesRead = maxSize;

   m_Position += maxSize;
}

void MemoryStream::Copy(
    IInputStream *pStream,
    uint32 amountToCopy
)
{
    if ( m_Position + amountToCopy > m_BufferSize )
    {
        Debug::Assert( Condition(m_OwnsMemory), "MemoryStream needs to realloc but it doesn't own its memory" );
        Realloc( m_Position + amountToCopy + (64 * 1024) );
    }

    pStream->Read( m_pBuffer + m_Position, amountToCopy );

    m_Position += amountToCopy;
    
    if ( m_Position > m_Size ) m_Size = m_Position;
}

uint32 MemoryStream::Seek(
   int amount,
   SeekOrigin origin
)
{
   int newPosition;

   if ( SeekBegin == origin )    newPosition = amount;
   else if ( SeekEnd == origin ) newPosition = m_Size + amount;
   else
   {
      newPosition = amount + m_Position;
   }

   if ( newPosition < 0 ) newPosition = 0;
   if ( (uint32) newPosition > m_Size ) newPosition = m_Size;

   m_Position = newPosition;

   return m_Position;
}

void MemoryStream::Write(
   const void *pBuffer,
   uint32 size,
   uint32 *pBytesWritten//= NULL
)
{
    if ( m_Position + size > m_BufferSize )
    {
        Debug::Assert( Condition(m_OwnsMemory), "MemoryStream needs to realloc but it doesn't own its memory" );
        Realloc( m_Position + size + (64 * 1024) );
    }

    memcpy( m_pBuffer + m_Position, pBuffer, size );

    m_Position += size;
    
    if ( m_Position > m_Size ) m_Size = m_Position;
}

void *MemoryStream::Write(
   uint32 size
)
{
   if ( m_Position + size > m_BufferSize )
    {
        Debug::Assert( Condition(m_OwnsMemory), "MemoryStream needs to realloc but it doesn't own its memory" );
        Realloc( m_Position + size + (64 * 1024) );
    }

   void *pData = m_pBuffer + m_Position;

   m_Position += size;
   
   if ( m_Position > m_Size ) m_Size = m_Position;

   return pData;
}
void MemoryStream::Realloc(
   uint32 newSize
)
{
   m_BufferSize= newSize;
   m_pBuffer   = (byte *) realloc( m_pBuffer, m_BufferSize );
   m_pConstBuffer = m_pBuffer;
}
