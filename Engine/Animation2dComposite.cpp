#include "EnginePch.h"

#include "Animation2dComposite.h"
#include "AnimatedTexture.h"
#include "AnimationWorld.h"
#include "List.h"
#include "FrameMapAsset.h"

void Animation2dComposite::Create(
   Id outputTextureId,
   ResourceHandle identityFramemap,
   Channel *pChannel
)
{
   m_Animations.Create( 16, 16, IdHash, IdCompare );
   
   m_pOutputTexture = new WrappedTexture;
   m_pOutputTexture->Create( );

   if ( identityFramemap != (int)NULL )
   {
      FrameMap *pFrameMap = GetResource( identityFramemap, FrameMap );
      m_pOutputTexture->Copy( pFrameMap->GetTexture(0) );
   }

   m_pChannel = pChannel;
}

void Animation2dComposite::Destroy( void )
{
   m_Queue.Clear( );

   uint32 i, size;

   List<AnimatedTexture *> animations;
   animations.Create( );

   {
      Enumerator<Id, AnimatedTexture *> e = m_Animations.GetEnumerator( );

      while ( e.EnumNext( ) )
      {
         animations.Add( e.Data() );
      }
   }

   m_Animations.Clear( );

   size = animations.GetSize( );

   for ( i = 0; i < size; i++ )
   {
      animations.GetAt( i )->Destroy( );
      delete animations.GetAt( i );
   }

   animations.Destroy( );

   m_pOutputTexture->Destroy( );
   delete m_pOutputTexture;

   m_Animations.Destroy( );
}

void Animation2dComposite::Play( 
   ResourceHandle framemap,
   int playHowManyTimes //= 1
)
{
   m_pPlayingAnimation = NULL;
   m_Queue.Clear( );
   
   if ( 0 != playHowManyTimes )
   {
      AnimatedTexture *pAnimation = CreateAnimation( framemap );

      if ( pAnimation->GetDuration( ) > 0 )
      {
         m_pPlayingAnimation = pAnimation;

         m_pPlayingAnimation->SetLocalTime( 0.0f );
         m_pPlayingAnimation->Update( 0.0f );

         m_PlayHowManyTimes = playHowManyTimes;
      }
   }
}

void Animation2dComposite::QueueNext(
   ResourceHandle framemap,
   int playHowManyTimes //= 1
)
{
   if ( 0 != playHowManyTimes )
   {
      if ( NULL != m_pPlayingAnimation )
      {
         QueueDesc desc = 
         {
            CreateAnimation(framemap),
            playHowManyTimes,
         };

         m_Queue.Add( desc );
      }
      else
      {
         Play( framemap, playHowManyTimes );
      }
   }
}

void Animation2dComposite::Stop( void )
{
   m_pPlayingAnimation = NULL;
   m_Queue.Clear( );
}

void Animation2dComposite::GetDeltaTransform(
   Transform *pTransform
)
{
   *pTransform = m_DeltaTransform;
}

void Animation2dComposite::Update(
   float deltaSeconds
)
{
   m_DeltaTransform = Math::IdentityTransform();

   if ( m_pPlayingAnimation )
   {
      bool isPlaying = true;

      float localTime = m_pPlayingAnimation->GetLocalTime( );
      float nextTime  = localTime + deltaSeconds;

      if ( m_pPlayingAnimation->WillLoop(deltaSeconds) )
      {
         isPlaying = false;

         if ( m_PlayHowManyTimes > 0 ) 
         {
            --m_PlayHowManyTimes;
         }
      
         if ( 0 == m_PlayHowManyTimes || -1 == m_PlayHowManyTimes )
         {
            if ( m_Queue.GetSize( ) )
            {
               QueueDesc desc = m_Queue.GetAt( 0 );
               
               m_pPlayingAnimation = desc.pAnimation;
               m_PlayHowManyTimes  = desc.playHowManyTimes;
               
               m_Queue.RemoveSorted( (uint32) 0 );

               m_pPlayingAnimation->SetLocalTime( 0.0f );

               localTime = 0.0f;
               nextTime  = localTime + deltaSeconds;

               isPlaying = true;
            }
            else if ( -1 == m_PlayHowManyTimes )
            {
               //if there is nothing queued
               //but we're infinitly looping
               //keep it going
               isPlaying = true;
            }
         }
         else if ( m_PlayHowManyTimes > 0 )
         {
            isPlaying = true;
         }
      }      

      if ( true == isPlaying )
      {
         m_pPlayingAnimation->GetDeltaTransform( localTime, nextTime, &m_DeltaTransform );
         m_pPlayingAnimation->Update( deltaSeconds );  
      }
      else
      {
         m_pPlayingAnimation = NULL;
         m_pChannel->SendEvent( "Finished", ArgList() );
      }
   }
}

void Animation2dComposite::AddToScene( void )
{
   AnimatedObject::AddToScene( );

   m_pOutputTexture->AddToScene( );
}

void Animation2dComposite::RemoveFromScene( void )
{
   m_pOutputTexture->RemoveFromScene( );

   AnimatedObject::RemoveFromScene( );
}

AnimatedTexture *Animation2dComposite::CreateAnimation(
   ResourceHandle framemap
)
{
   AnimatedTexture *pAnimation;

   if ( false == m_Animations.Get(framemap.GetId( ), &pAnimation) )
   {
      pAnimation = new AnimatedTexture;
      pAnimation->Create( framemap, m_pOutputTexture );
      
      m_Animations.Add( framemap.GetId(), pAnimation );
   }

   return pAnimation;
}
