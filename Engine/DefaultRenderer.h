#pragma once

#include "EngineGlobal.h"
#include "Renderer.h"
#include "Identifiable.h"
#include "RenderObject.h"
#include "MaterialAsset.h"
#include "HashTable.h"
#include "List.h"

class Viewport;
class IRenderModifier;

class DefaultRenderer : public Renderer
{
private:
    typedef List<RenderObjectDesc *> ObjectList;

    struct RenderList
    {
        List<ObjectList *> lists;
        ObjectList *pCurrent;
    };

public:
    typedef HashTable<RenderContext, RenderList *> PassHash;
    typedef Enumerator<RenderContext, RenderList *> PassEnum;

private:
    static const uint32 MaxCustomSamplePositions = 16;

private:
    RenderStats              m_Stats;
    IdList                   m_Groups;
    List<RenderStats>        m_RenderStats;
    List<RenderObject *>     m_Renderables;
    List<RenderObject *>     m_Lights;
    List<IRenderModifier *>  m_Modifiers;
    RendererDesc             m_RendererDesc;
    PassHash                 m_PassHash;
    Vector2                  m_CustomSamplePositions[ MaxCustomSamplePositions ];
    uint32                   m_NumCustomSamplePositions;
    int                      m_RenderGroupModification;
    const char              *m_pPass;
    bool                     m_Multithreaded;

public:
    DefaultRenderer(
        const char *pPass,
        bool multithreaded,
        Id groups[],
        int numGroups
    );

    ~DefaultRenderer( void );

    void AddModifier(
        IRenderModifier *pModifier
    );

    void AddGroup(
        Id groupId
    );

    void RemoveGroup(
        Id groupId
    );

    void SetSamplePositions(
        Vector2 *pPositions,
        byte count
    );

    virtual RenderStats GetStats( void ) const { return m_Stats; }

    virtual void GetRenderData(
        const Viewport &viewport
    );

    virtual void Render(
        const Viewport &viewport,
        GpuDevice::CommandList *pBatchCommandList
    );

    const RendererDesc *GetRendererDesc( void ) const { return &m_RendererDesc; }

private:
    void RenderMultithreaded(
        const Viewport &viewport
    );

    void RenderSequentially(
        const Viewport &viewport,
        GpuDevice::CommandList *pBatchCommandList
    );

private:
    static void Task_SequentialRender(
        const struct TaskExecution &task
    );

    static void Task_BatchRenderContext(
        const struct TaskExecution &task
    );
};

