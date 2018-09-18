#pragma once

#include "EngineGlobal.h"
#include "HashTable.h"
#include "SystemId.h"

class AnimatedObject;
class FullSkeleton;

class AnimationWorld
{
private:
   typedef HashTable<nuint,    AnimatedObject *> AnimatedObjectHash;
   //typedef HashTable<Id, const FullSkeleton *> FullSkeletonHash;

public:
   static AnimationWorld &Instance( void );

private:
   AnimatedObjectHash m_AnimatedObjectHash;
   //FullSkeletonHash   m_FullSkeletonHash;

public:
   AnimationWorld( void )
   {}
   
   void Create ( void ) 
   {
      m_AnimatedObjectHash.Create( 16, 16, HashFunctions::NUIntHash, HashFunctions::NUIntCompare );   
      //m_FullSkeletonHash.Create( 16, 16, IdHash, IdCompare );   
   }

   void Destroy( void );

   void AddObject(
      AnimatedObject *pObject
   );

   void RemoveObject(
      AnimatedObject *pObject
   );

   void Tick(
      float deltaSeconds
   );
};