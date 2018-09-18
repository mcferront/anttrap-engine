#pragma once

#include "EngineGlobal.h"
#include "Component.h"
#include "SystemId.h"
#include "HashTable.h"
#include "Animation2dComposite.h"
#include "List.h"

class AnimatedTexture;

class Animation2dComponent : public Component
{
public:
   DeclareComponentType(Animation2dComponent);
   
public:
   Animation2dComposite m_Animation;

public:
   Animation2dComponent( void )
   {}

   void Create(
      Id outputTextureId,
      ResourceHandle identityFramemap,
      Channel *pChannel
   );

   void Destroy( void );

   //-1 = infinite
   void Play( 
      ResourceHandle framemapId,
      int playHowManyTimes = 1
   );

   //-1 = infinite
   void QueueNext(
      ResourceHandle framemapId,
      int playHowManyTimes = 1
   );

   void Stop( void );

   void GetDeltaTransform(
      Transform *pTransform
   );

   void AddToScene( void );
   void RemoveFromScene( void );
};
