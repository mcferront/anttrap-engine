#include "EnginePch.h"

#include "Dx9Shader.h"
#include "TextureAsset.h"
#include "Log.h"
#include "Dx9.h"
#include "Geometry.h"

DefineResourceType(Shader, Asset, new ShaderSerializer);

void Shader::Create( void )
{
    m_pVertexShader = NULL;
    m_pPixelShader  = NULL;

    m_VertexRegisters.Create( );
    m_PixelRegisters.Create( );
}

void Shader::Destroy( void )
{
    if ( m_pVertexShader )    m_pVertexShader->Release( );
    if ( m_pPixelShader  )    m_pPixelShader ->Release( );

    m_pVertexShader = NULL;
    m_pPixelShader  = NULL;

    List<const char *> keys; keys.Create();
    
    {
        Enumerator<const char *, int> e = m_VertexRegisters.GetEnumerator();

        while (e.EnumNext())
            keys.Add(e.Key());

        e = m_PixelRegisters.GetEnumerator();

        while (e.EnumNext())
            keys.Add(e.Key());
    }

    m_VertexRegisters.Destroy( );
    m_PixelRegisters .Destroy( );

    for (uint32 i = 0; i < keys.GetSize(); i++)
        StringRel(keys.Get(i));

    keys.Destroy();

    Asset::Destroy( );
}

void Shader::MakeActive( void ) 
{
    if ( NULL != m_pVertexShader )
        Dx9::Instance( ).GetDevice( )->SetVertexShader( m_pVertexShader );

    if ( NULL != m_pPixelShader )
        Dx9::Instance( ).GetDevice( )->SetPixelShader ( m_pPixelShader );
}

void Shader::SetTransforms(
    const char *pName,
    const Transform *pTransforms,
    int numTransforms
    )
{
    Matrix matrices[ 128 ];

    int i;

    for ( i = 0; i < numTransforms; i++ )
        Math::Transpose( &matrices[ i ], pTransforms[ i ].ToMatrix(true) );

    unsigned int index;

    index = GetVertexRegister( pName );
    if ( -1 != index ) Dx9::Instance( ).GetDevice( )->SetVertexShaderConstantF( index, (const float *) matrices, numTransforms * (sizeof(Matrix) / sizeof(Vector)) );

    index = GetPixelRegister( pName );
    if ( -1 != index ) Dx9::Instance( ).GetDevice( )->SetPixelShaderConstantF( index, (const float *) matrices, numTransforms * (sizeof(Matrix) / sizeof(Vector)) );
}

void Shader::SetMatrices(
    const char *pName,
    const Matrix *pMatrices,
    int numMatrices
    )
{
    Matrix matrices[ 128 ];

    int i;

    for ( i = 0; i < numMatrices; i++ )
        Math::Transpose( &matrices[ i ], pMatrices[ i ] );

    unsigned int index;

    index = GetVertexRegister( pName );
    if ( -1 != index ) Dx9::Instance( ).GetDevice( )->SetVertexShaderConstantF( index, (const float *) matrices, numMatrices * (sizeof(Matrix) / sizeof(Vector)) );

    index = GetPixelRegister( pName );
    if ( -1 != index ) Dx9::Instance( ).GetDevice( )->SetPixelShaderConstantF( index, (const float *) matrices, numMatrices * (sizeof(Matrix) / sizeof(Vector)) );
}

void Shader::SetTexture(
    const char *pName,
    ResourceHandle texture,
    D3DTEXTUREADDRESS wrap,
    D3DTEXTUREFILTERTYPE filter
    )
{
    Texture *pTexture = GetResource( texture, Texture );
    if ( pTexture->GetTexture( ) == NULL ) return;

    unsigned int index = GetPixelRegister( pName );
    Dx9::Instance( ).GetDevice( )->SetTexture( index, pTexture->GetTexture( ) );

    Dx9::Instance( ).GetDevice( )->SetSamplerState( index, D3DSAMP_MINFILTER, filter );
    Dx9::Instance( ).GetDevice( )->SetSamplerState( index, D3DSAMP_MAGFILTER, filter );
    Dx9::Instance( ).GetDevice( )->SetSamplerState( index, D3DSAMP_MIPFILTER, filter );

    Dx9::Instance( ).GetDevice( )->SetSamplerState( index, D3DSAMP_ADDRESSU,  wrap );
    Dx9::Instance( ).GetDevice( )->SetSamplerState( index, D3DSAMP_ADDRESSV,  wrap );
}

