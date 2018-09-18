#include "EnginePch.h"

#include "Skeleton.h"
#include "IOStreams.h"
#include "Serializer.h"
#include "AnimAsset.h"

void Skeleton::Copy(
    const Skeleton &skeleton
    )
{
    if ( m_Count < skeleton.m_Count )
    {
        m_pTransforms = (Matrix *) realloc( m_pTransforms, skeleton.m_Count * sizeof( Matrix ) );
        m_Count = skeleton.m_Count;
    }

    memcpy( m_pTransforms, skeleton.m_pTransforms, skeleton.m_Count * sizeof( Matrix ) );
}

void Skeleton::Copy(
    const Skeleton &skeleton,
    float weight
    )
{
    if ( m_Count < skeleton.m_Count )
    {
        m_pTransforms = (Matrix  *) realloc( m_pTransforms, skeleton.m_Count * sizeof( Matrix ) );
        m_Count = skeleton.m_Count;
    }

    int i;

    for ( i = 0; i < skeleton.m_Count; i++ )
    {
        m_pTransforms[ i ] = skeleton.m_pTransforms[ i ] * weight;
    }
}

void Skeleton::Blend(
    const Skeleton &skeleton,
    float weight
    )
{
    int i;

    Debug::Assert( Condition( m_Count == skeleton.m_Count ), "Skeleton is trying to blend a skeleton with a different bone count (%d and %d).  "
        "Partial blends should be used for this scenario", m_Count, skeleton.m_Count );

    for ( i = 0; i < skeleton.m_Count; i++ )
    {
        m_pTransforms[ i ] += skeleton.m_pTransforms[ i ] * weight;
    }
}

void Skeleton::Destroy( void )
{
    free( m_pTransforms );

    m_pTransforms = NULL;

    m_Count = 0;
}

void SkeletonSerializer::Read(
    Serializer *pSerializer,
    Skeleton *pSkeleton
    )
{
    int count;

    pSerializer->GetInputStream( )->Read( &count, sizeof( count ) );

    pSkeleton->m_Count = count;
    pSkeleton->m_pTransforms = (Matrix *) malloc( sizeof(Matrix) * count );

    pSerializer->GetInputStream( )->Read( pSkeleton->m_pTransforms, sizeof(Matrix) * count );
}

void FullSkeleton::Create(
    ResourceHandle animAsset
    )
{
    Anim *pAnim = GetResource( animAsset, Anim );
    Create( *pAnim->GetSkeleton( 0 ), pAnim->GetBoneNames( ), NULL );
}

void FullSkeleton::Create(
    const Skeleton &skeleton,
    const char **pBoneNames,
    const int *pParentIndices
    )
{
    GrowTo( skeleton.GetNumBones() );

    m_Skeleton.Copy( skeleton );

    int i;

    for ( i = 0; i < skeleton.GetNumBones( ); i++ )
        m_pBoneNames[ i ] = StringRef(pBoneNames[ i ]);

    memcpy( m_pParentIndices, pParentIndices, skeleton.GetNumBones() * sizeof(int) );
}

void FullSkeleton::Copy(
    const Skeleton &skeleton
    )
{
    GrowTo( skeleton.GetNumBones() );
    m_Skeleton.Copy( skeleton );
}

void FullSkeleton::Copy(
    const Skeleton &skeleton,
    float initial_weight
    )
{
    GrowTo( skeleton.GetNumBones() );
    m_Skeleton.Copy( skeleton, initial_weight );
}

void FullSkeleton::Blend(
    const Skeleton &skeleton,
    float weight
    )
{
    GrowTo( skeleton.GetNumBones() );
    m_Skeleton.Blend( skeleton, weight );
}

void FullSkeleton::FillBoneNames(
    const char **pBoneNames,
    int numBones
    )
{
    GrowTo( numBones );

    int i;

    for ( i = 0; i < numBones; i++ )
    {
        StringRel(m_pBoneNames[i]);
        m_pBoneNames[ i ] = StringRef(pBoneNames[ i ]);
    }
}

void FullSkeleton::Destroy( void )
{
    for ( int i = 0; i < m_MaxBones; i++)
        StringRel( m_pBoneNames[i] );

    free( m_pBoneNames );
    free( m_pParentIndices );

    m_Skeleton.Destroy( );
}

void FullSkeleton::GrowTo( 
    int numBones 
    )
{
    if ( numBones > m_MaxBones )
    {
        m_pBoneNames = (const char **) realloc( m_pBoneNames, numBones * sizeof(char *) );
        m_pParentIndices = (int *) realloc( m_pParentIndices, numBones * sizeof(int) );

        for (int i = m_MaxBones; i < numBones; i++)
            m_pBoneNames[i] = NULL;

        m_MaxBones = numBones;
    }

}