#include "EnginePch.h"

#include "FloatController.h"

FloatController::FloatController( void ) 
{
};

FloatController::~FloatController( void )
{
}

void FloatController::Create ( void )
{
   m_Done = true;
}

void FloatController::Destroy( void )
{
}

void FloatController::StartLinear(
   float startValue,
   float endValue,
   float time
)
{
   m_Lerper.Start( startValue, endValue, time );
   m_Value = startValue;

   m_Done = false;
   m_AnimType = FloatController::AnimationType_Linear;
}

void FloatController::StartFiltered(
   float startValue,
   float endValue,
   float time
)
{
   m_Filter.Start( startValue, endValue, time );
   m_Value = startValue;
  
   m_Done = false;
   m_AnimType = FloatController::AnimationType_Filtered;
}

void FloatController::Update(
   float deltaSeconds
)
{ 
   if ( false == m_Done )
   {
      if (m_AnimType == FloatController::AnimationType_Linear)
      {
         m_Value = m_Lerper.Tick( deltaSeconds );
         
         if ( m_Lerper.IsDone( ) )  
         {
            m_Done = true;
         }
      }
      else
      {
         m_Value = m_Filter.Tick( deltaSeconds );
         
         if ( m_Filter.IsDone( ) )
         {
            m_Done = true;
         }
      }
   }
}