void Shader::SetVectors(
    const char *pName,
    const Vector *pVectors,
    int numVectors
    )
{
    unsigned int index;

    index = GetVertexRegister( pName );
    if ( -1 != index ) Dx9::Instance( ).GetDevice( )->SetVertexShaderConstantF( index, (const float *) pVectors, numVectors );

    index = GetPixelRegister( pName );
    if ( -1 != index ) Dx9::Instance( ).GetDevice( )->SetPixelShaderConstantF( index, (const float *) pVectors, numVectors );
}

//void Shader::SetLight(
//    int lightIndex,
//    const LightDesc &desc
//)
//{
//    if (desc.cast == LightDesc::CastAmbient)
//        SetVector("ambientColor", desc.color);
//    else
//    {
//        const char *pColor, *pPosition, *pDirection, *pIor;
//
//        if (0 == lightIndex)
//        {
//            pColor = "light0Color";
//            pPosition = "light0Position";
//            pDirection = "light0Direction";
//            pIor = "light0ior";
//        }
//        else if (1 == lightIndex)
//        {
//            pColor = "light1Color";
//            pPosition = "light1Position";
//            pDirection = "light1Direction";
//            pIor = "light1ior";
//        }
//        else if (2 == lightIndex)
//        {
//            pColor = "light2Color";
//            pPosition = "light2Position";
//            pDirection = "light2Direction";
//            pIor = "light2ior";
//        }
//        else if (3 == lightIndex)
//        {
//            pColor = "light3Color";
//            pPosition = "light3Position";
//            pDirection = "light3Direction";
//            pIor = "light3ior";
//        }
//        else
//            return;
//
//        Vector ci = desc.color;
//        ci.w = desc.nits;
//
//        SetVector( pColor, ci );
//        SetVector( pPosition, desc.position );
//        SetVector( pDirection, desc.direction );
//    
//        float innerAngle = desc.inner;
//        float outerAngle = desc.outer;
//
//        if (innerAngle != FLT_MAX)
//            innerAngle = 1.0f - Math::Cos(innerAngle / 2);
//
//        if (outerAngle != FLT_MAX)
//            outerAngle = 1.0f - Math::Cos(outerAngle / 2);
//
//        SetVector( pIor, Vector(innerAngle, outerAngle, desc.range, desc.cast == LightDesc::CastOmni ? 1.0f : 0.0f) );
//    }
//}

int Shader::GetPixelRegister(
    const char *pName
    )
{
    int map;

    if ( true == m_PixelRegisters.Get(pName, &map) )
        return map;

    return -1;
}

