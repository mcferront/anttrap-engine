#pragma once

#include "EngineGlobal.h"
#include "Identifiable.h"
#include "Renderer.h"

class ConvertToRenderer : public Renderer
{
    ResourceHandle m_Image;
    ImageBuffer::ViewType m_ConvertTo;

public:
    ConvertToRenderer(
        ResourceHandle image,
        ImageBuffer::ViewType converTo
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
    enum Type
    {
        Uav,
        Alias,
    };

    ResourceHandle m_Buffer;
    ImageBuffer::Barrier::Type m_Type;

public:
    BarrierRenderer(
        ResourceHandle buffer,
        ImageBuffer::Barrier::Type
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
