#include "Lerp.h"

template <typename t_type>
void Lerp<t_type>::Start(
   t_type startValue,
   t_type endValue,
   float time
)
{
   m_StartValue   = startValue;
   m_EndValue     = endValue;
   m_TotalTime    = time;
   m_CurrentTime  = 0.0f;

   if ( time ) m_InvTotalTime = 1.0f / time;
   else m_InvTotalTime = 0.0f;
}


template <typename t_type>
t_type Lerp<t_type>::Tick(
   float deltaSeconds
)
{ 
   t_type value;
   Tick( deltaSeconds, &value );

   return value;
}

template <typename t_type>
void Lerp<t_type>::Tick(
   float deltaSeconds,
   t_type *pT
)
{ 
   m_CurrentTime += deltaSeconds;
   m_CurrentTime = Math::Min( m_CurrentTime, m_TotalTime );
   
   float time = (m_CurrentTime * m_InvTotalTime);
   
   Math::Lerp( pT, m_StartValue, m_EndValue, time );  
}

template <typename t_type>
bool Lerp<t_type>::IsDone( void )
{
   return ( m_CurrentTime == m_TotalTime );
}
