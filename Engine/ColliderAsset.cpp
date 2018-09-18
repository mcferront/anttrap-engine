#include "EnginePch.h"

#include "IOStreams.h"
#include "ColliderAsset.h"
#include "Serializer.h"

void Collider::Destroy( void )
{
    free( m_pTriangles );
    m_pTriangles = NULL;

    free( m_pCells );
    m_pCells = NULL;

    free( m_pIds );
    m_pIds = NULL;
}

void Collider::GetCellTriangles( 
    int cellX, 
    int cellZ, 
    int *pIds, 
    int size, 
    int *pReturned
    ) const
{

    if (cellX >= 0 && cellX < m_CellsX && cellZ >= 0 && cellZ < m_CellsZ )
    {
        Cell *pCell = &m_pCells[ cellZ * m_CellsX + cellX ];

        int total = pCell->count;
        total = Math::Min( total, size );

        memcpy( pIds, pCell->pIds, total * sizeof(int) );

        *pReturned = total;
    }
    else
        *pReturned = 0;
}

void Collider::ProcessMesh( void )
{
    //TODO: future
    //create a 3d grid instead of 2d

    m_pCells = (Cell *) malloc( m_CellsX * m_CellsZ * sizeof(Cell) );
    memset( m_pCells, 0, m_CellsX * m_CellsZ * sizeof(Cell) );

    Cell *pCell1, *pCell2, *pCell3;

    int total = 0;

    // figure out how many ids for each cell
    for ( int i = 0; i < m_NumTriangles; i++ )
    {
        pCell1 = GetCell( m_pTriangles[ i ].positions[ 0 ] );  
        pCell2 = GetCell( m_pTriangles[ i ].positions[ 1 ] );  
        pCell3 = GetCell( m_pTriangles[ i ].positions[ 2 ] );  

        pCell1->count++;
        ++total;

        if ( pCell2 != pCell1 ) 
        {
            pCell2->count++;
            ++total;
        }
        if ( pCell3 != pCell2 && pCell3 != pCell1 ) 
        {
            pCell3->count++;
            ++total;
        }
    }

    m_pIds = (int *) malloc( total * sizeof(int) );

    //assign slots
    total = 0;
    for ( int i = 0; i < m_CellsX * m_CellsZ; i++ )
    {
        m_pCells[ i ].pIds = m_pIds + total;
        total += m_pCells[ i ].count;

        //clear count so we can repopulate it with the actual data
        //in the next loop
        m_pCells[ i ].count = 0;
    }

    // assign ids
    for ( int i = 0; i < m_NumTriangles; i++ )
    {
        pCell1 = GetCell( m_pTriangles[ i ].positions[ 0 ] );  
        pCell2 = GetCell( m_pTriangles[ i ].positions[ 1 ] );  
        pCell3 = GetCell( m_pTriangles[ i ].positions[ 2 ] );  

        pCell1->pIds[ pCell1->count ] = i;
        pCell1->count++;

        if ( pCell2 != pCell1 ) 
        {
            pCell2->pIds[ pCell2->count ] = i;
            pCell2->count++;
        }

        if ( pCell3 != pCell2 && pCell3 != pCell1 ) 
        {
            pCell3->pIds[ pCell3->count ] = i;
            pCell3->count++;
        }
    }
}

Collider::Cell *Collider::GetCell( 
    const Vector &position 
    )
{
    Vector v = position - m_GridStart; 

    int cx = (int) floorf(v.x / m_Cell.x);
    int cz = (int) floorf(v.z / m_Cell.z);

    Debug::Print( Condition(cx >= -1 && cx <= m_CellsX), Debug::TypeWarning, "Invalid cell for grid %s\n", m_Name.ToString() );
    Debug::Print( Condition(cz >= -1 && cz <= m_CellsZ), Debug::TypeWarning, "Invalid cell for grid %s\n", m_Name.ToString() );

    if ( cx >= m_CellsX ) cx = m_CellsX - 1;
    if ( cz >= m_CellsZ ) cz = m_CellsZ - 1;

    if ( cx < 0 ) cx = 0;
    if ( cz < 0 ) cz = 0;

    return &m_pCells[ cz * m_CellsX + cx ];
}

