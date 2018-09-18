#include "EnginePch.h"

#include "RenderObject.h"
#include "RenderWorld.h"

void RenderObject::AddToScene( void )
{
    RenderWorld::Instance( ).AddObject( this );
}

void RenderObject::RemoveFromScene( void )
{
   RenderWorld::Instance( ).RemoveObject( this );
}