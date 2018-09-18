#include "EnginePch.h"

#include "AnimatedTexture.h"
#include "TextureAsset.h"
#include "FrameMapAsset.h"

void AnimatedTexture::Create(
   ResourceHandle frameMapHandle,
   WrappedTexture *pTexture
)
{
   m_FrameMap    = frameMapHandle;
   m_CurrentTime = 0.0f;

   m_pTexture = pTexture;
}

void AnimatedTexture::Destroy( void )
{
   m_FrameMap = (int)NULL;
   m_pTexture = NULL;
}

void AnimatedTexture::Update( 
   float deltaSeconds
)
{
   //get frame map so we can swap textures
   FrameMap *pFrameMap = GetResource( m_FrameMap, FrameMap );

   //calculate new frame
   m_CurrentTime += deltaSeconds;

   uint32 index = (uint32) (m_CurrentTime * pFrameMap->GetFramerate( ));

   index = index % pFrameMap->GetCount( );
   
   Debug::Assert( Condition(false), "This must be redone using the new material system" );
   //m_pTexture->Copy( pFrameMap->GetTexture(index) );
}

void AnimatedTexture::GetDeltaTransform(
   float startTime,
   float endTime,
   Transform *pDeltaTransform
) const
{
   //get frame map so we can swap textures
   FrameMap *pFrameMap = GetResource( m_FrameMap, FrameMap );
   
   const Vector *pMovement = pFrameMap->GetMovementVector( );

   float frames = (endTime - startTime) * pFrameMap->GetCount( );

   Vector translation = *pMovement * (frames / pFrameMap->GetCount());

   *pDeltaTransform = Math::IdentityTransform();
   pDeltaTransform->SetTranslation( translation );
}

bool AnimatedTexture::WillLoop(
   float deltaSeconds
) const
{
   float localTime = m_CurrentTime;
   float duration  = GetDuration( );

   //if we're already looping, reset the local time
   //as if we never looped
   float loopCount = floorf(localTime / duration);
   localTime = localTime - (loopCount * duration);

   //test reset local time to see 
   //if we will loop next frame
   return (localTime + deltaSeconds) >= duration;
}

float AnimatedTexture::GetDuration ( void ) const
{  
   FrameMap *pFrameMap = GetResource( m_FrameMap, FrameMap );
   return pFrameMap->GetCount( ) / pFrameMap->GetFramerate( );
}
