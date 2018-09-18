#pragma once

#include "EngineGlobal.h"
#include "List.h"
#include "Renderer.h"

struct LightDesc;
class RenderDesc;

struct Vertex2
{
   Vector2 position;
   Vector2 uv;
   Vector  color;
};

struct Vertex
{
   Vector  position;
   Vector  color;
   Vector  normal;
   Vector2 uv;
};

struct BoneVertex
{
   Vertex vertex;
   float weights[ 4 ];
   unsigned char indices[ 4 ];
   char pad[ 12 ];
};

struct Triangle
{
public:
   Vertex vertices[ 3 ];

   static void Render(
      Triangle *pTriangles,
      uint32 numTriangles,
      bool selected,
      GraphicsMaterialObject *pMaterial,
      const Transform &transform,
      const RenderDesc &desc,
      RenderStats *pStats
   );

public:
   static VertexContext GetVertexContext( void );
};

void CalculateNormals( 
   Triangle *pTriangles, 
   uint32 numTriangles
);

struct StreamDecl
{
   static const int Float  = 0;
   static const int UInt   = 1;
   static const int UShort = 2;
   static const int Byte   = 3;
   static const int Color  = 4;

   static const int Positions   = 1 << 0;
   static const int Normals     = 1 << 1;
   static const int UV0s        = 1 << 2;
   static const int UV1s        = 1 << 3;
   static const int Colors      = 1 << 4;
   static const int Binormals   = 1 << 5;
   static const int Tangents    = 1 << 6;
   static const int BoneIndices = 1 << 7;
   static const int BoneWeights = 1 << 8;

   unsigned int dataType;
   unsigned int offset;
   unsigned int numElements;
};
