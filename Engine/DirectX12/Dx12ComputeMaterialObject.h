#pragma once

#include "EngineGlobal.h"
#include "MaterialAsset.h"

class ComputeMaterialObject
{
public:
   struct Pass
   {
      friend class ComputeMaterialObject;

   public:
      ComputeMaterial::PassData *GetData( void ) const { return pData; }

   private:
      void ComputeMaterialObject::Pass::SetComputeData(
         const ComputeMaterialObject::Pass *pPass,
         GpuDevice::CommandList *pCommandList
      ) const;

      ComputeMaterial::PassData *pData;
      D3D12_COMPUTE_PIPELINE_STATE_DESC psoDesc;
      ID3D12PipelineState *pPipelineState;
   };

public:
   ComputeMaterialObject( 
      ResourceHandle computeMaterial
      );
   
   ~ComputeMaterialObject( void );

   bool Prepare( void );

   void Dispatch(
      const ComputeContext &context,
      GpuDevice::CommandList *pCommandList
   );

   void SetComputeContext(
      const ComputeContext &context,
      GpuDevice::CommandList *pCommandList
   );

   ComputeContext GetComputeContext(
      const char *pPassName
   );

   ComputeMaterialObject::Pass *GetPass(
      const ComputeContext &context
   );

private:
   void SetComputeData(
      const ComputeMaterialObject::Pass *pPass,
      GpuDevice::CommandList *pCommandList
   ) const;

private:
   ResourceHandle m_Material;
   List<Pass*> m_PassContexts;

   ComputeMaterial::PassData *m_pPassDatas;
   uint32 m_NumPasses;

   bool m_Ready;
};
