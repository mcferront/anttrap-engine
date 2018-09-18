#include "EnginePch.h"

#include "GlShader.h"
#include "TextureAsset.h"
#include "Log.h"

DefineResourceType(Shader, Asset, NULL);

#if defined WIN32
PFNGLBINDATTRIBLOCATIONPROC  pglBindAttribLocation = NULL;
PFNGLLINKPROGRAMPROC         pglLinkProgram        = NULL;
PFNGLATTACHSHADERPROC        pglAttachShader       = NULL;
PFNGLCOMPILESHADERPROC       pglCompileShader      = NULL;
PFNGLSHADERSOURCEPROC        pglShaderSource       = NULL;
PFNGLCREATESHADERPROC        pglCreateShader       = NULL;
PFNGLCREATEPROGRAMPROC       pglCreateProgram      = NULL;
PFNGLUSEPROGRAMPROC          pglUseProgram         = NULL;
PFNGLUNIFORMMATRIX4FVPROC    pglUniformMatrix4fv   = NULL;
PFNGLUNIFORM1IPROC           pglUniform1i          = NULL;
PFNGLGETUNIFORMLOCATIONPROC  pglGetUniformLocation = NULL;
PFNGLACTIVETEXTUREPROC       pglActiveTexture      = NULL;
PFNGLUNIFORM4FPROC           pglUniform4f          = NULL;
PFNGLVERTEXATTRIBPOINTERPROC pglVertexAttribPointer= NULL;
PFNGLDELETEBUFFERSPROC       pglDeleteBuffers      = NULL;
PFNGLBINDBUFFERPROC          pglBindBuffer         = NULL;
PFNGLGENBUFFERSPROC          pglGenBuffers         = NULL;
PFNGLBUFFERDATAPROC          pglBufferData         = NULL;
PFNGLGETSHADERINFOLOGPROC    pglGetShaderInfoLog   = NULL;
PFNGLGETPROGRAMINFOLOGPROC   pglGetProgramInfoLog  = NULL;
PFNGLISSHADERPROC            pglIsShader           = NULL;
PFNGLGETPROGRAMIVPROC        pglGetProgramiv       = NULL;
PFNGLGETATTRIBLOCATIONPROC   pglGetAttribLocation  = NULL;
PFNGLDELETESHADERPROC        pglDeleteShader       = NULL;

PFNGLENABLEVERTEXATTRIBARRAYPROC  pglEnableVertexAttribArray  = NULL;
PFNGLDISABLEVERTEXATTRIBARRAYPROC pglDisableVertexAttribArray = NULL;
#endif

#if defined WIN32
#error Shaders must be written
#elif defined IOS || defined ANDROID
const char *s_3dSource = 
    "varying vec4 varyUV;"
    "varying vec4 varyColor;"
    ""
    "attribute vec4 position;"   
    "attribute vec4 uv0;"
    "attribute vec4 color;"
    ""
    "uniform mat4 world;"
    "uniform mat4 view;"
    "uniform mat4 projection;"
    ""
    "void main()"
    "{"
    ""
    "gl_Position = position * world * view * projection;"
    "varyUV = uv0;"
    "varyColor = color;"
    "}";

const char *s_3dSourceLit = 
    "varying vec4 varyUV;"
    "varying vec4 varyColor;"
    "varying vec4 varyNormal;"
    "varying vec4 varyWorldP;"
    ""
    "attribute vec4 position;"   
    "attribute vec4 uv0;"
    "attribute vec4 color;"
    "attribute vec4 normal;"
    ""
    "uniform mat4 world;"
    "uniform mat4 view;"
    "uniform mat4 projection;"
    ""                                  "\r\n"
    "void main()"
    "{"
    "  vec4 worldP = position * world; "  "\r\n"
    "  vec4 projPosition = worldP * view * projection; "                "\r\n"
    "  vec4 n = normal;"
    "n.w = 0.0;"                                                   "\r\n"
    "  vec4 worldNormal = n * world; "      "\r\n"
    ""                                  "\r\n"
    "gl_Position = projPosition;"
    "varyUV = uv0;"
    "varyColor = color;"
    "varyNormal = normalize(worldNormal);"
    "varyWorldP = worldP;"
    "}";

const char *s_3dVertexBlendSource = 
    "varying vec4 varyUV;"
    "varying vec4 varyColor;"
    ""
    "attribute vec4 position;"   
    "attribute vec2 uv0;"
    "attribute vec2 uv1;"
    "attribute vec4 color;"
    ""
    "uniform mat4 world;"
    "uniform mat4 view;"
    "uniform mat4 projection;"
    ""
    "void main()"
    "{"
    ""
    "gl_Position = position * world * view * projection;"
    "varyUV.xy = uv0;"
    "varyUV.zw = uv1;"
    "varyColor = color;"
    "}";

const char *s_3dParticleSource =
    "varying vec4 varyUV;"
    ""
    "attribute vec4 position;"   
    "attribute vec4 uv0;"
    ""
    "uniform mat4 world;"
    "uniform mat4 view;"
    "uniform mat4 projection;"
    ""
    "void main()"
    "{"
    ""
    "gl_Position = position * world * view * projection;"
    "varyUV = uv0;"
    "}";


const char *s_2dSource =
    "varying vec4 varyUV;"
    "varying vec4 varyColor;"
    ""
    "attribute vec4 position;"   
    "attribute vec4 uv0;"
    "attribute vec4 color;"
    ""
    "uniform mat4 world;"
    "uniform mat4 view;"
    "uniform mat4 projection;"
    ""
    "void main()"
    "{"
    ""
    "gl_Position = position * world * view * projection;"
    "varyUV = uv0;"
    "varyColor = color;"
    "}";

