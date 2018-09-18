#include "EnginePch.h"

#include "GlMaterial.h"
#include "TextureAsset.h"
#include "ShaderAsset.h"
#include "Renderer.h"
#include "Viewport.h"
#include "LightComponent.h"

DefineResourceType(Material, Asset, new MaterialSerializer);

void Material::Create(
    Id id,
    ResourceHandle texture,
    ResourceHandle shader,
    Vector color,
    bool depthTest,
    bool cull,
    bool alphaBlend,
    bool depthMask,
    bool filter
    )
{
    Asset::Create( id );

    m_Shader    = shader;
    m_Color     = color;
    m_SelfIllumColor = Math::ZeroVector;
    m_DepthTest = depthTest;
    m_CullMode  = cull ? Material::CullBack : Material::CullNone;
    m_AlphaBlend= alphaBlend;
    m_DepthMask = depthMask;
    m_Filter    = filter;
    m_DepthWrite= depthTest;

    m_pTextures = new TextureDesc[ 1 ];
    m_pTextures[ 0 ].texture  = texture;
    m_pTextures[ 0 ].wrapMode = Clamp;

    m_NumTextures = 1;
}

void Material::Destroy( void )
{
    delete []m_pTextures;

    m_pTextures = NULL;
    m_Shader    = NullHandle;

    Asset::Destroy( );
}

void Material::Submit(
    const Transform &worldTransform,
    bool selected,
    bool skinned,
    int parameters, //= 0,
    const ParameterDesc *pDescs, //= NULL
    const LightDescList *pLights// = NULL
    ) const
{
    Shader *pShader = GetResource( m_Shader, Shader );
    pShader->MakeActive( selected, skinned );

    if ( m_DepthWrite == true )
    {
        glDepthMask( GL_TRUE );
    }
    else
    {
        glDepthMask( GL_FALSE );
    }

    if ( m_DepthTest == true )
    {
        glEnable( GL_DEPTH_TEST );
        glDepthFunc( GL_LEQUAL );
    }
    else
    {
        glDisable( GL_DEPTH_TEST );
    }

    if ( true == m_DepthMask )
    {
        glColorMask( false, false, false, false );
    }
    else
    {
        glColorMask( true, true, true, true );
    }

    glFrontFace( GL_CW );

    if ( CullBack == m_CullMode )
    {
        glCullFace( GL_BACK );
        glEnable( GL_CULL_FACE );
    }
    else if ( CullFront == m_CullMode )
    {
        glCullFace( GL_FRONT );
        glEnable( GL_CULL_FACE );
    }
    else
    {
        glDisable( GL_CULL_FACE );
    }

    Resource *pWhite = GetLoadedResource("White.bmp");

    if ( m_NumTextures > 0 )
    {
        for ( int i = 0; i < m_NumTextures; i++ )
        {
            if ( IsResourceLoaded(m_pTextures[i].texture) )
                pShader->SetTexture( i, m_pTextures[i].texture, m_pTextures[i].wrapMode == Wrap, m_Filter );
            else if ( NULL != pWhite )
                pShader->SetTexture( i, pWhite->GetHandle(), false, m_Filter );
        }
    }
    else if ( NULL != pWhite )
        pShader->SetTexture( 0, pWhite->GetHandle(), false, m_Filter );


    pShader->SetVector( "color", m_Color );
    pShader->SetVector( "ambientColor", Math::ZeroVector );
    pShader->SetVector( "selfIllumColor", m_SelfIllumColor );

    Transform viewTransform;
    pRasterizer->GetViewport()->GetCamera( )->GetViewTransform( &viewTransform );

    Matrix projection;
    pRasterizer->GetViewport()->GetCamera( )->GetProjection( &projection );
    Math::Multiply( &projection, projection, pRasterizer->GetViewport()->GetMode( )->GetRotation( ) );

    pShader->SetTransforms( "world", &worldTransform, 1 );
    pShader->SetTransforms( "view", &viewTransform, 1 );
    pShader->SetMatrices  ( "projection", &projection, 1 );
    pShader->SetVector    ( "uvs",   Math::ZeroVector );


    {
        int lightIndex = 0;

        if ( NULL != pLights )
        {
            for ( uint32 i = 0; i < pLights->GetSize(); i++ )
            {
                const LightDesc *pDesc = pLights->GetPointer( i );
                pShader->SetLight( lightIndex, *pDesc );
            
                // Ambients don't count against the light count
                if (LightDesc::CastAmbient != pDesc->cast)
                    ++lightIndex;
            }
        }

        for ( ; lightIndex < 4; lightIndex++ )
            pShader->SetLight( lightIndex, LightDesc::Zero );
    }

    for ( int i = 0; i < parameters; i++ )
    {
        pShader->SetVector(pDescs[i].parameterId.ToString(), pDescs[i].parameter);
    }

    glCheckError( "Material:Should be Clear" );

    if ( true == m_AlphaBlend )
    {
        glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
        glCheckError( "Material:glBlendFunc" );

        glEnable( GL_BLEND );
        glCheckError( "Material:glEnable( GL_BLEND )" );
    }
    else
    {
        glDisable( GL_BLEND );
        glCheckError( "Material:glDisable( GL_BLEND )" );
    }
}

