#pragma once

#include "EngineGlobal.h"
#include "Geometry.h"
#include "ResourceMaps.h"
#include "Renderer.h"
#include "ISearchable.h"
#include "Component.h"

class RenderObject : public ISearchable
{
public:
   struct Type
   {
      static const int Mesh = 1;
      static const int Light = 2;
      static const int Sprite = 3;
      static const int Text = 4;
      static const int Particle = 5;
      static const int Indirect = 6;
   };

public:
   virtual ~RenderObject( void ) {}

   virtual void AddToScene( void );
   virtual void RemoveFromScene( void );

   virtual bool NeedsFlush( void ) const { return false; }
   virtual void Flush( void ) { };

   virtual int GetRenderType( void ) const = 0;

   virtual void GetRenderData(
      RendererDesc *pDesc
      )  const = 0;

   virtual void GetRenderGroups(
      IdList *pGroups
      ) const = 0;
};
