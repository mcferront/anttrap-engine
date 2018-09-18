#pragma once

#include "EngineGlobal.h"
#include "SystemId.h"

class Serializer;

class Collider
{
    friend class ColliderSerializer;

public:
    struct Cell
    {
        int *pIds;
        int count;
    };

    struct Triangle
    {
        Vector positions[3];
        Vector normals[3];
        Vector edges[3];

        Vector normal;
    };

private:
    Id     m_Name;
    Vector   m_Center;
    Vector   m_Extents;
    Vector   m_Cell;
    Vector   m_GridStart;
    Cell    *m_pCells;
    int      m_CellsX;
    int      m_CellsY;
    int      m_CellsZ;
    int     *m_pIds;

    Triangle *m_pTriangles;
    int m_NumTriangles;

public:
    virtual void Destroy( void );

    int GetNumTriangles( void ) const { return m_NumTriangles; }
    const Triangle *GetTriangles( void ) const { return m_pTriangles; }

    int GetGridCellsX( void ) const { return m_CellsX; }
    int GetGridCellsY( void ) const { return m_CellsY; }
    int GetGridCellsZ( void ) const { return m_CellsZ; }

    const Vector *GetGridCellDimensions( void ) const { return &m_Cell; }
    const Vector *GetGridStart( void ) const { return &m_GridStart; }

    const Vector *GetCenter ( void ) const { return &m_Center; }
    const Vector *GetExtents( void ) const { return &m_Extents; }

    void GetCellTriangles( 
        int cellX, 
        int cellZ, 
        int *pIds, 
        int size, 
        int *pReturned
        ) const;

private:
    void ProcessMesh( void );

    Cell *GetCell( 
        const Vector &position
        );
};

class ColliderSerializer
{
public:
    Collider *Deserialize(
        Serializer *pSerializer,
        Id name
        );
};