Collider *ColliderSerializer::Deserialize(
    Serializer *pSerializer,
    Id name
    )
{

    //TODO: (future)
    //load what type of data structre it is and then
    //create that as a pointer inside of Collider
    //so Collider maybe just has ->GetData
    //this is essentially what we want to do with all the assets anyway

    struct Header
    {
        unsigned int numTriangles;
        float cx, cy, cz;
        float ex, ey, ez;
        float cellX, cellY, cellZ;
    };

    Header header;

    pSerializer->GetInputStream( )->Read( &header, sizeof(header), NULL );

    Collider::Triangle *pTriangles = (Collider::Triangle *) malloc( sizeof(Collider::Triangle) * header.numTriangles );
    pSerializer->GetInputStream( )->Read( pTriangles, sizeof(Collider::Triangle) * header.numTriangles, NULL );

    /////
    //   pTriangles[0].positions[0] = Vector(-100, -100, 100);
    //   pTriangles[0].positions[1] = Vector(  0,   100, 100);
    //   pTriangles[0].positions[2] = Vector( 100, -100, 100);
    //
    //   Math::CrossProduct( &pTriangles[0].normal, pTriangles[0].positions[1] - pTriangles[0].positions[0], pTriangles[0].positions[2] - pTriangles[0].positions[0] );
    //   Math::Normalize( &pTriangles[0].normal, pTriangles[0].normal );
    //
    //   Math::Normalize( &pTriangles[0].edges[0], pTriangles[0].positions[1] - pTriangles[0].positions[0] );
    //   Math::CrossProduct( &pTriangles[0].normals[0], pTriangles[0].edges[0], pTriangles[0].normal ); 
    //   pTriangles[0].edges[0].w = Math::Magnitude( pTriangles[0].positions[1] - pTriangles[0].positions[0] );
    //
    //   Math::Normalize( &pTriangles[0].edges[1], pTriangles[0].positions[2] - pTriangles[0].positions[1] );
    //   Math::CrossProduct( &pTriangles[0].normals[1], pTriangles[0].edges[1], pTriangles[0].normal ); 
    //   pTriangles[0].edges[1].w = Math::Magnitude( pTriangles[0].positions[2] - pTriangles[0].positions[1] );
    //
    //   Math::Normalize( &pTriangles[0].edges[2], pTriangles[0].positions[0] - pTriangles[0].positions[2] );
    //   Math::CrossProduct( &pTriangles[0].normals[2], pTriangles[0].edges[2], pTriangles[0].normal ); 
    //   pTriangles[0].edges[2].w = Math::Magnitude( pTriangles[0].positions[0] - pTriangles[0].positions[2] );
    //
    //   header.numTriangles = 1;
    /////

    Collider *pCollider = new Collider;

    // Clamp cell division size to no less than one.  This way we don't end up
    // with checking a million tiny cells if the triangles are orientated in such
    // a way that they all fit super close together on X or Z.

    // TODO: for optimizations the minimum cell size could be user configurable
    if ( header.cellX < 1 ) header.cellX = 1;
    if ( header.cellY < 1 ) header.cellY = 1;
    if ( header.cellZ < 1 ) header.cellZ = 1;

    pCollider->m_NumTriangles = header.numTriangles;
    pCollider->m_pTriangles   = pTriangles;
    pCollider->m_Center       = Vector( header.cx, header.cy, header.cz );
    pCollider->m_Extents      = Vector( header.ex, header.ey, header.ez );
    pCollider->m_Cell         = Vector( header.cellX, header.cellY, header.cellZ );
    pCollider->m_CellsX       = (int) ceilf( header.ex * 2 / header.cellX );
    pCollider->m_CellsZ       = (int) ceilf( header.ez * 2 / header.cellZ );

    pCollider->m_CellsX = Math::Max( 1, pCollider->m_CellsX );
    pCollider->m_CellsZ = Math::Max( 1, pCollider->m_CellsZ );


    //This is just a 2d grid, so y should stay 0
    pCollider->m_CellsY  = 0;//(int) ceilf( header.ey * 2 / header.cellY );

    pCollider->m_GridStart.x = pCollider->m_Center.x - (pCollider->m_CellsX * pCollider->m_Cell.x) / 2;
    pCollider->m_GridStart.z = pCollider->m_Center.z - (pCollider->m_CellsZ * pCollider->m_Cell.z) / 2;

    //This is just a 2d grid, so y should stay 0
    pCollider->m_GridStart.y = 0;//pCollider->m_Center.y - (pCollider->m_CellsY * pCollider->m_Cell.y) / 2;
    pCollider->m_Name = name;
    pCollider->ProcessMesh( );

    return pCollider;
}
