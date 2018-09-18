#include "EnginePch.h"

#include "SceneAsset.h"
#include "Node.h"

DefineResourceType(Scene, Asset, new SceneSerializer);

void Scene::Create( )
{
    m_Nodes.Create( );
}

void Scene::Destroy( void )
{
    DestroyNodes( );
    
    m_Stream.Close( );

    Asset::Destroy( );
}

void Scene::Reload( void )
{
    RemoveFromScene( );
    DestroyNodes( );

    m_Nodes.Create( );

    Deserialize( );

    AddToScene( );
    Bind( );
}

void Scene::Bind( void )
{
    Asset::Bind( );

    List<Node*>::Enumerator e = m_Nodes.GetEnumerator( );

    while ( e.EnumNext() )
        e.Data( )->Bind( );
}

void Scene::AddToScene( void )
{
    Asset::AddToScene( );

    List<Node*>::Enumerator e = m_Nodes.GetEnumerator( );

    while ( e.EnumNext() )
        e.Data( )->AddToScene( );
}

void Scene::RemoveFromScene( void )
{
    List<Node*>::Enumerator e = m_Nodes.GetEnumerator( );

    while ( e.EnumNext() )
        e.Data( )->RemoveFromScene( );

    Asset::RemoveFromScene( );
}

void Scene::AddNode(
    Node *pNode
)
{
    m_Nodes.Add( pNode );
}

void Scene::RemoveNode(
    Node *pNode
)
{
    m_Nodes.Remove( pNode );
}

void Scene::DestroyNodes( void )
{
    List<Node*> nodes;
    nodes.Create( );

    nodes.CopyFrom( m_Nodes );

    m_Nodes.Clear( );

    //Copy list because calling destroy
    //on these nodes will cause them to attempt
    //to remove themselves from us
    {
        List<Node*>::Enumerator e = nodes.GetEnumerator( );

        while ( e.EnumNext() )
        {
            e.Data( )->DetachChildren( );
            e.Data( )->Destroy( );
            delete e.Data( );
        }
    }

    nodes.Destroy( );
    m_Nodes.Destroy( );
}

void Scene::Deserialize( void )
{
    int count;

    m_Stream.Seek( 0, SeekBegin );

    Serializer serializer( (IInputStream *) &m_Stream );
    
    serializer.GetInputStream()->Read( &count, sizeof(count) );

    for ( int i = 0; i < count; i++ )
    {
        Node *pNode = (Node *) serializer.Deserialize( NULL );
        pNode->AttachToScene( this );
    }
}

ISerializable *SceneSerializer::Deserialize(
   Serializer *pSerializer,
   ISerializable *pSerializable
)
{
    size_t size;
    pSerializer->GetInputStream( )->Read( &size, sizeof(size) );

    Scene *pScene = new Scene;
    
    pScene->m_Stream.Copy( pSerializer->GetInputStream(), size );

    pScene->Deserialize( );

    return pScene;
}
   