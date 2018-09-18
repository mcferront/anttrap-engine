#include "EnginePch.h"

#include "Dx12Contexts.h"
#include "TextureAsset.h"
#include "ShaderAsset.h"
#include "Renderer.h"
#include "Viewport.h"
#include "Dx12.h"
#include "LightComponent.h"
#include "RenderWorld.h"
#include "Log.h"
#include "Dx12Material.h"


List<RenderContexts::PipelineStateDesc> RenderContexts::s_PipelineStates;
List<RenderContexts::VertexContextDesc> RenderContexts::s_VertexContexts;
List<RenderContexts::ViewportContextDesc> RenderContexts::s_ViewportContexts;
List<RenderContexts::RootSignatureDesc*> RenderContexts::s_RootSignatureDescs;

RenderContexts::ViewportContextDesc::ViewportContextDesc(
   const Viewport &viewport
   )
{
   ResourceHandle dsHandle = viewport.GetDepthStencil( );

   if ( dsHandle != NullHandle )
   {
      ImageBuffer *pDS = GetResource( dsHandle, ImageBuffer );

      dsvFormat = (DXGI_FORMAT) pDS->GetDepthStencilFormat( );
      dsSampleCount = pDS->GetSampleCount( );
   }
   else
      dsvFormat = DXGI_FORMAT_UNKNOWN;

   numRTVFormats = viewport.GetNumRenderTargets( );

   for ( uint32 i = 0; i < numRTVFormats; i++ )
   {
      ImageBuffer *pRT = GetResource( viewport.GetRenderTarget(i), ImageBuffer );

      rtvFormats[i] = (DXGI_FORMAT) pRT->GetRenderTargetFormat( );
      rtSampleCounts[i] = pRT->GetSampleCount();
   }
}

bool RenderContexts::ViewportContextDesc::operator == (
   const ViewportContextDesc &rhs
   )
{
   if ( numRTVFormats != rhs.numRTVFormats )
      return false;

   if ( dsvFormat != rhs.dsvFormat )
      return false;

   if ( dsSampleCount != rhs.dsSampleCount )
      return false;

   for ( uint32 i = 0; i < numRTVFormats; i++ )
   {
      if ( rtvFormats[i] != rhs.rtvFormats[i] )
         return false;

      if ( rtSampleCounts[i] != rhs.rtSampleCounts[i] )
         return false;
   }

   return true;
}

RenderContexts::VertexContextDesc::VertexContextDesc(
   const VertexElementDesc &desc
   )
{
   numElements = desc.numElementDescs;
   topology = desc.topology;

   memcpy( elements, desc.pElementDescs, numElements * sizeof(D3D12_INPUT_ELEMENT_DESC) );
}

bool RenderContexts::VertexContextDesc::operator == (
   const VertexContextDesc &rhs
   )
{
   if ( numElements != rhs.numElements )
      return false;

   if ( topology != rhs.topology )
      return false;

   return 0 == memcmp( elements, rhs.elements, numElements * sizeof(D3D12_INPUT_ELEMENT_DESC) );
}