const char *s_3dSourceSkin =
    "varying vec4 varyUV;"
    "varying vec4 varyColor;"
    ""
    "attribute vec4 position;"   
    "attribute vec4 uv0;"
    "attribute vec4 color;"
    "attribute vec4 boneIndices;"
    "attribute vec4 boneWeights;"
    ""
    "uniform mat4 world;"
    "uniform mat4 view;"
    "uniform mat4 projection;"
    "uniform mat4 skin[64];"
    ""
    "void main()"
    "{"
    ""
    "ivec4 indices = ivec4( boneIndices );"
    ""
    "vec4 skinnedPosition = "
    "position * (skin[indices[0]] * boneWeights.x + "
    " skin[indices[1]] * boneWeights.y + "
    " skin[indices[2]] * boneWeights.z + "
    " skin[indices[3]] * boneWeights.w);"
    ""
    "gl_Position = skinnedPosition * world * view * projection;"
    "varyUV = uv0;"
    "varyColor = color;"
    "}";

const char *s_3dSourceSkinLit =
    "varying vec4 varyUV;"
    "varying vec4 varyColor;"
    "varying vec4 varyWorldP;"
    "varying vec4 varyNormal;"
    ""
    "attribute vec4 position;"   
    "attribute vec4 uv0;"
    "attribute vec4 color;"
    "attribute vec4 normal;"
    "attribute vec4 boneIndices;"
    "attribute vec4 boneWeights;"
    ""
    "uniform mat4 world;"
    "uniform mat4 view;"
    "uniform mat4 projection;"
    "uniform mat4 skin[64];"
    ""
    "void main()"
    "{"
    ""
    "ivec4 indices = ivec4( boneIndices );"
    " "
    "mat4 skinMatrix = "
    "(skin[indices[0]] * boneWeights.x + "
    " skin[indices[1]] * boneWeights.y + "
    " skin[indices[2]] * boneWeights.z + "
    " skin[indices[3]] * boneWeights.w);"
    ""
    "vec4 skinnedPosition = position * skinMatrix;"
    "vec4 worldP = skinnedPosition * world;"
    ""
    "varyUV = uv0;"
    "varyColor = color;"
    "vec4 n = normal;"
    "n.w = 0.0;"
    "varyNormal = normalize(n * skinMatrix * world);"
    "varyWorldP = worldP;"
    "gl_Position = worldP * view * projection;"
    "}";

const char *s_2dFragmentSource =
    "varying mediump vec4 varyUV;"
    "varying lowp    vec4 varyColor;"
    ""
    "uniform lowp vec4 color;"
    "uniform sampler2D textureMap0;"
    ""
    "void main()"
    "{"
    "lowp vec4 c = color * varyColor * texture2D( textureMap0, varyUV.xy );"
    //"if (c.a == 0.0 ) discard;"
    "gl_FragColor = c;"
    "}";

const char *s_2dFragmentSource_Sel = 
    "varying mediump vec4 varyUV;"
    "varying lowp    vec4 varyColor;"
    ""
    "uniform lowp vec4 color;"
    "uniform sampler2D textureMap0;"
    ""
    "void main()"
    "{"
    "lowp vec4 c = color * varyColor * texture2D( textureMap0, varyUV.xy );"
    //"if (c.a == 0.0 ) discard;"
    "gl_FragColor = vec4(0, 1, 0, 1) * sqrt(0.2126 * c.r + 0.7152 * c.g + 0.0722 * c.b);""\r\n"
    "gl_FragColor.a = c.a;"
    "}";


const char *s_ParticleFragmentSource =
    "varying mediump vec4 varyUV;"
    ""
    "uniform lowp vec4 color;"
    "uniform sampler2D textureMap0;"
    ""
    "void main()"
    "{"
    "lowp vec4 c = color * texture2D( textureMap0, varyUV.xy );"
    "gl_FragColor = c;"
    "}";

const char *s_PrelitFragmentSource = 
   "varying mediump vec4 varyUV;"
   "varying lowp vec4 varyColor;"
   ""
   "uniform lowp    vec4 color;"
   "uniform sampler2D textureMap0;"
   ""
   "void main()"
   "{"
      "lowp vec4 c = color * varyColor * texture2D( textureMap0, varyUV.xy);"
      //"if (c.a == 0.0 ) discard;"
      "gl_FragColor = c;"
   "}";

const char *s_PrelitFragmentSource_Sel = 
    "varying mediump vec4 varyUV;"
    "varying lowp vec4 varyColor;"
    ""
    "uniform lowp    vec4 color;"
    "uniform sampler2D textureMap0;"
    ""
    "void main()"
    "{"
    "lowp vec4 c = color * varyColor * texture2D( textureMap0, varyUV.xy);"
    //"if (c.a == 0.0 ) discard;"
    "gl_FragColor = vec4(0, 1, 0, 1) * sqrt(0.2126 * c.r + 0.7152 * c.g + 0.0722 * c.b);""\r\n"
    "gl_FragColor.a = c.a;"
    "}";

const char *s_LitFragmentSource = 
    "varying mediump vec4 varyUV;""\r\n"
    "varying lowp vec4 varyColor;""\r\n"
    "varying lowp vec4 varyNormal;""\r\n"
    "varying mediump vec4 varyWorldP;""\r\n"
"\r\n"
    "uniform lowp    vec4 color;""\r\n"
    "uniform lowp    vec4 ambientColor;""\r\n"
    "uniform lowp    vec4 selfIllumColor;""\r\n"
    "uniform sampler2D textureMap0;""\r\n"
"\r\n"
    "uniform mediump vec4 light0Position;"    "\r\n"
    "uniform lowp vec4 light0Direction;"   "\r\n"
    "uniform mediump vec4 light0ior;"         "\r\n" // inner, outer, range
    "uniform lowp vec4 light0Color;"    "\r\n"
"\r\n"
    "uniform mediump vec4 light1Position;"    "\r\n"
    "uniform lowp vec4 light1Direction;"   "\r\n"
    "uniform mediump vec4 light1ior;"         "\r\n" // inner, outer, range
    "uniform lowp vec4 light1Color;"    "\r\n"
"\r\n"
    "uniform mediump vec4 light2Position;"    "\r\n"
    "uniform lowp vec4 light2Direction;"   "\r\n"
    "uniform mediump vec4 light2ior;"         "\r\n" // inner, outer, range
    "uniform lowp vec4 light2Color;"    "\r\n"
