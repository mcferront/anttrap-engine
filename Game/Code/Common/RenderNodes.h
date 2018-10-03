#pragma once

#include "Global.h"
#include "StringPool.h"
#include "Identifiable.h"
#include "Renderer.h"

class GpuTimer;
class RenderTree;
class ComputeNode;
class GpuTimerNode;

const int HiZMipRes = 2048;
const int HiZMipLevels = 12;

const int HdrMipRes = 2048;
const int HdrMipLevels = 6;

const int HdrBlurIterations = 2;

static const Id GeometryGroup = Id( "Geometry" );
static const Id DebugGraphicsGroup = Id( "DebugGraphics" );

static const char *g_pHdrMipLevels[ HdrMipLevels ] =
{
    StringRef( "HdrMip0" ),
    StringRef( "HdrMip1" ),
    StringRef( "HdrMip2" ),
    StringRef( "HdrMip3" ),
    StringRef( "HdrMip4" ),
    StringRef( "HdrMip5" ),
};

static const char *g_pHiZMipLevels[ HiZMipLevels ] =
{
    StringRef( "HiZMip0" ),
    StringRef( "HiZMip1" ),
    StringRef( "HiZMip2" ),
    StringRef( "HiZMip3" ),
    StringRef( "HiZMip4" ),
    StringRef( "HiZMip5" ),
    StringRef( "HiZMip6" ),
    StringRef( "HiZMip7" ),
    StringRef( "HiZMip8" ),
    StringRef( "HiZMip9" ),
    StringRef( "HiZMip10" ),
    StringRef( "HiZMip11" ),
    //StringRef( "HiZMip12" ),
};

const int DofBlurs = 3;

struct DofDesc
{
    // see top of DofCoc.hlsl 
    // for dof description
    GpuTimerNode *pTimerNode;
    ComputeNode *pSplitPlanes;
    ComputeNode *pFarBlurs[ DofBlurs ];
    ComputeNode *pNearBlurs[ DofBlurs ];
    ComputeNode *pComposite;
    Renderer *pMainHdrTargetBarrier;
};

void CreateDof(
    List<GpuTimer*> *pTimers,
    DofDesc *pDesc
);

void AddDof(
    RenderTree *pTree,
    const DofDesc &desc
);

struct SsrDesc
{
    ComputeNode *pSSR;
    ComputeNode *pComposite;
};

void CreateSsr(
    List<GpuTimer*> *pTimers,
    SsrDesc *pDesc
);

void AddSsr(
    RenderTree *pTree,
    const SsrDesc &desc
);

struct SpecBloomDesc
{
    Renderer *pMainHdrBarrier1;
    Renderer *pMainHdrBarrier2;
    ComputeNode *pComposite;
};

void CreateSpecBloom(
    List<GpuTimer*> *pTimers,
    SpecBloomDesc *pDesc
);

void AddSpecBloom(
    RenderTree *pTree,
    const SpecBloomDesc &desc
);

struct HdrMipDesc
{
    struct HdrLevel
    {
        ComputeNode *pDownRes;
        Renderer *pMipBarrier;
        ComputeNode *pBlurIterations[ HdrBlurIterations ];
        Renderer *pBlurBarriers[ HdrBlurIterations ];
    };

    Renderer *pHdrMipBuffer;
    HdrLevel hdrLevels[ HdrMipLevels ];   
};

void CreateHdrMips(
    List<GpuTimer*> *pTimers,
    HdrMipDesc *pDesc
);

void AddHdrMips(
    RenderTree *pTree,
    const HdrMipDesc &desc
);

struct HiZMipDesc
{
    Renderer *pHiZMips[ HiZMipLevels ];
    Renderer *pHiZMipBarriers[ HiZMipLevels ];
    Renderer *pHiZBuffer;
};

void CreateHiZMips(
    HiZMipDesc *pDesc
);

void AddHiZMips(
    RenderTree *pTree,
    const HiZMipDesc &desc
);

struct SSAODesc
{
    Renderer *pBarriers[ 3 ];
    Renderer *pSsaos[ 3 ];
};

void CreateSSAO(
    SSAODesc *pDesc
);

void AddLinearZ( 
    RenderTree *pTree
);

void AddSSAO(
    RenderTree *pTree,
    const SSAODesc &desc
);


struct ShadowMapDesc
{
    Renderer *pShadowMapRenderer;
    Renderer *pShadowMapToTexture;
};

void CreateShadowMap(
    ResourceHandle shadowMapTarget,
    ShadowMapDesc *pDesc
);

void AddShadowMap(
    RenderTree *pTree,
    const ShadowMapDesc &desc
);
