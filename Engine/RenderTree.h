#pragma once

#include "EngineGlobal.h"
#include "Identifiable.h"
#include "Renderer.h"
#include "List.h"

class Viewport;
class RenderNode;

class RenderTree : public Identifiable
{
private:
    List<Renderer *> m_Nodes;
    bool m_Created;

public:
    void Create( Id id )
    {
        SetId( id );
        m_Nodes.Create();
        m_Created = true;
    }

    void Destroy( void );

    void BeginBatch( void );

    void EndBatch( void );

    void AddNode(
        Renderer *pNode
    );

    void GetRenderData(
        const Viewport &viewport
    );

    void Render(
        const Viewport &viewport
    );
};

