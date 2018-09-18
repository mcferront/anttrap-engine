#include "EnginePch.h"

#include "Timer.h"
#include "Channel.h"

Timer::Timer( void )
{
   m_Started  = false;
   m_pChannel = new Channel;
   m_pChannel->Create( Id::Create( ), NULL );
}

Timer::~Timer( void )
{
   m_pChannel->Destroy( );
   delete m_pChannel;
}

void Timer::Start(
   float seconds
)
{
   m_Seconds = seconds;
   m_Started = true;
}

void Timer::Stop( void )
{
   m_Started = false;
}

void Timer::Update(
   float deltaSeconds
)
{ 
   if ( true == m_Started )
   {
      m_Seconds -= deltaSeconds;
   
      if ( m_Seconds <= 0.0f )
      {
         Stop( );
         GetChannel( )->SendEvent( "Alarm", ArgList(this) );
      }
   }
}
