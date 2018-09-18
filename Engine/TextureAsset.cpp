#include "EnginePch.h"

#include "TextureAsset.h"
#include "IOStreams.h"


#if defined OPENGL

//for glCheckError
#include "GlShader.h"

DefineResourceType(Texture, Asset, new TextureSerializer);

ISerializable *TextureSerializer::Deserialize(
    Serializer *pSerializer,
    ISerializable *pSerializable
    )
{
    const uint32 Version = 4;

    glPixelStorei( GL_UNPACK_ALIGNMENT, 1 );

    Texture::Header header;

    if ( NULL == pSerializable ) pSerializable = new Texture; 

    Texture *pTexture = (Texture *) pSerializable;

    pSerializer->GetInputStream( )->Read( &header, sizeof(header), NULL );
    Debug::Assert( Condition(header.version == Version), "Incorrect texture version: %d, expecting: %d", header.version, Version );

    int *pMipSizes = (int *) malloc( header.mipLevels * sizeof(int) );
    pSerializer->GetInputStream( )->Read( pMipSizes, header.mipLevels * sizeof(int) );

    Id id = Id::Deserialize( pSerializer->GetInputStream() );

    pTexture->Create( id );

    pTexture->m_DesiredWidth  = header.desiredWidth;
    pTexture->m_DesiredHeight = header.desiredHeight;
    pTexture->m_ActualWidth   = header.actualWidth;
    pTexture->m_ActualHeight  = header.actualHeight;
    pTexture->m_MipLevels     = header.mipLevels;
    pTexture->m_pMipSizes     = pMipSizes;

    BYTE *pData = (BYTE *) malloc( header.size );
    pSerializer->GetInputStream( )->Read( pData, header.size, NULL );

    memcpy(&pTexture->m_Header, &header, sizeof(header));

    pTexture->m_pData = pData;
    pTexture->m_NeedsReload = true;

    return pSerializable;
}

void Texture::Reload( void )
{
    if ( false == m_NeedsReload ) return;
    m_NeedsReload = false;

    if ( 0 != m_glTexture ) glDeleteTextures( 1, &m_glTexture );

    Debug::Print( Debug::TypeInfo, "Recreating Texture %s\n", GetId().ToString() );

    if ( NULL == m_pData )
        Debug::Print( Debug::TypeError, "Recreating Texture %s with NULL Data!\n", GetId().ToString() );

    glPixelStorei( GL_UNPACK_ALIGNMENT, 1 );

    glGenTextures( 1, &m_glTexture );
    glBindTexture( GL_TEXTURE_2D, m_glTexture );

    char *pData = (char *) m_pData;

    int width = m_ActualWidth;
    int height= m_ActualHeight;

    for ( uint32 i = 0; i < m_MipLevels; i++ )
    {
        if ( 0 == m_Header.compressed )
        {
            glTexImage2D( GL_TEXTURE_2D, i, (GLenum) m_Header.pixelFormat, width, height, 0, (GLenum) m_Header.format, m_Header.type, pData );
            glCheckError("glTexImage2D");
        }
        else
        {
            glCompressedTexImage2D( GL_TEXTURE_2D, i, (GLenum) m_Header.format, width, height, 0, m_pMipSizes[i], pData );
            glCheckError("glCompressedTexImage2D");
        }

        if ( width > 1 )
            width = width / 2.0f;
        
        if ( height > 1 )
            height = height / 2.0f;

        pData += m_pMipSizes[ i ];
    }

    free( m_pData );
    free( m_pMipSizes );

    m_pMipSizes = NULL;
    m_pData = NULL;
}

#elif defined DIRECTX9

#include "Dx9.h"

DefineResourceType(Texture, Asset, new TextureSerializer);

ISerializable *TextureSerializer::Deserialize(
    Serializer *pSerializer,
    ISerializable *pSerializable
    )
{
    const uint32 Version = 1;

    struct Header
    {
        uint32 version;
        uint32 desiredWidth, desiredHeight;
        uint32 actualWidth, actualHeight;
        uint32 size, format, type, compressed;
        uint32 pixelFormat, mipLevels;
    };

    Header header;

    if ( NULL == pSerializable ) pSerializable = Instantiate(); 

    Texture *pTexture = (Texture *) pSerializable;

    pSerializer->GetInputStream( )->Read( &header, sizeof(header), NULL );
    Debug::Assert( Condition(header.version == Version), "Incorrect texture version: %d, expecting: %d", header.version, Version );

    int *pMipSizes = (int *) malloc( header.mipLevels * sizeof(int) );
    pSerializer->GetInputStream( )->Read( pMipSizes, header.mipLevels * sizeof(int) );

    pTexture->m_DesiredWidth  = header.desiredWidth;
    pTexture->m_DesiredHeight = header.desiredHeight;
    pTexture->m_ActualWidth   = header.actualWidth;
    pTexture->m_ActualHeight  = header.actualHeight;
    pTexture->m_HasMips       = header.mipLevels > 1;

    HRESULT hr = Dx9::Instance( ).GetDevice( )->CreateTexture( header.actualWidth, header.actualHeight, header.mipLevels, NULL, (D3DFORMAT) header.format, D3DPOOL_MANAGED, &pTexture->m_pTexture, NULL );
    Debug::Assert( Condition(SUCCEEDED(hr)), "Failed to create texture: 0x%08x", hr );

    IDirect3DSurface9 *pSurface;

    for ( uint32 i = 0; i < header.mipLevels; i++ )
    {
        hr = pTexture->m_pTexture->GetSurfaceLevel( i, &pSurface );
        Debug::Assert( Condition(SUCCEEDED(hr)), "Failed to get surface for texture: 0x%08x", hr );

        D3DLOCKED_RECT rect;
        hr = pSurface->LockRect( &rect, NULL, 0 );
        Debug::Assert( Condition(SUCCEEDED(hr)), "Failed to lock surface for texture: 0x%08x", hr );

        pSerializer->GetInputStream( )->Read( rect.pBits, pMipSizes[i], NULL );
        pSurface->UnlockRect( );

        pSurface->Release( );
    }


    free( pMipSizes );

    return pSerializable;
}
#endif
