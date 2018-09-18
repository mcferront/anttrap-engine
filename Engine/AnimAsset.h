#pragma once

#include "EngineGlobal.h"
#include "Asset.h"
#include "Skeleton.h"

class Anim : public Asset
{
    friend class AnimSerializer;

private:
    int m_FrameCount;

    const char **m_pBoneNames;
    Skeleton    *m_pSkeletons;
    Transform   *m_pTransforms;

    int   *m_pSkeletonMappings;
    int   *m_pTransformMappings;
    float *m_pTimes;

    int m_SkeletonCount;
    int m_TransformCount;

public:
    virtual void Destroy( void );

    int GetCount( void ) const { return m_FrameCount; }

    float GetDuration( void ) const { return m_pTimes[ m_FrameCount - 1 ]; }

    void GetFrame( float time, int *pFrameLow, int *pFrameHi, float *pWeight )
    {
        *pFrameLow = 0;
        *pFrameHi = 0;
        *pWeight = 0;

        for ( int i = 0; i < m_FrameCount; i++ )
        {
            if ( m_pTimes[ i ] <= time ) *pFrameLow = i;
            else if ( m_pTimes[ i ] >= time )
            {
                *pFrameHi = i;

                float range = ( m_pTimes[ *pFrameHi ] - m_pTimes[ *pFrameLow ] );
                *pWeight = ( time - m_pTimes[ *pFrameLow ] ) / range;
                break;
            }
        }
    }

    const Transform *GetTransform( int frame ) const
    {
        int index = Math::Clamp( frame, 0, m_FrameCount - 1 );
        index = m_pTransformMappings[ index ];

        return &m_pTransforms[ index ];
    }

    const Skeleton *GetSkeleton( int frame ) const
    {
        int index = Math::Clamp( frame, 0, m_FrameCount - 1 );
        index = m_pSkeletonMappings[ index ];

        return &m_pSkeletons[ index ];
    }

    const char **GetBoneNames( ) const
    {
        return m_pBoneNames;
    }

    int GetBoneCount( ) const
    {
        return m_pSkeletons[ 0 ].GetNumBones( );
    }

    DeclareResourceType( Anim );
};

class AnimSerializer
{
public:
    ISerializable *Deserialize(
        Serializer *pSerializer,
        ISerializable *pSerializable
        );
};
