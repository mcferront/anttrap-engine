#pragma once

#include "EngineGlobal.h"
#include "Asset.h"
#include "BezierSpline.h"

class Spline : public Asset
{
    friend class SplineSerializer;

private:
    int m_NumVerts;
    int m_NumControlPoints;
    int m_NumKnotElements;
    int m_Order;

    Vector *m_pVerts;
    Vector *m_pControlPoints;
    float  *m_pKnotVector;

    BezierSpline m_BezierSpline;

public:
    void Create( )
    {
        m_NumVerts = 0;
        m_NumControlPoints = 0;
        m_NumKnotElements = 0;
        m_Order = 0;

        m_pVerts = NULL;
        m_pControlPoints = NULL;
        m_pKnotVector = NULL;
    }

    void Destroy( void );

    const BezierSpline *GetBezierSpline() const { return &m_BezierSpline; }

    const Vector *GetVerts( void ) const { return m_pVerts; }
    int GetNumVerts( void ) const { return m_NumVerts; }

    const Vector *GetControlPoints( void ) const { return m_pControlPoints; }
    int GetNumControlPoints( void ) const { return m_NumControlPoints; }

    const float *GetKnotVector( void ) const { return m_pKnotVector; }
    int GetNumKnotElements( void ) const { return m_NumKnotElements; }

    int GetOrder( void ) const { return m_Order; }

public:
    DeclareResourceType(Spline);
};

class SplineSerializer
{
public:
    ISerializable *Deserialize(
        Serializer *pSerializer,
        ISerializable *pSerializable,
        Id id
        );
};
