#include "EnginePch.h"

#include "AnimAsset.h"
#include "IOStreams.h"
#include "Skeleton.h"

DefineResourceType( Anim, Asset, NULL );

void Anim::Destroy( void )
{
    int i;

    for ( i = 0; i < GetBoneCount( ); i++ )
        StringRel( m_pBoneNames[ i ] );

    for ( i = 0; i < m_SkeletonCount; i++ )
        m_pSkeletons[ i ].Destroy( );

    free( m_pTimes );
    free( m_pBoneNames );
    free( m_pSkeletonMappings );
    free( m_pTransformMappings );
    free( m_pSkeletons );
    free( m_pTransforms );

    Asset::Destroy( );
}

ISerializable *AnimSerializer::Deserialize(
    Serializer *pSerializer,
    ISerializable *pSerializable
    )
{
    struct Header
    {
        int   numFrames;
        int   numSkeletons;
        int   numTransforms;
        int   numBones;
    };

    Header header;
    int i;

    if ( NULL == pSerializable ) pSerializable = new Anim;

    const char **pBoneNames;

    pSerializer->GetInputStream( )->Read( &header, sizeof( header ), NULL );

    pBoneNames = (const char **) malloc( header.numBones * sizeof( char* ) );
    for ( i = 0; i < header.numBones; i++ )
        pBoneNames[ i ] = StringPool::Deserialize( pSerializer->GetInputStream() );

    Skeleton *pSkeletons = (Skeleton *) malloc( header.numSkeletons * sizeof( Skeleton ) );

    SkeletonSerializer serializer;
    for ( i = 0; i < header.numSkeletons; i++ )
    {
        serializer.Read( pSerializer, &pSkeletons[ i ] );
    }

    Transform *pTransforms = (Transform *) malloc( sizeof(Transform) * header.numTransforms );

    //convert to transforms until the converter spits out transform typess
    for ( int i = 0; i < header.numTransforms; i++ )
    {
        Matrix matrix;
        pSerializer->GetInputStream( )->Read( &matrix, sizeof( Matrix ) );

        pTransforms[ i ] = Transform( matrix );
    }

    int *pSkeletonMappings = (int *) malloc( sizeof(int) * header.numFrames );
    pSerializer->GetInputStream( )->Read( pSkeletonMappings, sizeof(int) * header.numFrames );

    int *pTransformMappings = (int *) malloc( sizeof(int) * header.numFrames );
    pSerializer->GetInputStream( )->Read( pTransformMappings, sizeof(int) * header.numFrames );

    float *pTimes = (float *) malloc( sizeof(float) * header.numFrames );
    pSerializer->GetInputStream( )->Read( pTimes, sizeof(int) * header.numFrames );

    Anim *pAnim = (Anim *) pSerializable;

    pAnim->m_FrameCount = header.numFrames;
    pAnim->m_SkeletonCount = header.numSkeletons;
    pAnim->m_TransformCount = header.numTransforms;

    pAnim->m_pBoneNames = pBoneNames;
    pAnim->m_pSkeletonMappings = pSkeletonMappings;
    pAnim->m_pTransformMappings = pTransformMappings;
    pAnim->m_pTimes = pTimes;
    pAnim->m_pSkeletons = pSkeletons;
    pAnim->m_pTransforms = pTransforms;

    return pSerializable;
}
