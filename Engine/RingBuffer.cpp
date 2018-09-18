#include "EnginePch.h"

#include "RingBuffer.h"

void RingBuffer::Create(
   uint32 startSize,
   uint32 growBy //=64 * 1024
)
{
   m_BufferSize = startSize;
   m_GrowBy     = growBy;
   m_pBuffer    = (char *) malloc( startSize );
   
   Reset( );
}

void RingBuffer::Destroy( void )
{
   free( m_pBuffer );
}

void RingBuffer::Reset( void )
{
   m_ReadPos    = 0;
   m_WritePos   = 0;
   m_WriteLooped= false;
}

void RingBuffer::Read(
   void *pBuffer,
   uint32 size,
   uint32 *pBytesRead// = NULL
)
{
   uint32 sizeToRead = size;
   uint32 delta;
   
   bool readingCanLoop = false;

   //read up to the write buffer
   if ( m_WritePos > m_ReadPos )
   {
      delta = m_WritePos - m_ReadPos;
   }
   else
   {
      if ( true == m_WriteLooped )
      {
         readingCanLoop = true;
         delta = m_BufferSize - m_ReadPos;
      }
      else
      {
         delta = 0;
      }
   }

   //if there isn't enough data
   //reduce the amount of data read to what we have left
   if ( delta < sizeToRead ) sizeToRead = delta;
   
   memcpy( pBuffer, m_pBuffer + m_ReadPos, sizeToRead );
   Advance( sizeToRead );


   //if there is more requested to read and there is
   //more to read at the beginning of the buffer
   if ( size > sizeToRead && true == readingCanLoop )
   {
      uint32 bytesRead;

      Read( ((BYTE *)pBuffer) + sizeToRead, size - sizeToRead, &bytesRead );

      sizeToRead += bytesRead;
   }

   if ( pBytesRead ) *pBytesRead = sizeToRead;
}

void RingBuffer::Peek(
   void *pBuffer,
   uint32 size,
   uint32 *pBytesRead// = NULL
)
{
   //cache old positions
   uint32 readPos     = m_ReadPos;
   bool   writeLooped = m_WriteLooped;

   Read( pBuffer, size, pBytesRead );

   //restore after the read so we 
   //haven't advanced our positions
   m_ReadPos     = readPos;
   m_WriteLooped = writeLooped;
}

void RingBuffer::Write(
   const void *pBuffer,
   uint32 size
)
{
   if ( 0 == size ) return;

   uint32 sizeToWrite = size;
   uint32 delta = sizeToWrite;

   //if we haven't looped
   //then see if we have enough room
   //to write to the end of the buffer
   if ( m_WritePos > m_ReadPos )
   {
      if ( m_WritePos + size > m_BufferSize )
      {
         delta = m_BufferSize - m_WritePos;
      }
   }
   else
   {
      //we have looped so see if we have enough room
      //to write to the read pos
      delta = m_ReadPos - m_WritePos;
      
      if ( size > delta )
      {
         //if the write has looped then there is no
         //room left to write and we expand our buffer
         if ( true == m_WriteLooped )
         {
            //see how many growby amounts we need to grow by
            //to hold the size left in delta
            uint32 growSlots = (size - delta) / m_GrowBy + 1;
            uint32 newSize = m_BufferSize + m_GrowBy * growSlots;
            uint32 oldSize = m_BufferSize;

            Debug::Print( Debug::TypeInfo, "Ringbuffer growing by: %d.  New Size %d\n", growSlots * m_GrowBy, newSize );
            
            //increase the buffer size
            m_pBuffer = (char *) realloc( m_pBuffer, newSize );
            
            //move the read data to the new tail which leaves a bigger gap
            //between write and read so we can add new data
            uint32 amountNotReadToEnd = oldSize - m_ReadPos;
            memmove( m_pBuffer + (newSize - amountNotReadToEnd), m_pBuffer + m_ReadPos, amountNotReadToEnd );
            
            m_ReadPos = (newSize - amountNotReadToEnd);

            m_BufferSize  = newSize;
            m_WriteLooped = true;
            
            delta = m_ReadPos - m_WritePos;
         }
         else if ( 0 == delta )
         {
            //if write and read are the same and their
            //loop count is the same then read caught up with write
            //so go ahead and write to the end of the buffer
            delta = m_BufferSize - m_WritePos;
         }
      }
   }

   //if we can't fit it, decrease the amount
   //we write
   if ( delta < sizeToWrite ) sizeToWrite = delta; 

   memcpy( m_pBuffer + m_WritePos, pBuffer, sizeToWrite );

   uint32 lastWritePos = m_WritePos;

   //increment (and loop if necessary)
   m_WritePos = (m_WritePos + sizeToWrite) % m_BufferSize;
   
   if ( false == m_WriteLooped && sizeToWrite )
   {
      m_WriteLooped = m_WritePos <= lastWritePos;
   }
   
   //if we haven't written all we needed
   //call this recursively
   //this could happen if we wrote to the end
   //of the buffer - and want to continue writing
   //at the head
   if ( sizeToWrite < size )
   {
      Write( (char *)pBuffer + sizeToWrite, size - sizeToWrite );
   }
}

void RingBuffer::Advance(
   uint32 amount   
)
{
   uint32 lastReadPos = m_ReadPos;

   //increment and loop back over to 0
   m_ReadPos = (m_ReadPos + amount) % m_BufferSize;

   //set the write loop flag to false
   //if our read just looped
   if ( true == m_WriteLooped && amount )
   {
      m_WriteLooped = ! (m_ReadPos <= lastReadPos);
   }
}

bool RingBuffer::IsEmpty( void ) const
{
   return (m_ReadPos == m_WritePos) && (false == m_WriteLooped);   
}