"\r\n"
    "uniform mediump vec4 light3Position;"    "\r\n"
    "uniform lowp vec4 light3Direction;"   "\r\n"
    "uniform mediump vec4 light3ior;"         "\r\n" // inner, outer, range
    "uniform lowp vec4 light3Color;"    "\r\n"
"\r\n"
    ""                                  "\r\n"
    "lowp vec3 CalculateLight(mediump vec4 dir, mediump vec4 pos, mediump vec4 ior, lowp vec4 color, mediump vec4 vertWorldPos, mediump vec4 normal)"   "\r\n"
    "{"   "\r\n"
    "lowp float nDotL  = dot(normal.xyz, -dir.xyz);""\r\n"
    "return color.xyz * nDotL * color.w;"
    ""
    //"lowp vec3 vDir  = normalize(vertWorldPos.xyz - pos.xyz);""\r\n"
    //"lowp float nDotL  = dot(normal.xyz, -dir.xyz) * (1.0 - ior.w);""\r\n"
    //"      nDotL += dot(normal.xyz, -vDir) * ior.w;""\r\n"
    //"lowp float v0 = clamp(dot(vDir.xyz, dir.xyz), 0.0, 1.0);""\r\n"
    //"lowp float inner = clamp((1.0 - (1.0 - v0) / ior.x), 0.0, 1.0);""\r\n"
    //"lowp float outer = clamp((1.0 - (1.0 - v0) / ior.y), 0.0, 1.0);""\r\n"
    //"lowp float cone = (inner + outer) / 2.0;""\r\n"
    //"lowp float range = clamp(1.0 - distance(pos, vertWorldPos) / ior.z, 0.0, 1.0);""\r\n"
    //"return clamp(color.xyz * nDotL * cone * range, 0.0, 1.0) * color.w;" "\r\n"
    "}"   "\r\n"
"\r\n"
    "void main()"    "\r\n"
    "{"                                 "\r\n"
    ""                                  "\r\n"
    "lowp vec3 lightColor = vec3(0,0,0);" "\r\n"
    "lowp vec4 texel = (color * varyColor * texture2D( textureMap0, varyUV.xy));""\r\n"
    "lightColor  = CalculateLight(light0Direction, light0Position, light0ior, light0Color, varyWorldP, varyNormal);" "\r\n"
    //"lightColor += CalculateLight(light1Direction, light1Position, light1ior, light1Color, varyWorldP, varyNormal);" "\r\n"
    //"lightColor += CalculateLight(light2Direction, light2Position, light2ior, light2Color, varyWorldP, varyNormal);" "\r\n"
    //"lightColor += CalculateLight(light3Direction, light3Position, light3ior, light3Color, varyWorldP, varyNormal);" "\r\n"
    "texel.xyz *= (lightColor + ambientColor.xyz + selfIllumColor.xyz);"           "\r\n"
    "gl_FragColor = texel;" "\r\n"
    "}";


const char *s_VideoFragmentSource = s_2dFragmentSource;

const char *s_VertexBlendFragmentSource =
    "varying mediump vec4 varyUV;"
    "varying lowp    vec4 varyColor;"
    ""
    "uniform lowp vec4 color;"
    "uniform sampler2D textureMap0;"
    "uniform sampler2D textureMap1;"
    ""
    "void main()"
    "{"
    "lowp vec4 t0 = texture2D( textureMap0, varyUV.xy );"
    "lowp vec4 t1 = texture2D( textureMap1, varyUV.zw );"
    "gl_FragColor = color * (varyColor * (t0 * varyColor.aaaa + t1 * (vec4(1,1,1,1) - varyColor.aaaa)));"
    "}";


const char *s_VertexBlendFragmentSource_Sel = 
    "varying mediump vec4 varyUV;"
    "varying lowp    vec4 varyColor;"
    ""
    "uniform lowp vec4 color;"
    "uniform sampler2D textureMap0;"
    "uniform sampler2D textureMap1;"
    ""
    "void main()"
    "{"
    "lowp vec4 t0 = texture2D( textureMap0, varyUV.xy );"
    "lowp vec4 t1 = texture2D( textureMap1, varyUV.zw );"
    "lowp vec4 c = color * (varyColor * (t0 * varyColor.aaaa + t1 * (vec4(1,1,1,1) - varyColor.aaaa)));"
    "gl_FragColor = vec4(0, 1, 0, 1) * sqrt(0.2126 * c.r + 0.7152 * c.g + 0.0722 * c.b);""\r\n"
    "gl_FragColor.a = c.a;"
    "}";

#elif defined MAC
#error Shaders must be written
#endif

void CheckError(
    const char *pString,
    GLuint obj,
    const char *pFile,
    int line
    )
{
#ifdef _DEBUG
    if ( true )
    {
        int error = glGetError( );

        int infologLength = 0;
        char status[ 512 ] = { 0 };

        if (obj)
        {
            if ( pglIsShader(obj) )
            {
                pglGetShaderInfoLog(obj, sizeof(status) - 1, &infologLength, status );
                //if (*status)
                //  Debug::Print( Debug::TypeError, "%s(%d): glError: %s: %s, error = 0x%04x", pFile, line, pString, status, error );
            }
            else
            {
                pglGetProgramInfoLog(obj, sizeof(status) - 1, &infologLength, status );
                //if (*status)
                //  Debug::Print( Debug::TypeError, "%s(%d): glError: %s: %s, error = 0x%04x", pFile, line, pString, status, error );
            }
        }

        if ( error )
        {
            Debug::Print( Debug::TypeError, "%s(%d): glError: %s: %s, error = 0x%04x\n", pFile, line, pString, status, error );
        }
    }
#endif
}

void Shader::Create(
    Id id
    )
{
    Asset::Create( id );

    m_StaticLocations.Create( );
    m_StaticSelectedLocations.Create( );
    m_SkinnedSelectedLocations.Create( );
    m_SkinnedLocations.Create( );

    m_StaticShader = 0;
    m_SkinnedSelectedShader = 0;
    m_StaticSelectedShader = 0;
    m_SkinnedShader = 0;
    m_ActiveShader = 0;

    m_NeedsCreate = true;
   
    CreateGl( );
}

