#pragma once

#include "EngineGlobal.h"
#include "TcpIpPipe.h"
#include "Threads.h"
#include "Socket.h"
#include "PipeStreams.h"

class PipeStream : public Pipe
{
private:
   friend class PipePipeThread;

   class PipeThread : public Thread
   {
   private:
      PipeStream *m_pPipe;

   public:
      void Create(
         PipeStream *pPipe
         )
      {
         m_pPipe = pPipe;
      }

      virtual void OnThreadRun( void );
   };


private:
   Lock                m_Lock;
   Socket             *m_pSocket;
   PipeSendStream      m_SendStream;
   PipeRecvStream      m_RecvStream;
   PipeThread          m_Thread;

public:
   void Create( 
      bool rawPackets 
      );

   void Destroy( void );

   virtual void Update( 
      float maxTimeSlice
      );

   void SetTimeout(
      float seconds
      );

   virtual void SendStream(
      int id,
      const void *pBuffer,
      uint32 size
      );

   virtual void SendStream(
      const char *pName,
      const void *pBuffer,
      uint32 size
      );

   void Start(
      Socket *pSocket
      );

   void Stop( void );

   Socket *GetSocket( void ) { return m_pSocket; }

   virtual void SetMaxMessageSize(
      uint32 maxSize
      )
   {
      m_RecvStream.SetMaxMessageSize( maxSize );
   }

   bool IsConnected( void ) const { return m_pSocket->IsConnected( ); }

private:
   void OnThreadRun( void );
};
