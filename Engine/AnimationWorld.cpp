#include "EnginePch.h"

#include "AnimationWorld.h"
#include "AnimatedObject.h"
#include "Skeleton.h"

AnimationWorld &AnimationWorld::Instance( void )
{
   static AnimationWorld s_instance;
   return s_instance;
}

void AnimationWorld::Destroy( void )
{
   m_AnimatedObjectHash.Destroy( );
   //m_FullSkeletonHash.Destroy( );
}

void AnimationWorld::AddObject(
   AnimatedObject *pAnimatedObject
)
{
   if ( false == m_AnimatedObjectHash.Contains((nuint)pAnimatedObject) )
   {
      m_AnimatedObjectHash.Add( (nuint) pAnimatedObject, pAnimatedObject );
   }
}

void AnimationWorld::RemoveObject(
   AnimatedObject *pObject
)
{
   m_AnimatedObjectHash.Remove( (nuint) pObject );
}

void AnimationWorld::Tick(
   float deltaSeconds
)
{
   Enumerator<nuint, AnimatedObject *> e = m_AnimatedObjectHash.GetEnumerator( );

   while ( e.EnumNext( ) )
   {
      e.Data( )->Update( deltaSeconds );
   }
}

