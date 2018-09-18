#include "EnginePch.h"

#include "LuaCoreModule.h"
#include "DebugGraphics.h"
#include "FileStreams.h"
#include "Database.h"

bool CastRay(
    RaycastResult *pResult,
    const Vector &start,
    const Vector &direction,
    float length,
    const char *pLayer,
    bool render
)
{
    Raycast cast( start, direction, length, pLayer );

    bool result = cast.Cast( pResult );

    if ( true == render )
    {
        if ( true == result )
        {
            DebugGraphics::Instance( ).RenderLine( start, pResult->point, Vector( 1, 0, 0, 1 ) );

            Transform t = Math::IdentityTransform( );
            t.SetTranslation( pResult->point );
            t.SetOrientation( pResult->normal, Vector( pResult->normal.x, pResult->normal.z, pResult->normal.y ) );
            DebugGraphics::Instance( ).RenderTransform( t, 4.0f );
        }
        else
        {
            DebugGraphics::Instance( ).RenderLine( start, start + direction * length, Vector( 1, 1, 0, 1 ) );
        }
    }

    return result;
}

void RenderTransform(
    const Vector &position,
    const Vector &up,
    const Vector &look,
    float size
)
{
    Transform t = Math::IdentityTransform( );
    t.SetTranslation( position );
    t.SetOrientation( look, up );

    DebugGraphics::Instance( ).RenderTransform( t, size );
}

void FindAllByCone(
    NodeList *pList,
    const Transform &transform,
    float angle,
    float distance
)
{
    pList->Clear( );

    Enumerator<Id, Node *> e = Node::GetEnumerator( );

    angle = Math::DegreesToRadians( angle );

    while ( e.EnumNext( ) )
    {
        Node *pNode = e.Data( );

        Vector localPosition = pNode->GetWorldTransform( ).GetTranslation( ) - transform.GetTranslation( );
        float  localDistance = Math::DotProduct( transform.GetLook( ), localPosition );

        if ( localDistance > distance ) continue;

        Vector direction;
        Math::Normalize( &direction, localPosition );

        float localAngle = Math::DotProduct( transform.GetLook( ), direction );
        localAngle = Math::ACos( localAngle );

        if ( localAngle > angle ) continue;

        pList->Add( pNode );
    }
}

void FindAllBySphere(
    NodeList *pList,
    const Vector &position,
    float radius
)
{
    pList->Clear( );

    Enumerator<Id, Node *> e = Node::GetEnumerator( );

    while ( e.EnumNext( ) )
    {
        Node *pNode = e.Data( );

        Vector localPosition = pNode->GetWorldTransform( ).GetTranslation( ) - position;
        float  distance = Math::MagnitudeSq( localPosition );

        if ( distance > radius ) continue;

        pList->Add( pNode );
    }
}

int LoadResources(
    const char *pName
)
{
    FileInputStream inputStream;

    char path[ 256 ];

    Database *pDatabase = Database::Create( false );

    BuildPlatformDataPath( path, pName, sizeof( path ), false );

    pDatabase->RequestAll( );

    if ( true == inputStream.Open( path ) )
        pDatabase->Deserialize( &inputStream );
    else
    {
        Database::Destroy( pDatabase );
        pDatabase = NULL;
    }

    inputStream.Close( );

    return (int) pDatabase;
}

void UnloadResources(
    int handle
)
{
    if ( NULL == handle )
        return;

    Database *pDatabase = (Database *) handle;

    pDatabase->UnloadResources( );

    Database::Destroy( pDatabase );
}

#include "MeshAsset.h"
#include "DataAsset.h"
#include "TextReader.h"

//TODO: temp function, it will parse all of the meshes
//out of a database and add them to the scene
static IdList meshes;
static bool mesh_list_created = false;

void AddMeshes(
    int handle
)
{
    if ( mesh_list_created == false )
        meshes.Create( );

    mesh_list_created = true;
    
    Database *pDatabase = (Database *) handle;
    pDatabase->GetResources( Mesh::StaticType( ), &meshes );
}

//TODO: temp function, it will parse all of the meshes
//out of a database and add them to the scene
void PlaceMeshes(
    Node *pParent,
    const char *mappingAlias
)
{
    IdList groups;

    groups.Create( );

    Data *pMappings = GetResource( ResourceHandle::FromAlias( mappingAlias ), Data );

    MemoryStream stream( pMappings->GetData( ), pMappings->GetSize( ) );
    TextReader reader( &stream );

    HashTable<Id, ResourceHandleList *> meshToMaterial;
    meshToMaterial.Create( 128, 128, IdHash, IdCompare );

    while ( const char *pLine = reader.ReadLine( ) )
    {
        if ( *pLine == 0 )
            break;

        char mesh[ 36 + 1 ] = { 0 };
        char material[ 36 + 1 ] { 0 };

        String::Copy( mesh, pLine, 37 );
        pLine += 36 + 3; //" = "

        String::Copy( material, pLine, 37 );

        Id meshId( mesh );
        Id matId( material );

        ResourceHandleList *pList;

        if ( false == meshToMaterial.Get( meshId, &pList ) )
        {
            pList = new ResourceHandleList; pList->Create( );
            meshToMaterial.Add( meshId, pList );
        }

        pList->Add( ResourceHandle( matId ) );
    }

    groups.Add( "Geometry" );

    List<Node*> nodes; nodes.Create( );
    char nodeAlias[ 256 ];

    for ( uint32 i = 0, n = meshes.GetSize( ); i < n; i++ )
    {
        String::Format( nodeAlias, sizeof( nodeAlias ), "%s_node", meshes.GetAt( i ).ToString( ) );

        Node *pInstance = CreateNode( Id::Create( ), NULL );
        nodes.Add( pInstance );

        MeshRendererComponent *pComponent = (MeshRendererComponent *) CreateComponent( pInstance, Id::Create( ), "MeshRendererComponent" );
        pInstance->AddComponent( pComponent );

        ResourceHandleList *pList;
        meshToMaterial.Get( meshes.GetAt( i ), &pList );

        pComponent->Create( ResourceHandle( meshes.GetAt( i ) ), *pList, groups );
        pInstance->SetParent( pParent );

        pInstance->AddToScene( );
        pInstance->GetHandle( ).Bind( nodeAlias, pInstance );
    }

    for ( uint32 i = 0, n = nodes.GetSize( ); i < n; i++ )
        nodes.GetAt( i )->Bind( );


    {
        Enumerator<Id, ResourceHandleList *> e = meshToMaterial.GetEnumerator( );

        while ( e.EnumNext( ) )
        {
            e.Data( )->Destroy( );
            delete e.Data( );
        }
    }

    meshToMaterial.Destroy( );
    nodes.Destroy( );
    groups.Destroy( );
    meshes.Destroy( );

    mesh_list_created = false;
}