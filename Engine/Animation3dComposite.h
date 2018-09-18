#pragma once

#include "EngineGlobal.h"
#include "AnimatedObject.h"
#include "SystemId.h"
#include "HashTable.h"
#include "List.h"
#include "Skeleton.h"

class Animation3d;
class Channel;

class Animation3dComposite : public AnimatedObject
{
private:
    struct FadeDesc
    {
        FullSkeleton skeleton;

        float currentTime;
        float invTotalTime;
        float totalTime;
    };

    struct QueueDesc
    {
        Animation3d *pAnimation;
        int playHowManyTimes;
    };

private:
    HashTable<Id, Animation3d *> m_Animations;
    List<QueueDesc>      m_Queue;
    List<FadeDesc*>      m_Fading;
    MemoryPool<FadeDesc> m_Fades;
    FullSkeleton         m_Skeleton;
    Transform            m_DeltaTransform;

    Channel     *m_pChannel;
    Animation3d *m_pPlayingAnimation;

    int  m_PlayHowManyTimes;
    bool m_SkeletonIsDirty;
    bool m_FillBoneNames;

public:
    Animation3dComposite( void )
    {
        m_SkeletonIsDirty = false;
        m_PlayHowManyTimes = false;
        m_pPlayingAnimation = NULL;
        m_FillBoneNames = false;
    }

    void Create( void );
    void Destroy( void );

    void Bind(
        Channel *pChannel
        );

    //-1 = infinite
    void Play(
        ResourceHandle animation,
        int playHowManyTimes = 1
        );

    //-1 = infinite
    void QueueNext(
        ResourceHandle animation,
        int playHowManyTimes = 1
        );

    void Stop( void );

    void GetDeltaTransform(
        Transform *pTransform
        );

    void Update(
        float deltaSeconds
        );

    const FullSkeleton *GetSkeleton( void );

private:
    Animation3d *CreateAnimation(
        ResourceHandle animation
        );
};
