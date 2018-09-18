#include "EnginePch.h"

#include "Dx9.h"
#include "Log.h"
#include "ChannelSystem.h"
#include "Channel.h"

Dx9 &Dx9::Instance( void )
{
    static Dx9 s_instance;
    return s_instance;
}

void Dx9::Create(
    HWND hwnd,
    bool windowed
    )
{
    HRESULT hr;
    D3DDISPLAYMODE dm;

    m_DeviceIsValid = false;
    m_NeedsReset = false;

    m_pD3d = Direct3DCreate9( D3D_SDK_VERSION );

    hr = m_pD3d->GetAdapterDisplayMode( D3DADAPTER_DEFAULT, &dm);
    Debug::Assert( Condition(SUCCEEDED(hr)), "Failed to create d3d" );

    D3DPRESENT_PARAMETERS pp = { 0 };
    pp.Windowed         = true == windowed;
    pp.hDeviceWindow    = hwnd;
    pp.SwapEffect       = D3DSWAPEFFECT_DISCARD;
    pp.BackBufferFormat = dm.Format;
    pp.BackBufferCount  = 1;
    pp.EnableAutoDepthStencil = TRUE;
    pp.AutoDepthStencilFormat = D3DFMT_D16;
    pp.PresentationInterval   = D3DPRESENT_INTERVAL_DEFAULT;

    if ( false == windowed )
    {
        pp.BackBufferWidth  = dm.Width;
        pp.BackBufferHeight = dm.Height;
    }

    hr = m_pD3d->CreateDevice( D3DADAPTER_DEFAULT,
        D3DDEVTYPE_HAL,
        hwnd,
        D3DCREATE_MIXED_VERTEXPROCESSING | 
        D3DCREATE_MULTITHREADED,
        &pp,
        &m_pD3dDevice );
    Debug::Assert( Condition(SUCCEEDED(hr)), "Failed to create d3d device" );

    hr = m_pD3dDevice->GetRenderTarget( 0, &m_pRenderTarget );
    Debug::Assert( Condition(SUCCEEDED(hr)), "Failed to get default render target" );

    m_PresentParams = pp;
    m_DeviceIsValid = true;

    {
        D3DVERTEXELEMENT9 declaration[] =
        {
            { 0, 0,  D3DDECLTYPE_FLOAT2,   D3DDECLMETHOD_DEFAULT,  D3DDECLUSAGE_POSITION, 0 },
            { 0, 8,  D3DDECLTYPE_FLOAT2,   D3DDECLMETHOD_DEFAULT,  D3DDECLUSAGE_TEXCOORD, 0 },
            { 0, 16, D3DDECLTYPE_FLOAT4,   D3DDECLMETHOD_DEFAULT,  D3DDECLUSAGE_COLOR,    0 },
            D3DDECL_END()
        };

        hr = Dx9::Instance( ).GetDevice( )->CreateVertexDeclaration( declaration, &m_pQuadDecl );
        Debug::Assert( Condition(SUCCEEDED(hr)), "Failed to create vertex decl" );
    }

    {
        D3DVERTEXELEMENT9 declaration[] =
        {
            { 0, 0,  D3DDECLTYPE_FLOAT3,   D3DDECLMETHOD_DEFAULT,  D3DDECLUSAGE_POSITION, 0 },
            { 0, 16, D3DDECLTYPE_FLOAT3,   D3DDECLMETHOD_DEFAULT,  D3DDECLUSAGE_NORMAL,   0 },
            { 0, 32, D3DDECLTYPE_FLOAT4,   D3DDECLMETHOD_DEFAULT,  D3DDECLUSAGE_COLOR,    0 },
            { 0, 48, D3DDECLTYPE_FLOAT2,   D3DDECLMETHOD_DEFAULT,  D3DDECLUSAGE_TEXCOORD, 0 },
            D3DDECL_END()
        };

        hr = Dx9::Instance( ).GetDevice( )->CreateVertexDeclaration( declaration, &m_pLit3dDecl );
        Debug::Assert( Condition(SUCCEEDED(hr)), "Failed to create vertex decl" );
    }

    {
        D3DVERTEXELEMENT9 declaration[] =
        {
            { 0, 0,   D3DDECLTYPE_FLOAT3,   D3DDECLMETHOD_DEFAULT,  D3DDECLUSAGE_POSITION, 0 },
            { 0, 12,  D3DDECLTYPE_FLOAT2,   D3DDECLMETHOD_DEFAULT,  D3DDECLUSAGE_TEXCOORD, 0 },
            D3DDECL_END()
        };

        hr = Dx9::Instance( ).GetDevice( )->CreateVertexDeclaration( declaration, &m_pParticleQuadDecl );
        Debug::Assert( Condition(SUCCEEDED(hr)), "Failed to create vertex decl" );
    }

    {
        D3DVERTEXELEMENT9 declaration[] =
        {
            { 0, 0,  D3DDECLTYPE_FLOAT3,   D3DDECLMETHOD_DEFAULT,  D3DDECLUSAGE_POSITION, 0 },
            { 0, 16, D3DDECLTYPE_FLOAT4,   D3DDECLMETHOD_DEFAULT,  D3DDECLUSAGE_COLOR,    0 },
            { 0, 32, D3DDECLTYPE_FLOAT2,   D3DDECLMETHOD_DEFAULT,  D3DDECLUSAGE_TEXCOORD, 0 },
            D3DDECL_END()
        };

        hr = Dx9::Instance( ).GetDevice( )->CreateVertexDeclaration( declaration, &m_pLineDecl );
        Debug::Assert( Condition(SUCCEEDED(hr)), "Failed to create vertex decl" );
    }

    SwapChain chain = {hwnd, NULL};
    m_SwapChain = chain;

    m_SwapChains.Create( );
    m_SwapChains.Add( m_SwapChain );
}

