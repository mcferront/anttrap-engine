#pragma once

#include "EngineGlobal.h"
#include "AnimatedObject.h"

class Channel;

class Timer : public AnimatedObject
{
private:
   Channel *m_pChannel;

   float m_Seconds;
   bool  m_Started;

public:
   Timer ( void );
   ~Timer( void );

public:
   void Start(
      float seconds
   );
   
   void Stop( void );
   
   virtual void Update(
      float deltaSeconds
   );

   Channel *GetChannel( void ) const { return m_pChannel; }
};