int Shader::GetVertexRegister(
    const char *pName
    )
{
    int map;

    if ( true == m_VertexRegisters.Get(pName, &map) )
        return map;

    return -1;
}
//
//void Shader::CreateDx9Shaders( void )
//{
//    m_VertexRegisters.Create( );
//    m_PixelRegisters .Create( );
//    m_PixelSelRegisters.Create( );
//
//    m_pActivePixelRegisters = &m_PixelRegisters;
//
//    const char *pVertex   = NULL;
//    const char *pSkin     = NULL;
//    const char *pPixel    = NULL;
//    const char *pSel      = NULL;
//
//    if ( 0 == strcmp("Video", m_pType) )
//    {
//        pVertex = s_2dSource;
//        pPixel   = s_VideoFragmentSource;
//    }
//    else if ( 0 == strcmp("2d", m_pType) )
//    {
//        pVertex = s_2dSource;
//        pPixel  = s_2dFragmentSource;
//        pSel    = s_2dFragmentSource_Sel;
//    }
//    else if ( 0 == strcmp("Prelit", m_pType) )
//    {
//        pSkin   = s_3dSourceSkin;
//        pVertex = s_3dSource;
//        pPixel  = s_PrelitFragmentSource;
//        pSel    = s_PrelitFragmentSource_Sel;
//    }
//    else if ( 0 == strcmp("Lit", m_pType) )
//    {
//        pSkin   = s_3dSourceSkinLit;
//        pVertex = s_3dSourceLit;
//        pPixel  = s_LitFragmentSource;
//        pSel    = s_PrelitFragmentSource_Sel;
//    }
//    else if ( 0 == strcmp("Picking", m_pType) )
//    {
//        pSkin   = s_3dSourceSkin;
//        pVertex = s_3dSource;
//        pPixel  = s_PickingFragmentSource;
//    }
//    else if ( 0 == strcmp("Particles", m_pType) )
//    {
//        pVertex = s_3dParticleSource;
//        pPixel  = s_ParticleFragmentSource;
//    }
//    else if ( 0 == strcmp("VertexBlend", m_pType) )
//    {
//        pVertex = s_3dVertexBlendSource;
//        pPixel  = s_VertexBlendFragmentSource;
//        pSel    = s_VertexBlendFragmentSource_Sel;
//    }
//
//    DWORD flags = 0;
//#ifdef _DEBUG
//    flags = D3DXSHADER_DEBUG;
//#endif
//
//    HRESULT hr;
//
//    if ( 0 == strcmp(m_pType, "Prelit") )
//    {
//        m_VertexRegisters.Add( "world",      0 );
//        m_VertexRegisters.Add( "view",       4 );
//        m_VertexRegisters.Add( "projection", 8 );
//
//        if ( NULL != pSkin ) m_VertexRegisters.Add( "skin",      12 );
//
//        m_PixelRegisters.Add( "textureMap", 0 );
//        m_PixelRegisters.Add( "color",      0 );
//        m_PixelRegisters.Add( "uvs",        1 );
//
//        m_PixelSelRegisters.Add( "textureMap", 0 );
//        m_PixelSelRegisters.Add( "color",      0 );
//        m_PixelSelRegisters.Add( "uvs",        1 );
//
//        m_Requires = StreamDecl::Positions | StreamDecl::UV0s | StreamDecl::Colors;
//    }
//    else if ( 0 == strcmp(m_pType, "Lit") )
//    {
//        m_VertexRegisters.Add( "world",      0 );
//        m_VertexRegisters.Add( "view",       4 );
//        m_VertexRegisters.Add( "projection", 8 );
//
//        if ( NULL != pSkin ) m_VertexRegisters.Add( "skin",      12 );
//
//        m_PixelRegisters.Add( "textureMap", 0 );
//        m_PixelRegisters.Add( "color",      0 );
//        m_PixelRegisters.Add( "uvs",        1 );
//        m_PixelRegisters.Add( "light0Direction", 2 );
//        m_PixelRegisters.Add( "light0Position", 3 );
//        m_PixelRegisters.Add( "light0ior", 4 );
//        m_PixelRegisters.Add( "light0Color", 5 );
//
//        m_PixelRegisters.Add( "light1Direction", 6 );
//        m_PixelRegisters.Add( "light1Position", 7 );
//        m_PixelRegisters.Add( "light1ior", 8 );
//        m_PixelRegisters.Add( "light1Color", 9 );
//
//        m_PixelRegisters.Add( "light2Direction", 10 );
//        m_PixelRegisters.Add( "light2Position", 11 );
//        m_PixelRegisters.Add( "light2ior", 12 );
//        m_PixelRegisters.Add( "light2Color", 13 );
//
//        m_PixelRegisters.Add( "light3Direction", 14 );
//        m_PixelRegisters.Add( "light3Position", 15 );
//        m_PixelRegisters.Add( "light3ior", 16 );
//        m_PixelRegisters.Add( "light3Color", 17 );
//
//        m_PixelRegisters.Add( "ambientColor", 18 );
//        m_PixelRegisters.Add( "selfIllumColor", 19 );
//
//        m_PixelSelRegisters.Add( "textureMap", 0 );
//        m_PixelSelRegisters.Add( "color",      0 );
//        m_PixelSelRegisters.Add( "uvs",        1 );
//    
//        m_Requires = StreamDecl::Positions | StreamDecl::UV0s | StreamDecl::Colors | StreamDecl::Normals;
//    }
//    else if ( 0 == strcmp(m_pType, "Picking") )
//    {
//        m_VertexRegisters.Add( "world",      0 );
//        m_VertexRegisters.Add( "view",       4 );
//        m_VertexRegisters.Add( "projection", 8 );
//
//        if ( NULL != pSkin ) m_VertexRegisters.Add( "skin",      12 );
//
//        m_PixelRegisters.Add( "textureMap", 0 );
//        m_PixelRegisters.Add( "color",      0 );
//        m_PixelRegisters.Add( "id",         1 );
//
//        m_Requires = StreamDecl::Positions | StreamDecl::UV0s | StreamDecl::Colors;
//    }
//    else if ( 0 == strcmp(m_pType, "VertexBlend") )
//    {
//        m_VertexRegisters.Add( "world",      0 );
//        m_VertexRegisters.Add( "view",       4 );
//        m_VertexRegisters.Add( "projection", 8 );
//
//        if ( NULL != pSkin ) m_VertexRegisters.Add( "skin",      12 );
//
//        m_PixelRegisters.Add( "textureMap1",0 );
//        m_PixelRegisters.Add( "textureMap2",1 );
//        m_PixelRegisters.Add( "color",      0 );
//        m_PixelRegisters.Add( "uvs",        1 );
//
//        m_PixelSelRegisters.Add( "textureMap1",0 );
//        m_PixelSelRegisters.Add( "textureMap2",1 );
//        m_PixelSelRegisters.Add( "color",      0 );
//        m_PixelSelRegisters.Add( "uvs",        2 );
//
//        m_Requires = StreamDecl::Positions | StreamDecl::UV0s | StreamDecl::Colors;
//    }
//    else      
//    {
//        m_VertexRegisters.Add( "world",      0 );
//        m_VertexRegisters.Add( "view",       4 );
//        m_VertexRegisters.Add( "projection", 8 );
//
//        m_PixelRegisters.Add( "textureMap", 0 );
//        m_PixelRegisters.Add( "color",      0 );
//        m_PixelRegisters.Add( "uvs",        1 );
//
//        m_PixelSelRegisters.Add( "textureMap", 0 );
//        m_PixelSelRegisters.Add( "color",      0 );
//        m_PixelSelRegisters.Add( "uvs",        2 );
//
//        m_Requires = StreamDecl::Positions | StreamDecl::UV0s | StreamDecl::Colors;
//    }
//
//
//    m_pVertexShader = NULL;
//    m_pSkinShader   = NULL;
//    m_pPixelShader  = NULL;
//    m_pPixelSelShader = NULL;
//
//    ID3DXBuffer *pBuffer = NULL, *pError = NULL;
//
//    //vertex shader
//    hr = D3DXCompileShader( pVertex, strlen(pVertex), NULL, NULL, "main", "vs_3_0", flags, &pBuffer, &pError, NULL );
//    if ( FALSE == SUCCEEDED(hr) )
//    {
//        const char *pResult = pError ? (const char *) pError->GetBufferPointer( ) : "No vsh error information available";
//        LOG( pResult );
//
//        Debug::Assert( Condition(SUCCEEDED(hr)), "Failed to compile the vertex shader: %s", pResult );
//    }
//
//    hr = Dx9::Instance( ).GetDevice( )->CreateVertexShader( (DWORD*)pBuffer->GetBufferPointer(), &m_pVertexShader );
//    Debug::Assert( Condition(SUCCEEDED(hr)), "Failed to create the vertex shader" );
//
//    if ( pError )  pError ->Release( );
//    if ( pBuffer ) pBuffer->Release( );
//
//
//    if ( pSkin )
//    {
//        pBuffer = NULL; pError = NULL;
//
//        //skin shader
//        hr = D3DXCompileShader( pSkin, strlen(pSkin), NULL, NULL, "main", "vs_3_0", flags, &pBuffer, &pError, NULL );
//        if ( FALSE == SUCCEEDED(hr) )
//        {
//            const char *pResult = pError ? (const char *) pError->GetBufferPointer( ) : "No vsh error information available";
//            LOG( pResult );
//
//            Debug::Assert( Condition(SUCCEEDED(hr)), "Failed to compile the vertex shader: %s", pResult );
//        }
//
//        hr = Dx9::Instance( ).GetDevice( )->CreateVertexShader( (DWORD*)pBuffer->GetBufferPointer(), &m_pSkinShader );
//        Debug::Assert( Condition(SUCCEEDED(hr)), "Failed to create the vertex shader" );
//
//        if ( pError )  pError ->Release( );
//        if ( pBuffer ) pBuffer->Release( );
//
//    }
//
//    if ( pSel )
//    {
//        pBuffer = NULL; pError = NULL;
//
//        //pixel sel shader
//        hr = D3DXCompileShader( pSel, strlen(pSel), NULL, NULL, "main", "ps_3_0", flags, &pBuffer, &pError, NULL );
//        if ( FALSE == SUCCEEDED(hr) )
//        {
//            const char *pResult = pError ? (const char *) pError->GetBufferPointer( ) : "No psh error information available";
//            LOG( pResult );
//
//            Debug::Assert( Condition(SUCCEEDED(hr)), "Failed to compile the pixel shader: %s", pResult );
//        }
//
//        hr = Dx9::Instance( ).GetDevice( )->CreatePixelShader( (DWORD*)pBuffer->GetBufferPointer(), &m_pPixelSelShader );
//        Debug::Assert( Condition(SUCCEEDED(hr)), "Failed to create the pixel shader" );
//
//        if ( pError ) pError->Release( );
//        if ( pBuffer ) pBuffer->Release( );
//    }
//
//    pBuffer = NULL; pError = NULL;
//
//    //pixel shader
//    hr = D3DXCompileShader( pPixel, strlen(pPixel), NULL, NULL, "main", "ps_3_0", flags, &pBuffer, &pError, NULL );
//    if ( FALSE == SUCCEEDED(hr) )
//    {
//        const char *pResult = pError ? (const char *) pError->GetBufferPointer( ) : "No psh error information available";
//        LOG( pResult );
//
//        Debug::Assert( Condition(SUCCEEDED(hr)), "Failed to compile the pixel shader: %s", pResult );
//    }
//
//    hr = Dx9::Instance( ).GetDevice( )->CreatePixelShader( (DWORD*)pBuffer->GetBufferPointer(), &m_pPixelShader );
//    Debug::Assert( Condition(SUCCEEDED(hr)), "Failed to create the pixel shader" );
//
//    if ( pError ) pError->Release( );
//    if ( pBuffer ) pBuffer->Release( );
//}
//
//void Shader::DestroyDx9Shaders( void )
//{   
//    if ( m_pVertexShader )    m_pVertexShader->Release( );
//    if ( m_pSkinShader )      m_pSkinShader  ->Release( );
//    if ( m_pPixelShader  )    m_pPixelShader ->Release( );
//    if ( m_pPixelSelShader  ) m_pPixelSelShader ->Release( );
//
//    m_pVertexShader = NULL;
//    m_pPixelShader  = NULL;
//    m_pPixelSelShader = NULL;
//    m_pSkinShader     = NULL;
//
//    m_VertexRegisters.Destroy( );
//    m_PixelRegisters .Destroy( );
//    m_PixelSelRegisters.Destroy( );
//}
//
//void Shader::OnDeviceLost(
//    const Channel *pSender,
//    const char *pName,
//    const ArgList &list
//    )
//{
//    DestroyDx9Shaders( );
//}
//
//void Shader::OnDeviceRestored(
//    const Channel *pSender,
//    const char *pName,
//    const ArgList &list
//    )
//{
//    CreateDx9Shaders( );
//}


