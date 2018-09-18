#pragma once

#include "EngineGlobal.h"
#include "IOStreams.h"
#include "RingBuffer.h"
#include "zlib.h"

class CompressedInputStream : public IInputStream
{
private:
   IInputStream *m_pRawStream;
   z_stream      m_Stream;
   uint32        m_UpperLimit;
   uint32        m_TotalRead;
   RingBuffer    m_Buffer;

public:
   CompressedInputStream( 
      IInputStream *pRawStream,
      uint32 upperLimit = UINT_MAX
   );

   ~CompressedInputStream( void );

   void Bind(
      IInputStream *pRawStream,
      uint32 upperLimit = UINT_MAX
   );

   virtual void Read(
      void *pBuffer,
      uint32 size,
      uint32 *pBytesRead = NULL
   );

   virtual uint32 Seek(
      int amount,
      SeekOrigin origin
   );

private:
   void CleanupInflate( void );
};
