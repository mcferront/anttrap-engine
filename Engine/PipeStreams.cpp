#include "EnginePch.h"

#include "PipeStreams.h"
#include "ResourceWorld.h"

void PipeRecvStream::Create(
   uint32 maxMessageSize,
   bool rawPackets
   )
{
   m_BufferSize = 0;
   m_pBuffer    = NULL;
   m_MaxMessageSize = maxMessageSize;
   m_RawPackets = rawPackets;

   Reset( );
}

void PipeRecvStream::Destroy( void )
{
   free( m_pBuffer );
}

void PipeRecvStream::Reset( void )
{
   m_ReadPos  = 0;
   m_WritePos = 0;
}

void PipeRecvStream::Advance( void )
{
   if ( false == m_RawPackets )
   {
      //advance to the next Pipe message
      //in our stream
      PipeHeader *pHeader = (PipeHeader *) (m_pBuffer + m_ReadPos);

   #ifdef ENABLE_ASSERT
      uint32 delta = m_WritePos - m_ReadPos;
      Debug::Assert( Condition(delta >= sizeof(PipeHeader)), "Advance called but there is not enough data to advance" );

      Debug::Assert( Condition(delta >= pHeader->size), "Advance called but there is not enough data to advance" );
   #endif

      m_ReadPos += pHeader->size;
   }
   else
      m_ReadPos = m_WritePos;

   if ( true == IsEmpty() ) Reset( );
}

void PipeRecvStream::Cleanup( void )
{
   //move everything we haven't read to the
   //front of the buffer so we have space
   //to fill as new messages come in
   uint32 delta = m_WritePos - m_ReadPos;
   memcpy( m_pBuffer, m_pBuffer + m_ReadPos, delta );

   m_ReadPos  = 0;
   m_WritePos = delta;
}

PipeMessage *PipeRecvStream::GetMessage( void )
{
   if ( false == m_RawPackets )
   {
      //verify we have enough data
      //to parse a Pipe header
      uint32 delta = m_WritePos - m_ReadPos;

      if ( delta >= sizeof(PipeHeader) )
      {
         //see if we have enough data for the
         //entire Pipe message
         PipeHeader *pHeader = (PipeHeader *) (m_pBuffer + m_ReadPos);

         if ( pHeader->magic == PipeHeader::Magic &&
            pHeader->size  <= m_MaxMessageSize )
         {
            if ( delta >= pHeader->size )
            {
               return (PipeMessage *) pHeader;
            }
         }
      }
   }
   else
      Debug::Assert( Condition(false), "GetMesssage called on a pipe stream with raw packets");

   return NULL;
}

void *PipeRecvStream::GetRawMessage( void )
{
   if ( true == m_RawPackets )
      return m_pBuffer + m_ReadPos;
   else
      Debug::Assert( Condition(false), "GetRawMessage called on a pipe stream with header packets");

   return NULL;
}

bool PipeRecvStream::HasInvalidPacket( void )
{
   if ( true == m_RawPackets )
   {
      //verify we have enough data
      //to parse a Pipe header
      uint32 delta = m_WritePos - m_ReadPos;

      if ( delta >= sizeof(PipeHeader) )
      {
         PipeHeader *pHeader = (PipeHeader *) (m_pBuffer + m_ReadPos);
         return ( pHeader->magic != PipeHeader::Magic || pHeader->size > m_MaxMessageSize );
      }
   }

   return false;
}

void PipeRecvStream::Write(
   const void *pBuffer,
   uint32 size
   )
{
   GrowIfNeeded( size );

   memcpy( m_pBuffer + m_WritePos, pBuffer, size );

   m_WritePos += size;
}

bool PipeRecvStream::IsEmpty( void ) const
{
   return m_ReadPos == m_WritePos;
}

void PipeRecvStream::GrowIfNeeded(
   uint32 size
   )
{
   if ( m_WritePos + size > m_BufferSize )
   {
      m_BufferSize = m_WritePos + size + 64 * 1024;
      m_pBuffer    = (char *) realloc( m_pBuffer, m_BufferSize);
   }
}

void PipeSendStream::Create(
   uint32 startSize,
   uint32 growBy, //=64 * 1024
   bool rawPackets //=false 
   )
{
   m_RingBuffer.Create( startSize, growBy );
   m_RawPackets = rawPackets;
}

void PipeSendStream::Destroy( void )
{
   m_RingBuffer.Destroy( );
}

void PipeSendStream::Reset( void )
{
   m_RingBuffer.Reset( );
}

void PipeSendStream::Write(
   uint32 id,
   const void *pBuffer,
   uint32 size
   )
{
   if ( false == m_RawPackets )
   {
      PipeHeader header;

      header.magic = PipeHeader::Magic;
      header.size  = size + sizeof(header);
      header.id    = id;

      m_RingBuffer.Write( &header,  sizeof(header) );
   }
   
   m_RingBuffer.Write( pBuffer, size );
}

void PipeSendStream::Peek(
   void *pBuffer,
   uint32 size,
   uint32 *pBytesRead // = NULL,
   )
{
   m_RingBuffer.Peek( pBuffer, size, pBytesRead );
}

void PipeSendStream::Advance(
   uint32 amount   
   )
{
   m_RingBuffer.Advance( amount );
}

bool PipeSendStream::IsEmpty( void ) const
{
   return m_RingBuffer.IsEmpty( );
}

