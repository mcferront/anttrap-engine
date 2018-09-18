#pragma once

#include "EngineGlobal.h"

#include "RenderObject.h"

class GraphicsMaterialObject;
class VertexBuffer;

class ExecuteIndirect : public RenderObject
{
public:
   struct CommandSignatureDesc
   {
      D3D12_COMMAND_SIGNATURE_DESC desc;
   };

private:
   // mutable so it can be created in GetRenderData
   // GetRenderData shouldn't be const - 
   // it calls material->Prepare which is not const
   mutable ID3D12CommandSignature *m_pCommandSignature;
   
   GraphicsMaterialObject *m_pMaterial;
   VertexBuffer *m_pVertexBuffer;

   ResourceHandle m_IndirectArgs;
   ResourceHandle m_Material;

   uint32 m_IndirectArgsOffset;
   Id m_RenderGroup;

public:
   void Create(
      Id renderGroup,
      ResourceHandle material,
      ResourceHandle indirectArgs,
      uint32 indirectArgsOffset,
      const CommandSignatureDesc &sigDesc,
      VertexBuffer *pVertexBuffer
   );
      
   void Destroy( void );

   virtual void GetRenderData(
      RendererDesc *pDesc
   )  const;

   virtual void GetRenderGroups(
      IdList *pGroups
   ) const;

   virtual int GetRenderType( void ) const { return RenderObject::Type::Indirect; }

   virtual bool IsVisible( 
      const Frustum &frustum
   ) const { return true; }

   virtual void GetWorldTransform(
      Transform *pWorldTransform
   ) const { *pWorldTransform = Math::IdentityTransform( ); }

   VertexBuffer *GetVertexBuffer( void ) { return m_pVertexBuffer; }

private:
   static void Render(
      const RenderDesc &desc,
      RenderStats *pStats
   );
};
