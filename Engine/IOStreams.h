#pragma once

#include "EngineGlobal.h"

enum SeekOrigin
{
    SeekBegin,
    SeekCurrent,
    SeekEnd
};

class IInputStream
{
public:
   virtual ~IInputStream( void ) {};

   virtual void Read(
      void *pBuffer,
      uint32 size,
      uint32 *pBytesRead = NULL
   ) = 0;
   
   virtual uint32 Seek(
      int amount,
      SeekOrigin origin
   ) = 0;
};

class IOutputStream
{
public:
   virtual void Write(
      const void *pBuffer,
      uint32 size,
      uint32 *pBytesWritten = NULL
   ) = 0;

   virtual uint32 Seek(
      int amount,
      SeekOrigin origin
   ) = 0;
};
