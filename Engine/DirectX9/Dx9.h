#pragma once

#include "EngineGlobal.h"
#include "ThreadLocks.h"
#include "List.h"

class Channel;

#ifndef DIRECTX9
   #error "This should not be included
#endif

class Dx9
{
public:
    struct SwapChain
    {
        HWND hWnd;
        int width;
        int height;

        IDirect3DSwapChain9 *pSwapChain;
        IDirect3DSurface9   *pBackBuffer;
    };

private:
    D3DPRESENT_PARAMETERS m_PresentParams;

   IDirect3DVertexDeclaration9 *m_pQuadDecl;
   IDirect3DVertexDeclaration9 *m_pLit3dDecl;
   IDirect3DVertexDeclaration9 *m_pParticleQuadDecl;
   IDirect3DVertexDeclaration9 *m_pLineDecl;

    IDirect3D9        *m_pD3d;
    IDirect3DDevice9  *m_pD3dDevice;
    IDirect3DSurface9 *m_pRenderTarget;
    Channel           *m_pChannel;
    SwapChain          m_SwapChain;
    List<SwapChain>    m_SwapChains;
    bool               m_DeviceIsValid;
    bool               m_NeedsReset;
    bool               m_InScene;

public:
    static Dx9 &Instance( void );

public:
    void Create( 
        HWND hwnd,
        bool windowed
    );

    void Destroy( void );

    void CreateChannel ( void );
    void DestroyChannel( void );

    void RestoreRenderTarget( void );

    HRESULT Present( void );

    void DeviceLost( void );
    void ResetDevice( void );

    void SetWindow( 
        HWND hwnd 
    );

    inline IDirect3DDevice9 *GetDevice  ( void ) { return m_pD3dDevice; }
    inline IDirect3D9       *GetDirect3D( void ) { return m_pD3d; }

    inline const SwapChain *GetSwapChain( void ) const { return &m_SwapChain; }

    inline IDirect3DVertexDeclaration9 *GetQuadDecl( void ) { return m_pQuadDecl; }
    inline IDirect3DVertexDeclaration9 *GetParticleQuadDecl( void ) const { return m_pParticleQuadDecl; }
    inline IDirect3DVertexDeclaration9 *GetLineDecl( void ) { return m_pLineDecl; }
    inline IDirect3DVertexDeclaration9 *GetLit3dDecl( void ) { return m_pLit3dDecl; }

    const D3DPRESENT_PARAMETERS *GetPresentParameters( void ) const { return &m_PresentParams; }

    HRESULT BeginScene( void )
    {
        m_InScene = true;
        return Dx9::Instance( ).GetDevice( )->BeginScene( );
    }

    HRESULT EndScene( void )
    {
        HRESULT hr = Dx9::Instance( ).GetDevice( )->EndScene( );
        m_InScene = false;
        
        return hr;
    }

    bool HasDevice( void ) const { return m_DeviceIsValid; }
    bool InScene( void ) const { return m_InScene; }

    Channel *GetChannel( void ) { return m_pChannel; }
};
