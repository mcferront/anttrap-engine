#include "EnginePch.h"

#include "Quad.h"
#include "Renderer.h"
#include "Geometry.h"
#include "Viewport.h"
#include "MaterialObject.h"
#include "RenderContexts.h"
#include "TaskWorld.h"

#ifdef DIRECTX9
   IDirect3DVertexDeclaration9 *g_pQuadDecl;
#endif

void Quad::Create(
   float _width,
   float _height,
   float _u1,
   float _v1
   )
{
   x = 0;
   y = 0;

   width = _width;
   height = _height;

   uvs.x = 0;
   uvs.y = 0;
   uvs.z = _u1;
   uvs.w = _v1;
}

void Quad::Render(
   bool selected,
   const Vector &color,
   const Transform &transform,
   const RenderDesc &desc,
   RenderStats *pStats
   )
{
   Quad::Render( this, 1, selected, color, transform, desc, pStats );
}

void Quad::Render(
   Quad *pQuads,
   uint32 numQuads,
   bool selected,
   const Vector &color,
   const Transform &transform,
   const RenderDesc &desc,
   RenderStats *pStats
   )
{
   if ( 0 == numQuads ) return;

   struct QuadVertex
   {
      Vector2 position;
      Vector2 uv;
      byte color[4];
   };

   struct MyQuad
   {
      QuadVertex vertices[ 4 ];
   };

   size_t indexSize = numQuads * 6 * sizeof( unsigned short );
   size_t quadSize = numQuads * sizeof( MyQuad );

   unsigned char *pBytes = (unsigned char *) desc.pTask->AllocTLM( indexSize + quadSize );

   MyQuad *pHead = (MyQuad *) pBytes;

   unsigned short *pIndices = (unsigned short *) ( pBytes + quadSize );

   MyQuad *pMyQuad = pHead;

   uint32 i;

   pStats->num_faces = numQuads * 2;
   pStats->num_verts = numQuads * 4;

   for ( i = 0; i < numQuads; i++ )
   {
      float halfWidth = pQuads[ i ].width / 2.0f;
      float halfHeight = pQuads[ i ].height / 2.0f;

      pIndices[ i * 6 + 0 ] = i * 4 + 0;
      pIndices[ i * 6 + 1 ] = i * 4 + 1;
      pIndices[ i * 6 + 2 ] = i * 4 + 2;

      pIndices[ i * 6 + 3 ] = i * 4 + 2;
      pIndices[ i * 6 + 4 ] = i * 4 + 1;
      pIndices[ i * 6 + 5 ] = i * 4 + 3;

      //top left
      pMyQuad->vertices[ 0 ].position.x = pQuads[ i ].x - halfWidth;
      pMyQuad->vertices[ 0 ].position.y = pQuads[ i ].y + halfHeight;
      pMyQuad->vertices[ 0 ].uv.x = pQuads[ i ].uvs.x;
      pMyQuad->vertices[ 0 ].uv.y = pQuads[ i ].uvs.y;
      pMyQuad->vertices[ 0 ].color[ 0 ] = Math::FloatToInt( color.x * 255.0f );
      pMyQuad->vertices[ 0 ].color[ 1 ] = Math::FloatToInt( color.y * 255.0f );
      pMyQuad->vertices[ 0 ].color[ 2 ] = Math::FloatToInt( color.z * 255.0f );
      pMyQuad->vertices[ 0 ].color[ 3 ] = Math::FloatToInt( color.w * 255.0f );

      //top right
      pMyQuad->vertices[ 1 ].position.x = pQuads[ i ].x + halfWidth;
      pMyQuad->vertices[ 1 ].position.y = pQuads[ i ].y + halfHeight;
      pMyQuad->vertices[ 1 ].uv.x = pQuads[ i ].uvs.z;
      pMyQuad->vertices[ 1 ].uv.y = pQuads[ i ].uvs.y;
      pMyQuad->vertices[ 1 ].color[ 0 ] = Math::FloatToInt( color.x * 255.0f );
      pMyQuad->vertices[ 1 ].color[ 1 ] = Math::FloatToInt( color.y * 255.0f );
      pMyQuad->vertices[ 1 ].color[ 2 ] = Math::FloatToInt( color.z * 255.0f );
      pMyQuad->vertices[ 1 ].color[ 3 ] = Math::FloatToInt( color.w * 255.0f );

      //bottom left
      pMyQuad->vertices[ 2 ].position.x = pQuads[ i ].x - halfWidth;
      pMyQuad->vertices[ 2 ].position.y = pQuads[ i ].y - halfHeight;
      pMyQuad->vertices[ 2 ].uv.x = pQuads[ i ].uvs.x;
      pMyQuad->vertices[ 2 ].uv.y = pQuads[ i ].uvs.w;
      pMyQuad->vertices[ 2 ].color[ 0 ] = Math::FloatToInt( color.x * 255.0f );
      pMyQuad->vertices[ 2 ].color[ 1 ] = Math::FloatToInt( color.y * 255.0f );
      pMyQuad->vertices[ 2 ].color[ 2 ] = Math::FloatToInt( color.z * 255.0f );
      pMyQuad->vertices[ 2 ].color[ 3 ] = Math::FloatToInt( color.w * 255.0f );

      //bottom right
      pMyQuad->vertices[ 3 ].position.x = pQuads[ i ].x + halfWidth;
      pMyQuad->vertices[ 3 ].position.y = pQuads[ i ].y - halfHeight;
      pMyQuad->vertices[ 3 ].uv.x = pQuads[ i ].uvs.z;
      pMyQuad->vertices[ 3 ].uv.y = pQuads[ i ].uvs.w;
      pMyQuad->vertices[ 3 ].color[ 0 ] = Math::FloatToInt( color.x * 255.0f );
      pMyQuad->vertices[ 3 ].color[ 1 ] = Math::FloatToInt( color.y * 255.0f );
      pMyQuad->vertices[ 3 ].color[ 2 ] = Math::FloatToInt( color.z * 255.0f );
      pMyQuad->vertices[ 3 ].color[ 3 ] = Math::FloatToInt( color.w * 255.0f );

      ++pMyQuad;
   }

   GraphicsMaterialObject::Pass *pPass = desc.pDesc->pMaterial->GetPass( desc.pDesc->renderContext );

   Transform viewTransform;
   desc.pViewport->GetCamera( )->GetViewTransform( &viewTransform );

   Matrix projection;
   desc.pViewport->GetCamera( )->GetReverseDepthProjection( &projection );

   static const char *pWorld = StringRef( "$WORLD_MATRIX" );
   static const char *pVP = StringRef( "$VP_MATRIX" );

   Matrix vp;
   Math::Multiply( &vp, viewTransform.ToMatrix(true), projection );
   pPass->GetData()->SetFloat4x4s( pVP, &vp, 1 );
   pPass->GetData()->SetFloat4x4s( pWorld, &transform.ToMatrix( true ), 1 );

   desc.pDesc->pMaterial->SetRenderData( pPass, desc.pCommandList );

#ifdef OPENGL
   glCheckError( "Should be Clear" );

   pglBindBuffer( GL_ARRAY_BUFFER, 0 );
   glCheckError( "Quad::Bind Array Buffer" );

   pglBindBuffer( GL_ELEMENT_ARRAY_BUFFER, 0 );
   glCheckError( "Quad::Bind Element Array Buffer" );

   char *pData = (char *) pHead;

   pglVertexAttribPointer( VertexBuffer::Positions, 2, GL_FLOAT, false, sizeof( QuadVertex ), pData );
   glCheckError( "Quad::Bind Position" );
   pglEnableVertexAttribArray( VertexBuffer::Positions );
   glCheckError( "Quad::Enable Position" );

   pglVertexAttribPointer( VertexBuffer::UV0s, 2, GL_FLOAT, false, sizeof( QuadVertex ), pData + sizeof( Vector2 ) );
   glCheckError( "Quad::Bind UV" );
   pglEnableVertexAttribArray( VertexBuffer::UV0s );
   glCheckError( "Quad::Enable UV" );

   pglVertexAttribPointer( VertexBuffer::Colors, 4, GL_FLOAT, false, sizeof( QuadVertex ), pData + sizeof( Vector2 ) + sizeof( Vector2 ) );
   glCheckError( "Quad::Bind Color" );
   pglEnableVertexAttribArray( VertexBuffer::Colors );
   glCheckError( "Quad::Enable Color" );

   glDrawElements( GL_TRIANGLES, numQuads * 6, GL_UNSIGNED_SHORT, pIndices );
   glCheckError( "Quad::Draw Triangles" );
#elif defined DIRECTX9
   char *pData = (char *) pHead;
   HRESULT hr;

   hr = Dx9::Instance( ).GetDevice( )->SetVertexDeclaration( Dx9::Instance( ).GetQuadDecl( ) );
   Debug::Assert( Condition( SUCCEEDED( hr ) ), "SetVertexDeclaration failed 0x%08x", hr );

   hr = Dx9::Instance( ).GetDevice( )->DrawIndexedPrimitiveUP( D3DPT_TRIANGLELIST, 0, numQuads * 4, numQuads * 2, pIndices, D3DFMT_INDEX16, pData, sizeof( QuadVertex ) );
   Debug::Assert( Condition( SUCCEEDED( hr ) ), "Failed to DrawIndexPrimitiveUp: 0x08x", hr );
#elif defined DIRECTX12
   char *pData = (char *) pHead;
   ID3D12Resource *pVertexBuffer, *pIndexBuffer;

   D3D12_HEAP_PROPERTIES heapProperties = { };
   heapProperties.Type = D3D12_HEAP_TYPE_UPLOAD;
   heapProperties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
   heapProperties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
   heapProperties.CreationNodeMask = 1;
   heapProperties.VisibleNodeMask = 1;

   D3D12_RESOURCE_DESC resourceDesc = { };
   resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
   resourceDesc.Alignment = 0;
   resourceDesc.Width = quadSize;
   resourceDesc.Height = 1;
   resourceDesc.DepthOrArraySize = 1;
   resourceDesc.MipLevels = 1;
   resourceDesc.Format = DXGI_FORMAT_UNKNOWN;
   resourceDesc.SampleDesc.Count = 1;
   resourceDesc.SampleDesc.Quality = 0;
   resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
   resourceDesc.Flags = D3D12_RESOURCE_FLAG_NONE;


   HRESULT hr;

   // Note: using upload heaps to transfer static data like vert buffers is not 
   // recommended. Every time the GPU needs it, the upload heap will be marshalled 
   // over. Please read up on Default Heap usage. An upload heap is used here for 
   // code simplicity and because there are very few verts to actually transfer.
   hr = GpuDevice::Instance( ).GetDevice( )->CreateCommittedResource(
      &heapProperties,
      D3D12_HEAP_FLAG_NONE,
      &resourceDesc,
      D3D12_RESOURCE_STATE_GENERIC_READ,
      NULL,
      __uuidof( ID3D12Resource ),
      (void **) &pVertexBuffer
      );
   Debug::Assert( Condition( SUCCEEDED( hr ) ), "CreateCommittedResource: 0x%08x", hr );
   pVertexBuffer->SetName( L"QuadVB" );

   // Copy the triangle data to the vertex buffer.
   void **pMappedData;
   D3D12_RANGE readRange = { };
   hr = pVertexBuffer->Map( 0, &readRange, reinterpret_cast<void**>( &pMappedData ) );
   Debug::Assert( Condition( SUCCEEDED( hr ) ), "pVertexBuffer->Map: 0x%08x", hr );

   memcpy( pMappedData, pData, quadSize );
   pVertexBuffer->Unmap( 0, NULL );


   // index buffer
   resourceDesc.Width = indexSize;

   hr = GpuDevice::Instance( ).GetDevice( )->CreateCommittedResource(
      &heapProperties,
      D3D12_HEAP_FLAG_NONE,
      &resourceDesc,
      D3D12_RESOURCE_STATE_GENERIC_READ,
      NULL,
      __uuidof( ID3D12Resource ),
      (void **) &pIndexBuffer
      );
   Debug::Assert( Condition( SUCCEEDED( hr ) ), "CreateCommittedResource: 0x%08x", hr );
   pIndexBuffer->SetName( L"QuadIB" );

   hr = pIndexBuffer->Map( 0, &readRange, reinterpret_cast<void**>( &pMappedData ) );
   Debug::Assert( Condition( SUCCEEDED( hr ) ), "pIndexBuffer->Map: 0x%08x", hr );

   memcpy( pMappedData, pIndices, indexSize );
   pIndexBuffer->Unmap( 0, NULL );

   // Initialize the vertex buffer view.
   D3D12_VERTEX_BUFFER_VIEW vertexBufferView = { };
   vertexBufferView.BufferLocation = pVertexBuffer->GetGPUVirtualAddress( );
   vertexBufferView.StrideInBytes = sizeof( QuadVertex );
   vertexBufferView.SizeInBytes = quadSize;

   D3D12_INDEX_BUFFER_VIEW indexBufferView = { };
   indexBufferView.BufferLocation = pIndexBuffer->GetGPUVirtualAddress( );
   indexBufferView.Format = DXGI_FORMAT_R16_UINT;
   indexBufferView.SizeInBytes = indexSize;

   desc.pCommandList->pList->IASetPrimitiveTopology( D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST );
   desc.pCommandList->pList->IASetVertexBuffers( 0, 1, &vertexBufferView );
   desc.pCommandList->pList->IASetIndexBuffer( &indexBufferView );
   desc.pCommandList->pList->DrawIndexedInstanced( numQuads * 6, 1, 0, 0, 0 );

   GpuDevice::Instance( ).AddPerFrameResource( pVertexBuffer );
   GpuDevice::Instance( ).AddPerFrameResource( pIndexBuffer );
#else
   #error Graphics API Undefined
#endif
}

VertexContext Quad::GetVertexContext( void )
{
   static VertexContext s_vertexContext = -1;

   if ( -1 == s_vertexContext )
   {
      D3D12_INPUT_ELEMENT_DESC inputElementDescs[ ] =
      {
         { "POSITION", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
         { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 8, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
         { "COLOR",    0, DXGI_FORMAT_R8G8B8A8_UINT, 0, 16, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
      };

      VertexElementDesc vertexDesc;
      vertexDesc.pElementDescs = inputElementDescs;
      vertexDesc.numElementDescs = 3;
      vertexDesc.topology = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;

      s_vertexContext = RenderContexts::RegisterVertexContext( vertexDesc );
   }

   return s_vertexContext;
}
