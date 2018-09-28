#include "EnginePch.h"

#include "Dx12GpuBuffer.h"

DefineResourceType(GpuResource, Resource, NULL);

GpuResource::GpuResource( void )
{
    m_pResource = NULL; 
    m_State = State::Unknown;
}

void GpuResource::AddToScene( void )
{
    Resource::AddToScene( );

#if defined(_DEBUG)
    wchar_t wname[MaxNameLength] = { };

    MultiByteToWideChar(CP_UTF8, 0, GetId().ToString(), -1, wname, _countof(wname));
    m_pResource->SetName( wname );
#endif
}

void GpuResource::RemoveFromScene( void )
{
    Resource::RemoveFromScene( );
}

void GpuResource::TransitionTo(
    GpuDevice::CommandList *pList,
    State::Type state
)
{
    State::Type oldState = m_State;

    {
        ScopeLock lock( m_Lock );

        if ( m_State == state )
            return;

        m_State = state;
    }

    Debug::Assert( Condition((state != State::UnorderedAccess) || (m_pResource->GetDesc().Flags & Flags::UnorderedAccess)), 
                        "Unordered Access Flag not set" );

     
    D3D12_RESOURCE_TRANSITION_BARRIER source = { };
    source.pResource = m_pResource;
    source.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
    source.StateBefore = (D3D12_RESOURCE_STATES) oldState;
    source.StateAfter = (D3D12_RESOURCE_STATES) m_State;

    D3D12_RESOURCE_BARRIER barrier = { };
    barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    barrier.Transition = source;

    pList->pList->ResourceBarrier( 1, &barrier );
}

void GpuResource::SwapApiResource(
    ID3D12Resource *pResource,
    State::Type state
)
{
    if (NULL != m_pResource)
        m_pResource->Release();

    m_pResource = pResource;
    
    m_State = state;

    if (m_pResource != NULL)
        m_pResource->AddRef();
}
