#pragma once

#include "EngineGlobal.h"
#include "Geometry.h"
#include "MaterialAsset.h"

class GpuBuffer;

class VertexBuffer
{
private:
   GpuBuffer *m_pVertexBuffer;
   GpuBuffer *m_pIndexBuffer;

   D3D12_VERTEX_BUFFER_VIEW   m_VertexBufferView;
   D3D12_INDEX_BUFFER_VIEW    m_IndexBufferView;
   VertexElementDesc          m_VertexElementDesc;
   VertexContext              m_VertexContext;
   uint32                     m_IndexCount;

   D3D12_INPUT_ELEMENT_DESC   m_ElementDescs[ 256 ];

public:
   void Create ( void );
   void Destroy( void );

   void SetVertices(
      const void *pVertices,
      uint32 size
   );

   void SetIndices(
      const void *pIndices,
      uint32 size,
      uint32 type,
      uint32 count
   );

   void AddAttributes(
      int type,
      StreamDecl decl
   );

   void EndAttributes( void );

   const VertexElementDesc *GetElementDesc( void ) const { return &m_VertexElementDesc; }

   VertexContext GetVertexContext( void ) const { return m_VertexContext; }

   void Set(
      GpuDevice::CommandList *pCommandList
   ) const;

   void Submit(
      GpuDevice::CommandList *pCommandList,
      RenderStats *pStats
   ) const;
};
