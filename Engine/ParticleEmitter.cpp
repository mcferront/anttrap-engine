#include "EnginePch.h"

#include "ParticleEmitter.h"
#include "DebugGraphics.h"
#include "MaterialObject.h"
#include "RenderWorld.h"
#include "Viewport.h"

void ParticleEmitter::Create(
    const Transform &worldTransform,
    const ParticleEmitterDesc &emitterDesc
)
{
    m_pComponent = NULL;

    m_Particles.Create( );

    m_Pool.Create( );

    m_Desc = emitterDesc;

    m_Desc.emitArc.x = Math::DegreesToRadians(emitterDesc.emitArc.x);
    m_Desc.emitArc.y = 0.0f;
    m_Desc.emitArc.z = Math::DegreesToRadians(emitterDesc.emitArc.z);


    m_TileDesc.tileWidth  = m_Desc.tileWidth;
    m_TileDesc.tileHeight = m_Desc.tileHeight;
    m_TileDesc.particleStartWidth  = m_Desc.particleStartWidth;
    m_TileDesc.particleStartHeight = m_Desc.particleStartHeight;
    m_TileDesc.particleEndWidth    = m_Desc.particleEndWidth;
    m_TileDesc.particleEndHeight   = m_Desc.particleEndHeight;
    m_TileDesc.startColorBlend     = m_Desc.startTintColorBlend;
    m_TileDesc.endColorBlend       = m_Desc.endTintColorBlend;

    m_Time       = 0;
    m_CycleTime  = 0;
    m_NextCycleTime = 0;
    m_ActiveTime    = Math::Rand( m_Desc.minActiveTime, m_Desc.maxActiveTime );
    m_GoForever     = -1 == m_Desc.minActiveTime;

    m_WorldTransform = worldTransform;

    RenderObject::AddToScene( );
}

void ParticleEmitter::Destroy( void )
{
    RenderObject::RemoveFromScene( );

    delete m_Desc.pMaterial;

    m_Pool.Destroy( );

    m_Particles.Destroy( );
}

bool ParticleEmitter::Update(
    float deltaSeconds
)
{
    bool shouldStop = false == m_GoForever && m_Time >= m_ActiveTime;

    if ( m_CycleTime >= m_NextCycleTime && false == shouldStop && m_Time >= m_Desc.delayTime )
    {
        EmitParticles( &m_Particles );

        m_CycleTime = 0;
        m_NextCycleTime = Math::Rand( m_Desc.minCycleTime, m_Desc.maxCycleTime );
    }

    for ( uint32 i = 0; i < m_Particles.GetSize(); i++ )
    {
        Particle *pParticle = m_Particles.GetAt(i);
        if ( false == pParticle->Update(deltaSeconds, m_Desc.gravity) )
        {
            m_Particles.RemoveAt( i-- );
            m_Pool.Free( pParticle );
        }
    }

    if ( true == shouldStop && m_Particles.GetSize() == 0 ) return false;
   
    //Vector emitArc;
    //Vector tintColor;
    //int tileWidth;
    //int tileHeight;

    m_CycleTime  += deltaSeconds;
    m_Time       += deltaSeconds;

    return true;
}
void ParticleEmitter::End( void )
{
   m_GoForever  = false;
   m_ActiveTime = 0;
   m_Time       = 0;
}

void ParticleEmitter::EmitParticles( 
    List<Particle*> *pAddTo 
)
{
    int count = Math::Rand( m_Desc.minEmitCount, m_Desc.maxEmitCount );
    
    Vector position(0, 0, 0, 1);
    
    //if it's not relative, give it a one time position for emission
    //if it is relative then it'll stay at 0 and the current emitter transform is sent to the shader
    if (false == m_Desc.relativeToEmitter) m_WorldTransform.GetTranslation( &position );

    for ( int i = 0; i < count; i++ )
    {
        float x = Math::Rand( - m_Desc.emitArc.x, m_Desc.emitArc.x );
        float z = Math::Rand( - m_Desc.emitArc.z, m_Desc.emitArc.z );
        
        x = Math::Sin( x );
        z = Math::Sin( z );

        float y = Math::Sqrt(1.0f - x * x + z * z);

        Vector arc( x, y, z );
        Math::Rotate( &arc, arc, m_WorldTransform );

        Particle *pParticle = m_Pool.Alloc( );

        pParticle->Create(
            position,
            arc * Math::Rand(m_Desc.minEmitVelocity, m_Desc.maxEmitVelocity),  
            Math::Rand(m_Desc.minLifeTime, m_Desc.maxLifeTime ),  
            Math::Rand(m_Desc.minRotationRate, m_Desc.maxRotationRate),
            m_Desc.fadeInTime,
            m_Desc.fadeOutTime);
    
        pAddTo->Add( pParticle );
    }
}

