#pragma once

#include "EngineGlobal.h"
#include "AnimatedObject.h"
#include "Lerp.h"
#include "Filter.h"

class FloatController : public AnimatedObject
{
public:
   enum AnimationType {
      AnimationType_Linear = 0,
      AnimationType_Filtered
   };

private:
   Lerp  <float> m_Lerper;
   Filter<float> m_Filter;
   AnimationType m_AnimType;
   float         m_Value;
   bool m_Done;
   
public:
   FloatController ( void );
   ~FloatController( void );

   virtual void Create ( void );
   virtual void Destroy( void );

   void StartLinear(
      float startValue,
      float endValue,
      float time
   );

   void StartFiltered(
      float startValue,
      float endValue,
      float time
   );

   virtual void Update(
      float deltaSeconds
   );

   bool IsDone( void ) const{ return m_Done; }

   float GetValue( void ) const { return m_Value; }
};