void Dx9::Destroy( void )
{
    {
        List<SwapChain>::Enumerator e = m_SwapChains.GetEnumerator( );

        while ( e.EnumNext( ) )
        {
            if ( e.Data().pSwapChain ) 
            {
                e.Data().pBackBuffer->Release( );
                e.Data().pSwapChain->Release( );
            }
        }
    }

    m_SwapChains.Destroy( );

    m_pLineDecl->Release( );
    m_pQuadDecl->Release( );
    m_pLit3dDecl->Release( );
    m_pParticleQuadDecl->Release( );
    m_pRenderTarget->Release( );
    m_pD3dDevice->Release( );
    m_pD3d->Release( );
}

void Dx9::CreateChannel( void )
{
    m_pChannel = new Channel;
    m_pChannel->Create( Id::Create( ), NULL );

    ChannelSystem::Instance( ).Add( m_pChannel );
}

void Dx9::DestroyChannel( void )
{
    ChannelSystem::Instance( ).Remove( m_pChannel );

    m_pChannel->Destroy( );
    delete m_pChannel;
}

HRESULT Dx9::Present( void )
{
    if (true == m_NeedsReset) return D3DERR_DEVICELOST;

    //use default back buffer?
    if ( NULL == m_SwapChain.pSwapChain )
    {
        return m_pD3dDevice->Present( NULL, NULL, NULL, NULL );
    }

    //use a swap chain
    else
    {
        return m_SwapChain.pSwapChain->Present( NULL, NULL, NULL, NULL, 0 );
    }
}

void Dx9::RestoreRenderTarget( void )
{
    HRESULT hr;

    //use default back buffer?
    if ( NULL == m_SwapChain.pSwapChain )
    {
        hr = m_pD3dDevice->SetRenderTarget( 0, m_pRenderTarget );
        Debug::Assert( Condition(SUCCEEDED(hr)), "Failed set render target: 0x%08x", hr );
    }

    //use a swap chain
    else
    {
        hr = m_pD3dDevice->SetRenderTarget( 0, m_SwapChain.pBackBuffer );
        Debug::Assert( Condition(SUCCEEDED(hr)), "Failed set render target: 0x%08x", hr );
    }
}

void Dx9::DeviceLost( void )
{
    if ( true == HasDevice( ) )
    {
        //must queue because we could be 
        //lost on a different rendering thread
        m_pChannel->QueueEvent( "DeviceLost", ArgList() );
        LOG( "Direct3d Device lost" );

        m_pRenderTarget->Release( );

        m_DeviceIsValid = false;
    }
}

