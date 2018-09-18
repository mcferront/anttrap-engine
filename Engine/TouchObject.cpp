#include "EnginePch.h"
#include "TouchObject.h"
#include "TouchWorld.h"

void TouchObject::AddToScene( 
   Id touchStageId
)
{
   TouchWorld::Instance( ).AddObject( touchStageId, this );
}

void TouchObject::RemoveFromScene( void )
{
   TouchWorld::Instance( ).RemoveObject( this );
}
