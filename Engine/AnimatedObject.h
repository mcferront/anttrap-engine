#pragma once

#include "EngineGlobal.h"

class AnimatedObject
{
public:
   virtual ~AnimatedObject( void ) {}

   virtual void Update(
      float deltaSeconds
   ) = 0;

   virtual void Create( void ) {}
   virtual void Destroy( void ) {}

   virtual void AddToScene( void );
   virtual void RemoveFromScene( void );
};
