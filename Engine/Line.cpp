#include "EnginePch.h"

#include "Line.h"
#include "Geometry.h"
#include "Renderer.h"
#include "Viewport.h"
#include "MaterialObject.h"
#include "TaskWorld.h"
#include "RenderContexts.h"

void Line::Create(
   const Vector &_start,
   const Vector &_startColor,
   const Vector &_end,
   const Vector &_endColor
   )
{
   start = _start;
   startColor = _startColor;
   end = _end;
   endColor = _endColor;
}

void Line::Render(
   GraphicsMaterialObject *pMaterial,
   const Transform &transform,
   const RenderDesc &desc,
   RenderStats *pStats
   )
{
   Line::Render( this, 1, false, pMaterial, transform,  desc, pStats );
}

void Line::Render(
   Line *pLines,
   uint32 numLines,
   bool selected,
   GraphicsMaterialObject *pMaterial,
   const Transform &transform,
   const RenderDesc &desc,
   RenderStats *pStats
   )
{
   if ( 0 == numLines ) return;

   struct LineVertex
   {
      Vector position;
      byte color[4];
      Vector2 uv;
   };

   uint32 lineSizeInBytes = numLines * 2 * sizeof( LineVertex );
   unsigned char *pBytes = (unsigned char *) desc.pTask->AllocTLM( lineSizeInBytes );

   LineVertex *pHead = (LineVertex *) pBytes;
   LineVertex *pVert = pHead;

   uint32 i;

   for ( i = 0; i < numLines; i++ )
   {
      pVert->position = pLines[ i ].start;
      pVert->color[0] = Math::FloatToInt( pLines[ i ].startColor.x * 255.0f );
      pVert->color[1] = Math::FloatToInt( pLines[ i ].startColor.y * 255.0f );
      pVert->color[2] = Math::FloatToInt( pLines[ i ].startColor.z * 255.0f );
      pVert->color[3] = Math::FloatToInt( pLines[ i ].startColor.w * 255.0f );
      pVert->uv = Math::ZeroVector2( );
      pVert++;

      pVert->position = pLines[ i ].end;
      pVert->color[0] = Math::FloatToInt( pLines[ i ].endColor.x * 255.0f );
      pVert->color[1] = Math::FloatToInt( pLines[ i ].endColor.y * 255.0f );
      pVert->color[2] = Math::FloatToInt( pLines[ i ].endColor.z * 255.0f );
      pVert->color[3] = Math::FloatToInt( pLines[ i ].endColor.w * 255.0f );
      pVert->uv = Math::ZeroVector2( );
      pVert++;
   }

   GraphicsMaterialObject::Pass *pPass = pMaterial->GetPass( desc.pDesc->renderContext );

   Transform viewTransform;
   desc.pViewport->GetCamera( )->GetViewTransform( &viewTransform );

   Matrix projection;
   desc.pViewport->GetCamera( )->GetReverseDepthProjection( &projection );

   static const char *pWorld = StringRef( "$WORLD_MATRIX" );
   static const char *pView = StringRef( "$VIEW_MATRIX" );
   static const char *pVP = StringRef( "$VP_MATRIX" );
   static const char *pProjection = StringRef( "$PROJECTION_MATRIX" );
   static const char *pColor = StringRef( "$COLOR" );

   pPass->GetData()->SetMacro( pWorld, &transform.ToMatrix( true ), 1 );
   pPass->GetData()->SetMacro( pView, &viewTransform.ToMatrix( true ), 1 );
   pPass->GetData()->SetMacro( pProjection, &projection, 1 );
   pPass->GetData()->SetMacro( pColor, &Math::OneVector( ), 1 );

   Matrix vp;
   Math::Multiply( &vp, viewTransform.ToMatrix(true), projection );
   pPass->GetData()->SetMacro( pVP, &vp, 1 );

   pMaterial->SetRenderData( pPass, desc.pCommandList );

#ifdef OPENGL
   pglBindBuffer( GL_ARRAY_BUFFER, 0 );
   glCheckError( "Line::Bind Array Buffer" );

   pglBindBuffer( GL_ELEMENT_ARRAY_BUFFER, 0 );
   glCheckError( "Line::Bind Element Array Buffer" );

   char *pData = (char *) pHead;

   pglVertexAttribPointer( VertexBuffer::Positions, 3, GL_FLOAT, false, sizeof( LineVertex ), pData );
   pglEnableVertexAttribArray( VertexBuffer::Positions );
   glCheckError( "Line::Bind Position" );

   pglVertexAttribPointer( VertexBuffer::Colors, 4, GL_FLOAT, false, sizeof( LineVertex ), pData + sizeof( Vector ) );
   pglEnableVertexAttribArray( VertexBuffer::Colors );
   glCheckError( "Line::Bind Color" );

   pglVertexAttribPointer( VertexBuffer::UV0s, 2, GL_FLOAT, false, sizeof( LineVertex ), pData + sizeof( Vector ) + sizeof( Vector ) );
   glCheckError( "Line::VertexAttribPointer" );

   pglEnableVertexAttribArray( VertexBuffer::UV0s );
   glCheckError( "Line::EnableVertexArray" );

   glDrawArrays( GL_LINES, 0, numLines );
#elif defined DIRECTX9
   char *pData = (char *) pHead;

   HRESULT hr;

   hr = Dx9::Instance( ).GetDevice( )->SetVertexDeclaration( Dx9::Instance( ).GetLineDecl( ) );
   Debug::Print( Condition( SUCCEEDED( hr ) ), Debug::TypeError, "SetVertexDeclaration failed 0x%08x\n", hr );

   hr = Dx9::Instance( ).GetDevice( )->DrawPrimitiveUP( D3DPT_LINELIST, numLines, pData, sizeof( LineVertex ) );
   Debug::Print( Condition( SUCCEEDED( hr ) ), Debug::TypeError, "Failed to DrawPrimitiveUP: 0x%08x\n", hr );
#elif defined DIRECTX12
   // TODO: DX12 - should we have a large vb mapped for line/quad/tri and everything gets batched to that?
   ID3D12Resource *pVertexBuffer;
   D3D12_VERTEX_BUFFER_VIEW vertexBufferView = { };

   D3D12_HEAP_PROPERTIES heapProperties = { };
   heapProperties.Type = D3D12_HEAP_TYPE_UPLOAD;
   heapProperties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
   heapProperties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
   heapProperties.CreationNodeMask = 1;
   heapProperties.VisibleNodeMask = 1;

   D3D12_RESOURCE_DESC resourceDesc = { };
   resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
   resourceDesc.Alignment = 0;
   resourceDesc.Width = lineSizeInBytes;
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

   // Copy the triangle data to the vertex buffer.
   void *pMappedData;
   D3D12_RANGE readRange = {0, 0};
   hr = pVertexBuffer->Map( 0, &readRange, reinterpret_cast<void**>( &pMappedData ) );
   Debug::Assert( Condition( SUCCEEDED( hr ) ), "pVertexBuffer->Map: 0x%08x", hr );
   
   memcpy( pMappedData, pHead, lineSizeInBytes );
   pVertexBuffer->Unmap( 0, NULL );
   
   // Initialize the vertex buffer view.
   vertexBufferView.BufferLocation = pVertexBuffer->GetGPUVirtualAddress( );
   vertexBufferView.StrideInBytes = sizeof( LineVertex );
   vertexBufferView.SizeInBytes = lineSizeInBytes;

   desc.pCommandList->pList->IASetPrimitiveTopology( D3D_PRIMITIVE_TOPOLOGY_LINELIST );
   desc.pCommandList->pList->IASetVertexBuffers( 0, 1, &vertexBufferView );
   desc.pCommandList->pList->DrawInstanced( numLines * 2, 1, 0, 0 );

   // TODO: DX12 - how should we handle this stuff?
   GpuDevice::Instance( ).AddPerFrameResource( pVertexBuffer );
#else
#error Graphics API Undefined
#endif
}

