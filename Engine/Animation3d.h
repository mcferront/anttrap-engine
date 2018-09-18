#pragma once

#include "EngineGlobal.h"
#include "Resource.h"
#include "AnimatedObject.h"

class FullSkeleton;

class Animation3d
{
private:
   ResourceHandle  m_Animation;

   float m_CurrentTime;

public:
   void Create(
      ResourceHandle animation
   );

   void Destroy( void );

   virtual void Update( 
      float deltaSeconds
   );

   void GetDeltaTransform(
      float startTime,
      float endTime,
      Transform *pDeltaTransform
   ) const;

   void SetLocalTime( 
      float localTime 
   ) 
   { m_CurrentTime = localTime; }

   bool WillLoop( 
      float deltaSeconds 
   ) const;

   void FillSkeleton(
      FullSkeleton *pSkeleton,
      bool fillBoneNames = false
   ) const;

   float GetDuration ( void ) const;
   float GetLocalTime( void ) const { return m_CurrentTime; }

   ResourceHandle GetAnimation( void ) const { return m_Animation; }
};