RenderContexts::RootSignatureDesc::RootSignatureDesc(
   const D3D12_ROOT_SIGNATURE_DESC &d3d12RootDesc
   )
{
   // TODO: replace with hash so we don't have to compare/copy everything
   rootDesc = d3d12RootDesc;

   uint32 rootSize = sizeof(D3D12_ROOT_PARAMETER) * d3d12RootDesc.NumParameters +
                     sizeof(D3D12_STATIC_SAMPLER_DESC) * d3d12RootDesc.NumStaticSamplers;

   uint32 descriptorSize = 0;

   for (uint32 i = 0; i < d3d12RootDesc.NumParameters; i++)
   {
      if ( d3d12RootDesc.pParameters[i].ParameterType == D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE )
         descriptorSize += sizeof(D3D12_DESCRIPTOR_RANGE) * d3d12RootDesc.pParameters[i].DescriptorTable.NumDescriptorRanges;
   }

   pHead = (byte *) malloc( rootSize + descriptorSize );

   D3D12_ROOT_PARAMETER *pParameters = (D3D12_ROOT_PARAMETER *) pHead;
   byte *pStaticSamplers = ((byte *) pParameters) + sizeof(D3D12_ROOT_PARAMETER) * d3d12RootDesc.NumParameters;

   uint32 descOffset = 0;

   for (uint32 i = 0; i < d3d12RootDesc.NumParameters; i++)
   {
      pParameters[i].ParameterType = d3d12RootDesc.pParameters[i].ParameterType;
      pParameters[i].ShaderVisibility = d3d12RootDesc.pParameters[i].ShaderVisibility;

      if ( d3d12RootDesc.pParameters[i].ParameterType == D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE )
      {
         pParameters[i].DescriptorTable.NumDescriptorRanges = d3d12RootDesc.pParameters[i].DescriptorTable.NumDescriptorRanges;

         D3D12_DESCRIPTOR_RANGE *pRanges = (D3D12_DESCRIPTOR_RANGE *) ((byte*) pHead + rootSize + descOffset);

         uint32 size = d3d12RootDesc.pParameters[i].DescriptorTable.NumDescriptorRanges * sizeof(D3D12_DESCRIPTOR_RANGE);
         memcpy( pRanges, d3d12RootDesc.pParameters[i].DescriptorTable.pDescriptorRanges, size );
         pParameters[i].DescriptorTable.pDescriptorRanges = pRanges;

         descOffset += size;
      }
      else
         memcpy( &pParameters[i], &d3d12RootDesc.pParameters[i], sizeof(D3D12_ROOT_PARAMETER) );
   }

   memcpy( pStaticSamplers, d3d12RootDesc.pStaticSamplers, sizeof(D3D12_STATIC_SAMPLER_DESC) * d3d12RootDesc.NumStaticSamplers );

   rootDesc.NumParameters = d3d12RootDesc.NumParameters;
   rootDesc.NumStaticSamplers = d3d12RootDesc.NumStaticSamplers;
   rootDesc.pParameters = (D3D12_ROOT_PARAMETER *) pParameters;
   rootDesc.pStaticSamplers = (D3D12_STATIC_SAMPLER_DESC *) pStaticSamplers;
}

bool RenderContexts::RootSignatureDesc::operator == (
   const D3D12_ROOT_SIGNATURE_DESC &rhs
   )
{
   // TODO: replace with hash so we don't have to compare/copy everything
   if ( rootDesc.NumParameters != rhs.NumParameters )
      return false;

   if ( rootDesc.NumStaticSamplers != rhs.NumStaticSamplers )
      return false;

   if ( rootDesc.Flags != rhs.Flags )
      return false;

   if ( 0 != memcmp(rootDesc.pStaticSamplers, rhs.pStaticSamplers, rootDesc.NumStaticSamplers * sizeof(D3D12_STATIC_SAMPLER_DESC)) )
      return false;

   for (uint32 i = 0; i < rootDesc.NumParameters; i++)
   {
      const D3D12_ROOT_PARAMETER *pParam = &rootDesc.pParameters[i];
      const D3D12_ROOT_PARAMETER *pRhs = &rhs.pParameters[i];

      if ( pParam->ParameterType != pRhs->ParameterType)
         return false;

      if ( pParam->ShaderVisibility != pRhs->ShaderVisibility )
         return false;

      if ( pParam->ParameterType == D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE )
      {
         const D3D12_ROOT_DESCRIPTOR_TABLE *pTable = &pParam->DescriptorTable;
         const D3D12_ROOT_DESCRIPTOR_TABLE *pRhsTable = &pRhs->DescriptorTable;

         if ( pTable->NumDescriptorRanges != pRhsTable->NumDescriptorRanges )
            return false;

         if ( 0 != memcmp(pTable->pDescriptorRanges, pRhsTable->pDescriptorRanges, sizeof(D3D12_DESCRIPTOR_RANGE) * pTable->NumDescriptorRanges) )
            return false;
      }
      else
      {
         if ( 0 != memcmp(pParam, pRhs, sizeof(D3D12_ROOT_PARAMETER)) )
            return false;
      }
   }

   return true;
}