void Shader::Destroy( void )
{
    DestroyGl( );

    m_StaticLocations.Destroy( );
    m_StaticSelectedLocations.Destroy( );
    m_SkinnedSelectedLocations.Destroy( );
    m_SkinnedLocations.Destroy( );
}

void Shader::MakeActive( bool selected, bool skinned )
{
    if ( true == m_NeedsCreate )
    {
        CreateGl( );
        m_NeedsCreate = false;
    }

    m_NumTextures = 0;

    glCheckError( "Shader::MakeActive Should be clear" );

    if (skinned && selected)
        m_ActiveShader = m_SkinnedSelectedShader;
    else if (skinned)
        m_ActiveShader = m_SkinnedShader;
    else if (selected)
        m_ActiveShader = m_StaticSelectedShader;
    else
        m_ActiveShader = m_StaticShader;

    pglUseProgram( m_ActiveShader );
    glCheckErrorParam( "Use Program", m_ActiveShader );

    pglDisableVertexAttribArray( VertexBuffer::Positions );
    glCheckError( "Shader: pglDisableVertexAttribArray( VertexBuffer::Position );" );

    pglDisableVertexAttribArray( VertexBuffer::Colors );
    glCheckError( "Shader: pglDisableVertexAttribArray( VertexBuffer::Color );" );

    pglDisableVertexAttribArray( VertexBuffer::Normals );
    glCheckError( "Shader: pglDisableVertexAttribArray( VertexBuffer::Normal );" );

    pglDisableVertexAttribArray( VertexBuffer::UV0s );
    glCheckError( "Shader: pglDisableVertexAttribArray( VertexBuffer::UV );" );

    pglDisableVertexAttribArray( VertexBuffer::UV1s );
    glCheckError( "Shader: pglDisableVertexAttribArray( VertexBuffer::UV );" );

    pglDisableVertexAttribArray( VertexBuffer::BoneIndices );
    glCheckError( "Shader: pglDisableVertexAttribArray( VertexBuffer::BoneIndices );" );

    pglDisableVertexAttribArray( VertexBuffer::BoneWeights );
    glCheckError( "Shader: pglDisableVertexAttribArray( VertexBuffer::BoneWeights );" );
}

void Shader::SetTransforms(
    const char *pName,
    const Transform *pTransforms,
    int numTransforms
    )
{
    glCheckError( "Shader::SetTransforms Should be clear" );

    unsigned int index = GetUniformLocation( pName );

    Matrix matrices[ 128 ];

    int i;

    for ( i = 0; i < numTransforms; i++ )
    {
        Math::Transpose( &matrices[ i ], pTransforms[ i ].ToMatrix(true) );
    }

    pglUniformMatrix4fv( index, numTransforms, false, (float *) matrices ); 
    glCheckError( "Shader: pglUniformMatrix4fv" );
}

void Shader::SetMatrices(
    const char *pName,
    const Matrix *pMatrices,
    int numMatrices
    )
{
    glCheckError( "Shader::SetMatrices Should be clear" );

    unsigned int index = GetUniformLocation( pName );

    Matrix matrices[ 128 ];

    int i;

    for ( i = 0; i < numMatrices; i++ )
    {
        Math::Transpose( &matrices[ i ], pMatrices[ i ] );
    }

    pglUniformMatrix4fv( index, numMatrices, false, (float *) matrices ); 
    glCheckError( "Shader: pglUniformMatrix4fv" );
}

void Shader::SetTexture(
    int sampler,
    ResourceHandle texture,
    bool wrap,
    bool filter
    )
{
    glCheckError( "Shader::SetTexture Should be clear" );

    Texture *pTexture = GetResource( texture, Texture );
    pTexture->Reload( );

    unsigned int index = sampler == 0 ? GetUniformLocation( "textureMap0" ) : GetUniformLocation( "textureMap1" );

    pglActiveTexture( GL_TEXTURE0 + m_NumTextures );
    glCheckError( "Shader: pglActiveTexture" );

    glBindTexture( pTexture->GetTextureType( ), pTexture->GetTexture( ) );
    glCheckError( "Shader: glBindTexture" );

    if ( true == filter )
    {
        if ( pTexture->HasMipMaps() )
            glTexParameteri( pTexture->GetTextureType( ), GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR );
        else
            glTexParameteri( pTexture->GetTextureType( ), GL_TEXTURE_MIN_FILTER, GL_LINEAR );

        glCheckError( "Shader: glTexParameteri" );

        glTexParameteri( pTexture->GetTextureType( ), GL_TEXTURE_MAG_FILTER, GL_LINEAR );
        glCheckError( "Shader: glTexParameteri" );
    }
    else
    {
        if ( pTexture->HasMipMaps() )
            glTexParameteri( pTexture->GetTextureType( ), GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST );
        else
            glTexParameteri( pTexture->GetTextureType( ), GL_TEXTURE_MIN_FILTER, GL_NEAREST );

        glCheckError( "Shader: glTexParameteri" );

        glTexParameteri( pTexture->GetTextureType( ), GL_TEXTURE_MAG_FILTER, GL_NEAREST );
        glCheckError( "Shader: glTexParameteri" );
    }

    glTexParameteri( pTexture->GetTextureType( ), GL_TEXTURE_WRAP_S, wrap ? GL_REPEAT : GL_CLAMP_TO_EDGE );
    glCheckError( "Shader: glTexParameteri" );

    glTexParameteri( pTexture->GetTextureType( ), GL_TEXTURE_WRAP_T, wrap ? GL_REPEAT : GL_CLAMP_TO_EDGE );
    glCheckError( "Shader: glTexParameteri" );

    pglUniform1i( index, m_NumTextures );
    glCheckError( "Shader: pglUniform1i" );

    ++m_NumTextures;
}

