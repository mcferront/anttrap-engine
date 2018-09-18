#pragma once

#include "EngineGlobal.h"
#include "AnimatedObject.h"
#include "Lerp.h"
#include "Filter.h"

class VectorController : public AnimatedObject
{
public:
   enum AnimationType {
      AnimationType_Linear = 0,
      AnimationType_Filtered
   };

private:
   Lerp  <Vector> m_Lerper;
   Filter<Vector> m_Filter;
   AnimationType  m_AnimType;
   Vector         m_Value;
   bool           m_Done;
   
public:
   VectorController ( void );
   ~VectorController( void );

   virtual void Create ( void );
   virtual void Destroy( void );

   void StartLinear(
      const Vector &startValue,
      const Vector &endValue,
      float time
   );

   void StartFiltered(
      const Vector &startValue,
      const Vector &endValue,
      float time
   );

   virtual void Update(
      float deltaSeconds
   );

   bool IsDone( void ) const{ return m_Done; }

   void GetValue( 
      Vector *pVector 
   ) const 
   { *pVector = m_Value; }
};
