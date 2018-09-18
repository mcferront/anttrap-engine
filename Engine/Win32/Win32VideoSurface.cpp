#include "EnginePch.h"

#include "Win32VideoSurface.h"
#include "Dx9.h"
#include "TextureAsset.h"
#include "Log.h"
#include "RenderWorld.h"

void VideoSurface::Create( 
   ResourceHandle texture,
   Channel *pChannel
)
{
   ScopeLock lock( m_ValidLock );

   m_Texture  = texture;
   m_pChannel = pChannel;

   m_pSurfaces   = NULL;
   m_NumSurfaces = 0;
   m_Initialized = true;
}

void VideoSurface::Destroy( void )
{
   ScopeLock lock( m_ValidLock );

   m_Initialized = false;

   DeleteResources( );

   m_Texture = NULL;
}

 // IVMRSurfaceAllocator9
HRESULT VideoSurface::InitializeDevice( 
         /* [in] */ DWORD_PTR dwUserID,
         /* [in] */ VMR9AllocationInfo *lpAllocInfo,
         /* [out][in] */ DWORD *lpNumBuffers)
{
   ScopeLock lock( m_ValidLock );
   if ( false == m_Initialized ) return E_FAIL;

   D3DCAPS9 d3dcaps;

   if ( NULL == lpNumBuffers ) return E_POINTER;
   if ( NULL == m_pAllocatorNotify ) return E_FAIL;

   HRESULT hr;

   Dx9::Instance( ).GetDevice( )->GetDeviceCaps( &d3dcaps );

   int desiredWidth  = lpAllocInfo->dwWidth;
   int desiredHeight = lpAllocInfo->dwHeight;

   if( d3dcaps.TextureCaps & D3DPTEXTURECAPS_POW2 )
   {
      lpAllocInfo->dwWidth  = Math::MakePowerOf2( lpAllocInfo->dwWidth );
      lpAllocInfo->dwHeight = Math::MakePowerOf2( lpAllocInfo->dwHeight );
   }

   DeleteResources( );

   Texture *pTexture;;
   
   if ( TRUE == IsResourceLoaded(m_Texture) )
   {
      pTexture = GetResource( m_Texture, Texture );

      pTexture->RemoveFromScene( );
      pTexture->Destroy( );

      delete pTexture;
   }

   pTexture = new Texture;

   pTexture->Create( );
   pTexture->SetTexture( NULL, lpAllocInfo->dwWidth, lpAllocInfo->dwHeight, desiredWidth, desiredHeight, false );
   pTexture->AddToScene( );

   D3DDISPLAYMODE displayMode; 
   hr = Dx9::Instance( ).GetDevice( )->GetDisplayMode( NULL,  &displayMode );
   Debug::Assert( Condition(SUCCEEDED(hr)), "Failed to get d3d display mode" );


   // NOTE:
   // we need to make sure that we create textures because
   // surfaces can not be textured onto a primitive.
   lpAllocInfo->dwFlags |= VMR9AllocFlag_TextureSurface;

   m_NumSurfaces = *lpNumBuffers;
   m_pSurfaces = (IDirect3DSurface9 **) malloc( sizeof(IDirect3DSurface9 *) * m_NumSurfaces );
   memset( m_pSurfaces, 0, sizeof(IDirect3DSurface9 *) * m_NumSurfaces );

   m_pAllocatorNotify->AllocateSurfaceHelper( lpAllocInfo, lpNumBuffers, m_pSurfaces );

   // If we couldn't create a texture surface and 
   // the format is not an alpha format,
   // then we probably cannot create a texture.
   // So what we need to do is create a private texture
   // and copy the decoded images onto it.
   if ( FAILED(hr) && !(lpAllocInfo->dwFlags & VMR9AllocFlag_3DRenderTarget) )
   {
      LOG( "VMR Mixer failed" );
      return E_FAIL;
   }
   else
   {
      LOG( "Using VMR Mixer" );
   }

   return hr;
}

