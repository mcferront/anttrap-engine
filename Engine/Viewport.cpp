#include "EnginePch.h"

#include "GraphicsApi.h"
#include "Viewport.h"
#include "RenderContexts.h"
#include "Node.h"
#include "CameraComponent.h"
#include "MaterialAsset.h"

void Viewport::Create(
   int width,
   int height,
   ResourceHandle camera,
   ResourceHandle renderTargets[],
   uint32 numTargets,
   ResourceHandle depthStencil
   )
{
   Create( Id::Create( ), width, height, camera, renderTargets, numTargets, depthStencil );
}

void Viewport::Create(
   Id id,
   int width,
   int height,
   ResourceHandle camera,
   ResourceHandle renderTargets[],
   uint32 numTargets,
   ResourceHandle depthStencil
   )
{
   SetId( id );

   memset( &m_Scissor, 0, sizeof( m_Scissor ) );

   m_IsScissored = false;

   m_hCamera = camera;
   m_hDepthStencil = depthStencil;
   m_NumRenderTargets = numTargets;

   for ( uint32 i = 0; i < numTargets; i++ )
      m_hRenderTargets[ i ] = renderTargets[ i ];

   m_ViewportContext = RenderContexts::RegisterViewportContext(*this);

   m_Width = width;
   m_Height = height;

   m_RenderScale.x = 1.0f;
   m_RenderScale.y = 1.0f;
   m_ClearFlags = ClearFlagsNone;

   m_ClearColor.a = 255;
   m_ClearColor.r = 0;
   m_ClearColor.g = 0;
   m_ClearColor.b = 0;
}

void Viewport::Create(
   const Viewport &viewport
   )
{
   m_hCamera = NullHandle;
   m_hDepthStencil = NullHandle;
   m_NumRenderTargets = 0;

   Copy( viewport );
}

void Viewport::Copy(
   const Viewport &copyFrom
   )
{
   SetId( copyFrom.GetId( ) );

   SetCamera( copyFrom.m_hCamera );

   m_Camera.Copy( copyFrom.m_Camera );

   m_Width = copyFrom.m_Width;
   m_Height = copyFrom.m_Height;
   m_Scissor = copyFrom.m_Scissor;
   m_Viewport = copyFrom.m_Viewport;
   m_Orthographic = copyFrom.m_Orthographic;
   m_IsScissored = copyFrom.m_IsScissored;
   m_NumRenderTargets = copyFrom.m_NumRenderTargets;
   m_ViewportContext = copyFrom.m_ViewportContext;
   m_NumRenderTargets = copyFrom.m_NumRenderTargets;
   m_RenderScale = copyFrom.m_RenderScale;
   m_ClearFlags = copyFrom.m_ClearFlags;
   m_ClearColor = copyFrom.m_ClearColor;

   m_hDepthStencil = copyFrom.m_hDepthStencil;
   
   for ( uint32 i = 0; i < m_NumRenderTargets; i++ )
      m_hRenderTargets[ i ] = copyFrom.m_hRenderTargets[ i ];
}

bool Viewport::ShouldRender( void )
{
   if ( m_hCamera != NullHandle )
   {
      if ( false == IsResourceLoaded( m_hCamera ) ) return false;

      CameraComponent *pCamera = GetResource( m_hCamera, Node )->GetComponent<CameraComponent>( );
      if ( NULL == pCamera ) return false;

      if ( false == pCamera->IsEnabled( ) ) return false;

      pCamera->GetCamera( )->PushVPMatrix( );

      m_Camera = *pCamera->GetCamera( );
   }

   return true;
}

