#pragma once

#include "EngineGlobal.h"
#include "GraphicsApi.h"
#include "Hashtable.h"

class GpuTimer;

class Texture;
typedef Texture ImageBuffer;

class GpuProfiler
{
private:
    static const int MaxTimers = 64;

    ID3D12QueryHeap *m_pHeap;
    ResourceHandle m_ReadBuffers[ GpuDevice::FrameCount ];

    GpuTimer *m_pTimers[ MaxTimers ];

    
public:
    static GpuProfiler &Instance( void );

    void Create( void );
    void Destroy( void );

    GpuTimer *AllocTimer( 
        const char *pName
    );

    void FreeTimer(
        GpuTimer *pTimer
    );

    void ResolveTimers( 
        int frameIndex
    );
};

class GpuTimer
{
    friend class GpuProfiler;

private:
    const char *pName;
    ID3D12QueryHeap *pHeap;
    ResourceHandle readBuffers[ GpuDevice::FrameCount ];
    uint32 slot;
    uint64 startTime;
    uint64 endTime;
    float duration;
    bool isCompute;

private:
    GpuTimer( 
        const char *pName,
        uint32 slot,
        ID3D12QueryHeap *pHeap,
        ResourceHandle readBuffers[ GpuDevice::FrameCount ]
        );

    ~GpuTimer( void );

public:
    void Begin( 
        GpuDevice::CommandList *pCommandList
    );

    void End( 
        GpuDevice::CommandList *pCommandList
    );

    void Resolve(
        uint64 gfxFrequency,
        uint64 computeFrequency,
        uint64 *pSourceData
    );

    const char *GetName( void ) const { return pName; }
    float GetDuration( void ) const { return duration; }
};