RenderContexts::PipelineStateDesc::PipelineStateDesc(
   const D3D12_GRAPHICS_PIPELINE_STATE_DESC &pipelineDesc,
   ViewportContext _viewportContext,
   VertexContext _vertexContext
   )
{
   memcpy( &graphics, &pipelineDesc, sizeof(graphics) );

   type = Graphics;

   viewportContext = _viewportContext;
   vertexContext = _vertexContext;

   const RenderContexts::ViewportContextDesc *pViewportDesc = RenderContexts::GetViewportContextDesc( viewportContext );
   const RenderContexts::VertexContextDesc *pVertexDesc = RenderContexts::GetVertexContextDesc( vertexContext );

   memcpy( graphics.RTVFormats, pViewportDesc->rtvFormats, sizeof(graphics.RTVFormats[0]) * pViewportDesc->numRTVFormats );
   graphics.NumRenderTargets = pViewportDesc->numRTVFormats;
   graphics.DSVFormat = pViewportDesc->dsvFormat;

   int sampleCount = 0;

   if ( pViewportDesc->dsvFormat != DXGI_FORMAT_UNKNOWN )
      sampleCount = pViewportDesc->dsSampleCount;
   else
      sampleCount = pViewportDesc->rtSampleCounts[0];

   graphics.RasterizerState.MultisampleEnable = sampleCount > 1;
   graphics.SampleDesc.Count = sampleCount;
   graphics.SampleDesc.Quality = sampleCount > 1 ? DXGI_STANDARD_MULTISAMPLE_QUALITY_PATTERN : 0;
   graphics.SampleMask = UINT_MAX;

   graphics.InputLayout = { (D3D12_INPUT_ELEMENT_DESC *) pVertexDesc->elements, pVertexDesc->numElements };
   graphics.PrimitiveTopologyType = pVertexDesc->topology;

   pPipelineState = NULL;
}

RenderContexts::PipelineStateDesc::PipelineStateDesc(
   const D3D12_COMPUTE_PIPELINE_STATE_DESC &pipelineDesc
)
{
   memcpy( &compute, &pipelineDesc, sizeof(compute) );

   type = Compute;

   viewportContext = -1;
   vertexContext = -1;

   pPipelineState = NULL;
}

bool RenderContexts::PipelineStateDesc::operator == (
   const PipelineStateDesc &rhs
   )
{
   if ( type != rhs.type )
      return false;

   if ( type == Graphics )
   {
      if ( viewportContext != rhs.viewportContext )
         return false;

      if ( vertexContext != rhs.vertexContext )
         return false;

      return 0 == memcmp(&graphics, &rhs.graphics, sizeof(graphics));
   }
   else
   {
      return 0 == memcmp(&compute, &rhs.compute, sizeof(compute));
   }
}
const RenderContexts::ViewportContextDesc *RenderContexts::GetViewportContextDesc(
   ViewportContext context
   )
{
   MainThreadCheck;

   return s_ViewportContexts.GetPointer( (uint32) context );
}

const RenderContexts::VertexContextDesc *RenderContexts::GetVertexContextDesc(
   VertexContext context
   )
{
   MainThreadCheck;

   return s_VertexContexts.GetPointer( (uint32) context );
}

ViewportContext RenderContexts::RegisterViewportContext(
   const Viewport &viewport
   )
{
   MainThreadCheck;
      
   ViewportContextDesc desc( viewport );

   int i, n;

   for ( i = 0, n = s_ViewportContexts.GetSize(); i < n; i++ )
   {
      ViewportContextDesc *pDesc = s_ViewportContexts.GetPointer( i );

      if (*pDesc == desc)
         return i;
   }

   s_ViewportContexts.Add( ViewportContextDesc(viewport) );

   return i;
}

VertexContext RenderContexts::RegisterVertexContext(
   const VertexElementDesc &vertexDesc
   )
{
   VertexContextDesc desc( vertexDesc );

   MainThreadCheck;

   int i, n;

   for ( i = 0, n = s_VertexContexts.GetSize(); i < n; i++ )
   {
      VertexContextDesc *pDesc = s_VertexContexts.GetPointer( i );

      if (*pDesc == desc)
         return i;
   }

   s_VertexContexts.Add( VertexContextDesc(vertexDesc) );

   return i;
}