bool Viewport::MakeActive( void )
{
#if defined DIRECTX12
   ImageBuffer *pDepthStencil;

   if ( m_hDepthStencil != NullHandle )
      pDepthStencil = GetResource( m_hDepthStencil, ImageBuffer );
   else 
      pDepthStencil = NULL;

   if ( NULL == pDepthStencil && 0 == m_NumRenderTargets )
      return false;

   ImageBuffer *pRenderTargets[ RenderContexts::MaxRenderTargets ];

   for ( uint32 i = 0; i < m_NumRenderTargets; i++ )
      pRenderTargets[i] = GetResource( m_hRenderTargets[i], ImageBuffer );
 
   ImageBuffer *pDimn = m_NumRenderTargets > 0 ? pRenderTargets[0] : pDepthStencil;
   
   ViewportRect viewport =
   {
      (float) 0,
      (float) 0,
      (float) m_Width,
      (float) m_Height,
      0.0f,
      1.0f
   };

   Vector camViewport;
   
   if ( m_hCamera != NullHandle )
      m_Camera.GetViewportRect( &camViewport );
   else
      camViewport.Set( 0, 0, 1, 1 );

   m_Viewport.left = viewport.left + viewport.width * camViewport.x;
   m_Viewport.top = viewport.top + viewport.height * camViewport.y;
   m_Viewport.width = viewport.width * camViewport.z;
   m_Viewport.height = viewport.height * camViewport.w;
   m_Viewport.nearZ = viewport.nearZ;
   m_Viewport.farZ = viewport.farZ;

   if ( (int) viewport.width > GpuDevice::Instance( ).GetSwapChain( )->width )
   viewport.width = (float) GpuDevice::Instance( ).GetSwapChain( )->width;

   if ( (int) viewport.height > GpuDevice::Instance( ).GetSwapChain( )->height )
   viewport.height = (float) GpuDevice::Instance( ).GetSwapChain( )->height;

   if ( ( (int) viewport.width ) <= 0 || ( (int) viewport.height ) <= 0 )
      return false;

   GpuDevice::CommandList *pList = GpuDevice::Instance( ).AllocGraphicsCommandList( );

   // TODO: these should be handled in the render trees
   // but it requires the viewport to not be made active before they're sest
   // maybe we make setting the viewport part of the render tree operations or nodes
   for ( uint32 i = 0; i < m_NumRenderTargets; i++ )
      pRenderTargets[i]->ConvertTo( ImageBuffer::RenderTarget, pList );

   if ( NULL != pDepthStencil )
      pDepthStencil->ConvertTo( ImageBuffer::DepthWriteResource, pList );

   Set( pList );

   //TODO: clear stencil
   bool clearColor = ( m_ClearFlags & Viewport::ClearFlagsColor ) != 0;
   bool clearDepth = ( m_ClearFlags & Viewport::ClearFlagsDepth ) != 0;
   bool clearStencil = false;

	for ( uint32 i = 0; i < m_NumRenderTargets; i++ )
		GpuDevice::ClearRenderTarget( pList, pRenderTargets[i], clearColor, &m_ClearColor );
   
	if ( NULL != pDepthStencil )
		GpuDevice::ClearDepthStencil( pList, pDepthStencil, clearDepth, 0.0f, clearStencil, 0 );

   GpuDevice::Instance( ).ExecuteCommandLists( &pList, 1 );
#else
#error Graphics API Undefined
#endif

   return true;
}

void Viewport::Set( 
   GpuDevice::CommandList *pCommandList
   )
{
#if defined DIRECTX12
   D3D12_RECT scissor;
   D3D12_VIEWPORT viewport;

   viewport.TopLeftX = m_Viewport.left;
   viewport.TopLeftY = m_Viewport.top;
   viewport.Height = m_Viewport.height;
   viewport.Width = m_Viewport.width;
   viewport.MinDepth = m_Viewport.nearZ;
   viewport.MaxDepth = m_Viewport.farZ;

   if ( true == m_IsScissored )
   {
      scissor.left = m_Scissor.left;
      scissor.top = m_Scissor.top;
      scissor.right = m_Scissor.right;
      scissor.bottom = m_Scissor.bottom;
   }
   else
   {
      scissor.left = 0;
      scissor.top = 0;
      scissor.right = (LONG) m_Viewport.width;
      scissor.bottom = (LONG) m_Viewport.height;
   }

   ImageBuffer *pDepthStencil, *pRenderTargets[ RenderContexts::MaxRenderTargets ];

   for ( uint32 i = 0; i < m_NumRenderTargets; i++ )
      pRenderTargets[i] = GetResource( m_hRenderTargets[i], ImageBuffer );

   if ( m_hDepthStencil != NullHandle )
      pDepthStencil = GetResource( m_hDepthStencil, ImageBuffer );
   else
      pDepthStencil = NULL;

   GpuDevice::SetRenderTargets( pCommandList, pRenderTargets, m_NumRenderTargets, pDepthStencil );

   pCommandList->pList->RSSetViewports( 1, &viewport );
   pCommandList->pList->RSSetScissorRects( 1, &scissor );
#endif
}

ResourceHandle Viewport::GetRenderTarget( 
   uint32 index
) const
{
   Debug::Assert( Condition(index < m_NumRenderTargets), "Invalid Index" );
   return m_hRenderTargets[ index ];
}

ResourceHandle Viewport::GetDepthStencil( void ) const
{
   return m_hDepthStencil;
}

void Viewport::SetCamera(
   ResourceHandle camera
   )
{
   m_hCamera = camera;
}

void Viewport::SetScissor(
   int left,
   int top,
   int right,
   int bottom
   )
{
   m_Scissor.left = left;
   m_Scissor.top = top;
   m_Scissor.right = right;
   m_Scissor.bottom = bottom;

   m_IsScissored = true;
}

float Viewport::GetProjectedWidth(
   uint32 pixels
   ) const
{
   return pixels / (float) m_Width * 2.0f;
}

