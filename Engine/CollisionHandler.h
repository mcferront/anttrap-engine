#pragma once

#include "EngineGlobal.h"
#include "HashTable.h"

class CollisionObject;

class ICollisionHandler
{
public:
   virtual ~ICollisionHandler( void ) {};

   virtual void Run(
      const CollisionObject *pObjectA,
      const CollisionObject *pObjectB
   ) = 0;
};
