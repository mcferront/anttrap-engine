#pragma once

#include "EngineGlobal.h"

template <typename t_type>
class Lerp
{
private:
   t_type m_StartValue;
   t_type m_EndValue;
   float  m_TotalTime;
   float  m_InvTotalTime;
   float  m_CurrentTime;

public:
   Lerp( void )
   {
      m_CurrentTime  = 0.0f;
      m_TotalTime    = 0.0f;
      m_InvTotalTime = 0.0f;
   }

   void Start(
      t_type startValue,
      t_type endValue,
      float time
   );
   
   t_type Tick(
      float deltaSeconds
   );

   void Tick(
      float deltaSeconds,
      t_type *t
   );

   bool IsDone( void );
};

#include "Lerp.inl"
