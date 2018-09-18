#include "EnginePch.h"

#include "AnimatedObject.h"
#include "AnimationWorld.h"

void AnimatedObject::AddToScene( void )
{
   AnimationWorld::Instance( ).AddObject( this );
}

void AnimatedObject::RemoveFromScene( void )
{
   AnimationWorld::Instance( ).RemoveObject( this );
}

