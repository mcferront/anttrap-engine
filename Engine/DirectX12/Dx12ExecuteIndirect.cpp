#include "EnginePch.h"

#include "Dx12ExecuteIndirect.h"
#include "Dx12.h"
#include "Dx12MaterialObject.h"
#include "Dx12VertexBuffer.h"
#include "RenderWorld.h"

void ExecuteIndirect::Create(
   Id renderGroup,
   ResourceHandle material,
   ResourceHandle indirectArgs,
   uint32 indirectArgsOffset,
   const CommandSignatureDesc &sigDesc,
   VertexBuffer *pVertexBuffer
)
{
   m_RenderGroup = renderGroup;
   m_Material = material;
   m_IndirectArgs = indirectArgs;
   m_IndirectArgsOffset = indirectArgsOffset;

   m_pVertexBuffer = pVertexBuffer;
   m_pMaterial = new GraphicsMaterialObject( m_Material );

   HRESULT hr = GpuDevice::Instance( ).GetDevice( )->CreateCommandSignature( &sigDesc.desc, NULL, __uuidof(ID3D12CommandSignature), (void **) &m_pCommandSignature );
   Debug::Assert( Condition(SUCCEEDED(hr)), "Error CreateCommandSignature: 0x%08x", hr );
}

void ExecuteIndirect::Destroy( void )
{
   if ( m_pCommandSignature )
      m_pCommandSignature->Release( );

   m_IndirectArgs = NullHandle;
   m_Material = NullHandle;

   delete m_pMaterial;
}

void ExecuteIndirect::GetRenderGroups(
   IdList *pGroups
) const
{
   pGroups->Add( m_RenderGroup );  
}

void ExecuteIndirect::GetRenderData(
   RendererDesc *pDesc
)  const
{

   if ( false == m_pMaterial->Prepare( ) ) return;
   if ( false == m_pMaterial->HasPass( pDesc->pPass ) ) return;

   RenderContext renderContext = m_pMaterial->GetRenderContext( pDesc->pPass, pDesc->viewport.GetContext(), m_pVertexBuffer->GetVertexContext() );
   RenderObjectDesc *pRODesc = RenderWorld::Instance( ).AllocRenderObjectDesc( );

   pRODesc->renderContext = renderContext;
   pRODesc->pMaterial = m_pMaterial;
   pRODesc->pObject = this;
   pRODesc->argList = ArgList( m_IndirectArgs, m_pCommandSignature, m_pVertexBuffer, m_IndirectArgsOffset );
   pRODesc->renderFunc = ExecuteIndirect::Render;

   pDesc->renderObjectDescs.Add( pRODesc );
}

void ExecuteIndirect::Render(
   const RenderDesc &desc,
   RenderStats *pStats
)
{
   ResourceHandle indirectArgs;
   ID3D12CommandSignature *pSig;
   uint32 indirectArgsOffset;
   VertexBuffer *pVertexBuffer;

   desc.pDesc->argList.GetArg( 0, &indirectArgs );
   desc.pDesc->argList.GetArg( 1, &pSig );
   desc.pDesc->argList.GetArg( 2, &pVertexBuffer );
   desc.pDesc->argList.GetArg( 3, &indirectArgsOffset );

   GraphicsMaterialObject *pMaterial = desc.pDesc->pMaterial;
   GraphicsMaterialObject::Pass *pPass = pMaterial->GetPass( desc.pDesc->renderContext );

   pMaterial->SetRenderData( pPass, desc.pCommandList );

   pVertexBuffer->Set( desc.pCommandList );

   ImageBuffer *pIndirectArgs = GetResource( indirectArgs, ImageBuffer );

   //pIndirectArgs->Barrier( Texture::Barrier::Uav, desc.pCommandList );
   pIndirectArgs->ConvertTo( ImageBuffer::IndirectArg, desc.pCommandList );

   desc.pCommandList->pList->ExecuteIndirect(
      pSig,
      1,
      pIndirectArgs->GetD3D12Resource(),
      indirectArgsOffset,
      NULL,
      0 );

   pIndirectArgs->ConvertTo( ImageBuffer::UavResource, desc.pCommandList );
}
