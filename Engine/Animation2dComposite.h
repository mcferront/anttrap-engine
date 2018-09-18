#pragma once

#include "EngineGlobal.h"
#include "AnimatedObject.h"
#include "SystemId.h"
#include "HashTable.h"
#include "List.h"

class AnimatedTexture;
class WrappedTexture;
class Channel;

class Animation2dComposite : public AnimatedObject
{
private:
   struct QueueDesc
   {
      AnimatedTexture *pAnimation;
      int playHowManyTimes;
   };
   
private:
   HashTable<Id,  AnimatedTexture *> m_Animations;
   List<QueueDesc>      m_Queue;
   Transform            m_DeltaTransform;

   AnimatedTexture *m_pPlayingAnimation;
   Channel         *m_pChannel;
   WrappedTexture  *m_pOutputTexture;

   int  m_PlayHowManyTimes;

public:
   Animation2dComposite( void )
   {
      m_PlayHowManyTimes  = false;
      m_pPlayingAnimation = NULL;
   }

   void Create( 
      Id outputTextureId,
      ResourceHandle identityFramemap,
      Channel *pChannel
   );

   void Destroy( void );

   //-1 = infinite
   void Play( 
      ResourceHandle framemap,
      int playHowManyTimes = 1
   );

   //-1 = infinite
   void QueueNext(
      ResourceHandle framemap,
      int playHowManyTimes = 1
   );

   void Stop( void );

   void GetDeltaTransform(
      Transform *pTransform
   );

   void Update(
      float deltaSeconds
   );

   virtual void AddToScene( void );
   virtual void RemoveFromScene( void );

private:
   AnimatedTexture *CreateAnimation(
      ResourceHandle framemap
   );
};
