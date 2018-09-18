#pragma once

#include "EngineGlobal.h"

class RingBuffer
{
private:
   uint32 m_ReadPos;
   uint32 m_WritePos;
   uint32 m_BufferSize;
   uint32 m_GrowBy;
   char  *m_pBuffer;
   bool   m_WriteLooped;

public:
   void Create(
      uint32 startSize,
      uint32 growBy = 64 * 1024
   );

   void Destroy( void );
   void Reset  ( void );
   
   void Read(
      void *pBuffer,
      uint32 size,
      uint32 *pBytesRead = NULL
   );

   void Peek(
      void *pBuffer,
      uint32 size,
      uint32 *pBytesRead = NULL
   );

   void Write(
      const void *pBuffer,
      uint32 size
   );

   void Advance(
      uint32 amount   
   );

   bool IsEmpty( void ) const;
};
