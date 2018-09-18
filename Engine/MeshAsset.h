#pragma once

#include "EngineGlobal.h"
#include "MaterialAsset.h"
#include "Asset.h"

class Collider;
struct BoneInfluence;
struct Vertex;

class VertexBuffer;
class Material;
class Skeleton;
class FullSkeleton;

class Surface
{
    friend class Mesh;
    friend class SurfaceSerializer;

private:
    uint32 m_NumTransforms;
    VertexContext m_VertexContext;
    VertexBuffer *m_pVertexBuffer;
    unsigned char *m_pTransformPalette;

public:
    void ComputeSkin(
        Matrix *pDestMatrices,
        const Transform &transform,
        const Matrix *pInvTransform,
        const Matrix *pTransforms,
        int numTransforms
        ) const;

    void Render( 
       GpuDevice::CommandList *pList,
       RenderStats *pStats
      ) const;

    const VertexElementDesc *GetElementDesc( void ) const;

    VertexContext GetVertexContext( void ) const;

private:
    void Destroy( void );
};

class Mesh : public Asset
{
private:
    friend class MeshSerializer;

private:
    FullSkeleton  *m_pFullSkeleton;
    Skeleton      *m_pInvSkeleton;
    Surface       *m_pSurfaces;
    uint32         m_NumSurfaces;
    Vector         m_Center;
    Vector         m_Extents;

public:
    uint32 GetNumSurfaces( void ) const { return m_NumSurfaces; }

    const Surface      *GetSurface( uint32 index ) const { return &m_pSurfaces[ index ]; }
    const Skeleton     *GetInvSkeleton( void ) const { return m_pInvSkeleton; }
    const FullSkeleton *GetSkeleton   ( void ) const { return m_pFullSkeleton; }

    const Vector *GetCenter ( void ) const { return &m_Center; }
    const Vector *GetExtents( void ) const { return &m_Extents; }

    virtual void Destroy( void );

    DeclareResourceType(Mesh);
};

class MeshSerializer : public ISerializer
{
public:
    virtual bool Serialize(
        Serializer *pSerializer,
        const ISerializable *pSerializable
        )
    {
        return false;
    }

    virtual ISerializable *Deserialize(
        Serializer *pSerializer,
        ISerializable *pSerializable
        );

    virtual ISerializable *Instantiate( ) const { return new Mesh; }

    virtual const SerializableType &GetSerializableType( void ) const { return Mesh::StaticSerializableType( ); }

    virtual uint32 GetVersion( void ) const { return 1; }
};

class SurfaceSerializer
{
public:    
    void Read(
        Serializer *pSerializer,
        Surface *pSurface
        );
};
