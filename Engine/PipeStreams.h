#pragma once

#include "EngineGlobal.h"
#include "RingBuffer.h"
#include "Resource.h"

class IPipeRecvStream
{
public:
   virtual void Write(
      const void *pBuffer,
      uint32 size
   ) = 0;
};

class IPipeSendStream
{
public:
   virtual void Peek(
      void *pBuffer,
      uint32 size,
      uint32 *pBytesRead = NULL
   ) = 0;

   virtual void Advance(
      uint32 amount   
   ) = 0;
};

struct PipeHeader
{
   static const uint32 Magic = 0x01234567;

   uint32 magic;
   uint32 size;
   uint32 id;
};

struct PipeMessage
{
private:
   PipeHeader header;

public:
   uint32 Id( void ) { return header.id; }

   void *Data( void )
   {
      return ((char *)&header) + sizeof(header);
   }

   uint32 DataSize( void )
   {
      return header.size - sizeof(header);
   }
};

class PipeRecvStream : public IPipeRecvStream
{
private:
   uint32 m_ReadPos;
   uint32 m_WritePos;
   uint32 m_BufferSize;
   uint32 m_MaxMessageSize;
   char  *m_pBuffer;
   bool   m_RawPackets;

public:
   void Create(
      uint32 maxMessageSize,
      bool rawPackets
   );
   
   void Destroy( void );
   void Reset  ( void );

   void Advance( void );
   void Cleanup( void );

   PipeMessage *GetMessage( void );

   void *GetRawMessage( void );

   bool HasInvalidPacket( void );

   void Write(
      const void *pBuffer,
      uint32 size
   );

   bool IsEmpty( void ) const;

   void SetMaxMessageSize(
      uint32 maxSize
   )
   {
      m_MaxMessageSize = maxSize;
   }

private:
   void GrowIfNeeded(
      uint32 size
   );
};

class PipeSendStream : public IPipeSendStream
{
private:
   RingBuffer  m_RingBuffer;
   bool        m_RawPackets;

public:
   void Create(
      uint32 startSize,
      uint32 growBy,
      bool rawPackets
   );

   void Reset  ( void );
   void Destroy( void );

   void Write(
      uint32 id,
      const void *pBuffer,
      uint32 size
   );

   void Peek(
      void *pBuffer,
      uint32 size,
      uint32 *pBytesRead = NULL
   );

   void Advance(
      uint32 amount   
   );

   bool IsEmpty( void ) const;
};
