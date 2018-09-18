#include "EnginePch.h"
#include "UtilityClock.h"

float Clock::MarkSample( void )
{
   float time = TestSample( );

   Reset( );

   return time;
}

#ifdef WIN32
void  Clock::Start( void )
{
    unsigned int old = _control87( 0, 0 );
    
    _control87( _PC_64, _MCW_PC );

    QueryPerformanceFrequency( (LARGE_INTEGER *) &m_Frequency );

    _control87( old, 0xffffffff );

    Reset( );
}

float Clock::TestSample( void )
{
    unsigned int old = _control87( 0, 0 );
    
    _control87( _PC_64, _MCW_PC );

    LARGE_INTEGER counter;

    QueryPerformanceCounter( &counter );
   
    counter.QuadPart = counter.QuadPart - m_Zero;

    double result = (counter.QuadPart / (double) m_Frequency);

    _control87( old, 0xffffffff );

    return (float) result;
}

void Clock::Reset( void )
{
    unsigned int old = _control87( 0, 0 );
    
    _control87( _PC_64, _MCW_PC );
    
    QueryPerformanceCounter( (LARGE_INTEGER *) &m_Zero );
    
    _control87( old, 0xffffffff );
}

#elif defined IOS || defined MAC || defined ANDROID
void Clock::Start( void )
{
   m_Frequency = 1000000;

   Reset( );
}

float Clock::TestSample( void )
{
   timeval t;
   struct timezone tz;

   gettimeofday( &t, &tz );

   t.tv_sec  = t.tv_sec - m_Zero.tv_sec;

   //place in a signed int so it can be negative (e.g. now = 1.0, base = 0.9)
   sint64 usec = t.tv_usec - m_Zero.tv_usec;

   uint64 nowTime = t.tv_sec * m_Frequency + usec;

   return (float) (nowTime / (double) m_Frequency);
}

void Clock::Reset( void )
{
   struct timezone tz;

   gettimeofday( &m_Zero, &tz );
}

#elif defined LINUX
void Clock::Start( void )
{
   m_Frequency = 1000000000;

   Reset( );
}

float Clock::TestSample( void )
{
   timespec t;
   clock_gettime( CLOCK_REALTIME, &t );

   t.tv_sec  = t.tv_sec - m_Zero.tv_sec;

   //place in a signed int so it can be negative (e.g. now = 1.0, base = 0.9)
   sint64 nsec = t.tv_nsec - m_Zero.tv_nsec;

   uint64 nowTime = t.tv_sec * m_Frequency + nsec;

   return (float) (nowTime / (double) m_Frequency);
}

void Clock::Reset( void )
{
   clock_gettime( CLOCK_REALTIME, &m_Zero );
}
#else
   #error "Platform not defined"
#endif