VertexContext Triangle::GetVertexContext( void )
{
   static VertexContext s_vertexContext = -1;

   if ( -1 == s_vertexContext )
   {
      D3D12_INPUT_ELEMENT_DESC inputElementDescs[ ] =
      {
         { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
         { "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 16, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
         { "COLOR", 0, DXGI_FORMAT_R8G8B8A8_UINT, 0, 32, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
         { "TEXCOORD",    0, DXGI_FORMAT_R32G32_FLOAT, 0, 36, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
      };

      VertexElementDesc vertexDesc;
      vertexDesc.pElementDescs = inputElementDescs;
      vertexDesc.numElementDescs = 3;
      vertexDesc.topology = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
   
      s_vertexContext = RenderContexts::RegisterVertexContext( vertexDesc );
   }

   return s_vertexContext;
}

void Triangle::Render(
   Triangle *pTriangles,
   uint32 numTriangles,
   bool selected,
   GraphicsMaterialObject *pMaterial,
   const Transform &transform,
   const RenderDesc &desc,
   RenderStats *pStats
   )
{
   if ( 0 == numTriangles ) return;

   struct TriangleVertex
   {
      Vector position;
      Vector normal;
      byte color[4];
      Vector2 uv;
   };

   uint32 triangleSizeInBytes = numTriangles * 3 * sizeof( TriangleVertex );

   unsigned char *pBytes = (unsigned char *) desc.pTask->AllocTLM( triangleSizeInBytes );

   TriangleVertex *pHead = (TriangleVertex *) pBytes;
   TriangleVertex *pVert = pHead;

   uint32 i;

   pStats->num_faces = numTriangles;
   pStats->num_verts = numTriangles * 3;

   for ( i = 0; i < numTriangles; i++ )
   {
      pVert->position = pTriangles[ i ].vertices[ 0 ].position;
      pVert->normal = pTriangles[ i ].vertices[ 0 ].normal;
      pVert->color[ 0 ] = Math::FloatToInt( pTriangles[ i ].vertices[ 0 ].color.x * 255.0f );
      pVert->color[ 1 ] = Math::FloatToInt( pTriangles[ i ].vertices[ 0 ].color.y * 255.0f );
      pVert->color[ 2 ] = Math::FloatToInt( pTriangles[ i ].vertices[ 0 ].color.z * 255.0f );
      pVert->color[ 3 ] = Math::FloatToInt( pTriangles[ i ].vertices[ 0 ].color.w * 255.0f );
      pVert->uv = Math::ZeroVector2( );
      pVert++;

      pVert->position = pTriangles[ i ].vertices[ 1 ].position;
      pVert->normal = pTriangles[ i ].vertices[ 1 ].normal;
      pVert->color[ 0 ] = Math::FloatToInt( pTriangles[ i ].vertices[ 1 ].color.x * 255.0f );
      pVert->color[ 1 ] = Math::FloatToInt( pTriangles[ i ].vertices[ 1 ].color.y * 255.0f );
      pVert->color[ 2 ] = Math::FloatToInt( pTriangles[ i ].vertices[ 1 ].color.z * 255.0f );
      pVert->color[ 3 ] = Math::FloatToInt( pTriangles[ i ].vertices[ 1 ].color.w * 255.0f );
      pVert->uv = Math::ZeroVector2( );
      pVert++;

      pVert->position = pTriangles[ i ].vertices[ 2 ].position;
      pVert->normal = pTriangles[ i ].vertices[ 2 ].normal;
      pVert->color[ 0 ] = Math::FloatToInt( pTriangles[ i ].vertices[ 2 ].color.x * 255.0f );
      pVert->color[ 1 ] = Math::FloatToInt( pTriangles[ i ].vertices[ 2 ].color.y * 255.0f );
      pVert->color[ 2 ] = Math::FloatToInt( pTriangles[ i ].vertices[ 2 ].color.z * 255.0f );
      pVert->color[ 3 ] = Math::FloatToInt( pTriangles[ i ].vertices[ 2 ].color.w * 255.0f );
      pVert->uv = Math::ZeroVector2( );
      pVert++;
   }

   GraphicsMaterialObject::Pass *pPass = pMaterial->GetPass( desc.pDesc->renderContext );

   pPass->GetData()->SetMacro( "$WORLD_MATRIX", &transform.ToMatrix( true ), 1 );
   pMaterial->SetRenderData( pPass, desc.pCommandList );

#ifdef OPENGL
   pglBindBuffer( GL_ARRAY_BUFFER, 0 );
   glCheckError( "Triangle::Bind Array Buffer" );

   pglBindBuffer( GL_ELEMENT_ARRAY_BUFFER, 0 );
   glCheckError( "Triangle::Bind Element Array Buffer" );

   char *pData = (char *) pHead;

   pglVertexAttribPointer( VertexBuffer::Positions, 3, GL_FLOAT, false, sizeof( TriangleVertex ), pData );
   pglEnableVertexAttribArray( VertexBuffer::Positions );
   glCheckError( "Triangle::Bind Position" );

   pglVertexAttribPointer( VertexBuffer::Normals, 3, GL_FLOAT, false, sizeof( TriangleVertex ), pData + sizeof( Vector ) );
   pglEnableVertexAttribArray( VertexBuffer::Normals );
   glCheckError( "Triangle::Bind Color" );

   pglVertexAttribPointer( VertexBuffer::Colors, 4, GL_FLOAT, false, sizeof( TriangleVertex ), pData + sizeof( Vector ) + sizeof( Vector ) );
   pglEnableVertexAttribArray( VertexBuffer::Colors );
   glCheckError( "Triangle::Bind Color" );

   pglVertexAttribPointer( VertexBuffer::UV0s, 2, GL_FLOAT, false, sizeof( TriangleVertex ), pData + sizeof( Vector )++sizeof( Vector ) + sizeof( Vector ) );
   pglEnableVertexAttribArray( VertexBuffer::UV0s );
   glCheckError( "Triangle::VertexAttribPointer" );

   glDrawArrays( GL_TRIANGLES, 0, numTriangles );
#elif defined DIRECTX9
   char *pData = (char *) pHead;

   HRESULT hr;

   hr = Dx9::Instance( ).GetDevice( )->SetVertexDeclaration( Dx9::Instance( ).GetLit3dDecl( ) );
   Debug::Print( Condition( SUCCEEDED( hr ) ), Debug::TypeError, "SetVertexDeclaration failed 0x%08x\n", hr );

   hr = Dx9::Instance( ).GetDevice( )->DrawPrimitiveUP( D3DPT_TRIANGLELIST, numTriangles, pData, sizeof( TriangleVertex ) );
   Debug::Print( Condition( SUCCEEDED( hr ) ), Debug::TypeError, "Failed to DrawPrimitiveUP: 0x%08x\n", hr );
#elif defined DIRECTX12
   char *pData = (char *) pHead;
   ID3D12Resource *pVertexBuffer;

   D3D12_HEAP_PROPERTIES heapProperties = { };
   heapProperties.Type = D3D12_HEAP_TYPE_UPLOAD;
   heapProperties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
   heapProperties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
   heapProperties.CreationNodeMask = 1;
   heapProperties.VisibleNodeMask = 1;

   D3D12_RESOURCE_DESC resourceDesc = { };
   resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
   resourceDesc.Alignment = 0;
   resourceDesc.Width = triangleSizeInBytes;
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

   // Copy the triangle data to the vertex buffer.
   void **pMappedData;
   D3D12_RANGE readRange = { };
   hr = pVertexBuffer->Map( 0, &readRange, reinterpret_cast<void**>( &pMappedData ) );
   Debug::Assert( Condition( SUCCEEDED( hr ) ), "pVertexBuffer->Map: 0x%08x", hr );

   memcpy( pMappedData, pData, triangleSizeInBytes );
   pVertexBuffer->Unmap( 0, NULL );

   // Initialize the vertex buffer view.
   D3D12_VERTEX_BUFFER_VIEW vertexBufferView = { };
   vertexBufferView.BufferLocation = pVertexBuffer->GetGPUVirtualAddress( );
   vertexBufferView.StrideInBytes = sizeof( TriangleVertex );
   vertexBufferView.SizeInBytes = triangleSizeInBytes;

   desc.pCommandList->pList->IASetPrimitiveTopology( D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST );
   desc.pCommandList->pList->IASetVertexBuffers( 0, 1, &vertexBufferView );
   desc.pCommandList->pList->DrawInstanced( numTriangles * 3, 1, 0, 0 );

   GpuDevice::Instance( ).AddPerFrameResource( pVertexBuffer );
#else
#error Graphics API Undefined
#endif
}

VertexContext Line::GetVertexContext( void )
{
   static VertexContext s_vertexContext = -1;

   if ( -1 == s_vertexContext )
   {
      D3D12_INPUT_ELEMENT_DESC inputElementDescs[ ] =
      {
         { "POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
         { "COLOR",    0, DXGI_FORMAT_R8G8B8A8_UINT, 0, 16, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
         { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 20, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
      };

      VertexElementDesc vertexDesc;
      vertexDesc.pElementDescs = inputElementDescs;
      vertexDesc.numElementDescs = 3;
      vertexDesc.topology = D3D12_PRIMITIVE_TOPOLOGY_TYPE_LINE;

      s_vertexContext = RenderContexts::RegisterVertexContext( vertexDesc );
   }

   return s_vertexContext;
}
