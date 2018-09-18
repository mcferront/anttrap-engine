#include "EnginePch.h"

#include "TcpIpPipe.h"
#include "Channel.h"

void Pipe::Create( void )
{
   m_NameIdHash.Create( );
    
   m_pChannel = new Channel;
   m_pChannel->Create( Id::Create( ), NULL );

   memset( m_pNameIdLookup, 0, sizeof(m_pNameIdLookup) );
}

void Pipe::Destroy( void )
{
   m_pChannel->Destroy( );

   delete m_pChannel;

   uint32 i;

   for ( i = 0; i < MaxLookups; i++ )
   {
      if ( m_pNameIdLookup[i] )
      {
         StringRel( m_pNameIdLookup[i] );
      }
   }

   m_NameIdHash.Destroy( );
}

