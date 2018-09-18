#include "EnginePch.h"

#include "Animation3dComposite.h"
#include "Animation3d.h"
#include "AnimationWorld.h"
#include "Skeleton.h"
#include "List.h"

void Animation3dComposite::Create( void )
{
    m_pChannel = NULL;

    m_Animations.Create( 16, 16, IdHash, IdCompare );
    m_Fades.Create( 2 );
    m_Fading.Create( );
    m_Queue.Create( );
}

void Animation3dComposite::Destroy( void )
{
    m_Queue.Clear( );

    FadeDesc *pFades[ 16 ];
    m_Fading.CopyTo( pFades, 16 );

    uint32 i, size = m_Fading.GetSize( );

    for ( i = 0; i < size; i++ )
    {
        FadeDesc *pDesc = pFades[ i ];
        pDesc->skeleton.Destroy( );

        m_Fades.Free( pDesc );
    }

    List<Animation3d *> animations;
    animations.Create( );

    {
        Enumerator<Id, Animation3d *> e = m_Animations.GetEnumerator( );

        while ( e.EnumNext( ) )
        {
            animations.Add( e.Data( ) );
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

    m_Skeleton.Destroy( );

    m_Fading.Destroy( );
    m_Queue.Destroy( );

    m_Animations.Destroy( );
    m_Fades.Destroy( );
}

void Animation3dComposite::Bind(
    Channel *pChannel
    )
{
    m_pChannel = pChannel;
}

void Animation3dComposite::Play(
    ResourceHandle animation,
    int playHowManyTimes //= 1
    )
{
    if ( m_pPlayingAnimation != NULL )
    {
        FadeDesc *pDesc = m_Fades.Alloc( );

        pDesc->totalTime = .20f;
        pDesc->invTotalTime = 1.0f / pDesc->totalTime;
        pDesc->currentTime = 0.0f;

        m_pPlayingAnimation->FillSkeleton( &pDesc->skeleton );

        m_Fading.Add( pDesc );
    }

    m_pPlayingAnimation = NULL;
    m_Queue.Clear( );

    if ( 0 != playHowManyTimes )
    {
        Animation3d *pAnimation = CreateAnimation( animation );

        if ( pAnimation->GetDuration( ) > 0 )
        {
            m_pPlayingAnimation = pAnimation;

            m_pPlayingAnimation->SetLocalTime( 0.0f );
            m_pPlayingAnimation->Update( 0.0f );

            m_SkeletonIsDirty = true;
            m_FillBoneNames = true;
            m_PlayHowManyTimes = playHowManyTimes;
        }
    }
}

void Animation3dComposite::QueueNext(
    ResourceHandle animation,
    int playHowManyTimes //= 1
    )
{
    if ( 0 != playHowManyTimes )
    {
        if ( NULL != m_pPlayingAnimation )
        {
            QueueDesc desc =
            {
                CreateAnimation( animation ),
                playHowManyTimes,
            };

            m_Queue.Add( desc );
        }
        else
        {
            Play( animation, playHowManyTimes );
        }
    }
}

void Animation3dComposite::Stop( void )
{
    m_pPlayingAnimation = NULL;
    m_Queue.Clear( );
}

void Animation3dComposite::GetDeltaTransform(
    Transform *pTransform
    )
{
    *pTransform = m_DeltaTransform;
}

void Animation3dComposite::Update(
    float deltaSeconds
    )
{
    m_DeltaTransform = Math::IdentityTransform( );

    if ( m_pPlayingAnimation )
    {
        float localTime = m_pPlayingAnimation->GetLocalTime( );
        float nextTime = localTime + deltaSeconds;

        bool isPlaying = true;

        if ( m_pPlayingAnimation->WillLoop( deltaSeconds ) )
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
                    FadeDesc *pDesc = m_Fades.Alloc( );

                    pDesc->totalTime = .20f;
                    pDesc->invTotalTime = 1.0f / pDesc->totalTime;
                    pDesc->currentTime = 0.0f;

                    m_pPlayingAnimation->FillSkeleton( &pDesc->skeleton );

                    m_Fading.Add( pDesc );

                    QueueDesc desc = m_Queue.GetAt( 0 );

                    m_pPlayingAnimation = desc.pAnimation;
                    m_PlayHowManyTimes = desc.playHowManyTimes;

                    m_Queue.RemoveSorted( (uint32) 0 );

                    m_pPlayingAnimation->SetLocalTime( 0.0f );

                    localTime = 0.0f;
                    nextTime = localTime + deltaSeconds;

                    m_FillBoneNames = true;
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

            m_SkeletonIsDirty = true;
        }
        else
        {
            m_pPlayingAnimation = NULL;
            m_pChannel->SendEvent( "Finished", ArgList( ) );
        }
    }

    int i, size = m_Fading.GetSize( );

    for ( i = 0; i < size; i++ )
    {
        m_Fading.GetAt( i )->currentTime += deltaSeconds;
    }
}

const FullSkeleton *Animation3dComposite::GetSkeleton( void )
{
    if ( true == m_SkeletonIsDirty && NULL != m_pPlayingAnimation )
    {
        int i, size = m_Fading.GetSize( );

        float fadeWeight = 0.0f;

        if ( size )
        {
            FadeDesc *pFades[ 16 ];
            m_Fading.CopyTo( pFades, 16 );

            for ( i = 0; i < size; i++ )
            {
                FadeDesc *pDesc = pFades[ i ];

                if ( pDesc->currentTime >= pDesc->totalTime )
                {
                    pDesc->skeleton.Destroy( );

                    m_Fading.Remove( pDesc );
                    m_Fades.Free( pDesc );
                }
            }

            size = m_Fading.GetSize( );

            for ( i = 0; i < size; i++ )
            {
                FadeDesc *pDesc = m_Fading.GetAt( i );

                fadeWeight += 1.0f - ( pDesc->currentTime * pDesc->invTotalTime );
            }
        }

        m_pPlayingAnimation->FillSkeleton( &m_Skeleton, m_FillBoneNames );
        m_FillBoneNames = false;

        m_Skeleton.Copy( *m_Skeleton.GetSkeleton( ), 1.0f - fadeWeight );

        for ( i = 0; i < size; i++ )
        {
            FadeDesc *pDesc = m_Fading.GetAt( i );
            m_Skeleton.Blend( *pDesc->skeleton.GetSkeleton( ), 1.0f - ( pDesc->currentTime * pDesc->invTotalTime ) );
        }
    }

    m_SkeletonIsDirty = false;

    return &m_Skeleton;
}

Animation3d *Animation3dComposite::CreateAnimation(
    ResourceHandle animation
    )
{
    Animation3d *pAnimation;

    if ( false == m_Animations.Get( animation.GetId( ), &pAnimation ) )
    {
        pAnimation = new Animation3d;
        pAnimation->Create( animation );

        m_Animations.Add( pAnimation->GetAnimation( ).GetId( ), pAnimation );
    }

    return pAnimation;
}
