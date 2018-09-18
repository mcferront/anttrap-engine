#include "EnginePch.h"

#include "MeshRendererComponent.h"
#include "MeshAsset.h"
#include "Node.h"

DefineComponentType( MeshRendererComponent, new MeshRendererComponentSerializer );

void MeshRendererComponent::Create(
    ResourceHandle model,
    const ResourceHandleList &materials,
    const IdList &renderGroups
    )
{
    m_Model.Create( this, renderGroups, model );

    m_Materials.Create( );
    m_Materials.CopyFrom( materials );

    for ( uint32 i = 0; i < m_Materials.GetSize( ); i++ )
       AddMaterial( i, m_Materials.Get( i ) );
}

void MeshRendererComponent::Destroy( void )
{
    m_Model.Destroy( );
    m_Materials.Destroy( );

    Component::Destroy( );
}

void MeshRendererComponent::Bind( void )
{
    Component::Bind( );
    m_Model.Bind( );
}

void MeshRendererComponent::AddMaterial(
    int surface,
    ResourceHandle material
    )
{
    m_Model.AddMaterial( surface, material );
}

void MeshRendererComponent::AddToScene( void )
{
    if ( false == GetParent( )->IsInScene( ) ) return;

    Component::AddToScene( );
    m_Model.AddToScene( );
}

void MeshRendererComponent::RemoveFromScene( void )
{
    m_Model.RemoveFromScene( );

    Component::RemoveFromScene( );
}

void MeshRendererComponent::CreateNodesFromBones( void )
{
    Mesh *pMesh = GetResource( m_Model.GetModel( ), Mesh );

    // need parent bone information
    int bones = pMesh->GetSkeleton( )->GetSkeleton( )->GetNumBones( );

    struct Bone { Node *pNode; int parentIndex; };

    List<Bone> nodes; nodes.Create( );

    for ( int i = 0; i < bones; i++ )
    {
        const char *pBoneName = pMesh->GetSkeleton( )->GetBoneName( i );
        int index = pMesh->GetSkeleton( )->GetParentIndex( i );

        Node *pNode = GetParent( )->FindChildByName( pBoneName );

        if ( NULL == pNode )
        {
            pNode = new Node;
            pNode->Create( );

            ResourceHandle handle( Id::Create( ) );
            handle.Bind( pBoneName, pNode );
        }

        Bone bone { pNode, index };
        nodes.Add( bone );
    }

    for ( int i = 0; i < bones; i++ )
    {
        Bone b = nodes.GetAt( i );

        if ( -1 == b.parentIndex )
            b.pNode->SetParent( GetParent( ) );
        else
            b.pNode->SetParent( nodes.GetAt( b.parentIndex ).pNode );
    }

    for ( int i = 0; i < bones; i++ )
        nodes.GetAt( i ).pNode->AddToScene( );

    for ( int i = 0; i < bones; i++ )
        nodes.GetAt( i ).pNode->Bind( );

    nodes.Destroy( );
}

ISerializable *MeshRendererComponentSerializer::Deserialize(
    Serializer *pSerializer,
    ISerializable *pSerializable
    )
{
    if ( NULL == pSerializable ) pSerializable = new MeshRendererComponent;
    return NULL;

    //MeshRendererComponent *pMeshRendererComponent = (MeshRendererComponent *) pSerializable;

    //Id id = Id::Deserialize( pSerializer->GetInputStream() );
    //Id modelId = Id::Deserialize( pSerializer->GetInputStream() );

    //IdList renderGroups;
    //Id::DeserializeList( pSerializer->GetInputStream(), &renderGroups );

    //pMeshRendererComponent->Create( id, renderGroups, ResourceHandle(modelId) );

    //int materialCount;
    //pSerializer->GetInputStream()->Read( &materialCount, sizeof(materialCount) );

    //for ( int i = 0; i < materialCount; i++ )
    //{
    //    Id materialId = Id::Deserialize( pSerializer->GetInputStream() );
    //    pMeshRendererComponent->AddMaterial( i, ResourceHandle(materialId) );
    //}

    //return pSerializable;
}
