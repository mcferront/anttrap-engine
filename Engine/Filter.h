#pragma once

#include "EngineGlobal.h"

template <typename t_type>
class Filter
{
private:
   t_type m_StartValue;
   t_type m_EndValue;
   float  m_CurrentTime;
   float  m_TotalTime;
   float  m_InvTotalTime;

public:
   Filter( void )
   {
      m_CurrentTime  = 0.0f;
      m_TotalTime    = 0.0f;
      m_InvTotalTime = 0.0f;
   }

   void Start(
      t_type startValue,
      t_type targetValue,
      float time
   );
   
   t_type Tick(
      float deltaSeconds
   );

   void Tick(
      float deltaSeconds,
      t_type *pT
   );

   void SetNewTarget(
      t_type target
   );

   t_type GetTargetValue() { return m_EndValue; }

   bool IsDone( void );
};

#include "Filter.inl"
