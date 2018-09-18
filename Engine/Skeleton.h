#pragma once

#include "EngineGlobal.h"
#include "HashTable.h"
#include "SystemId.h"

class Skeleton
{
    friend class SkeletonSerializer;

private:
    Matrix *m_pTransforms;
    int m_Count;

public:
    Skeleton( void )
    {
        m_Count = 0;
        m_pTransforms = NULL;
    }

    void Copy(
        const Skeleton &skeleton
        );

    void Copy(
        const Skeleton &skeleton,
        float weight
        );

    void Blend(
        const Skeleton &skeleton,
        float weight
        );

    void Destroy( void );

    const Matrix *GetBones( void ) const { return m_pTransforms; }

    void SetBone( int index, Matrix &matrix ) { m_pTransforms[ index ] = matrix; }
    const Matrix *GetBone( int index ) const { return &m_pTransforms[ index ]; }

    int GetNumBones( void ) const { return m_Count; }
};


class Serializer;

class SkeletonSerializer
{
public:
    void Read(
        Serializer *pSerializer,
        Skeleton *pSkeleton
        );
};

class FullSkeleton
{
private:
    Skeleton  m_Skeleton;
    int       m_MaxBones;
    int      *m_pParentIndices;
    const char **m_pBoneNames;

public:
    FullSkeleton( void )
    {
        m_pParentIndices = NULL;
        m_pBoneNames = NULL;
        m_pParentIndices = NULL;
        m_MaxBones = 0;
    }

    void Create(
        ResourceHandle animAsset
        );
     
    void Create(
        const Skeleton &skeleton,
        const char **pBoneNames,
        const int *pParentIndices
        );

    void Copy(
        const Skeleton &skeleton
        );

    void Copy(
        const Skeleton &skeleton,
        float weight
        );

    void Blend(
        const Skeleton &skeleton,
        float weight
        );

    void FillBoneNames(
        const char **pBoneNames,
        int numBones
        );

    const char *GetBoneName(
        int index
        ) const {
        return m_pBoneNames[ index ];
    }

    const int GetParentIndex(
        int index
        ) const {
        return m_pParentIndices[ index ];
    }

    void Destroy( void );

    const char **GetBoneNames( void ) const { return m_pBoneNames; }

    const Skeleton *GetSkeleton( void ) const { return &m_Skeleton; }

private:
    void GrowTo( 
        int numBones 
        );
};