HRESULT VideoSurface::TerminateDevice( 
     /* [in] */ DWORD_PTR dwID
)
{
   ScopeLock lock( m_ValidLock );
   if ( false == m_Initialized ) return E_FAIL;

   DeleteResources( );
   return S_OK;
}
 
 HRESULT VideoSurface::GetSurface( 
     /* [in] */ DWORD_PTR dwUserID,
     /* [in] */ DWORD surfaceIndex,
     /* [in] */ DWORD surfaceFlags,
     /* [out] */ IDirect3DSurface9 **lplpSurface)
{
   {
      ScopeLock lock( m_ValidLock );
      if ( false == m_Initialized ) return E_FAIL;

      if ( NULL == lplpSurface )          return E_POINTER;
      if (surfaceIndex >= m_NumSurfaces ) return E_FAIL;

      *lplpSurface = m_pSurfaces[ surfaceIndex ];
       (*lplpSurface)->AddRef( );
   }

   //make sure renderer holds here
   RenderWorld::Instance( ).UseEvents( );

   //wait for renderer to finish
   RenderWorld::Instance( ).Wait( );

   return S_OK;
 }

 HRESULT VideoSurface::AdviseNotify(    
   IVMRSurfaceAllocatorNotify9 *pNotify
)
{
   ScopeLock lock( m_ValidLock );
   if ( false == m_Initialized ) return E_FAIL;

   HRESULT hr;
   HMONITOR hMonitor;

   m_pAllocatorNotify = pNotify;

   hMonitor = Dx9::Instance( ).GetDirect3D( )->GetAdapterMonitor( D3DADAPTER_DEFAULT );

   hr = m_pAllocatorNotify->SetD3DDevice( Dx9::Instance( ).GetDevice( ), hMonitor );
   Debug::Assert( Condition(SUCCEEDED(hr)), "Failed to set surface alloc notify d3d device" );

   return hr;
}

 // IVMRImagePresenter9
 HRESULT VideoSurface::StartPresenting( 
     /* [in] */ DWORD_PTR dwUserID)
 {
   return S_OK;
 }

 HRESULT VideoSurface::StopPresenting( 
     /* [in] */ DWORD_PTR dwUserID)
 {
    return S_OK;
 }
 
 HRESULT VideoSurface::PresentImage( 
     /* [in] */ DWORD_PTR dwUserID,
     /* [in] */ VMR9PresentationInfo *lpPresInfo)
 {

   ScopeLock lock( m_ValidLock );
   if ( false == m_Initialized ) return E_FAIL;

   HRESULT hr;
   
   // parameter validation
   if( lpPresInfo == NULL ) return E_POINTER;
   if( lpPresInfo->lpSurf == NULL ) return E_POINTER;

   IDirect3DTexture9 *pVideoTexture;

   hr = lpPresInfo->lpSurf->GetContainer(  __uuidof(IDirect3DTexture9), (LPVOID*) &pVideoTexture );
   Debug::Assert( Condition(SUCCEEDED(hr)), "Failed to get Direct3dTexture9 from D3D Resource" );

   Texture *pTexture = GetResource( m_Texture, Texture );

   pTexture->SetTexture( pVideoTexture, pTexture->GetActualWidth( ), pTexture->GetActualHeight( ), 
                         pTexture->GetDesiredWidth( ), pTexture->GetDesiredHeight( ), false );

   pVideoTexture->Release( );

   //tell the renderer to go
   RenderWorld::Instance( ).Go( );

   return hr;
 }
 
 // IUnknown
 HRESULT VideoSurface::QueryInterface( 
     REFIID riid,
     void** ppvObject)
 {
     HRESULT hr = E_NOINTERFACE;

    if( ppvObject == NULL ) {
        hr = E_POINTER;
    } 
    else if( riid == IID_IVMRSurfaceAllocator9 ) {
        *ppvObject = static_cast<IVMRSurfaceAllocator9*>( this );
        AddRef();
        hr = S_OK;
    } 
    else if( riid == IID_IVMRImagePresenter9 ) {
        *ppvObject = static_cast<IVMRImagePresenter9*>( this );
        AddRef();
        hr = S_OK;
    } 
    else if( riid == IID_IUnknown ) {
        *ppvObject = 
            static_cast<IUnknown*>( 
            static_cast<IVMRSurfaceAllocator9*>( this ) );
        AddRef();
        hr = S_OK;    
    }

    return hr;
 }

ULONG VideoSurface::AddRef( void )
{
   return InterlockedIncrement( &m_RefCount );
}

ULONG VideoSurface::Release( void )
{
   ULONG ret = InterlockedDecrement( &m_RefCount );
   
   if( ret == 0 )
   {
      delete this;
   }

   return ret; 
}

 void VideoSurface::DeleteResources( void )
{
   if ( IsResourceLoaded(m_Texture) )
   {
      Texture *pTexture = GetResource( m_Texture, Texture );

      pTexture->RemoveFromScene( );
      pTexture->Destroy( );

      delete pTexture;
   }

   DeleteSurfaces( );
}

void VideoSurface::DeleteSurfaces( void )
{
   ScopeLock lock( m_ValidLock );

   if ( true == IsResourceLoaded(m_Texture) )
   {
      Texture *pTexture = GetResource( m_Texture, Texture );

      pTexture->SetTexture( NULL, pTexture->GetActualWidth( ), pTexture->GetActualHeight( ), 
                                  pTexture->GetDesiredWidth( ), pTexture->GetDesiredHeight( ), false );
   }

   uint32 i;

   for ( i = 0; i < m_NumSurfaces; i++ )
   {
      if ( NULL != m_pSurfaces[i] ) m_pSurfaces[ i ]->Release( );
   }

   free( m_pSurfaces );

   m_pSurfaces   = NULL;
   m_NumSurfaces = 0;
}
