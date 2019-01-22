#include "EnginePch.h"

#include "Dx12ComputeMaterialObject.h"
#include "GpuResource.h"
#include "GpuBuffer.h"
#include "RegistryAsset.h"

ComputeMaterialObject::ComputeMaterialObject(
    ResourceHandle computeMaterial
)
{
    m_PassContexts.Create( 4, 4 );
    m_NumPasses = 0;
    m_pPassDatas = NULL;
    m_Material = computeMaterial;
    m_Ready = false;
}

ComputeMaterialObject::~ComputeMaterialObject( void )
{
    m_Material = NullHandle;

    for ( uint32 i = 0, n = m_PassContexts.GetSize( ); i < n; i++ )
        delete m_PassContexts.GetAt( i );

    m_PassContexts.Destroy( );

    delete[] m_pPassDatas;
}

bool ComputeMaterialObject::Prepare( void )
{
    if ( true == m_Ready )
        return true;

    if ( false == IsResourceLoaded( m_Material ) )
        return false;

    Material *pMaterial = GetResource( m_Material, Material );

    ComputeMaterial *pComputeMaterial = pMaterial->GetComputeMaterial( );
    if ( NULL == pComputeMaterial )
        return false;

    m_NumPasses = pComputeMaterial->m_NumPasses;

    m_pPassDatas = new ComputeMaterial::PassData[m_NumPasses];

    for ( uint32 i = 0; i < m_NumPasses; i++ )
        pComputeMaterial->m_pPassDatas[i].CloneTo( &m_pPassDatas[i] );

    m_Ready = true;

    return true;
}

void ComputeMaterialObject::Dispatch(
    const ComputeContext &context,
    GpuDevice::CommandList *pCommandList
)
{
    ComputeMaterialObject::Pass *pPass = GetPass( context );
    pPass->SetComputeData( pPass, pCommandList );

    int x, y, z;

    if ( pPass->GetData( )->header.dyn_group )
    {
        GpuBuffer *pImage = GetResource( pPass->GetData( )->groupSizeTarget, GpuBuffer );

        x = pImage->GetWidth( ) / pPass->GetData( )->header.div.x;
        y = pImage->GetHeight( ) / pPass->GetData( )->header.div.y;
        z = 1;
    }
    else
    {
        x = pPass->GetData( )->header.group.x;
        y = pPass->GetData( )->header.group.y;
        z = pPass->GetData( )->header.group.z;
    }

    pCommandList->pList->Dispatch( x, y, z );
}

void ComputeMaterialObject::SetComputeContext(
    const ComputeContext &context,
    GpuDevice::CommandList *pCommandList
)
{
    ComputeMaterialObject::Pass *pPass = GetPass( context );

    pCommandList->pList->SetComputeRootSignature( pPass->psoDesc.pRootSignature );
    pCommandList->pList->SetPipelineState( pPass->pPipelineState );
}

ComputeContext ComputeMaterialObject::GetComputeContext(
    const char *pPassName
)
{
    uint32 i, n;

    const char *pName = StringRef( pPassName );

    Pass *pPass = NULL;

    for ( i = 0, n = m_PassContexts.GetSize( ); i < n; i++ )
    {
        pPass = m_PassContexts.GetAt( i );

        if ( StringRefEqual( pName, pPass->pData->pName ) == false )
            continue;

        // found a match
        break;
    }

    if ( i == n )
    {
        for ( i = 0; i < m_NumPasses; i++ )
        {
            if ( StringRefEqual( pName, m_pPassDatas[i].pName ) )
                break;
        }

        Debug::Assert( Condition( i < m_NumPasses ), "Pass %s can not be found", pName );

        pPass = new Pass;
        pPass->pData = &m_pPassDatas[i];
        pPass->psoDesc = m_pPassDatas[i].psoDesc;

        char full_path[256];
#ifdef _DEBUG
        String::Format( full_path, sizeof(full_path), "%s %s", m_Material.GetId().ToString(), pPassName );
 #endif
        pPass->pPipelineState = RenderContexts::RegisterPipelineState( full_path, pPass->psoDesc );

        i = m_PassContexts.Add( pPass );
    }

    ComputeContext computeContext;
    computeContext.pipelineContext = (nuint) pPass->pPipelineState;

    return computeContext;
}

ComputeMaterialObject::Pass *ComputeMaterialObject::GetPass(
    const ComputeContext &context
)
{
    for ( uint32 i = 0, n = m_PassContexts.GetSize( ); i < n; i++ )
    {
        Pass *pPass = m_PassContexts.GetAt( i );

        if ( (nuint) pPass->pPipelineState == context.pipelineContext )
            return pPass;
    }

    Debug::Assert( Condition( false ), "Could not find pass for compute context" );

    return NULL;
}

void ComputeMaterialObject::Pass::SetComputeData(
    const ComputeMaterialObject::Pass *pPass,
    GpuDevice::CommandList *pCommandList
) const
{
    const ComputeMaterial::PassData *pData = pPass->pData;

#ifndef _DISTRIBUTION
    static RegistryBool validate("material.validate", false);

    if ( validate.GetValue() )
    {
        for ( int c = 0; c < pData->header.numBuffers; c++ )
        {
            ResourceHandle h = pData->pBuffers[c].buffer;

            for ( int i = c + 1; i < pData->header.numBuffers; i++ )
                Debug::Print( Condition(h != pData->pBuffers[i].buffer), Debug::Type::TypeWarning, "%s is used twice in %s\r\n", h.GetId(), pPass->GetData()->GetName() );
        }
    }
#endif

    for ( int c = 0; c < pData->header.numBuffers; c++ )
    {
        GpuBuffer *pBuffer = GetResource( pData->pBuffers[c].buffer, GpuBuffer );

        if ( pData->pBuffers[c].header.type == ComputeMaterial::PassData::Buffer::UAV )
            pBuffer->TransitionTo( pCommandList, GpuResource::State::UnorderedAccess );
        else if ( pData->pBuffers[c].header.type == ComputeMaterial::PassData::Buffer::SRV )
            pBuffer->TransitionTo( pCommandList, GpuResource::State::ShaderResource );
    }

    for ( int c = 0; c < pData->header.numBuffers; c++ )
    {
        if ( pData->pBuffers[c].pName[0] == '$' )
        {
            D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle = { pData->viewHandles.cpuHandle.ptr + (c * pData->viewHandles.pHeap->descHandleIncSize) };
            
            GpuBuffer *pBuffer = GetResource( pData->pBuffers[c].buffer, GpuBuffer );

            GpuDevice::ViewHandle view;

            if ( pData->pBuffers[c].header.type == ComputeMaterial::PassData::Buffer::UAV  )
                view = pBuffer->GetUav()->view;
            else if ( pData->pBuffers[c].header.type == ComputeMaterial::PassData::Buffer::SRV )
                view = pBuffer->GetSrv()->view;

            GpuDevice::Instance().GetDevice()->CopyDescriptorsSimple( 1, cpuHandle, view.cpuHandle, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV );
        }
    }

    int rootIndex = 0;

    if ( pData->constantBuffer.pData )
    {
        D3D12_GPU_VIRTUAL_ADDRESS gpuAddress = GpuDevice::Instance( ).UpdateCbv( pData->constantBuffer );
        pCommandList->pList->SetComputeRootConstantBufferView( rootIndex++, gpuAddress );
    }

    if ( pData->viewHandles.pHeap )
        pCommandList->pList->SetComputeRootDescriptorTable( rootIndex++, pData->viewHandles.gpuHandle );

}