void Dx9::ResetDevice( void )
{
    //sometimes reset is called by our window activate as the device
    //is being created - we definitely want to ignore this
    if ( NULL == m_pD3dDevice ) return;

    if ( true == HasDevice( ) ) return;

    HRESULT hr;

    LOG( "Attempting to Restore Device" );

    int size = m_SwapChains.GetSize();

    for ( int i = 0; i < size; i++ )
    {
        SwapChain *pChain = m_SwapChains.GetPointer(i);

        //if there is a swapchain with this window
        //and the window size changed then force a recreate
        if ( NULL != pChain->pSwapChain )
        {
            pChain->pSwapChain->Release( );
            pChain->pBackBuffer->Release( );
        }
    }

    m_SwapChains.Clear( );

    hr = m_pD3dDevice->Reset( &m_PresentParams );

    if ( SUCCEEDED(hr) )
    {
        m_NeedsReset = false;

        hr = m_pD3dDevice->GetRenderTarget( 0, &m_pRenderTarget );
        Debug::Assert( Condition(SUCCEEDED(hr)), "Failed get render target: 0x%08x", hr );

        m_pChannel->QueueEvent( "DeviceRestored", ArgList() );      
        LOG( "Direct3d Device restored" );

        m_DeviceIsValid = true;

        m_pD3dDevice->BeginScene( );
        m_pD3dDevice->Clear( 0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, RGB(0, 0, 0), 0.0f, 0 ); 
        m_pD3dDevice->EndScene( );

        m_pD3dDevice->Present( NULL, NULL, NULL, NULL );
    }
}

void Dx9::SetWindow( 
    HWND hWnd 
    )
{
    RECT rect;

    if (FALSE == GetClientRect(hWnd, &rect))
        return;

    if (rect.right == 0 || rect.bottom == 0)
        return;

    SwapChain *pSwapChain = NULL;

    int size = m_SwapChains.GetSize();

    for ( int i = 0; i < size; i++ )
    {
        SwapChain *pChain = m_SwapChains.GetPointer(i);

        if ( pChain->hWnd == hWnd )
        {
            //if there is a swapchain with this window
            //and the window size changed then force a recreate
            if ( NULL == pChain->pSwapChain ||
                    pChain->width  != rect.right ||
                    pChain->height != rect.bottom )
            {
                if ( NULL != pChain->pSwapChain )
                    pChain->pSwapChain->Release( );
                
                if ( NULL != pChain->pBackBuffer )
                    pChain->pBackBuffer->Release( );

                m_SwapChains.Remove( i );                    
                pChain = NULL;
            }

            pSwapChain = pChain;
            break;
        }
    }

    SwapChain chain = { 0 };

    if ( NULL == pSwapChain )
    {
        chain.hWnd = hWnd;
        chain.width = rect.right;
        chain.height= rect.bottom;

        HRESULT hr;

        D3DPRESENT_PARAMETERS pp = m_PresentParams;
        pp.hDeviceWindow = hWnd;

        pp.BackBufferHeight = rect.bottom;
        pp.BackBufferWidth  = rect.right;
        
        hr = m_pD3dDevice->CreateAdditionalSwapChain( &pp, &chain.pSwapChain );
        Debug::Assert( Condition(SUCCEEDED(hr)), "Failed to create additional swap chain0x%08x", hr );   

        if (pp.BackBufferWidth > m_PresentParams.BackBufferWidth)
        {
            m_NeedsReset = true;
            m_PresentParams.BackBufferWidth = pp.BackBufferWidth;
        }

        if (pp.BackBufferHeight > m_PresentParams.BackBufferHeight)
        {
            m_NeedsReset = true;
            m_PresentParams.BackBufferHeight = pp.BackBufferHeight;
        }

        hr = chain.pSwapChain->GetBackBuffer(0, D3DBACKBUFFER_TYPE_MONO, &chain.pBackBuffer);
        Debug::Assert( Condition(SUCCEEDED(hr)), "Failed get back buffer: 0x%08x", hr );
        m_SwapChains.Add(chain);
    }
    else
    {
        chain = *pSwapChain;
    }

    m_SwapChain = chain;
}


