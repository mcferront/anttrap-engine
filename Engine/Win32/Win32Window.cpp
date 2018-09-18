#include "EnginePch.h"

#include "Win32Window.h"
#include "Viewport.h"
#include "GraphicsApi.h"

void Window::Create(
    Id id
    )
{
    SetId( id );
    m_hWnd = NULL;
}

void Window::Create(
    Id id,
    const Window &copyFrom
    )
{
    Copy( copyFrom );
    SetId( id );
}

void Window::Destroy( void )
{
}

void Window::SetHandle(
    HWND hWnd
    )
{
    m_hWnd = hWnd;
}

void Window::BeginRender( void )
{
#ifdef DIRECTX9
    HRESULT hr;

    //visual asserts might interrupt our rendering
    //so we'll handle begin/end transparently
    if ( false == Dx9::Instance( ).InScene( ) )
    {
        Dx9::Instance( ).SetWindow( m_hWnd );

        Dx9::Instance( ).RestoreRenderTarget( );

        hr = Dx9::Instance( ).BeginScene( );
        Debug::Assert( Condition( SUCCEEDED( hr ) ), "Failed to begin scene: 0x%08x", hr );
    }

#elif defined DIRECTX12
   GpuDevice::Instance( ).BeginRender( );
#endif
}

void Window::EndRender(
   bool present
    )
{
#ifdef DIRECTX9
    HRESULT hr;

    //visual asserts might interrupt our rendering
    //so we'll handle begin/end transparently
    if ( true == Dx9::Instance( ).InScene( ) )
    {
        hr = Dx9::Instance( ).EndScene( );
        //Debug::Print( Condition(SUCCEEDED(hr)), Debug::TypeWarning, "Failed to end scene: 0x%08x", hr );

        if ( true == present )
        {
            hr = Dx9::Instance( ).Present( );
            if ( D3DERR_DEVICELOST == hr )
            {
                Dx9::Instance( ).DeviceLost( );
            }
            else
            {
                Debug::Assert( Condition( SUCCEEDED( hr ) ), "Failed to present: 0x%08x", hr );
            }
        }
    }
#elif defined DIRECTX12
   GpuDevice::Instance( ).EndRender( );
#endif
}

void Window::GetDimensions(
    int *pWidth,
    int *pHeight
    ) const
{
    RECT rect;
    GetClientRect( m_hWnd, &rect );

    *pWidth = rect.right;
    *pHeight = rect.bottom;
}

bool Window::CanRender( void )
{
    return NULL != m_hWnd && GpuDevice::Instance( ).HasDevice( );
}

