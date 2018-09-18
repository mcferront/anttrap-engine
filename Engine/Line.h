#pragma once

#include "EngineGlobal.h"
#include "Renderer.h"

class Line
{
public:
   Vector start;
   Vector startColor;

   Vector end;
   Vector endColor;

public:
   void Create(
      const Vector &start,
      const Vector &startColor,
      const Vector &end,
      const Vector &endColor
      );

   void Render(
      GraphicsMaterialObject *pMaterial,
      const Transform &transform,
      const RenderDesc &desc,
      RenderStats *pStats
      );

   static void Render(
      Line *pLines,
      uint32 numLines,
      bool selected,
      GraphicsMaterialObject *pMaterial,
      const Transform &transform,
      const RenderDesc &desc,
      RenderStats *pStats
      );

public:
   static VertexContext GetVertexContext( void );
};
