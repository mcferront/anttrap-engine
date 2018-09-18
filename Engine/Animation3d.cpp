#include "EnginePch.h"

#include "Animation3d.h"
#include "AnimAsset.h"
#include "Skeleton.h"

void Animation3d::Create(
   ResourceHandle animation
)
{
   m_Animation   = animation;
   m_CurrentTime = 0;
}

void Animation3d::Destroy( void )
{
   m_Animation = (int)NULL;
}

void Animation3d::Update( 
   float deltaSeconds
)
{
   float duration = GetDuration( );

   m_CurrentTime += deltaSeconds;

   if ( m_CurrentTime >= duration )
   {
      float looped = duration ? m_CurrentTime / duration : m_CurrentTime;      
      m_CurrentTime = m_CurrentTime - (floorf(looped) * duration);
   }
}

void Animation3d::GetDeltaTransform(
   float startTime,
   float endTime,
   Transform *pDeltaTransform
) const
{
   Anim *pAnimation = GetResource( m_Animation, Anim );

   float weight;
   int frameA, frameB;

   pAnimation->GetFrame( m_CurrentTime, &frameA, &frameB, &weight );

   const Transform *pTransformA = pAnimation->GetTransform( frameA );
   const Transform *pTransformB = pAnimation->GetTransform( frameB );

   Transform invTransformA;

   Math::Invert  ( &invTransformA, *pTransformA );
   Math::Multiply( pDeltaTransform, invTransformA, *pTransformB );
}

bool Animation3d::WillLoop(
   float deltaSeconds
) const
{
   return (m_CurrentTime + deltaSeconds) >= GetDuration( );
}

float Animation3d::GetDuration( void ) const
{  
   Anim *pAnim = GetResource( m_Animation, Anim );
   return pAnim->GetDuration( );
}

void Animation3d::FillSkeleton(
   FullSkeleton *pSkeleton,
   bool fillBoneNames
) const
{
   Anim *pAnim = GetResource( m_Animation, Anim );

   float weight;
   int frameA, frameB;

   pAnim->GetFrame( m_CurrentTime, &frameA, &frameB, &weight );

   if ( weight < .10f )
   {
      pSkeleton->Copy( *pAnim->GetSkeleton(frameA) );
   }
   else if ( weight > .90f )
   {
      pSkeleton->Copy( *pAnim->GetSkeleton(frameB) );
   }
   else
   {
      pSkeleton->Copy( *pAnim->GetSkeleton(frameA), 1.0f - weight );
      pSkeleton->Blend( *pAnim->GetSkeleton(frameB), weight );
   }

   if ( true == fillBoneNames )
        pSkeleton->FillBoneNames( pAnim->GetBoneNames( ), pAnim->GetBoneCount( ) ); 
}
