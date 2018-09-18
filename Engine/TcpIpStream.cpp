#include "EnginePch.h"

#include "TcpIpStream.h"
#include "ResourceWorld.h"
#include "MemoryStreams.h"
#include "ChannelSystem.h"
#include "UtilityClock.h"

void PipeStream::PipeThread::OnThreadRun( void )
{
   while ( ShouldRun( ) )
      m_pPipe->OnThreadRun( );
}

void PipeStream::Create( 
   bool rawPackets
   )
{
   Pipe::Create( );
   
   m_RecvStream.Create( INT_MAX, rawPackets );
   m_SendStream.Create( 16 * 1024, 16 * 1024, rawPackets );
}

void PipeStream::Destroy( void )
{
   Stop( );
   
   m_RecvStream.Destroy( );
   m_SendStream.Destroy( );

   if ( NULL != m_pSocket )
   {
       m_pSocket->Destroy( );

       delete m_pSocket;
       m_pSocket = NULL;
   }

   Pipe::Destroy( );
}

void PipeStream::SendStream(
   int id,
   const void *pBuffer,
   uint32 size
)
{
   MainThreadCheck;

   ScopeLock lock( m_Lock );

   m_SendStream.Write( id, pBuffer, size );
}

void PipeStream::SendStream(
   const char *pName,
   const void *pBuffer,
   uint32 size
)
{
   MainThreadCheck;

   uint32 id;

   id = NameToId(pName);
   Debug::Assert( Condition(id != 0xffffffff), "Network Stream %s has no associated Id", pName );

   SendStream( id, pBuffer, size );
}

void PipeStream::Start( 
    Socket *pSocket
)
{
   m_pSocket = pSocket;

   m_Thread.Create( this );
   m_Thread.Run( );
}

void PipeStream::Stop( void )
{
   m_Thread.Stop( );
}

void PipeStream::Update(
   float maxTimeSlice
)
{
   Clock clock;

   clock.Start( );

   if ( true == m_pSocket->IsConnected( ) )
   {
      bool invalidMessage = false;

      {
         ScopeLock lock( m_Lock );
      
         PipeMessage *pMessage;

         while ( NULL != (pMessage = m_RecvStream.GetMessage( )) )
         {
            const char *pName = IdToName( pMessage->Id( ) );
            if ( NULL == pName )
            {
               Debug::Assert( Condition(false), "Unrecognized incoming id %d", pMessage->Id() );
               invalidMessage = true;
               break;
            }

            //Debug::Print( Debug::TypeInfo, "PipeStream Received: %s", pName );

            MemoryStream stream( pMessage->Data( ), pMessage->DataSize( ), false );

            GetChannel( )->SendEvent( pName, ArgList(&stream) );
            m_RecvStream.Advance( );

            if ( clock.TestSample( ) >= maxTimeSlice )
            {
               break;   
            }
         }

         m_RecvStream.Cleanup( );
      }

      //outside the scope lock
      //because we might need to disconnect the socket
      if ( true == invalidMessage || m_RecvStream.HasInvalidPacket( ) )
      {
         GetChannel( )->SendEvent( "InvalidPacket", ArgList() );
         Stop( );
      }
   }
}

void PipeStream::OnThreadRun( void )
{   
   int sleepTime = 1;

   if ( true == m_pSocket->IsConnected( ) )
   {
      ScopeLock lock( m_Lock );
   
      m_pSocket->Read( &m_RecvStream );
      m_pSocket->Write( &m_SendStream );
   
      if (m_RecvStream.IsEmpty() && m_SendStream.IsEmpty())
         sleepTime = 100;
   }

   Thread::Sleep( sleepTime );
}