ISerializable *ShaderSerializer::Deserialize(
    Serializer *pSerializer,
    ISerializable *pSerializable
    )
{
    const uint32 Version = 1;

    struct Header
    {
        unsigned int version;
    };

    Header header;

    if ( NULL == pSerializable ) pSerializable = new Shader;

    Shader *pShader = (Shader *) pSerializable;

    pSerializer->GetInputStream( )->Read( &header, sizeof(header), NULL );
    Debug::Assert( Condition(header.version == Version), "Incorrect shader version: %d, expecting: %d", header.version, Version );

    #ifdef _DEBUG
        const DWORD flags = D3DXSHADER_DEBUG;
    #else
        const DWORD flags = 0;
    #endif

    pShader->Create( );
    pShader->m_pVertexShader = NULL;
    pShader->m_pPixelShader = NULL;

    const char *pVSEntry = StringPool::Deserialize( pSerializer->GetInputStream() );

    if (0 != *pVSEntry)
    {
        HRESULT hr;
        ID3DXBuffer *pBuffer = NULL, *pError = NULL;

        uint32 size;
        pSerializer->GetInputStream()->Read( &size, sizeof(size) );

        char *pVS = (char *) malloc( size + 1 );
        pSerializer->GetInputStream()->Read( pVS, size );
        pVS[ size ] = 0;

        hr = D3DXCompileShader( pVS, strlen(pVS), NULL, NULL, pVSEntry, "vs_3_0", flags, &pBuffer, &pError, NULL );
        if ( FALSE == SUCCEEDED(hr) )
        {
            const char *pResult = pError ? (const char *) pError->GetBufferPointer( ) : "No vsh error information available";
            LOG( pResult );

            Debug::Assert( Condition(SUCCEEDED(hr)), "Failed to compile the vertex shader: %s", pResult );
        }

        hr = Dx9::Instance( ).GetDevice( )->CreateVertexShader( (DWORD*)pBuffer->GetBufferPointer(), &pShader->m_pVertexShader );
        Debug::Assert( Condition(SUCCEEDED(hr)), "Failed to create the vertex shader" );

        if ( pError )  pError ->Release( );
        if ( pBuffer ) pBuffer->Release( );
 
        free( pVS );
        
        while ( true )
        {
            const char *pName = StringPool::Deserialize( pSerializer->GetInputStream() );
            if ( 0 == *pName ) break;

            int reg;
            pSerializer->GetInputStream( )->Read( &reg, sizeof(reg) );
            pShader->m_VertexRegisters.Add( pName, reg );
        }
    }

    const char *pPSEntry = StringPool::Deserialize( pSerializer->GetInputStream() );

    if (0 != *pPSEntry)
    {
        HRESULT hr;
        ID3DXBuffer *pBuffer = NULL, *pError = NULL;
        
        uint32 size;
        pSerializer->GetInputStream()->Read( &size, sizeof(size) );

        char *pPS = (char *) malloc( size + 1 );
        pSerializer->GetInputStream()->Read( pPS, size );
        pPS[ size ] = 0;

        hr = D3DXCompileShader( pPS, strlen(pPS), NULL, NULL, pPSEntry, "ps_3_0", flags, &pBuffer, &pError, NULL );
        if ( FALSE == SUCCEEDED(hr) )
        {
            const char *pResult = pError ? (const char *) pError->GetBufferPointer( ) : "No psh error information available";
            LOG( pResult );

            Debug::Assert( Condition(SUCCEEDED(hr)), "Failed to compile the pixel shader: %s", pResult );
        }

        hr = Dx9::Instance( ).GetDevice( )->CreatePixelShader( (DWORD*)pBuffer->GetBufferPointer(), &pShader->m_pPixelShader );
        Debug::Assert( Condition(SUCCEEDED(hr)), "Failed to create the pixel shader" );

        if ( pError ) pError->Release( );
        if ( pBuffer ) pBuffer->Release( );

        free( pPS );

        while ( true )
        {
            const char *pName = StringPool::Deserialize( pSerializer->GetInputStream() );
            if ( 0 == *pName ) break;

            int reg;
            pSerializer->GetInputStream( )->Read( &reg, sizeof(reg) );
            pShader->m_PixelRegisters.Add( pName, reg );
        }
    }

    const char *pCSEntry = StringPool::Deserialize( pSerializer->GetInputStream() );

    if (0 != *pCSEntry)
    {

    }

    StringRel(pVSEntry);
    StringRel(pPSEntry);
    StringRel(pCSEntry);

    return pSerializable;
}

