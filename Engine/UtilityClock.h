#pragma once

#include "EngineGlobal.h"
#include "Types.h"

class Clock
{
private:
#ifdef WIN32
   uint64 m_Zero;
#elif defined LINUX
   timespec m_Zero;
#else
   timeval  m_Zero;
#endif

   uint64 m_Frequency;

public:
   void  Start     ( void );
   float TestSample( void );
   float MarkSample( void );
   void  Reset     ( void );

private:
};