void Shader::SetVector(
    const char *pName,
    const Vector &vector
    )
{
    glCheckError( "Shader::SetVector Should be clear" );

    unsigned int index = GetUniformLocation( pName );
    if ( ((unsigned int)-1) == index ) return;
    
    pglUniform4f( index, vector.x, vector.y, vector.z, vector.w );
    glCheckError( "Shader: pglUniform4f" );
}

void Shader::SetLight(
    int lightIndex,
    const LightDesc &desc
)
{
    if (desc.cast == LightDesc::CastAmbient)
        SetVector("ambientColor", desc.color);
    else
    {
        const char *pColor, *pPosition, *pDirection, *pIor;

        if (0 == lightIndex)
        {
            pColor = "light0Color";
            pPosition = "light0Position";
            pDirection = "light0Direction";
            pIor = "light0ior";
        }
        else if (1 == lightIndex)
        {
            pColor = "light1Color";
            pPosition = "light1Position";
            pDirection = "light1Direction";
            pIor = "light1ior";
        }
        else if (2 == lightIndex)
        {
            pColor = "light2Color";
            pPosition = "light2Position";
            pDirection = "light2Direction";
            pIor = "light2ior";
        }
        else if (3 == lightIndex)
        {
            pColor = "light3Color";
            pPosition = "light3Position";
            pDirection = "light3Direction";
            pIor = "light3ior";
        }
        else
            return;

        Vector ci = desc.color;
        ci.w = desc.nits;

        SetVector( pColor, ci );
        SetVector( pPosition, desc.position );
        SetVector( pDirection, desc.direction );
    
        float innerAngle = desc.inner;
        float outerAngle = desc.outer;

        if (innerAngle != FLT_MAX)
            innerAngle = 1.0f - Math::Cos(innerAngle / 2);

        if (outerAngle != FLT_MAX)
            outerAngle = 1.0f - Math::Cos(outerAngle / 2);

        SetVector( pIor, Vector(innerAngle, outerAngle, desc.range, desc.cast == LightDesc::CastOmni ? 1.0f : 0.0f) );
    }
}

int Shader::GetUniformLocation(
    const char *pName
    )
{
    int index;

    if ( m_ActiveShader == m_StaticShader )
    {
        if ( true == m_StaticLocations.Get(pName, &index) )
            return index;
    }
    else if ( m_ActiveShader == m_SkinnedShader )
    {
        if ( true == m_SkinnedLocations.Get(pName, &index) )
            return index;
    }
    else if ( m_ActiveShader == m_StaticSelectedShader )
    {
        if ( true == m_StaticSelectedLocations.Get(pName, &index) )
            return index;
    }
    else if ( m_ActiveShader == m_SkinnedSelectedShader )
    {
        if ( true == m_SkinnedSelectedLocations.Get(pName, &index) )
            return index;
    }

    return -1;
}

uint32 CreateProgram(
    const char *pVertex,
    const char *pPixel,
    bool skinned
    )
{
    uint32 program = pglCreateProgram( );

    GLenum vsh, fsh;

    vsh = pglCreateShader( GL_VERTEX_SHADER );
    fsh = pglCreateShader( GL_FRAGMENT_SHADER );

    pglShaderSource( vsh, 1, &pVertex, NULL );
    pglShaderSource( fsh, 1, &pPixel, NULL );

    pglCompileShader( vsh );
    glCheckErrorParam( "CompileVSH", vsh );

    pglCompileShader( fsh );
    glCheckErrorParam( "CompileFSH", fsh );

    pglAttachShader( program, vsh );
    pglAttachShader( program, fsh );

    //LOG( "Binding Shader Attributes" );

    pglBindAttribLocation( program, VertexBuffer::Positions, "position" );
    glCheckErrorParam( "Bind Position", program );

    pglBindAttribLocation( program, VertexBuffer::Normals, "normal" );
    glCheckErrorParam( "Bind Normal", program );

    pglBindAttribLocation( program, VertexBuffer::UV0s, "uv0" );
    glCheckErrorParam( "Bind UV", program );

    pglBindAttribLocation( program, VertexBuffer::UV1s, "uv1" );
    glCheckErrorParam( "Bind UV", program );

    pglBindAttribLocation( program, VertexBuffer::Colors, "color" );
    glCheckErrorParam( "Bind Color", program );

    if (true == skinned)
    {
        pglBindAttribLocation( program, VertexBuffer::BoneIndices, "boneIndices" );
        glCheckErrorParam( "Bind boneIndices", program );

        pglBindAttribLocation( program, VertexBuffer::BoneWeights, "boneWeights" );
        glCheckErrorParam( "Bind boneWeights", program );   
    }

    //LOG( "Linking Shaders" );

    pglLinkProgram( program );
    glCheckErrorParam( "Link Program", program );

    GLint linked;
    pglGetProgramiv( program, GL_LINK_STATUS, &linked );
    Debug::Assert( Condition(linked), "Whoa it didn't link" );

    pglDeleteShader( vsh );
    pglDeleteShader( fsh );

    return program;
}

