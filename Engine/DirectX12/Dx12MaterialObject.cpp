#include "EnginePch.h"

#include "Dx12MaterialObject.h"
#include "TextureAsset.h"
#include "Dx12Contexts.h"
#include "ShaderAsset.h"
#include "Renderer.h"
#include "Viewport.h"
#include "Dx12.h"
#include "LightComponent.h"
#include "RenderWorld.h"
#include "Log.h"

GraphicsMaterialObject::GraphicsMaterialObject( 
   ResourceHandle material 
   )
{
   m_NumPasses = 0;
   m_pPassDatas = NULL;
   m_Material = material;
   m_PassContexts.Create( 4, 4 );
   m_Ready = false;
}

GraphicsMaterialObject::~GraphicsMaterialObject( void )
{
   for ( uint32 i = 0, n = m_PassContexts.GetSize(); i < n; i++ )
      delete m_PassContexts.GetAt(i);

   m_PassContexts.Destroy( );

   delete [] m_pPassDatas;

   m_Material = NullHandle;
}

bool GraphicsMaterialObject::Prepare( void )
{
   if ( true == m_Ready )
      return true;

   if ( false == IsResourceLoaded(m_Material) )
      return false;

   Material *pMaterial = GetResource( m_Material, Material );

   GraphicsMaterial *pGraphicsMaterial = pMaterial->GetGraphicsMaterial( );
   Debug::Assert( Condition(NULL != pGraphicsMaterial), "%s is not a graphics material", m_Material.GetName() );

   m_NumPasses = pGraphicsMaterial->m_NumPasses;

   m_pPassDatas = new GraphicsMaterial::PassData[ m_NumPasses ];

   for ( uint32 i = 0; i < m_NumPasses; i++ )
      pGraphicsMaterial->m_pPassDatas[ i ].CloneTo( &m_pPassDatas[ i ] );

   m_Ready = true;

   return true;
}

void GraphicsMaterialObject::SetRenderContext(
   const RenderContext &context,
   GpuDevice::CommandList *pCommandList
   )
{
   Pass *pPass = GetPass( context );

   pCommandList->pList->SetGraphicsRootSignature( pPass->psoDesc.pRootSignature );
   pCommandList->pList->SetPipelineState( pPass->pPipelineState );
}

void GraphicsMaterialObject::CreateCommandSignature(
   const Pass *pPass,
   const D3D12_COMMAND_SIGNATURE_DESC &desc,
   ID3D12CommandSignature **pCommandSignature
) const
{
   HRESULT hr = GpuDevice::Instance( ).GetDevice( )->CreateCommandSignature( &desc, pPass->psoDesc.pRootSignature, __uuidof(ID3D12CommandSignature), (void **) &pCommandSignature );
   Debug::Assert( Condition(SUCCEEDED(hr)), "Error CreateCommandSignature: 0x%08x", hr );
}

void GraphicsMaterialObject::SetRenderData(
   const GraphicsMaterialObject::Pass *pPass,
   GpuDevice::CommandList *pCommandList
) const
{
   const GraphicsMaterial::PassData *pData = pPass->pData;

   ID3D12DescriptorHeap* pHeaps[ 1 ];

   int heapIndex = 0;
   int descIndex = 0;

   pHeaps[ heapIndex++ ] = GpuDevice::Instance( ).GetDescHeap( );
   pCommandList->pList->SetDescriptorHeaps( heapIndex, pHeaps );

   if ( pData->constantBuffer.gpuHandle != GpuDevice::GpuHandle::Invalid )
   {
      D3D12_GPU_DESCRIPTOR_HANDLE gpuHandle = GpuDevice::Instance( ).GetGpuCbvHandle( pData->constantBuffer.gpuHandle );

      pCommandList->pList->SetGraphicsRootDescriptorTable( descIndex++, gpuHandle ); //vtx shader constats
      pCommandList->pList->SetGraphicsRootDescriptorTable( descIndex++, gpuHandle ); //pxl shader constants
   }
   else
   {
      // for now vtx/pxl shader constants are slot 0 and 1 even if no constants exist
      descIndex = 2;
   }

   for ( int c = 0; c < pData->header.numTextures; c++ )
   {
      ImageBuffer *pImage = GetResource(pData->pTextures[ c ].texture, ImageBuffer);
      GpuDevice::GpuHandle gpuHandle = pImage->GetGpuHandle();

      D3D12_GPU_DESCRIPTOR_HANDLE d3d12GpuHandle = GpuDevice::Instance( ).GetGpuSrvHandle( gpuHandle );
      pCommandList->pList->SetGraphicsRootDescriptorTable( descIndex++, d3d12GpuHandle );
   }
}

RenderContext GraphicsMaterialObject::GetRenderContext(
   const char *pPassName,
   ViewportContext viewportContext,
   VertexContext vertexContext
   )
{
   uint32 i, n;

   const char *pName = StringRef(pPassName);

   Pass *pPass = NULL;

   for ( i = 0, n = m_PassContexts.GetSize(); i < n; i++ )
   {
      pPass = m_PassContexts.GetAt( i );

      if ( StringRefEqual(pName, pPass->pData->pName) == false )
         continue;

      if ( pPass->vertexContext != vertexContext )
         continue;

      if ( pPass->viewportContext != viewportContext )
         continue;

      // found a match
      break;
   }

   if ( i == n )
   {
      for ( i = 0; i < m_NumPasses; i++ )
      {
         if ( StringRefEqual(pName, m_pPassDatas[ i ].pName) )
            break;
      }

      Debug::Assert( Condition(i < m_NumPasses), "Pass %s can not be found", pName );

      pPass = new Pass;
      pPass->pData = &m_pPassDatas[ i ];
      pPass->psoDesc = m_pPassDatas[ i ].psoDesc;
      pPass->vertexContext = vertexContext;
      pPass->viewportContext = viewportContext;
      
      pPass->pPipelineState = RenderContexts::RegisterPipelineState( pPass->psoDesc, viewportContext, vertexContext ); 

      i = m_PassContexts.Add( pPass );
   }

   RenderContext renderContext;
   renderContext.pipelineContext = (nuint) pPass->pPipelineState;

   return renderContext;
}

GraphicsMaterialObject::Pass *GraphicsMaterialObject::GetPass(
   const RenderContext &context
   )
{
   for ( uint32 i = 0, n = m_PassContexts.GetSize(); i < n; i++ )
   {
      Pass *pPass = m_PassContexts.GetAt( i );

      if ( (nuint) pPass->pPipelineState == context.pipelineContext )
         return pPass;
   }

   Debug::Assert( Condition(false), "Could not find pass for render context" );

   return NULL;
}

const GraphicsMaterial::PassData *GraphicsMaterialObject::GetPassData(
   const char *pName
   ) const
{
   uint32 i;

   pName = StringRef(pName);

   for ( i = 0; i < m_NumPasses; i++ )
   {
      if ( StringRefEqual(pName, m_pPassDatas[ i ].pName) )
         break;
   }

   StringRel(pName);

   if ( i < m_NumPasses ) 
      return &m_pPassDatas[ i ];
   
   return NULL;
}

bool GraphicsMaterialObject::HasPass(
   const char *pName
   ) const
{
   pName = StringRef(pName);

   for ( uint32 i = 0; i < m_NumPasses; i++ )
   {
      if ( StringRefEqual(pName, m_pPassDatas[ i ].pName) )
         return true;
   }

   StringRel(pName);

   return false;
}