void ParticleEmitter::GetRenderData(
   RendererDesc *pDesc
) const
{
    if ( m_Particles.GetSize() > 0 )
    {
        RenderObjectDesc *pRODesc = RenderWorld::Instance( ).AllocRenderObjectDesc( );

        Transform transform = Math::IdentityTransform( );

        if (true == m_Desc.relativeToEmitter) 
        {
            Vector position;
            m_WorldTransform.GetTranslation( &position );
            transform.SetTranslation( position );
        }

        bool uniqueRotations = false == Math::FloatTest(m_Desc.minRotationRate, m_Desc.maxRotationRate, 0.001f);

        pRODesc->renderContext = m_Desc.pMaterial->GetRenderContext( pDesc->pPass, pDesc->viewport.GetContext(), m_Desc.vertexContext );
        pRODesc->pMaterial  = m_Desc.pMaterial;
        pRODesc->pObject    = this;
        pRODesc->argList    = ArgList( transform, m_Desc.startTintColor, m_Desc.endTintColor, m_Particles.GetSize(), uniqueRotations );
        pRODesc->renderFunc = ParticleEmitter::Render;

        unsigned char *pPositions = (unsigned char *) pRODesc->buffer.Write( m_Particles.GetSize() * sizeof(Vector) );
        float *pTimes = (float *) pRODesc->buffer.Write( m_Particles.GetSize() * sizeof(float) );
        float *pAlphas = (float *) pRODesc->buffer.Write( m_Particles.GetSize() * sizeof(float) );
        
        pRODesc->buffer.Write( &m_TileDesc, sizeof(m_TileDesc) );

        for ( uint32 i = 0; i < m_Particles.GetSize(); i++ )
        {
            Vector position = *m_Particles.GetAt( i )->GetPosition( );

            memcpy( pPositions, &position, sizeof(position) );
            pPositions += sizeof(position);
        
            pTimes[ i ] = m_Particles.GetAt( i )->GetTime( ) / m_Particles.GetAt( i )->GetLifetime( );
            pAlphas[ i ] = m_Particles.GetAt( i )->GetAlpha( );
        }

        pDesc->renderObjectDescs.Add( pRODesc );
    }
}

