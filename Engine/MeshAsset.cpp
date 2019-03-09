#include "EnginePch.h"

#include "MeshAsset.h"
#include "IOStreams.h"
#include "Geometry.h"
#include "Skeleton.h"
#include "Renderer.h"
#include "Viewport.h"
#include "MaterialAsset.h"
#include "DebugGraphics.h"
#include "ColliderAsset.h"
#include "VertexBuffer.h"

DefineResourceType(Mesh, Asset, NULL);

void Surface::ComputeSkin(
    Matrix *pDestMatrices,
    const Transform &transform,
    const Matrix *pInvTransforms,
    const Matrix *pTransforms,
    int numTransforms
    ) const
{
    Matrix matrices[ 128 ];
    uint32 i;

    for ( i = 0; i < m_NumTransforms; i++ )
    {
        int index = (int) m_pTransformPalette[ i ];

        if ( index < numTransforms )
            Math::Multiply( &matrices[ i ], pInvTransforms[ index ], pTransforms[ index ] );
        else
            matrices[ i ] = Math::IdentityMatrix( );

        //static float size = 2.0f;
        //DebugGraphics::Instance( ).RenderTransform( Transform(pTransforms[index]), size );
    }

    //DebugGraphics::Instance( ).RenderTransform( Math::IdentityTransform( ), 100.0f );
    memcpy( pDestMatrices, matrices, m_NumTransforms * sizeof(Matrix) );
}

const VertexElementDesc *Surface::GetElementDesc( void ) const
{
   return m_pVertexBuffer->GetElementDesc( );
}

VertexContext Surface::GetVertexContext( void ) const
{
   return m_pVertexBuffer->GetVertexContext( );
}

void Surface::Render( 
   GpuDevice::CommandList *pCommandList,
   RenderStats *pStats
   ) const
{
    m_pVertexBuffer->Submit( pCommandList, pStats );
}

void Surface::Destroy( void )
{
    m_pVertexBuffer->Destroy( );
    delete m_pVertexBuffer;

    free( m_pTransformPalette );
}

void Mesh::Destroy( void )
{
    uint32 i;

    for ( i = 0; i < m_NumSurfaces; i++ )
        m_pSurfaces[ i ].Destroy( );

    if ( m_pFullSkeleton )
        m_pFullSkeleton->Destroy( );

    if ( m_pInvSkeleton )
        m_pInvSkeleton->Destroy( );

    delete [] m_pSurfaces;

    delete m_pFullSkeleton;
    delete m_pInvSkeleton;

    Asset::Destroy( );
}

ISerializable *MeshSerializer::Deserialize(
    Serializer *pSerializer,
    ISerializable *pSerializable
    )
{
    if ( NULL == pSerializable ) pSerializable = Instantiate( ); 

    Mesh *pMesh = (Mesh *) pSerializable;

    struct ModelHeader
    {
        unsigned int numSurfaces;
        unsigned int numBones;

        float cx, cy, cz;
        float ex, ey, ez;
    };

    ModelHeader header;

    pSerializer->GetInputStream( )->Read( &header, sizeof(header), NULL );

    pMesh->m_NumSurfaces = header.numSurfaces;
    pMesh->m_pSurfaces   = new Surface[ header.numSurfaces ];
    pMesh->m_Center  = Vector(header.cx, header.cy, header.cz);
    pMesh->m_Extents = Vector(header.ex, header.ey, header.ez);
    
    uint32 i;
    SurfaceSerializer serializer;

    for ( i = 0; i < header.numSurfaces; i++ )
        serializer.Read( pSerializer, &pMesh->m_pSurfaces[ i ] );

    if ( header.numBones )
    {
        SkeletonSerializer serializer;

        const char **pBoneNames = (const char **) malloc( header.numBones * sizeof(char *) );
        
        for ( i = 0; i < header.numBones; i++ )
            pBoneNames[ i ] = StringRef(Id::Deserialize(pSerializer->GetInputStream()).ToString());

        Skeleton skeleton;
        serializer.Read( pSerializer, &skeleton );

        int indices[ 256 ];
        Debug::Assert( Condition(header.numBones <= sizeof(indices) / sizeof(indices[0])), "Up the bone index count in code" );
        
        int numBones = Math::Min( (int) header.numBones, (int) sizeof(indices) / sizeof(indices[0]) );
        pSerializer->GetInputStream()->Read( indices, numBones * sizeof(indices[0]) );

        pMesh->m_pFullSkeleton = new FullSkeleton;
        pMesh->m_pFullSkeleton->Create( skeleton, pBoneNames, indices );

        free( pBoneNames );

        pMesh->m_pInvSkeleton = new Skeleton;
        serializer.Read( pSerializer, pMesh->m_pInvSkeleton );
    }
    else
    {
        pMesh->m_pFullSkeleton = NULL;
        pMesh->m_pInvSkeleton  = NULL;
    }

    return pMesh;
}

