#pragma once

#include "EngineGlobal.h"
#include "Identifiable.h"
#include "Renderer.h"

class MipGenNode : public Renderer
{
private:
    ResourceHandleList m_MipLevels;
    ResourceHandle m_Target;

    byte *m_pDestLayouts;

public:
    MipGenNode(
        ResourceHandle target,
        ResourceHandle mipLevels[],
        int numMipLevels
    );

    virtual ~MipGenNode( void );

    virtual void GetRenderData(
        const Viewport &viewport
    );

    virtual void Render(
        const Viewport &viewport,
        GpuDevice::CommandList *pBatchCommandList
    );
};
