#include "EnginePch.h"

#include "SplineAsset.h"
#include "IOStreams.h"
#include "DebugGraphics.h"

DefineResourceType(Spline, Asset, NULL);

void Spline::Destroy( void )
{
    free( m_pVerts );
    free( m_pControlPoints );
    free( m_pKnotVector );

    Asset::Destroy( );
}

ISerializable *SplineSerializer::Deserialize(
    Serializer *pSerializer,
    ISerializable *pSerializable,
    Id id
    )
{
    struct SplineHeader
    {
        int numVerts;
        int order;
        int numControlPoints;
        int numKnotElements;
    };

    SplineHeader header;

    if ( NULL == pSerializable ) pSerializable = new Spline(); 

    pSerializer->GetInputStream( )->Read( &header, sizeof(header), NULL );

    Spline *pSpline = (Spline *) pSerializable;

    pSpline->m_NumVerts = header.numVerts;
    pSpline->m_NumControlPoints = header.numControlPoints;
    pSpline->m_NumKnotElements = header.numKnotElements;
    pSpline->m_Order = header.order;

    pSpline->m_pVerts = (Vector *) malloc( header.numVerts * sizeof(Vector) );
    pSerializer->GetInputStream( )->Read( pSpline->m_pVerts, header.numVerts * sizeof(Vector), NULL );

    pSpline->m_pControlPoints = (Vector *) malloc( header.numControlPoints * sizeof(Vector) );
    pSerializer->GetInputStream( )->Read( pSpline->m_pControlPoints, header.numControlPoints * sizeof(Vector), NULL );

    pSpline->m_pKnotVector = (float *) malloc( header.numKnotElements * sizeof(float) );
    pSerializer->GetInputStream( )->Read( pSpline->m_pKnotVector, header.numKnotElements * sizeof(float), NULL );

    pSpline->m_BezierSpline.Init(header.numControlPoints);
    memcpy(pSpline->m_BezierSpline.GetControlPoints(), pSpline->m_pControlPoints, header.numControlPoints * sizeof(Vector));
    pSpline->m_BezierSpline.UpdateCache();

    return pSerializable;
}
