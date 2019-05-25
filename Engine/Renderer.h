#pragma once

#include "EngineGlobal.h"
#include "List.h"
#include "Resource.h"
#include "Viewport.h"

class Viewport;
class RenderObject;
class RenderDesc;
class IRenderModifier;
class GraphicsMaterialObject;

struct RenderStats;
struct TaskExecution;

typedef void( *RenderFunc )( const RenderDesc &, RenderStats *pStats );

class RenderBuffer
{
private:
    MemoryStream m_Stream;

    int m_Slots[ 16 ];
    int m_SlotsUsed;

public:
    RenderBuffer( )
    {
        m_SlotsUsed = 0;
    }

    ~RenderBuffer( )
    {
        m_Stream.Close( );
    }

    void Reset( void )
    {
        m_Stream.Seek( 0, SeekBegin );
        m_SlotsUsed = 0;
    }

    void Write(
        const void *pData,
        size_t size
    )
    {
        Debug::Assert( Condition( m_SlotsUsed < sizeof( m_Slots ) / sizeof( int ) ), "Too many items have been written to the RenderBuffer" );

        m_Slots[ m_SlotsUsed++ ] = m_Stream.Seek( 0, SeekCurrent );
        m_Stream.Write( pData, (uint32) size );
    }

    void *Write(
        size_t size
    )
    {
        m_Slots[ m_SlotsUsed++ ] = m_Stream.Seek( 0, SeekCurrent );
        return m_Stream.Write( size );
    }

    void *Read(
        int slot
    ) const
    {
        return m_Stream.GetBuffer( ) + m_Slots[ slot ];
    }
};

struct RenderContext
{
   PipelineContext pipelineContext;

   static inline uint32 Hash(
      RenderContext rc
   )
   {
      return HashFunctions::NUIntHash( (nuint) rc.pipelineContext );
   }

   static inline bool Compare(
      RenderContext rc1,
      RenderContext rc2
   )
   {
      return rc1.pipelineContext == rc2.pipelineContext;
   }
};

typedef RenderContext ComputeContext;

class RenderObjectDesc
{
public:
    void Reset( void )
    {
        buffer.Reset( );
        pMaterial = NULL;
        pObject = NULL;
        renderFunc = NULL;
        argList.StartPack( );
    }

    const RenderObject   *pObject;
    ArgList         argList;
    GraphicsMaterialObject *pMaterial;
    RenderFunc      renderFunc;
    RenderBuffer    buffer;
    RenderContext   renderContext;
};

struct LightDesc
{
    LightDesc( )
    {}

    LightDesc( int null )
    {
        color = Math::ZeroVector( );
        position = Math::ZeroVector( );
        direction = Math::ZeroVector( );
        nits = 0;
        inner = 0;
        outer = 0;
        range = 0;
        cast = CastOmni;
    }

    enum Cast
    {
        CastOmni,
        CastDirectional,
        CastSpot,
	
		  CastAmbient,
	 };

    // light this desc references
    // (for visibility and other renderobject checking)
    const class Light *pLight;
    
    Vector color;
    Vector position;
    Vector direction;
    float nits;
    float inner;
    float outer;
    float range;
    Cast cast;

    static const LightDesc Zero;
};

typedef List<RenderObjectDesc*> RenderObjectDescList;
typedef List<LightDesc>  LightDescList;

struct PackedLights
{
   Matrix shadowProjMatrix;
   Matrix shadowViewMatrix;
   Vector light_dir[ 1 ];
   Vector light_color[ 1 ];
   Vector light_pos_type[ 1 ];
   Vector light_atten[ 1 ];
   Vector ambient;
   uint32 num_lights;
};

class RenderDesc
{
public:
    const char *pPass;
    const RenderObjectDesc *pDesc;
    const TaskExecution *pTask;
    GpuDevice::CommandList *pCommandList;
    const Viewport *pViewport;
};

class RendererDesc
{
public:
    void Create( void )
    {
        renderObjectDescs.Create( );
        lightDescs.Create( );
        ambient_light = Math::ZeroVector();
    }

    void Destroy( void )
    {
        renderObjectDescs.Destroy( );
        lightDescs.Destroy( );
    }

    void Clear( void )
    {
        renderObjectDescs.Clear( );
        lightDescs.Clear( );
        ambient_light = Math::ZeroVector();
    }

    RenderObjectDescList renderObjectDescs;
    LightDescList  lightDescs;
    PackedLights packedLights;
    Matrix shadowProjMatrix;
    Matrix shadowViewMatrix;
    Vector ambient_light;
    Viewport viewport;
    const char *pPass;
};

struct RenderStats
{
   uint64 num_faces;
   uint64 num_verts;
   
   static const RenderStats Empty;

   void operator += (const RenderStats &rhs)
   {
      num_faces += rhs.num_faces;
      num_verts += rhs.num_verts;
   }
};

class Renderer : public Identifiable
{
public:
    virtual void GetRenderData(
        const Viewport &viewport
    ) = 0;

    virtual void Render(
        const Viewport &viewport,
        GpuDevice::CommandList *pBatchCommandList
    ) = 0;

    virtual RenderStats GetStats( void ) { return RenderStats::Empty; }
};

typedef List<Renderer*> RendererList;
typedef HashTable<const char *, Renderer *> RendererHash;

class IRenderTarget
{
public:
    virtual ~IRenderTarget( void ) {}

    virtual void Create(
        const Id &id
    ) = 0;

    virtual void Destroy( void ) = 0;

    virtual void Push( void ) = 0;
    virtual void Pop( void ) = 0;
};
