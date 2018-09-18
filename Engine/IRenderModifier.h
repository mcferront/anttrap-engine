#pragma once

#include "EngineGlobal.h"
#include "List.h"

class Renderer;
class Viewport;
class RendererDesc;

class IRenderModifier
{
public:
    virtual ~IRenderModifier( void ) {};

    virtual void Begin( void ) = 0;

    virtual void Process( 
       RendererDesc *pRendererDesc
       ) = 0;
};
