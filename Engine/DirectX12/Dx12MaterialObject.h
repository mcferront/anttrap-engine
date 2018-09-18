#pragma once

#include "EngineGlobal.h"
#include "MaterialAsset.h"

class GraphicsMaterialObject
{
public:
   class Pass
   {
   friend class GraphicsMaterialObject;
   
   public:
      GraphicsMaterial::PassData *GetData( void ) { return pData; }

   private:
      VertexContext vertexContext;
      ViewportContext viewportContext;
      GraphicsMaterial::PassData *pData;
      D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc;
      ID3D12PipelineState *pPipelineState;
      ID3D12CommandSignature *m_pCommandSignature;
   };

public:
   GraphicsMaterialObject( 
      ResourceHandle material
      );
   
   ~GraphicsMaterialObject( void );

   bool Prepare( void );

   void SetRenderContext(
      const RenderContext &context,
      GpuDevice::CommandList *pCommandList
   );

   void SetRenderData(
      const Pass *pPass,
      GpuDevice::CommandList *pCommandList
   ) const;

   void CreateCommandSignature(
      const Pass *pPass,
      const D3D12_COMMAND_SIGNATURE_DESC &desc,
      ID3D12CommandSignature **pCommandSignature
   ) const;

   bool HasPass(
      const char *pPass
   ) const;

   RenderContext GetRenderContext(
      const char *pName,
      ViewportContext viewportContext,
      VertexContext vertexContext
   );

   GraphicsMaterialObject::Pass *GetPass(
      const RenderContext &context
   );
      
   const GraphicsMaterial::PassData *GetPassData(
      const char *pName
   ) const;

   bool HasAlpha( 
      const char *pPass
      ) const
   {

      const GraphicsMaterial::PassData *pPassData = GetPassData( pPass );
      if ( NULL == pPassData )
         return false;

      return pPassData->header.blendEnable != 0;
   }

private:
   ResourceHandle m_Material;
   List<Pass*> m_PassContexts;

   GraphicsMaterial::PassData *m_pPassDatas;
   uint32  m_NumPasses;

   bool m_Ready;
};
