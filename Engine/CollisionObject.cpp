#include "EnginePch.h"

#include "CollisionObject.h"
#include "PhysicsWorld.h"

void CollisionObject::AddToScene( void )
{
   PhysicsWorld::Instance( ).AddObject( this );
}

void CollisionObject::RemoveFromScene( void )
{
   PhysicsWorld::Instance( ).RemoveObject( this );
}