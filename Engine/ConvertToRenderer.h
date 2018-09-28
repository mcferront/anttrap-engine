#pragma once

#include "EngineGlobal.h"
#include "Identifiable.h"
#include "Renderer.h"
#include "GpuResource.h"

class ConvertToRenderer : public Renderer
{
    ResourceHandle m_Resource;
    GpuResource::State::Type m_ConvertTo;

public:
    ConvertToRenderer(
        ResourceHandle resource,
        GpuResource::State::Type converTo
    );

    ~ConvertToRenderer( void );

    virtual void GetRenderData(
        const Viewport &viewport
    );

    virtual void Render(
        const Viewport &viewport,
        GpuDevice::CommandList *pBatchCommandList
    );
};

class BarrierRenderer : public Renderer
{
    ResourceHandle m_Buffer;
    GpuResource::Barrier::Type m_Barrier;

public:
    BarrierRenderer(
        ResourceHandle buffer,
        GpuResource::Barrier::Type
    );

    ~BarrierRenderer( void );

    virtual void GetRenderData(
        const Viewport &viewport
    );

    virtual void Render(
        const Viewport &viewport,
        GpuDevice::CommandList *pBatchCommandList
    );
};


class GpuTimer;

class GpuTimerNode : public Renderer
{
private:
    bool m_Started;
    GpuTimer *m_pGpuTimer;

public:
    GpuTimerNode(
        const char *pName
    );

    ~GpuTimerNode( void );

    GpuTimer *GetTimer( void ) const { return m_pGpuTimer; }

    virtual void GetRenderData(
        const Viewport &viewport
    ) {}

    virtual void Render(
        const Viewport &viewport,
        GpuDevice::CommandList *pBatchCommandList
    );
};