void SurfaceSerializer::Read(
    Serializer *pSerializer,
    Surface *pSurface
    )
{
    struct SurfaceHeader
    {
        unsigned int numIndices;
        unsigned int numTransforms;
        unsigned int vertexBufferSize;
        unsigned int declMask;

        StreamDecl positionDecl;
        StreamDecl normalDecl;
        StreamDecl uv0Decl;
        StreamDecl uv1Decl;
        StreamDecl colorDecl;
        StreamDecl binormalDecl;
        StreamDecl tangentDecl;
        StreamDecl boneIndexDecl;
        StreamDecl boneWeightDecl;
    };

    SurfaceHeader header;

    pSerializer->GetInputStream( )->Read( &header, sizeof(header), NULL );

    pSurface->m_NumTransforms = header.numTransforms;

    void *pVertices = malloc( header.vertexBufferSize );
    void *pIndices  = malloc( header.numIndices * sizeof(uint32) );

    if ( pSurface->m_NumTransforms )
    {
        pSurface->m_pTransformPalette = (unsigned char *) malloc( pSurface->m_NumTransforms * sizeof(unsigned char) );
        pSerializer->GetInputStream( )->Read( pSurface->m_pTransformPalette, header.numTransforms * sizeof(unsigned char), NULL );
    }
    else
    {
        pSurface->m_pTransformPalette = NULL;
    }

    pSerializer->GetInputStream( )->Read( pIndices,  header.numIndices * sizeof(uint32), NULL );
    pSerializer->GetInputStream( )->Read( pVertices, header.vertexBufferSize, NULL );

    pSurface->m_pVertexBuffer = new VertexBuffer;

    pSurface->m_pVertexBuffer->Create( );
    pSurface->m_pVertexBuffer->SetVertices( pVertices, header.vertexBufferSize );
    pSurface->m_pVertexBuffer->SetIndices ( pIndices, header.numIndices * sizeof(uint32), StreamDecl::UInt, header.numIndices );

    if ( header.positionDecl.numElements > 0 )   pSurface->m_pVertexBuffer->AddAttributes( StreamDecl::Positions, header.positionDecl );
    if ( header.normalDecl.numElements > 0 )     pSurface->m_pVertexBuffer->AddAttributes( StreamDecl::Normals, header.normalDecl );
    if ( header.uv0Decl.numElements > 0 )        pSurface->m_pVertexBuffer->AddAttributes( StreamDecl::UV0s, header.uv0Decl );
    if ( header.uv1Decl.numElements > 0 )        pSurface->m_pVertexBuffer->AddAttributes( StreamDecl::UV1s, header.uv1Decl );
    if ( header.colorDecl.numElements > 0 )      pSurface->m_pVertexBuffer->AddAttributes( StreamDecl::Colors, header.colorDecl );
    if ( header.binormalDecl.numElements > 0 )   pSurface->m_pVertexBuffer->AddAttributes( StreamDecl::Binormals, header.binormalDecl );
    if ( header.tangentDecl.numElements > 0 )    pSurface->m_pVertexBuffer->AddAttributes( StreamDecl::Tangents, header.tangentDecl );
    if ( header.boneIndexDecl.numElements > 0 )  pSurface->m_pVertexBuffer->AddAttributes( StreamDecl::BoneIndices, header.boneIndexDecl );
    if ( header.boneWeightDecl.numElements > 0 ) pSurface->m_pVertexBuffer->AddAttributes( StreamDecl::BoneWeights, header.boneWeightDecl );

    pSurface->m_pVertexBuffer->EndAttributes( );

    free( pVertices );
    free( pIndices );
}
