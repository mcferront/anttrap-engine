#include "EnginePch.h"

#include "VertexBuffer.h"
#include "ShaderAsset.h"

void VertexBuffer::Create( void )
{
   m_pVertexBuffer = NULL;
   m_pIndexBuffer  = NULL;
   m_pVertexDecl   = NULL;

   m_Stride = 0;
   m_NumTriangles = 0;
   m_VertexBufferSize = 0;
   m_NumElements = 0;
}

void VertexBuffer::Destroy( void )
{
   if ( NULL != m_pVertexDecl   ) m_pVertexDecl->Release( );
   if ( NULL != m_pVertexBuffer ) m_pVertexBuffer->Release( );
   if ( NULL != m_pIndexBuffer  ) m_pIndexBuffer->Release( );
}

void VertexBuffer::SetVertices(
   const void *pVertices,
   uint32 size
)
{
   HRESULT hr;

   hr = Dx9::Instance( ).GetDevice( )->CreateVertexBuffer( size, D3DUSAGE_WRITEONLY, NULL, D3DPOOL_MANAGED, &m_pVertexBuffer, NULL );
   Debug::Assert( Condition(SUCCEEDED(hr)), "CreateVertexBuffer failed 0x%08x", hr );

   void *pData;

   hr = m_pVertexBuffer->Lock( 0, 0, &pData, 0 );
   Debug::Assert( Condition(SUCCEEDED(hr)), "VertexBuffer Lock failed 0x%08x", hr );

   memcpy( pData, pVertices, size );

   hr = m_pVertexBuffer->Unlock( );
   Debug::Assert( Condition(SUCCEEDED(hr)), "VertexBuffer Unlock failed 0x%08x", hr );

   m_VertexBufferSize = size;   
}

void VertexBuffer::SetIndices(
   const void *pIndices,
   uint32 size,
   uint32 type,
   uint32 count
)
{
   HRESULT hr;
   D3DFORMAT format;
   
   format = type == StreamDecl::UShort ? D3DFMT_INDEX16 : D3DFMT_INDEX32;

   hr = Dx9::Instance( ).GetDevice( )->CreateIndexBuffer( size, D3DUSAGE_WRITEONLY, format, D3DPOOL_MANAGED, &m_pIndexBuffer, NULL );
   Debug::Assert( Condition(SUCCEEDED(hr)), "CreateIndexBuffer failed 0x%08x", hr );

   void *pData;

   hr = m_pIndexBuffer->Lock( 0, 0, &pData, 0 );
   Debug::Assert( Condition(SUCCEEDED(hr)), "IndexBuffer Lock failed 0x%08x", hr );

   memcpy( pData, pIndices, size );

   hr = m_pIndexBuffer->Unlock( );
   Debug::Assert( Condition(SUCCEEDED(hr)), "IndexBuffer Unlock failed 0x%08x", hr );

   m_NumTriangles = count / 3;
}

void VertexBuffer::AddAttributes( 
   int usage,
   StreamDecl decl
)
{
   int stride = 0;
   BYTE d3dType;

   switch ( decl.dataType )
   {
      case StreamDecl::Float  : 
         stride = 4 * decl.numElements; 
         d3dType = D3DDECLTYPE_FLOAT1 + (decl.numElements - 1);
         break;

      case StreamDecl::UShort : 
         stride = 2 * decl.numElements; 
         Debug::Assert( Condition(decl.numElements == 4), "Only 2 ushort usage is supported" );
         d3dType = D3DDECLTYPE_SHORT4;
         break;
      case StreamDecl::Byte   : 
         stride = 1 * decl.numElements; 
         Debug::Assert( Condition(decl.numElements == 4), "Only 4 byte usage is supported" );
         d3dType = D3DDECLTYPE_UBYTE4;
         break;

      case StreamDecl::Color  : 
         stride = 1 * decl.numElements; 
         d3dType = D3DDECLTYPE_D3DCOLOR;
         break;

      default:
         Debug::Assert( Condition(false), "Unspecified dataType type: %d", decl.dataType );
         break;
   }

   D3DDECLUSAGE d3dUsage;
   BYTE usageIndex;

   switch ( usage )
   {
         case StreamDecl::Positions   : d3dUsage = D3DDECLUSAGE_POSITION;     usageIndex = 0; break;
         case StreamDecl::Normals     : d3dUsage = D3DDECLUSAGE_NORMAL;       usageIndex = 0; break;
         case StreamDecl::Colors      : d3dUsage = D3DDECLUSAGE_COLOR;        usageIndex = 0; break;
         case StreamDecl::UV0s        : d3dUsage = D3DDECLUSAGE_TEXCOORD;     usageIndex = 0; break;
         case StreamDecl::UV1s        : d3dUsage = D3DDECLUSAGE_TEXCOORD;     usageIndex = 1; break;
         case StreamDecl::BoneIndices : d3dUsage = D3DDECLUSAGE_BLENDINDICES; usageIndex = 0; break;
         case StreamDecl::BoneWeights : d3dUsage = D3DDECLUSAGE_BLENDWEIGHT;  usageIndex = 0; break;

         default:
            Debug::Assert( Condition(false), "Unspecified Usage type: %d", usage );
            break;
   }

   D3DVERTEXELEMENT9 declaration = { (WORD) 0, (WORD) m_Stride, (BYTE) d3dType, (BYTE) D3DDECLMETHOD_DEFAULT, (BYTE) d3dUsage, (BYTE) usageIndex };

   m_Elements[ m_NumElements ] = declaration;
   m_NumElements++;

   D3DVERTEXELEMENT9 declEnd = D3DDECL_END();
   m_Elements[ m_NumElements ] = declEnd;

   m_Stride += stride;
   m_NumVertices = m_VertexBufferSize / m_Stride;
}

void VertexBuffer::Submit( void ) const
{
   HRESULT hr;

   if ( NULL == m_pVertexDecl )
   {
      hr = Dx9::Instance( ).GetDevice( )->CreateVertexDeclaration( m_Elements, &m_pVertexDecl );
      Debug::Assert( Condition(SUCCEEDED(hr)), "CreateVertexDeclaration failed 0x%08x", hr );
   }

   hr = Dx9::Instance( ).GetDevice( )->SetVertexDeclaration( m_pVertexDecl );
   Debug::Assert( Condition(SUCCEEDED(hr)), "SetVertexDeclaration failed 0x%08x", hr );

   hr = Dx9::Instance( ).GetDevice( )->SetStreamSource( 0, m_pVertexBuffer, 0, m_Stride );   
   Debug::Assert( Condition(SUCCEEDED(hr)), "SetStreamSource failed 0x%08x", hr );

   hr = Dx9::Instance( ).GetDevice( )->SetIndices( m_pIndexBuffer );   
   Debug::Assert( Condition(SUCCEEDED(hr)), "SetIndices failed 0x%08x", hr );

   hr = Dx9::Instance( ).GetDevice( )->DrawIndexedPrimitive( D3DPT_TRIANGLELIST, 0, 0, m_NumVertices, 0, m_NumTriangles );
   Debug::Assert( Condition(SUCCEEDED(hr)), "DrawIndexedPrimitive failed 0x%08x", hr );
}
