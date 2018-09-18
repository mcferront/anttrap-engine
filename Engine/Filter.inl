#include "Filter.h"

template <typename t_type>
void Filter<t_type>::Start(
   t_type startValue,
   t_type targetValue,
   float time
)
{
   m_StartValue   = startValue;
   m_EndValue     = targetValue;
   m_TotalTime    = time;
   m_CurrentTime  = 0.0f;

   if ( time ) m_InvTotalTime = 1.0f / time;
   else m_InvTotalTime = 0.0f;
}

template <typename t_type>
t_type Filter<t_type>::Tick(
   float deltaSeconds
)
{ 
   t_type value;
   Tick( deltaSeconds, &value );

   return value;
}

template <typename t_type>
void Filter<t_type>::Tick(
   float deltaSeconds,
   t_type *pT
)
{ 
   m_CurrentTime += deltaSeconds;
   m_CurrentTime = Math::Min( m_CurrentTime, m_TotalTime );
   
   float time   = (m_CurrentTime * m_InvTotalTime);

   //time^4 to give an 'ease in'
   time = 1.0f - time;
   time = time * time * time * time;
   time = 1.0f - time;

   Math::Lerp( pT, m_StartValue, m_EndValue, time );  
}

template <typename t_type>
void Filter<t_type>::SetNewTarget(
   t_type target
)
{
   m_EndValue = target;
}

template <typename t_type>
bool Filter<t_type>::IsDone( void )
{
   return ( m_CurrentTime == m_TotalTime );
}
