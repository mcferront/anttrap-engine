#include "EnginePch.h"

#include "VectorController.h"

VectorController::VectorController( void ) 
{
};

VectorController::~VectorController( void )
{
}

void VectorController::Create ( void )
{
   m_Done = true;
}

void VectorController::Destroy( void )
{
}

void VectorController::StartLinear(
   const Vector &startValue,
   const Vector &endValue,
   float time
)
{
   m_Lerper.Start( startValue, endValue, time );
   m_Value = startValue;

   m_Done = false;
   m_AnimType = VectorController::AnimationType_Linear;
}

void VectorController::StartFiltered(
   const Vector &startValue,
   const Vector &endValue,
   float time
)
{
   m_Filter.Start( startValue, endValue, time );
   m_Value = startValue;
  
   m_Done = false;
   m_AnimType = VectorController::AnimationType_Filtered;
}

void VectorController::Update(
   float deltaSeconds
)
{ 
   if ( false == m_Done )
   {
      if (m_AnimType == VectorController::AnimationType_Linear)
      {
         m_Lerper.Tick( deltaSeconds, &m_Value );
         
         if ( m_Lerper.IsDone( ) )  
         {
            m_Done = true;
         }
      }
      else
      {
         m_Filter.Tick( deltaSeconds, &m_Value );
         
         if ( m_Filter.IsDone( ) )
         {
            m_Done = true;
         }
      }
   }
}
