#include "EnginePch.h"

#include "ShapeRendererComponent.h"
#include "Node.h"
#include "Line.h"

DefineComponentType(ShapeRendererComponent, new ShapeRendererComponentSerializer);

void ShapeRendererComponent::Create(
    Id id,
    ResourceHandle material,
    const IdList &renderGroups,
    const Triangle *pTriangles,
    int numTriangles,
    const Line *pLines,
    int numLines
)
{
    SetId( id );

    m_Shape.Create( this, material, renderGroups, pTriangles, numTriangles, pLines, numLines );
}

void ShapeRendererComponent::Bind( void )
{
    Component::Bind( );
}

void ShapeRendererComponent::Destroy( void )
{
    m_Shape.Destroy( );

    Component::Destroy( );
}

void ShapeRendererComponent::AddToScene( void )
{
    if ( false == GetParent()->IsInScene() ) return;

    m_Shape.AddToScene( );
    Component::AddToScene( );
}

void ShapeRendererComponent::RemoveFromScene( void )
{
    m_Shape.RemoveFromScene( );
    Component::RemoveFromScene( );
}

ISerializable *ShapeRendererComponentSerializer::Deserialize(
   Serializer *pSerializer,
   ISerializable *pSerializable
)
{
    if ( NULL == pSerializable ) pSerializable = new ShapeRendererComponent; 

    ShapeRendererComponent *pShapeRendererComponent = (ShapeRendererComponent *) pSerializable;
    
    Id id = Id::Deserialize( pSerializer->GetInputStream() );
    Id material = Id::Deserialize( pSerializer->GetInputStream() );

    IdList renderGroups;
    Id::DeserializeList( pSerializer->GetInputStream(), &renderGroups );

    byte isFixedSize;
    float fixedSize;
    int count;

    pSerializer->GetInputStream()->Read( &isFixedSize, sizeof(isFixedSize) );
    pSerializer->GetInputStream()->Read( &fixedSize, sizeof(fixedSize) );
    
    // read positions
    pSerializer->GetInputStream()->Read( &count, sizeof(count) );

    Vector *pPositions = (Vector *) malloc( sizeof(Vector) * count );
    pSerializer->GetInputStream()->Read( pPositions, sizeof(Vector) * count );

    // read normals
    pSerializer->GetInputStream()->Read( &count, sizeof(count) );

    Vector *pNormals = (Vector *) malloc( sizeof(Vector) * count );
    pSerializer->GetInputStream()->Read( pNormals, sizeof(Vector) * count );

    // read colors
    pSerializer->GetInputStream()->Read( &count, sizeof(count) );

    Vector *pColors = (Vector *) malloc( sizeof(Vector) * count );
    pSerializer->GetInputStream()->Read( pColors, sizeof(Vector) * count );

    int numTriangles = count / 3;
    Triangle *pTriangles = (Triangle *) malloc( sizeof(Triangle) * numTriangles );

    numTriangles = 0;

    for ( int i = 0; i < count; i += 3 )
    {
        pTriangles[ numTriangles ].vertices[ 0 ].position = pPositions[ i + 0 ];
        pTriangles[ numTriangles ].vertices[ 0 ].normal   = pNormals[ i + 0 ];
        pTriangles[ numTriangles ].vertices[ 0 ].color    = pColors[ i + 0 ];

        pTriangles[ numTriangles ].vertices[ 1 ].position = pPositions[ i + 1 ];
        pTriangles[ numTriangles ].vertices[ 1 ].normal   = pNormals[ i + 1 ];
        pTriangles[ numTriangles ].vertices[ 1 ].color    = pColors[ i + 1 ];

        pTriangles[ numTriangles ].vertices[ 2 ].position = pPositions[ i + 2 ];
        pTriangles[ numTriangles ].vertices[ 2 ].normal   = pNormals[ i + 2 ];
        pTriangles[ numTriangles ].vertices[ 2 ].color    = pColors[ i + 2 ];
    
        ++numTriangles;
    }

    // read positions
    pSerializer->GetInputStream()->Read( &count, sizeof(count) );

    pPositions = (Vector *) realloc( pPositions, sizeof(Vector) * count );
    pSerializer->GetInputStream()->Read( pPositions, sizeof(Vector) * count );

    // read colors
    pSerializer->GetInputStream()->Read( &count, sizeof(count) );

    pColors = (Vector *) realloc( pColors, sizeof(Vector) * count );
    pSerializer->GetInputStream()->Read( pColors, sizeof(Vector) * count );

    int numLines = count / 2;
    Line *pLines = (Line *) malloc( sizeof(Line) * numLines );

    numLines = 0;
    for ( int i = 0; i < count; i += 2 )
    {
        pLines[ numLines ].start      = pPositions[ i + 0 ];
        pLines[ numLines ].startColor = pColors[ i + 0 ];
        pLines[ numLines ].end        = pPositions[ i + 1 ];
        pLines[ numLines ].endColor   = pColors[ i + 1 ];
    
        ++numLines;
    }

    pShapeRendererComponent->Create( id, ResourceHandle(material), renderGroups, pTriangles, numTriangles, pLines, numLines );
    pShapeRendererComponent->m_Shape.SetFixedSize( 0 != isFixedSize, fixedSize );

    free( pPositions );
    free( pNormals );
    free( pColors );
    free( pTriangles );
    free( pLines );

    return pSerializable;
}
