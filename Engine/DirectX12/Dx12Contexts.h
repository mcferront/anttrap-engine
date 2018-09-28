#pragma once

#include "EngineGlobal.h"
#include "Dx12.h"

class Viewport;
struct VertexElementDesc;

class RenderContexts
{
public:
   const static uint32 MaxRenderTargets = 8;

private:
   struct ViewportContextDesc
   {
   public:
      DXGI_FORMAT rtvFormats[MaxRenderTargets];
      DXGI_FORMAT dsvFormat;
      uint32 numRTVFormats;
      byte rtSampleCounts[MaxRenderTargets];
      byte dsSampleCount;

      ViewportContextDesc(
         const Viewport &viewport
         );

      bool operator == (
         const ViewportContextDesc &rhs
         );
   };

   struct VertexContextDesc
   {
   public:
      uint32 elements[ 256 * sizeof( D3D12_INPUT_ELEMENT_DESC ) ];
      D3D12_PRIMITIVE_TOPOLOGY_TYPE topology;
      uint32 numElements;

      VertexContextDesc( void )
      {
         numElements = 0;
         topology = D3D12_PRIMITIVE_TOPOLOGY_TYPE_UNDEFINED;
      }

      VertexContextDesc(
         const VertexElementDesc &desc
         );

      bool operator == (
         const VertexContextDesc &rhs
         );
   };

   struct RootSignatureDesc
   {
      RootSignatureDesc(
         const D3D12_ROOT_SIGNATURE_DESC &rootDesc
         );

      ~RootSignatureDesc( void )
      {
         free( pHead );
      }

      bool operator == (
         const D3D12_ROOT_SIGNATURE_DESC &rootDesc
         );

      byte *pHead;
      D3D12_ROOT_SIGNATURE_DESC rootDesc;
      ID3D12RootSignature *pRootSignature;
   };

   struct PipelineStateDesc
   {
      enum Type
      {
         Graphics,
         Compute
      };

      PipelineStateDesc(
         const D3D12_GRAPHICS_PIPELINE_STATE_DESC &pipelineDesc,
         ViewportContext viewportContext,
         VertexContext vertexContext
      );

      PipelineStateDesc(
         const D3D12_COMPUTE_PIPELINE_STATE_DESC &pipelineDesc
      );

      bool operator == (
         const PipelineStateDesc &rhs
         );

      ID3D12PipelineState *pPipelineState;
      
      union
      {
         D3D12_GRAPHICS_PIPELINE_STATE_DESC graphics;
         D3D12_COMPUTE_PIPELINE_STATE_DESC compute;
      };
      
      Type type;
      ViewportContext viewportContext;
      VertexContext vertexContext;
   };

private:
   static const ViewportContextDesc *GetViewportContextDesc(
      ViewportContext context
      );

   static const VertexContextDesc *GetVertexContextDesc(
      VertexContext context
      );

public:
   static ViewportContext RegisterViewportContext(
      const Viewport &viewport
      );

   static VertexContext RegisterVertexContext(
      const VertexElementDesc &vertexDesc
      );

   static ID3D12RootSignature *RegisterRootSignature(
      const D3D12_ROOT_SIGNATURE_DESC &desc
      );

   static ID3D12PipelineState *RegisterPipelineState(
      const char *pDebugName,
      const D3D12_GRAPHICS_PIPELINE_STATE_DESC &pipelineDesc,
      ViewportContext viewportContext,
      VertexContext vertexContext
      );

   static ID3D12PipelineState *RegisterPipelineState(
      const char *pDebugName,
      const D3D12_COMPUTE_PIPELINE_STATE_DESC &pipelineDesc
   );

   static void CreateContexts( void );
   static void DestroyContexts( void );

private:
   static List<PipelineStateDesc> s_PipelineStates;
   static List<VertexContextDesc> s_VertexContexts;
   static List<ViewportContextDesc> s_ViewportContexts;
   static List<RootSignatureDesc*> s_RootSignatureDescs;
};