void Shader::CreateGl( void )
{
#ifdef WIN32
    if ( NULL == pglBindAttribLocation )
    {
        pglBindAttribLocation = (PFNGLBINDATTRIBLOCATIONPROC) wglGetProcAddress( "glBindAttribLocation" );
        pglLinkProgram        = (PFNGLLINKPROGRAMPROC)        wglGetProcAddress( "glLinkProgram" );
        pglAttachShader       = (PFNGLATTACHSHADERPROC)       wglGetProcAddress( "glAttachShader" );
        pglCompileShader      = (PFNGLCOMPILESHADERPROC)      wglGetProcAddress( "glCompileShader" );
        pglShaderSource       = (PFNGLSHADERSOURCEPROC)       wglGetProcAddress( "glShaderSource" );
        pglCreateShader       = (PFNGLCREATESHADERPROC)       wglGetProcAddress( "glCreateShader" );
        pglCreateProgram      = (PFNGLCREATEPROGRAMPROC)      wglGetProcAddress( "glCreateProgram" );
        pglUseProgram         = (PFNGLUSEPROGRAMPROC)         wglGetProcAddress( "glUseProgram" );
        pglUniformMatrix4fv   = (PFNGLUNIFORMMATRIX4FVPROC)   wglGetProcAddress( "glUniformMatrix4fv" );
        pglUniform1i          = (PFNGLUNIFORM1IPROC)          wglGetProcAddress( "glUniform1i" );
        pglGetUniformLocation = (PFNGLGETUNIFORMLOCATIONPROC )wglGetProcAddress( "glGetUniformLocation" );
        pglActiveTexture      = (PFNGLACTIVETEXTUREPROC)      wglGetProcAddress( "glActiveTexture" );
        pglUniform4f          = (PFNGLUNIFORM4FPROC)          wglGetProcAddress( "glUniform4f" );
        pglVertexAttribPointer= (PFNGLVERTEXATTRIBPOINTERPROC)wglGetProcAddress( "glVertexAttribPointer" );
        pglDeleteBuffers      = (PFNGLDELETEBUFFERSPROC)      wglGetProcAddress( "glDeleteBuffers" );
        pglBindBuffer         = (PFNGLBINDBUFFERPROC)         wglGetProcAddress( "glBindBuffer" );
        pglGenBuffers         = (PFNGLGENBUFFERSPROC)         wglGetProcAddress( "glGenBuffers" );
        pglBufferData         = (PFNGLBUFFERDATAPROC)         wglGetProcAddress( "glBufferData" );
        pglGetProgramInfoLog  = (PFNGLGETPROGRAMINFOLOGPROC)  wglGetProcAddress( "glGetProgramInfoLog" );
        pglGetShaderInfoLog   = (PFNGLGETSHADERINFOLOGPROC)   wglGetProcAddress( "glGetShaderInfoLog" );
        pglIsShader           = (PFNGLISSHADERPROC)           wglGetProcAddress( "glIsShader" );
        pglGetProgramiv       = (PFNGLGETPROGRAMIVPROC)       wglGetProcAddress( "glGetProgramiv" );
        pglGetAttribLocation  = (PFNGLGETATTRIBLOCATIONPROC)  wglGetProcAddress( "glGetAttribLocation" );
        pglDeleteShader       = (PFNGLDELETESHADERPROC)       wglGetProcAddress( "glDeleteShader" );

        pglEnableVertexAttribArray  = (PFNGLENABLEVERTEXATTRIBARRAYPROC) wglGetProcAddress( "glEnableVertexAttribArray" );
        pglDisableVertexAttribArray = (PFNGLDISABLEVERTEXATTRIBARRAYPROC) wglGetProcAddress( "glDisableVertexAttribArray" );
    }
#endif


    const char *pVertex = NULL;
    const char *pSkin   = NULL;
    const char *pPixel  = NULL;
    const char *pSel      = NULL;

    if ( GetId( ) == Id("Video.shader") )
    {
        pVertex = s_2dSource;
        pPixel   = s_VideoFragmentSource;
    }
    else if ( GetId( ) == Id("2d.shader") )
    {
        pVertex = s_2dSource;
        pPixel  = s_2dFragmentSource;
        pSel    = s_2dFragmentSource_Sel;
    }
    else if ( GetId( ) == Id("Prelit.shader") )
    {
        pSkin   = s_3dSourceSkin;
        pVertex = s_3dSource;
        pPixel  = s_PrelitFragmentSource;
        pSel    = s_PrelitFragmentSource_Sel;
    }
    else if ( GetId( ) == Id("Lit.shader") )
    {
        pSkin   = s_3dSourceSkinLit;
        pVertex = s_3dSourceLit;
        pPixel  = s_LitFragmentSource;
        pSel    = s_PrelitFragmentSource_Sel;
    }
    else if ( GetId( ) == Id("Particles.shader") )
    {
        pVertex = s_3dParticleSource;
        pPixel  = s_ParticleFragmentSource;
    }
    else if ( GetId( ) == Id("VertexBlend.shader") )
    {
        pVertex = s_3dVertexBlendSource;
        pPixel  = s_VertexBlendFragmentSource;
        pSel    = s_VertexBlendFragmentSource_Sel;
    }

    glCheckError( "Shader::CreateGl Should be clear" );

    m_StaticShader  = CreateProgram( pVertex, pPixel, false );
    glCheckError( "Shader::CreateProgram Should be clear" );

    m_StaticLocations.Clear( );

    m_StaticLocations.Add( "world",      pglGetUniformLocation(m_StaticShader, "world") );
    glCheckError( "Shader: pglGetUniformLocation" );

    m_StaticLocations.Add( "view",       pglGetUniformLocation(m_StaticShader, "view") );
    glCheckError( "Shader: pglGetUniformLocation" );

    m_StaticLocations.Add( "projection", pglGetUniformLocation(m_StaticShader, "projection") );
    glCheckError( "Shader: pglGetUniformLocation" );

    m_StaticLocations.Add( "textureMap0", pglGetUniformLocation(m_StaticShader, "textureMap0") );
    glCheckError( "Shader: pglGetUniformLocation" );

    m_StaticLocations.Add( "textureMap1", pglGetUniformLocation(m_StaticShader, "textureMap1") );
    glCheckError( "Shader: pglGetUniformLocation" );

    m_StaticLocations.Add( "color",      pglGetUniformLocation(m_StaticShader, "color") );
    glCheckError( "Shader: pglGetUniformLocation" );

    m_StaticLocations.Add( "ambientColor", pglGetUniformLocation(m_StaticShader, "ambientColor") );
    glCheckError( "Shader: pglGetUniformLocation" );

    m_StaticLocations.Add( "selfIllumColor", pglGetUniformLocation(m_StaticShader, "selfIllumColor") );
    glCheckError( "Shader: pglGetUniformLocation" );

    m_StaticLocations.Add( "uvs",      pglGetUniformLocation(m_StaticShader, "uvs") );
    glCheckError( "Shader: pglGetUniformLocation" );

    m_StaticLocations.Add( "light0Direction", pglGetUniformLocation(m_StaticShader, "light0Direction") );
    glCheckError( "Shader: pglGetUniformLocation" );

    m_StaticLocations.Add( "light0Position", pglGetUniformLocation(m_StaticShader, "light0Position") );
    glCheckError( "Shader: pglGetUniformLocation" );

    m_StaticLocations.Add( "light0ior", pglGetUniformLocation(m_StaticShader, "light0ior") );
    glCheckError( "Shader: pglGetUniformLocation" );

    m_StaticLocations.Add( "light0Color", pglGetUniformLocation(m_StaticShader, "light0Color") );
    glCheckError( "Shader: pglGetUniformLocation" );

    m_StaticLocations.Add( "light1Direction", pglGetUniformLocation(m_StaticShader, "light1Direction") );
    glCheckError( "Shader: pglGetUniformLocation" );

    m_StaticLocations.Add( "light1Position", pglGetUniformLocation(m_StaticShader, "light1Position") );
    glCheckError( "Shader: pglGetUniformLocation" );

    m_StaticLocations.Add( "light1ior", pglGetUniformLocation(m_StaticShader, "light1ior") );
    glCheckError( "Shader: pglGetUniformLocation" );

    m_StaticLocations.Add( "light1Color", pglGetUniformLocation(m_StaticShader, "light1Color") );
    glCheckError( "Shader: pglGetUniformLocation" );

    m_StaticLocations.Add( "light2Direction", pglGetUniformLocation(m_StaticShader, "light2Direction") );
    glCheckError( "Shader: pglGetUniformLocation" );

    m_StaticLocations.Add( "light2Position", pglGetUniformLocation(m_StaticShader, "light2Position") );
    glCheckError( "Shader: pglGetUniformLocation" );

    m_StaticLocations.Add( "light2ior", pglGetUniformLocation(m_StaticShader, "light2ior") );
    glCheckError( "Shader: pglGetUniformLocation" );

    m_StaticLocations.Add( "light2Color", pglGetUniformLocation(m_StaticShader, "light2Color") );
    glCheckError( "Shader: pglGetUniformLocation" );

    m_StaticLocations.Add( "light3Direction", pglGetUniformLocation(m_StaticShader, "light3Direction") );
    glCheckError( "Shader: pglGetUniformLocation" );

    m_StaticLocations.Add( "light3Position", pglGetUniformLocation(m_StaticShader, "light3Position") );
    glCheckError( "Shader: pglGetUniformLocation" );

    m_StaticLocations.Add( "light3ior", pglGetUniformLocation(m_StaticShader, "light3ior") );
    glCheckError( "Shader: pglGetUniformLocation" );

    m_StaticLocations.Add( "light3Color", pglGetUniformLocation(m_StaticShader, "light3Color") );
    glCheckError( "Shader: pglGetUniformLocation" );

    m_SkinnedLocations.Clear( );

    if ( pSkin )
    {
        m_SkinnedShader = CreateProgram( pSkin, pPixel, true );

        m_SkinnedLocations.Add( "world", pglGetUniformLocation(m_SkinnedShader, "world") );
        glCheckError( "Shader: pglGetUniformLocation" );

        m_SkinnedLocations.Add( "view",        pglGetUniformLocation(m_SkinnedShader, "view") );
        glCheckError( "Shader: pglGetUniformLocation" );

        m_SkinnedLocations.Add( "projection",  pglGetUniformLocation(m_SkinnedShader, "projection") );
        glCheckError( "Shader: pglGetUniformLocation" );

        m_SkinnedLocations.Add( "textureMap0",  pglGetUniformLocation(m_SkinnedShader, "textureMap0") );
        glCheckError( "Shader: pglGetUniformLocation" );

        m_SkinnedLocations.Add( "textureMap1",  pglGetUniformLocation(m_SkinnedShader, "textureMap1") );
        glCheckError( "Shader: pglGetUniformLocation" );

        m_SkinnedLocations.Add( "color",       pglGetUniformLocation(m_SkinnedShader, "color") );
        glCheckError( "Shader: pglGetUniformLocation" );

        m_SkinnedLocations.Add( "uvs",       pglGetUniformLocation(m_SkinnedShader, "uvs") );
        glCheckError( "Shader: pglGetUniformLocation" );

        m_SkinnedLocations.Add( "skin",       pglGetUniformLocation(m_SkinnedShader, "skin") );
        glCheckError( "Shader: pglGetUniformLocation" );

        m_SkinnedLocations.Add( "ambientColor", pglGetUniformLocation(m_SkinnedShader, "ambientColor") );
        glCheckError( "Shader: pglGetUniformLocation" );

        m_SkinnedLocations.Add( "selfIllumColor", pglGetUniformLocation(m_SkinnedShader, "selfIllumColor") );
        glCheckError( "Shader: pglGetUniformLocation" );

        m_SkinnedLocations.Add( "light0Direction", pglGetUniformLocation(m_SkinnedShader, "light0Direction") );
        glCheckError( "Shader: pglGetUniformLocation" );

        m_SkinnedLocations.Add( "light0Position", pglGetUniformLocation(m_SkinnedShader, "light0Position") );
        glCheckError( "Shader: pglGetUniformLocation" );

        m_SkinnedLocations.Add( "light0ior", pglGetUniformLocation(m_SkinnedShader, "light0ior") );
        glCheckError( "Shader: pglGetUniformLocation" );

        m_SkinnedLocations.Add( "light0Color", pglGetUniformLocation(m_SkinnedShader, "light0Color") );
        glCheckError( "Shader: pglGetUniformLocation" );

        m_SkinnedLocations.Add( "light1Direction", pglGetUniformLocation(m_SkinnedShader, "light1Direction") );
        glCheckError( "Shader: pglGetUniformLocation" );

        m_SkinnedLocations.Add( "light1Position", pglGetUniformLocation(m_SkinnedShader, "light1Position") );
        glCheckError( "Shader: pglGetUniformLocation" );

        m_SkinnedLocations.Add( "light1ior", pglGetUniformLocation(m_SkinnedShader, "light1ior") );
        glCheckError( "Shader: pglGetUniformLocation" );

        m_SkinnedLocations.Add( "light1Color", pglGetUniformLocation(m_SkinnedShader, "light1Color") );
        glCheckError( "Shader: pglGetUniformLocation" );

        m_SkinnedLocations.Add( "light2Direction", pglGetUniformLocation(m_SkinnedShader, "light2Direction") );
        glCheckError( "Shader: pglGetUniformLocation" );

        m_SkinnedLocations.Add( "light2Position", pglGetUniformLocation(m_SkinnedShader, "light2Position") );
        glCheckError( "Shader: pglGetUniformLocation" );

        m_SkinnedLocations.Add( "light2ior", pglGetUniformLocation(m_SkinnedShader, "light2ior") );
        glCheckError( "Shader: pglGetUniformLocation" );

        m_SkinnedLocations.Add( "light2Color", pglGetUniformLocation(m_SkinnedShader, "light2Color") );
        glCheckError( "Shader: pglGetUniformLocation" );

        m_SkinnedLocations.Add( "light3Direction", pglGetUniformLocation(m_SkinnedShader, "light3Direction") );
        glCheckError( "Shader: pglGetUniformLocation" );

        m_SkinnedLocations.Add( "light3Position", pglGetUniformLocation(m_SkinnedShader, "light3Position") );
        glCheckError( "Shader: pglGetUniformLocation" );

        m_SkinnedLocations.Add( "light3ior", pglGetUniformLocation(m_SkinnedShader, "light3ior") );
        glCheckError( "Shader: pglGetUniformLocation" );

        m_SkinnedLocations.Add( "light3Color", pglGetUniformLocation(m_SkinnedShader, "light3Color") );
        glCheckError( "Shader: pglGetUniformLocation" );
    }


    m_StaticSelectedLocations.Clear( );
    m_SkinnedSelectedLocations.Clear( );
    
    if ( pSel )
    {
        m_StaticSelectedShader = CreateProgram( pVertex, pSel, true );

        m_StaticSelectedLocations.Add( "world", pglGetUniformLocation(m_StaticSelectedShader, "world") );
        glCheckError( "Shader: pglGetUniformLocation" );

        m_StaticSelectedLocations.Add( "view",        pglGetUniformLocation(m_StaticSelectedShader, "view") );
        glCheckError( "Shader: pglGetUniformLocation" );

        m_StaticSelectedLocations.Add( "projection",  pglGetUniformLocation(m_StaticSelectedShader, "projection") );
        glCheckError( "Shader: pglGetUniformLocation" );

        m_StaticSelectedLocations.Add( "textureMap0",  pglGetUniformLocation(m_StaticSelectedShader, "textureMap0") );
        glCheckError( "Shader: pglGetUniformLocation" );

        m_StaticSelectedLocations.Add( "textureMap1",  pglGetUniformLocation(m_StaticSelectedShader, "textureMap1") );
        glCheckError( "Shader: pglGetUniformLocation" );

        m_StaticSelectedLocations.Add( "color",       pglGetUniformLocation(m_StaticSelectedShader, "color") );
        glCheckError( "Shader: pglGetUniformLocation" );

        m_StaticSelectedLocations.Add( "uvs",       pglGetUniformLocation(m_StaticSelectedShader, "uvs") );
        glCheckError( "Shader: pglGetUniformLocation" );

        m_StaticSelectedLocations.Add( "skin",       pglGetUniformLocation(m_StaticSelectedShader, "skin") );
        glCheckError( "Shader: pglGetUniformLocation" );

        if ( pSkin )
        {
            m_SkinnedSelectedShader = CreateProgram( pSkin, pSel, true );

            m_SkinnedSelectedLocations.Add( "world", pglGetUniformLocation(m_SkinnedSelectedShader, "world") );
            glCheckError( "Shader: pglGetUniformLocation" );

            m_SkinnedSelectedLocations.Add( "view",        pglGetUniformLocation(m_SkinnedSelectedShader, "view") );
            glCheckError( "Shader: pglGetUniformLocation" );

            m_SkinnedSelectedLocations.Add( "projection",  pglGetUniformLocation(m_SkinnedSelectedShader, "projection") );
            glCheckError( "Shader: pglGetUniformLocation" );

            m_SkinnedSelectedLocations.Add( "textureMap0",  pglGetUniformLocation(m_SkinnedSelectedShader, "textureMap0") );
            glCheckError( "Shader: pglGetUniformLocation" );

            m_SkinnedSelectedLocations.Add( "textureMap1",  pglGetUniformLocation(m_SkinnedSelectedShader, "textureMap1") );
            glCheckError( "Shader: pglGetUniformLocation" );

            m_SkinnedSelectedLocations.Add( "color",       pglGetUniformLocation(m_SkinnedSelectedShader, "color") );
            glCheckError( "Shader: pglGetUniformLocation" );

            m_SkinnedSelectedLocations.Add( "uvs",       pglGetUniformLocation(m_SkinnedSelectedShader, "uvs") );
            glCheckError( "Shader: pglGetUniformLocation" );

            m_SkinnedSelectedLocations.Add( "skin",       pglGetUniformLocation(m_SkinnedSelectedShader, "skin") );
            glCheckError( "Shader: pglGetUniformLocation" );
        }
    }

    m_NumTextures = 0;
}

void Shader::DestroyGl( void )
{
    glDeleteProgram( m_SkinnedShader );
    glDeleteProgram( m_StaticShader );

    m_NeedsCreate = true;
}

Gl &Gl::Instance( void )
{
    static Gl s_instance;
    return s_instance;
}

void Gl::CreateChannel( void )
{
    m_pChannel = new Channel;
    m_pChannel->Create( Id("GlInstance"), NULL );

    ChannelSystem::Instance( ).Add( m_pChannel );
}

void Gl::DestroyChannel( void )
{
    ChannelSystem::Instance( ).Remove( m_pChannel );

    m_pChannel->Destroy( );
    delete m_pChannel;
}

