void ParticleEmitter::Render( 
   const RenderDesc &desc,
   RenderStats *pStats
)
{
   *pStats = RenderStats::Empty;

//    Transform transform;
//    uint32 startTintColor;
//    uint32 endTintColor;
//    uint32 numParticles;
//    bool uniqueRotations;
//
//    desc.pDesc->argList.GetArg( 0, &transform );
//    desc.pDesc->argList.GetArg( 1, &startTintColor );
//    desc.pDesc->argList.GetArg( 2, &endTintColor );
//    desc.pDesc->argList.GetArg( 3, &numParticles );
//    desc.pDesc->argList.GetArg( 4, &uniqueRotations );
//
//    unsigned char startR = startTintColor >> 16;
//    unsigned char startG = startTintColor >> 8;
//    unsigned char startB = startTintColor >> 0;
//    unsigned char endR = endTintColor >> 16;
//    unsigned char endG = endTintColor >> 8;
//    unsigned char endB = endTintColor >> 0;
//    int rangeR = (int) endR - startR;
//    int rangeG = (int) endG - startG;
//    int rangeB = (int) endB - startB;
//    struct ParticleVertex
//    {
//        float x, y, z;
//        float u, v;
//        uint32 color;
//    };
//
//    struct MyQuad
//    {
//        ParticleVertex vertices[ 4 ];
//    };
//
//    size_t indexSize = numParticles * 6 * sizeof(unsigned short);
//    size_t quadSize  = numParticles * sizeof(MyQuad);
//   
//    unsigned char *pBytes = (unsigned char *) pRasterizer->Alloc( indexSize + quadSize );
//
//
//    MyQuad *pHead = (MyQuad *) pBytes;
//
//    unsigned short *pIndices = (unsigned short *) (pBytes + quadSize);
//
//    MyQuad *pMyQuad = pHead;
//
//    uint32 i;
//
//    Vector   *pPositions = (Vector *) desc.pDesc->buffer.Read( 0 );
//    float    *pTimes     = (float *) desc.pDesc->buffer.Read( 1 );
//    float    *pAlphas    = (float *) desc.pDesc->buffer.Read( 2 );
//
//    ParticleTileDesc *pTileDesc  = (ParticleTileDesc *) desc.pDesc->buffer.Read( 3 );
//
//    // Always face the camera 
//    Transform worldTransform;
//    desc.viewport.GetCamera( )->GetWorldTransform( &worldTransform );
//
//    Material *pMaterial = GetResource( desc.pDesc->material, Material );
//    Texture  *pTexture  = GetResource( pMaterial->GetTexture( ), Texture );
//
//    int tilesAcross = pTexture->GetActualWidth( )  / pTileDesc->tileWidth;
//    int tilesDown   = pTexture->GetActualHeight( ) / pTileDesc->tileHeight;
//
//    int totalTiles = tilesAcross * tilesDown;
//
//    float uSize = (float) pTileDesc->tileWidth / pTexture->GetActualWidth( );
//    float vSize = (float) pTileDesc->tileHeight/ pTexture->GetActualHeight( );
//
//    float previousRotation = pPositions[ 0 ].w;
//    float prevWidth = -1, prevHeight = -1;
//
//    float colorBlendRange = pTileDesc->endColorBlend - pTileDesc->startColorBlend;
//    float invColorBlendRange = 1.0f / colorBlendRange;
//    Transform rotate;
//
//    Math::RotateZ( &rotate, previousRotation );
//    Math::Multiply( &rotate, rotate, worldTransform );
//
//    Vector s[ 4 ];
//
//    //int optCount = 0;
//
//
//    for ( i = 0; i < numParticles; i++ )
//    {  
//        int frame = (int) (pTimes[ i ] * totalTiles);
//
//        int pU = frame % tilesAcross;
//        int pV = frame / tilesAcross;
//
//        float u0 = pU * uSize;
//        float v0 = pV * vSize;
//
//        float u1 = u0 + uSize;
//        float v1 = v0 + vSize;
//
//        int indexSlot = i * 6;
//        int vertSlot  = i * 4;
//
//        float width = Math::Lerp( pTileDesc->particleStartWidth,  pTileDesc->particleEndWidth, pTimes[ i ] );
//        float height= Math::Lerp( pTileDesc->particleStartHeight, pTileDesc->particleEndHeight, pTimes[ i ] );
//
//        bool matchingRotation = true;
//
//        if (uniqueRotations && false == Math::FloatTest(pPositions[ i ].w, previousRotation, 0.001f))
//        {
//            matchingRotation = false;
//
//            previousRotation = pPositions[ i ].w;
//
//            Math::RotateZ( &rotate, previousRotation );
//            Math::Multiply( &rotate, rotate, worldTransform );
//        }
//
//        //remake quad if otations need to be computed 
//        //width / height is different
//        if ( false == matchingRotation || false == Math::FloatTest(width, prevWidth, 0.001f) || false == Math::FloatTest(height, prevHeight, 0.001f) )
//        {
//            prevWidth = width;
//            prevHeight= height;
//
//            float halfWidth  = width  * 0.5f;
//            float halfHeight = height * 0.5f;
//
//            s[ 0 ].x = - halfWidth;
//            s[ 0 ].y = + halfHeight;
//            s[ 0 ].z = 0;
//            s[ 0 ].w = 1;
//
//            s[ 1 ].x = + halfWidth;
//            s[ 1 ].y = + halfHeight;
//            s[ 1 ].z = 0;
//            s[ 1 ].w = 1;
//
//            s[ 2 ].x = - halfWidth;
//            s[ 2 ].y = - halfHeight;
//            s[ 2 ].z = 0;
//            s[ 2 ].w = 1;
//
//            s[ 3 ].x = + halfWidth;
//            s[ 3 ].y = - halfHeight;
//            s[ 3 ].z = 0;
//            s[ 3 ].w = 1;
//
//            Math::Rotate( &s[ 0 ], s[ 0 ], rotate );
//            Math::Rotate( &s[ 1 ], s[ 1 ], rotate );
//            Math::Rotate( &s[ 2 ], s[ 2 ], rotate );
//            Math::Rotate( &s[ 3 ], s[ 3 ], rotate );
//        }
//    
//        pIndices[ indexSlot + 0 ] = vertSlot + 0;
//        pIndices[ indexSlot + 1 ] = vertSlot + 1;
//        pIndices[ indexSlot + 2 ] = vertSlot + 2;
//
//        pIndices[ indexSlot + 3 ] = vertSlot + 2;
//        pIndices[ indexSlot + 4 ] = vertSlot + 1;
//        pIndices[ indexSlot + 5 ] = vertSlot + 3;
//
//        float t = pTimes[i] - pTileDesc->startColorBlend;
//        if (t < 0)
//           t = 0;
//        else if (t > colorBlendRange)
//           t = 1;
//        else
//           t = t * invColorBlendRange;
//        unsigned char r = startR + Math::FloatToInt(rangeR * t);
//        unsigned char g = startG + Math::FloatToInt(rangeG * t);
//        unsigned char b = startB + Math::FloatToInt(rangeB * t);
//       #ifdef DIRECTX9
//         uint32 color = (r << 16) | (g << 8) | b | (Math::FloatToInt(pAlphas[i] * 255.0f) << 24);
//       #elif defined OPENGL
//         uint32 color = (b << 16) | (g << 8) | (r << 0) | (Math::FloatToInt(pAlphas[i] * 255.0f) << 24);
//       #else
//         #error Graphics API Undefined
//       #endif
//        //top left
//        pMyQuad->vertices[ 0 ].x = s[ 0 ].x + pPositions[ i ].x;
//        pMyQuad->vertices[ 0 ].y = s[ 0 ].y + pPositions[ i ].y;
//        pMyQuad->vertices[ 0 ].z = s[ 0 ].z + pPositions[ i ].z;
//        pMyQuad->vertices[ 0 ].u = u0;
//        pMyQuad->vertices[ 0 ].v = v0;
//        pMyQuad->vertices[ 0 ].color = color;
//
//        //top right
//        pMyQuad->vertices[ 1 ].x = s[ 1 ].x + pPositions[ i ].x;
//        pMyQuad->vertices[ 1 ].y = s[ 1 ].y + pPositions[ i ].y;
//        pMyQuad->vertices[ 1 ].z = s[ 1 ].z + pPositions[ i ].z;
//        pMyQuad->vertices[ 1 ].u = u1;
//        pMyQuad->vertices[ 1 ].v = v0;
//        pMyQuad->vertices[ 1 ].color = color;
//
//        //bottom left
//        pMyQuad->vertices[ 2 ].x = s[ 2 ].x + pPositions[ i ].x;
//        pMyQuad->vertices[ 2 ].y = s[ 2 ].y + pPositions[ i ].y;
//        pMyQuad->vertices[ 2 ].z = s[ 2 ].z + pPositions[ i ].z;
//        pMyQuad->vertices[ 2 ].u = u0;
//        pMyQuad->vertices[ 2 ].v = v1;
//        pMyQuad->vertices[ 2 ].color = color;
//
//        //bottom right
//        pMyQuad->vertices[ 3 ].x = s[ 3 ].x + pPositions[ i ].x;
//        pMyQuad->vertices[ 3 ].y = s[ 3 ].y + pPositions[ i ].y;
//        pMyQuad->vertices[ 3 ].z = s[ 3 ].z + pPositions[ i ].z;
//        pMyQuad->vertices[ 3 ].u = u1; 
//        pMyQuad->vertices[ 3 ].v = v1;
//        pMyQuad->vertices[ 3 ].color = color;
//
//        ++pMyQuad;
//    }
//
//    //Debug::Print( Debug::TypeInfo, "Opt %d out of %d", optCount, numParticles );
//
//    Material::ParameterDesc parameterDesc;
//
//    pMaterial->Submit( transform, false, false, pRasterizer, 1 );
//
//#ifdef OPENGL
//    glCheckError( "Should be Clear" );
//
//    pglBindBuffer( GL_ARRAY_BUFFER, 0 );
//    glCheckError( "Quad::Bind Array Buffer" );
//
//    pglBindBuffer( GL_ELEMENT_ARRAY_BUFFER, 0 );
//    glCheckError( "Quad::Bind Element Array Buffer" );
//
//    char *pData = (char *) pHead;
//
//    pglVertexAttribPointer( VertexBuffer::Positions, 3, GL_FLOAT, 0, sizeof(ParticleVertex), pData );
//    glCheckError( "Quad::Bind Position" );
//    pglEnableVertexAttribArray( VertexBuffer::Positions );
//    glCheckError( "Quad::Enable Position" );
//
//    pglVertexAttribPointer( VertexBuffer::UV0s, 2, GL_FLOAT, 0, sizeof(ParticleVertex), pData + sizeof(float) * 3 );
//    glCheckError( "Quad::Bind UV" );
//    pglEnableVertexAttribArray( VertexBuffer::UV0s );
//    glCheckError( "Quad::Enable UV" );
//    pglVertexAttribPointer( VertexBuffer::Colors, 4, GL_UNSIGNED_BYTE, 1, sizeof(ParticleVertex), pData + sizeof(float) * 5 );
//    glCheckError( "Quad::Bind Color" );
//    pglEnableVertexAttribArray( VertexBuffer::Colors );
//    glCheckError( "Quad::Enable Color" );
//
//    glDrawElements( GL_TRIANGLES, numParticles * 6, GL_UNSIGNED_SHORT, pIndices );
//    glCheckError( "Quad::Draw Triangles" );
//#elif defined DIRECTX9
//    char *pData = (char *) pHead;
//
//    HRESULT hr;
//
//    hr = Dx9::Instance( ).GetDevice( )->SetVertexDeclaration( Dx9::Instance( ).GetParticleQuadDecl( ) );
//    Debug::Assert( Condition(SUCCEEDED(hr)), "SetVertexDeclaration failed 0x%08x", hr );
//
//    hr = Dx9::Instance( ).GetDevice( )->DrawIndexedPrimitiveUP( D3DPT_TRIANGLELIST, 0, numParticles * 4, numParticles * 2, pIndices, D3DFMT_INDEX16, pData, sizeof(ParticleVertex) );
//    Debug::Assert( Condition(SUCCEEDED(hr)), "Failed to DrawIndexPrimitiveUp: 0x08x", hr );
//#else
//    #error Graphics API Undefined
//#endif
}

void Particle::Create(
    const Vector &startPosition,
    const Vector &velocity,
    float lifeTime,
    float rotationRate,
    float fadeInTime,
    float fadeOutTime
)
{
    m_Position = startPosition;
    m_Velocity = velocity;
    m_Lifetime = lifeTime;
    m_Time     = 0.0f;
    m_Position.w   = 0.0f;
    m_RotationRate = rotationRate;
    m_FadeInTime = fadeInTime * m_Lifetime;
    m_FadeOutTime = m_Lifetime - (m_Lifetime * fadeOutTime);
}

bool Particle::Update(
    float deltaSeconds,
    float gravity
)
{
    if (m_Time >= m_Lifetime) return false;

    m_Velocity.y += - gravity * m_Time;

    m_Position += m_Velocity * deltaSeconds;

    m_Position.w += m_RotationRate * deltaSeconds;

    m_Time += deltaSeconds;

    if (m_Time <= m_FadeInTime)
       m_Alpha = m_Time / m_FadeInTime;
    else if (m_Time >= m_FadeOutTime)
       m_Alpha = m_FadeOutTime / m_Time;
    else
       m_Alpha = 1.0f;
    return true;
}