ID3D12RootSignature *RenderContexts::RegisterRootSignature(
   const D3D12_ROOT_SIGNATURE_DESC &desc
   )
{
   MainThreadCheck;
   
   RootSignatureDesc *pDesc;

   int i, n;

   for ( i = 0, n = s_RootSignatureDescs.GetSize(); i < n; i++ )
   {
      pDesc = s_RootSignatureDescs.GetAt( i );

      if (*pDesc == desc)
         return pDesc->pRootSignature;
   }

   pDesc = new RootSignatureDesc(desc);

   ID3DBlob *pSignature;
   ID3DBlob *pError = NULL;
   HRESULT hr;
   
   hr = D3D12SerializeRootSignature( &pDesc->rootDesc, D3D_ROOT_SIGNATURE_VERSION_1, &pSignature, &pError );
   Debug::Assert( Condition( SUCCEEDED( hr ) ), "Failed to D3D12SerializeRootSignature (0x%08x): %s", hr, pError ? (const char *) pError->GetBufferPointer() : "" );
   
   hr = GpuDevice::Instance( ).GetDevice( )->CreateRootSignature( 0, pSignature->GetBufferPointer( ), pSignature->GetBufferSize( ), __uuidof( ID3D12RootSignature ), (void **) &pDesc->pRootSignature );
   Debug::Assert( Condition( SUCCEEDED( hr ) ), "Failed to CreateRootSignature (0x%08x)", hr );
   
   pSignature->Release( );
   
   if ( NULL != pError )
      pError->Release( );

   s_RootSignatureDescs.Add( pDesc );

   return pDesc->pRootSignature;
}

ID3D12PipelineState *RenderContexts::RegisterPipelineState(
   const D3D12_GRAPHICS_PIPELINE_STATE_DESC &pipelineDesc,
   ViewportContext viewportContext,
   VertexContext vertexContext
)
{
   MainThreadCheck;

   PipelineStateDesc desc(pipelineDesc, viewportContext, vertexContext);

   int i, n;

   for ( i = 0, n = s_PipelineStates.GetSize(); i < n; i++ )
   {
      PipelineStateDesc *pDesc = s_PipelineStates.GetPointer( i );

      if (*pDesc == desc)
         return pDesc->pPipelineState;
   }

   HRESULT hr;

   hr = GpuDevice::Instance( ).GetDevice( )->CreateGraphicsPipelineState( &desc.graphics, __uuidof(ID3D12PipelineState), (void **) &desc.pPipelineState );
   Debug::Assert( Condition( SUCCEEDED( hr ) ), "Failed to CreateGraphicsPipelineState 0x%08x", hr );

   s_PipelineStates.Add( desc );

   return desc.pPipelineState;
}

ID3D12PipelineState *RenderContexts::RegisterPipelineState(
      const D3D12_COMPUTE_PIPELINE_STATE_DESC &pipelineDesc
   )
   {
      MainThreadCheck;

      PipelineStateDesc desc(pipelineDesc);

      int i, n;

      for ( i = 0, n = s_PipelineStates.GetSize(); i < n; i++ )
      {
         PipelineStateDesc *pDesc = s_PipelineStates.GetPointer( i );

         if (*pDesc == desc)
            return pDesc->pPipelineState;
      }

      HRESULT hr;

      hr = GpuDevice::Instance( ).GetDevice( )->CreateComputePipelineState( &desc.compute, __uuidof(ID3D12PipelineState), (void **) &desc.pPipelineState );
      Debug::Assert( Condition( SUCCEEDED( hr ) ), "Failed to CreateComputePipelineState 0x%08x", hr );

      s_PipelineStates.Add( desc );

      return desc.pPipelineState;
   }

void RenderContexts::CreateContexts( void )
{
   RenderContexts::s_PipelineStates.Create( );
   RenderContexts::s_VertexContexts.Create( );
   RenderContexts::s_ViewportContexts.Create( );
   RenderContexts::s_RootSignatureDescs.Create( );
}

void RenderContexts::DestroyContexts( void )
{
   for ( uint32 i = 0; i < s_PipelineStates.GetSize( ); i++ )
      s_PipelineStates.GetPointer( i )->pPipelineState->Release( );

   for ( uint32 i = 0; i < s_RootSignatureDescs.GetSize( ); i++ )
   {
      s_RootSignatureDescs.GetAt( i )->pRootSignature->Release();
      delete s_RootSignatureDescs.GetAt( i );
   }

   RenderContexts::s_VertexContexts.Destroy( );
   RenderContexts::s_ViewportContexts.Destroy( );
   RenderContexts::s_PipelineStates.Destroy( );
   RenderContexts::s_RootSignatureDescs.Destroy( );
}