float Viewport::GetProjectedHeight(
   uint32 pixels
   ) const
{
   return pixels / (float) m_Height * 2.0f;
}

void Viewport::VirtualToProjected(
   float *pX,
   float *pY,
   int x,
   int y
   ) const
{
   *pX = 2.0f * ( (float) x / m_Width ) + -1.0f;
   *pY = 2.0f * ( (float) y / m_Height ) + -1.0f;
}

void Viewport::PhysicalToProjected(
   float *pX,
   float *pY,
   int x,
   int y
   ) const
{
   ::PhysicalToProjected( pX, pY, x, y, (int) m_Camera.GetWidth( ), (int) m_Camera.GetHeight( ), 0, 0, m_Width, m_Height );
}

#include "MaterialObject.h"

Vector2 GetMaterialSize(
   const GraphicsMaterialObject *pMaterial
   )
{
   if ( NULL == pMaterial )
      return Math::ZeroVector2( );

   const GraphicsMaterial::PassData *pPass = pMaterial->GetPassData( "UI" );

   Vector2 v = Math::ZeroVector2( );

   do
   {
      if ( NULL == pPass )
         break;

      ResourceHandle texture = pPass->GetTexture( 0 );
      if ( false == IsResourceLoaded( texture ) )
         break;

      Texture *pTexture = GetResource( texture, Texture );

      float quadWidth = (float) pTexture->GetDesiredWidth( );
      float quadHeight = (float) pTexture->GetDesiredHeight( );

      v = Vector2( quadWidth, quadHeight );

   } while ( false );

   return v;
}

Vector2 GetMaterialUV(
   const GraphicsMaterialObject *pMaterial
   )
{
   if ( NULL == pMaterial )
      return Math::ZeroVector2( );

   const GraphicsMaterial::PassData *pPass = NULL;

   float u = 1.0f, v = 1.0f;

   do
   {
      pPass = pMaterial->GetPassData( "UI" );
      if ( NULL == pPass )
         break;

      ResourceHandle texture = pPass->GetTexture( 0 );
      if ( false == IsResourceLoaded( texture ) )
         break;

      Texture *pTexture = GetResource( texture, Texture );

      u = pTexture->GetDesiredWidth( ) / (float) pTexture->GetActualWidth( );
      v = pTexture->GetDesiredHeight( ) / (float) pTexture->GetActualHeight( );

   } while ( false );

   return Vector2( u, v );
}

Vector2 VirtualSizeToProjected(
   const Viewport &viewport,
   const GraphicsMaterialObject *pMaterial,
   int x,
   int y
   )
{
   Vector2 quad = GetMaterialSize( pMaterial );

   float quadWidth = quad.x;
   float quadHeight = quad.y;

   float aspectRatio = quadWidth / quadHeight;

   Vector2 project;

   project.x = x == 0.0f ? viewport.GetProjectedWidth( (uint32) quadWidth ) : x;

   //if y == 0
   //we always return pSize->x / aspectRatio - which will be the same as projected quadHeight if x == 0
   //or it will be the correct aspect ratio height for the user specified m_Size.x
   project.y = y == 0.0f ? project.x / aspectRatio : y;

   return project;
}

Vector2 VirtualSizeToProjected(
   const Viewport &viewport,
   const GraphicsMaterialObject *pMaterial
   )
{
   return VirtualSizeToProjected( viewport, pMaterial, 0, 0 );
}

Vector2 VirtualSizeToProjected(
   const Viewport &viewport,
   int x,
   int y
   )
{
   Vector2 project;

   project.x = viewport.GetProjectedWidth( x );
   project.y = viewport.GetProjectedHeight( y );

   return project;
}

Vector2 VirtualPosToProjected(
   const Viewport &viewport,
   int x,
   int y
   )
{
   Vector2 project;

   viewport.VirtualToProjected( &project.x, &project.y, x, y );

   return project;
}

void PhysicalToProjected(
   float *pX,
   float *pY,
   int x,
   int y,
   int cameraWidth,
   int cameraHeight,
   int viewportLeft,
   int viewportTop,
   int viewportWidth,
   int viewportHeight
   )
{
   Vector touch = Math::ZeroVector( );

   x = x - viewportLeft;
   y = y - viewportTop;

   touch.x = 2.0f * ( (float) x / viewportWidth ) + -1.0f;
   touch.y = 2.0f * ( 1.0f - (float) y / viewportHeight ) + -1.0f;

   //Matrix invRotation;

   //Math::Invert( &invRotation, mode.GetRotation( ) );
   //Math::Rotate( &touch, touch, invRotation );

   *pX = touch.x * ( cameraWidth / 2.0f );
   *pY = touch.y * ( cameraHeight / 2.0f );
}