void Material::SetMatrices(
    const char *pName,
    const Matrix *pMatrices,
    int numMatrices
    )
{
    Shader *pShader = GetResource( m_Shader, Shader );
    pShader->SetMatrices( pName, pMatrices, numMatrices );
}

ISerializable *MaterialSerializer::Deserialize(
    Serializer *pSerializer,
    ISerializable *pSerializable
    )
{
    const uint32 Version = 3;

    const unsigned char CullNone  = 0;
    const unsigned char CullBack  = 1;
    const unsigned char CullFront = 2;

    struct Header
    {
        unsigned int version;

        float r, g, b, a;
        float selfR, selfG, selfB, selfA;

        unsigned char depthTest;
        unsigned char depthWrite;
        unsigned char cull;
        unsigned char alphaBlend;
        unsigned char textureFilter;
        unsigned char numTextures;   
    };

    Header header;

    if ( NULL == pSerializable ) pSerializable = new Material;

    Material *pMaterial = (Material *) pSerializable;

    pSerializer->GetInputStream( )->Read( &header, sizeof(header), NULL );
    Debug::Assert( Condition(header.version == Version), "Incorrect material version: %d, expecting: %d", header.version, Version );

    Id materialId = Id::Deserialize( pSerializer->GetInputStream() );
    Id shaderId   = Id::Deserialize( pSerializer->GetInputStream() );
    
    pMaterial->Create( materialId );

    pMaterial->m_pTextures = new Material::TextureDesc[ header.numTextures ];
    pMaterial->m_NumTextures = header.numTextures;

    for ( int i = 0; i < header.numTextures; i++ )
    {
        Id id = Id::Deserialize( pSerializer->GetInputStream() );
        pMaterial->m_pTextures[ i ].texture = ResourceHandle( id );

        int wrapMode;

        pSerializer->GetInputStream( )->Read( &wrapMode, sizeof(wrapMode), NULL );
        pMaterial->m_pTextures[ i ].wrapMode = (Material::WrapMode) wrapMode;   
    }

    pMaterial->m_Shader  = ResourceHandle( shaderId );
    pMaterial->m_Color   = Vector( header.r, header.g, header.b, header.a );
    pMaterial->m_SelfIllumColor = Vector( header.selfR, header.selfG, header.selfB, header.selfA );

    pMaterial->m_DepthTest  = 0 != header.depthTest;
    pMaterial->m_DepthWrite = 0 != header.depthWrite;
    pMaterial->m_AlphaBlend = 0 != header.alphaBlend;
    pMaterial->m_Filter     = 0 != header.textureFilter;
    pMaterial->m_DepthMask  = false;

    switch ( header.cull )
    {
        case CullNone  : pMaterial->m_CullMode = Material::CullNone;  break;
        case CullFront : pMaterial->m_CullMode = Material::CullFront; break;
        case CullBack  : pMaterial->m_CullMode = Material::CullBack;  break;
    }

    return pSerializable;
}

