#include "EnginePch.h"

#include "PhysicsObject.h"
#include "PhysicsWorld.h"

void PhysicsObject::AddToScene( void )
{
   PhysicsWorld::Instance( ).AddObject( this );
}

void PhysicsObject::RemoveFromScene( void )
{
   PhysicsWorld::Instance( ).RemoveObject( this );
}
