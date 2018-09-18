#pragma once

#include "EngineGlobal.h"

class RenderDesc;
class GraphicsMaterialObject;
struct RenderStats;

class Quad
{
public:
   Vector uvs;
   float x;
   float y;
   float width;
   float height;

public:
   void Create(
      float width,
      float height,
      float u1,
      float v1
      );

   void Render(
      bool selected,
      const Vector &color,
      const Transform &transform,
      const RenderDesc &desc,
      RenderStats *pStats
      );

   static void Render(
      Quad *pQuads,
      uint32 numQuads,
      bool selected,
      const Vector &color,
      const Transform &transform,
      const RenderDesc &desc,
      RenderStats *pStats
      );

public:
   static VertexContext GetVertexContext( void );
};
