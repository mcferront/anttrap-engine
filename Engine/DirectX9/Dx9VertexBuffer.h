#pragma once

#include "EngineGlobal.h"
#include "Geometry.h"

class VertexBuffer
{
private:
   IDirect3DVertexBuffer9      *m_pVertexBuffer;
   IDirect3DIndexBuffer9       *m_pIndexBuffer;
   D3DVERTEXELEMENT9            m_Elements[8];

   //created on an as-needed basis
   mutable IDirect3DVertexDeclaration9 *m_pVertexDecl;

   uint32 m_Stride;
   uint32 m_VertexBufferSize;
   uint32 m_NumTriangles;
   uint32 m_NumVertices;
   uint32 m_NumElements;

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

   void Submit( void ) const;
};