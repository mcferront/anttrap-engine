#include "GamePch.h"
#include "App.h"
#include "RenderNodes.h"
#include "RenderWorld.h"
#include "TouchWorld.h"
#include "ResourceWorld.h"
#include "FileStreams.h"
#include "DataAsset.h"
#include "TextureAsset.h"
#include "FontMapAsset.h"
#include "FrameMapAsset.h"
#include "Node.h"
#include "TcpIpPipe.h"
#include "UtilityClock.h"
#include "MemoryStreams.h"
#include "DefaultRenderer.h"
#include "ComputeNode.h"
#include "MipGenNode.h"
#include "ExecuteIndirect.h"
#include "FrameGrabRenderer.h"
#include "CopyResourceRenderer.h"
#include "ConvertToRenderer.h"
#include "MaterialAsset.h"
#include "RenderContexts.h"
#include "Window.h"
#include "RenderTree.h"
#include "Sprite.h"
#include "InputSystem.h"
#include "DebugGraphics.h"
#include "PhysicsWorld.h"
#include "Raycast.h"
#include "LuaAsset.h"
#include "LuaVM.h"
#include "LuaCoreModule.h"
#include "Asset.h"
#include "RigidBody.h"
#include "RegistryAsset.h"
#include "RegistryWorld.h"
#include "AnimationWorld.h"
#include "AudioWorld.h"
#include "TaskWorld.h"
#include "AnimAsset.h"
#include "WavAsset.h"
#include "MeshAsset.h"
#include "FbxAsset.h"
#include "CollisionComponent.h"
#include "LabelComponent.h"
#include "SpriteComponent.h"
#include "ButtonComponent.h"
#include "MemoryManager.h"
#include "SceneAsset.h"
#include "VideoComponent.h"
#include "Localization.h"
#include "Log.h"
#include "MeshRendererComponent.h"
#include "CameraComponent.h"
#include "RigidBodyComponent.h"
#include "ScriptComponent.h"
#include "ShapeRendererComponent.h"
#include "Animation3dComponent.h"
#include "SearchHierarchy.h"
#include "ParticleWorld.h"
#include "LightComponent.h"
#include "SceneAsset.h"
#include "SplineAsset.h"
#include "SplineComponent.h"
#include "VertexBuffer.h"
#include "GpuProfiler.h"
#include "GpuBuffer.h"

//void PrintHierarchy( int nested, Node *pNode )
//{
//    for ( int i = 0; i < nested; i++ )
//        Debug::Print( Debug::TypeInfo, ">" );
//
//    Debug::Print( Debug::TypeInfo, "%s\n", pNode->GetName() );
//
//    NodeList list;
//    pNode->GetChildren( &list );
//
//    for ( int i = 0; i < list.GetSize(); i++ )
//        PrintHierarchy( nested + 1, list.Get(i) );
//}
//

const float FarClip = 10000.0f;

bool g_typed_uav_load_support;
float g_resolution_scale;
const Color ClearColor( 0, 0, 0, 0 );
float DeltaSeconds;

int g_frameIndex;
int g_scene_buffer_width;
int g_scene_buffer_height;

const int g_shadow_width = 2048;
const int g_shadow_height = 2048;

enum RenderOrder
{
    RenderOrder_ShadowMap,
    RenderOrder_DepthPrepass,
    RenderOrder_3D,
    RenderOrder_Bokeh,
    RenderOrder_CTB,
    RenderOrder_UpRes,
    RenderOrder_UI
};

void App_DeleteRegKey(
    const char *pArgs[ ],
    int numArgs
)
{
    if ( numArgs > 1 )
        RegistryWorld::Instance( ).RemoveEntries( pArgs[ 1 ] );
}

void App_Quit(
    const char *pArgs[ ],
    int numArgs
)
{
#ifdef WIN32
    PostQuitMessage( NULL );
#endif
}

void App_SaveRegKey(
    const char *pArgs[ ],
    int numArgs
)
{
    if ( numArgs > 1 )
    {
        char path[ 256 ];
        String::Format( path, sizeof( path ), "%s.registry", pArgs[ 1 ] );
        ExportRegistry( pArgs[ 1 ], path );
    }
}

bool g_RebuildRenderer = false;

void RebuildRenderer( const char * )
{
    g_RebuildRenderer = true;
}

void ShowHemisphere( void );

Matrix g_prevInvVPMatrix = Math::IdentityMatrix( );

DefaultRenderer *g_pOpaqueRenderer;
DefaultRenderer *g_pTransparentRenderer;

void TonemapProc(
    ComputeMaterialObject *pCompute,
    RenderContext renderContext,
    const void *pUserData
)
{
    ComputeMaterialObject::Pass *pPass = pCompute->GetPass( renderContext );

    static const char *pTechnique = StringRef( "$TONEMAP" );

    static RegistryInt method( "Tonemap/method" );
    static RegistryFloat exposure( "Tonemap/target_exposure", .1f );
    static RegistryFloat filter( "Tonemap/filter", .05f );

    Vector params( (float) method.GetValue( ), exposure.GetValue( ), filter.GetValue( ), 0.0f );
    pPass->GetData( )->SetMacro( pTechnique, &params, 1 );

    {
        static RegistryInt hdr_width( "Render/hdr_width" );
        static RegistryInt hdr_height( "Render/hdr_height" );

        static const char *pDimensions = StringRef( "$DIMENSIONS" );

        Vector params( (float) hdr_width.GetValue( ), (float) hdr_height.GetValue( ) );
        pPass->GetData( )->SetMacro( pDimensions, &params, 1 );
    }
}

void HdrToLdrProc(
    ComputeMaterialObject *pCompute,
    RenderContext renderContext,
    const void *pUserData
)
{
    ComputeMaterialObject::Pass *pPass = pCompute->GetPass( renderContext );

    static const char *pParams = StringRef( "$PARAMS" );
    static RegistryFloat tonemap_a( "Tonemap/a", 1.25f );
    static RegistryFloat tonemap_b( "Tonemap/b", 1.5f );

    Vector params( tonemap_a.GetValue( ), tonemap_b.GetValue( ), 0, 0 );
    pPass->GetData( )->SetMacro( pParams, &params, 1 );
}

void GBufferResolveProc(
    ComputeMaterialObject *pCompute,
    RenderContext renderContext,
    const void *pUserData
)
{
    ComputeMaterialObject::Pass *pPass = pCompute->GetPass( renderContext );

    static const char *pMainCam = StringRef( "MainCamera" );
    ResourceHandle hCamera = ResourceHandle::FromAlias( pMainCam );
    Node *pNode = GetResource( hCamera, Node );

    Camera *pCamera = pNode->GetComponent<CameraComponent>( )->GetCamera( );

    {
        static const char *pParams = StringRef( "$PARAMS" );

        bool is_even_frame = ( g_frameIndex & 0x1 ) == 0;

        // bit fields, tolerance, res scale
        uint32 bit_fields = 0;
        if ( is_even_frame ) bit_fields |= 0x01;

        Vector params( *(float *) (uint32 *) &bit_fields, 0, 0, 0 );
        pPass->GetData( )->SetMacro( pParams, &params, 1 );
    }

    {
        static const char *pWorldCameraView = StringRef( "$WORLD_CAMERA_VIEW" );
        static const char *pLightsDir = StringRef( "$LIGHT_DIR" );
        static const char *pLightsColor = StringRef( "$LIGHT_COLOR" );
        static const char *pLightsPosType = StringRef( "$LIGHT_POS_TYPE" );
        static const char *pLightsAtten = StringRef( "$LIGHT_ATTEN" );
        static const char *pLightAmbient = StringRef( "$LIGHT_AMBIENT" );
        static const char *pShadowView = StringRef( "$SHADOW_VIEW_MATRIX" );
        static const char *pShadowProj = StringRef( "$SHADOW_PROJECTION_MATRIX" );

        Vector worldCameraView = pCamera->GetWorldTransform( )->GetLook( );
        pPass->GetData( )->SetMacro( pWorldCameraView, &worldCameraView, 1 );

        const PackedLights *pPackedLights = &g_pOpaqueRenderer->GetRendererDesc( )->packedLights;

        pPass->GetData( )->SetMacro( pLightsDir, pPackedLights->light_dir, sizeof( pPackedLights->light_dir ) / sizeof( pPackedLights->light_dir[ 0 ] ) );
        pPass->GetData( )->SetMacro( pLightsColor, pPackedLights->light_color, sizeof( pPackedLights->light_color ) / sizeof( pPackedLights->light_color[ 0 ] ) );
        pPass->GetData( )->SetMacro( pLightsAtten, pPackedLights->light_atten, sizeof( pPackedLights->light_atten ) / sizeof( pPackedLights->light_atten[ 0 ] ) );
        pPass->GetData( )->SetMacro( pLightsPosType, pPackedLights->light_pos_type, sizeof( pPackedLights->light_pos_type ) / sizeof( pPackedLights->light_pos_type[ 0 ] ) );

        pPass->GetData( )->SetMacro( pLightAmbient, &pPackedLights->ambient, 1 );
        pPass->GetData( )->SetMacro( pShadowView, &pPackedLights->shadowViewMatrix, 1 );
        pPass->GetData( )->SetMacro( pShadowProj, &pPackedLights->shadowProjMatrix, 1 );
    }
    static const char *pCurrInvVPMatrix = StringRef( "$INV_VIEW_PROJ" );
    {
        Matrix vpMatrix, invVpMatrix;
        Vector2 jitter;
        pCamera->GetJitter( &jitter );

        pCamera->SetJitter( Math::ZeroVector2( ) );
        {
            Transform view;
            pCamera->GetViewTransform( &view );
            pCamera->GetReverseDepthProjection( &vpMatrix );
            Math::Multiply( &vpMatrix, view.ToMatrix( true ), vpMatrix );
            Math::Invert( &invVpMatrix, vpMatrix );

            pPass->GetData( )->SetMacro( pCurrInvVPMatrix, &invVpMatrix, 1 );

        }pCamera->SetJitter( jitter );

        if ( false )
        {
            const float width = 512;
            const float height = 512;

            Vector orig_ws( -100, 100, 500, 1 );

            Vector orig_projected_pre_w;
            Vector orig_projected_post_w;

            Math::Transform4( &orig_projected_pre_w, orig_ws, vpMatrix );
            Math::ScaleVector( &orig_projected_post_w, orig_projected_pre_w, 1.0f / orig_projected_pre_w.w );

            int orig_screen_x = (int) ( orig_projected_post_w.x * width / 2 + width / 2 );
            int orig_screen_y = (int) ( orig_projected_post_w.y * height / 2 + height / 2 );
            float orig_depth = orig_projected_post_w.z;

            int pixel_x = orig_screen_x;
            int pixel_y = orig_screen_y;

            float screen_reprojected_x = pixel_x / width * 2 - 1;
            float screen_reprojected_y = pixel_y / height * 2 - 1;

            Vector re_projected_pre_w = Vector( screen_reprojected_x, screen_reprojected_y, orig_depth, 1.0f );

            Vector prev_ws;
            Math::Transform4( &prev_ws, re_projected_pre_w, invVpMatrix );

            Math::ScaleVector( &prev_ws, prev_ws, 1.0f / prev_ws.w );

            Vector prev_ws_to_current_projection;
            Math::Transform4( &prev_ws_to_current_projection, prev_ws, vpMatrix );
            prev_ws_to_current_projection.x = prev_ws_to_current_projection.x / prev_ws_to_current_projection.w;
            prev_ws_to_current_projection.y = prev_ws_to_current_projection.y / prev_ws_to_current_projection.w;

            int prev_ws_to_screen_space_x, prev_ws_to_screen_space_y;
            prev_ws_to_screen_space_x = (int) round( prev_ws_to_current_projection.x * width / 2 + width / 2 );
            prev_ws_to_screen_space_y = (int) round( prev_ws_to_current_projection.y * height / 2 + height / 2 );

            int zz = 0;
        }
    }
}

void BuildConfigPath(
    char *pOut,
    const char *pFilename,
    uint32 length,
    bool createDirectories
)
{
    char filename[ 260 ];
    String::Copy( filename, pFilename, sizeof( filename ) );

    String::Replace( filename, '\\', '/' );
    String::Replace( filename, '?', '/' );

    String::Replace( filename, ':', '^' );
    String::Replace( filename, '*', '^' );
    String::Replace( filename, '\"', '^' );
    String::Replace( filename, '<', '^' );
    String::Replace( filename, '>', '^' );
    String::Replace( filename, '|', '^' );

    if ( *App::Instance( ).GetConfigPath( ) )
    {
        if ( *filename )
            String::Format( pOut, length, "%s/%s", App::Instance( ).GetConfigPath( ), filename );
        else
            String::Copy( pOut, App::Instance( ).GetConfigPath( ), length );
    }
    else
        String::Copy( pOut, filename, length );

    if ( true == createDirectories )
    {
        //Make sure the directory structure up to our file exists
        String::Copy( filename, pOut, sizeof( filename ) );

        char *pSlash = strrchr( filename, '/' );

        if ( pSlash )
        {
            *pSlash = NULL;
            FileStream::CreateDirectory( filename );
        }
    }
}

void BuildPlatformConfigPath(
    char *pOut,
    const char *pFilename,
    uint32 length,
    bool createDirectories
)
{
    char filename[ 260 ];
    String::Copy( filename, pFilename, sizeof( filename ) );

    String::Replace( filename, '\\', '/' );
    String::Replace( filename, '?', '/' );

    String::Replace( filename, ':', '^' );
    String::Replace( filename, '*', '^' );
    String::Replace( filename, '\"', '^' );
    String::Replace( filename, '<', '^' );
    String::Replace( filename, '>', '^' );
    String::Replace( filename, '|', '^' );

    char *pDot = strrchr( filename, '.' );
    if ( NULL != pDot )
    {
        char name[ 260 ], ext[ 16 ];
        String::Copy( ext, pDot + 1, sizeof( ext ) );

        *pDot = NULL;

        String::Copy( name, filename, sizeof( name ) );
        String::Format( filename, sizeof( filename ), "%s%s%s", name, "." PlatformType ".", ext );
    }
    else
        strncat( filename, "." PlatformType ".__", sizeof( filename ) - strlen( filename ) - 1 );

    if ( *App::Instance( ).GetDataPath( ) )
    {
        if ( *filename )
            String::Format( pOut, length, "%s/%s", App::Instance( ).GetConfigPath( ), filename );
        else
            String::Copy( pOut, App::Instance( ).GetConfigPath( ), length );
    }
    else
        String::Copy( pOut, filename, length );


    if ( true == createDirectories )
    {
        //Make sure the directory structure up to our file exists
        String::Copy( filename, pOut, sizeof( filename ) );

        char *pSlash = strrchr( filename, '/' );

        if ( pSlash )
        {
            *pSlash = NULL;
            FileStream::CreateDirectory( filename );
        }
    }
}

void BuildDataPath(
    char *pOut,
    const char *pFilename,
    uint32 length,
    bool createDirectories
)
{
    char filename[ 260 ];
    String::Copy( filename, pFilename, sizeof( filename ) );

    String::Replace( filename, '\\', '/' );
    String::Replace( filename, '?', '/' );

    String::Replace( filename, ':', '^' );
    String::Replace( filename, '*', '^' );
    String::Replace( filename, '\"', '^' );
    String::Replace( filename, '<', '^' );
    String::Replace( filename, '>', '^' );
    String::Replace( filename, '|', '^' );

    if ( *App::Instance( ).GetDataPath( ) )
    {
        if ( *filename )
            String::Format( pOut, length, "%s/%s", App::Instance( ).GetDataPath( ), filename );
        else
            String::Copy( pOut, App::Instance( ).GetDataPath( ), length );
    }
    else
        String::Copy( pOut, filename, length );


    if ( true == createDirectories )
    {
        //Make sure the directory structure up to our file exists
        String::Copy( filename, pOut, sizeof( filename ) );

        char *pSlash = strrchr( filename, '/' );

        if ( pSlash )
        {
            *pSlash = NULL;
            FileStream::CreateDirectory( filename );
        }
    }
}

void BuildPlatformDataPath(
    char *pOut,
    const char *pFilename,
    uint32 length,
    bool createDirectories
)
{
    char filename[ 260 ];
    String::Copy( filename, pFilename, sizeof( filename ) );

    String::Replace( filename, '\\', '/' );
    String::Replace( filename, '?', '/' );

    String::Replace( filename, ':', '^' );
    String::Replace( filename, '*', '^' );
    String::Replace( filename, '\"', '^' );
    String::Replace( filename, '<', '^' );
    String::Replace( filename, '>', '^' );
    String::Replace( filename, '|', '^' );

    char *pDot = strrchr( filename, '.' );
    if ( NULL != pDot )
    {
        char name[ 260 ], ext[ 16 ];
        String::Copy( ext, pDot + 1, sizeof( ext ) );

        *pDot = NULL;

        String::Copy( name, filename, sizeof( name ) );
        String::Format( filename, sizeof( filename ), "%s%s%s", name, "." PlatformType ".", ext );
    }
    else
        strncat( filename, "." PlatformType ".__", sizeof( filename ) - strlen( filename ) - 1 );

    if ( *App::Instance( ).GetDataPath( ) )
    {
        if ( *filename )
            String::Format( pOut, length, "%s/%s", App::Instance( ).GetDataPath( ), filename );
        else
            String::Copy( pOut, App::Instance( ).GetDataPath( ), length );
    }
    else
        String::Copy( pOut, filename, length );

    if ( true == createDirectories )
    {
        //Make sure the directory structure up to our file exists
        String::Copy( filename, pOut, sizeof( filename ) );

        char *pSlash = strrchr( filename, '/' );

        if ( pSlash )
        {
            *pSlash = NULL;
            FileStream::CreateDirectory( filename );
        }
    }
}

void ExportRegistry(
    const char *pKey,
    const char *pFilename
)
{
    Registry registry;
    RegistryWorld::Instance( ).ExportRegistry( pKey, &registry );

    char path[ 260 ];
    String::Format( path, sizeof( path ), "%s/%s", App::Instance( ).GetConfigPath( ), pFilename );

    FileOutputStream stream;
    stream.Open( path );

    Serializer serializer( &stream );
    serializer.Serialize( &registry );

    registry.Destroy( );

    stream.Close( );
}

bool ImportRegistry(
    const char *pFilename
)
{
    Registry registry;
    FileInputStream stream;

    char path[ 260 ];
    String::Format( path, sizeof( path ), "%s/%s", App::Instance( ).GetConfigPath( ), pFilename );

    bool existed = false;

    if ( stream.Open( path ) )
    {
        Serializer serializer( &stream );
        serializer.Deserialize( &registry );

        RegistryWorld::Instance( ).ImportRegistry( &registry );

        registry.Destroy( );

        existed = true;
    }

    stream.Close( );

    return existed;
}

App &App::Instance( void )
{
    static App s_instance;
    return s_instance;
}

App::App( void )
{
}

App::~App( void )
{
}

void App::Create(
    const Window &window
)
{
    m_WindowId = window.GetId( );

    Thread::InitMainThread( );

    Debug::Create( );

    m_FrameReady = true;

    ChannelSystem::Instance( ).Create( );
    ResourceWorld::Instance( ).Create( );

#ifdef DIRECTX9
    Dx9::Instance( ).CreateChannel( );
#elif defined OPENGL
    Gl::Instance( ).CreateChannel( );
#endif

    TypeRegistry::Create( );
    Resource::Register( );
    SystemResource::Register( );
    Id::Registrar::Create( );

    MemoryManager::Instance( ).CreateHeap( "Lua", 0, 1024 );

    Math::Initialize( );

#ifdef _DISTRIBUTION
    Debug::SetAllowFlags( Debug::TypeNone );
#elif defined _RELEASE
    Debug::SetAllowFlags( Debug::TypeWarning | Debug::TypeError | Debug::TypeScript );
#else
    Debug::SetAllowFlags( Debug::TypeAll );
#endif

    RenderWorld::Instance( ).Create( );
    AnimationWorld::Instance( ).Create( );
    TouchWorld::Instance( ).Create( );
    SerializerWorld::Instance( ).Create( );
    RegistryWorld::Instance( ).Create( );
    InputSystem::Instance( ).Create( );
    PhysicsWorld::Instance( ).Create( Vector( 0, -9.8f, 0 ), 1 / 60.0f );
    LuaVM::Instance( ).Create( );
    AudioWorld::Instance( ).Create( );
    Localization::Instance( ).Create( );
    GpuProfiler::Instance( ).Create( );

#ifdef WIN32
    int numThreads;

#ifdef _DEBUG
    numThreads = 2;
#else
    SYSTEM_INFO info;
    {
        GetSystemInfo( &info );
        numThreads = info.dwNumberOfProcessors - 1;
    }
#endif
#endif

    TaskWorld::Instance( ).Create( numThreads );

    TaskWorld::Instance( ).Run( );

    Database::BeginInstancing( );

    //for future implementations we would like these in a packfile 
    const char *pPackFile = NULL;
    VideoManager::Instance( ).Create( pPackFile );

    ArgList::Register( );
    Asset::Register( );
    //Game::Register( );
    Material::Register( );
    GpuResource::Register( );
    GpuBuffer::Register( );
    Texture::Register( );
    Data::Register( );
    FontMap::Register( );
    FrameMap::Register( );
    Registry::Register( );
    Node::Register( );
    Lua::Register( );
    Wav::Register( );
    Anim::Register( );
    Mesh::Register( );
    Fbx::Register( );
    Shader::Register( );
    Spline::Register( );
    Scene::Register( );

    RenderContexts::CreateContexts( );

    MeshRendererComponent::Register( );
    LabelComponent::Register( );
    ButtonComponent::Register( );
    SpriteComponent::Register( );
    CollisionComponent::Register( );
    CameraComponent::Register( );
    ScriptComponent::Register( );
    ShapeRendererComponent::Register( );
    AnimationComponent::Register( );
    DirectionalLightComponent::Register( );
    SpotLightComponent::Register( );
    PointLightComponent::Register( );
    AmbientLightComponent::Register( );
    SplineComponent::Register( );
    RigidBodyComponent::Register( );
    Component::s_ActiveComponents.Create( 1024, 128, IdHash, IdCompare );
    Node::s_ActiveNodes.Create( 1024, 128, IdHash, IdCompare );
    Node::s_AllNodes.Create( 1024, 128, IdHash, IdCompare );

    GpuDevice::Instance( ).AppReady( );

    g_typed_uav_load_support = GpuDevice::Instance( ).CheckSupport( GpuDevice::Support::Feature::UavTypedLoad );

    RenderWorld::Instance( ).AddRenderGroup( Id( "Default" ) );

    DebugGraphics::Instance( ).Create( );

    RenderWorld::Instance( ).AddWindow( window );

    //we use <br> for newlines in our text field
    //<> also deliminates registry keys
    //so technically the printing will look up br
    //in the registry and get a newline from that
    RegistryString newline( StringRef( "br" ) );
    newline.SetValue( StringRef( "\n" ) );

    TypeRegistry_RegisterTypeInfo( ResourceHandle );
    TypeRegistry_RegisterTypeInfo( Id );

    RegistryMethod quit( "quit", App_Quit );
    RegistryMethod exit( "exit", App_Quit );
    RegistryMethod deleteRegKey( "App/delete_reg_key", App_DeleteRegKey );

    RegistryInt app_interval( "App/interval", 1, 0, 3 );
    RegistryMethod value( "App/reg_key_save", App_SaveRegKey );

    RegistryBool deferred( "Render/deferred", false, RebuildRenderer );
    RegistryBool depth_prepass( "Render/depth_prepass", true, RebuildRenderer );
    RegistryInt method( "Tonemap/method", 0 );

    RegistryInt hdr_width( "Render/hdr_width", 0, -INT_MAX, INT_MAX, RebuildRenderer );
    RegistryInt hdr_height( "Render/hdr_height", 0, -INT_MAX, INT_MAX, RebuildRenderer );

    RegistryBool dof_enable = RegistryBool( "dof.enable", true, RebuildRenderer );
    RegistryBool spec_bloom_enable = RegistryBool( "SpecBloom/enable", true, RebuildRenderer );
    RegistryBool ssao_enable = RegistryBool( "SSAO/enable", true, RebuildRenderer );
    RegistryBool ssr_enable = RegistryBool( "SSR/enable", true, RebuildRenderer );

    RegistryInt ssr_iterations = RegistryInt( "SSR/iterations", 128 );

    LOG( "Creating Renderers" );
    {
        SetupRenderers( );
    }

    TouchWorld::Instance( ).AddTouchStage( Id( "Debug" ), 10.0f );
    TouchWorld::Instance( ).AddTouchStage( Id( "UI" ), 1.0f );

    LOG( "Loading Core File" );


    Database *pDatabase = Database::Create( true );

    {
        FileInputStream inputStream;

        char path[ 256 ];
        BuildPlatformDataPath( path, "System.resources", sizeof( path ), false );

        pDatabase->RequestAll( );

        if ( true == inputStream.Open( path ) )
            pDatabase->Deserialize( &inputStream );

        inputStream.Close( );
    }

    int w = GpuDevice::Instance( ).GetSwapChain( )->width;
    int h = GpuDevice::Instance( ).GetSwapChain( )->height;

    ResourceHandle default_font_map = ResourceHandle::FromAlias( StringRef( "DefaultFont.fontmap" ) );
    ResourceHandle default_font_texture = ResourceHandle::FromAlias( StringRef( "DefaultFont.bmp" ) );
    ResourceHandle default_font_material = ResourceHandle::FromAlias( StringRef( "DefaultFont.material" ) );
    ResourceHandle grey_material = ResourceHandle::FromAlias( StringRef( "Grey.material" ) );

    if ( default_font_map != NullHandle && grey_material != NullHandle &&
        default_font_texture != NullHandle && default_font_material != NullHandle )
    {
        Debug::EnableVisualAssert( grey_material, default_font_material,
            default_font_map, default_font_texture, (float) w, (float) h );

        Debug::EnableVisualLog( grey_material, default_font_material, default_font_map,
            default_font_texture, 0, -h / 2.0f + h / 16.0f, (float) w, h / 8.0f );
    }

    DebugGraphics::Instance( ).AddToScene( );

    m_Clock.Start( );

    //m_EditorConnection.Create( );

    ResourceHandle api = ResourceHandle::FromAlias( StringRef( "API.lua" ) );
    ResourceHandle start_up = ResourceHandle::FromAlias( StringRef( "Startup.lua" ) );

    m_LuaApiExists = IsResourceLoaded( api );

    if ( true == m_LuaApiExists )
        LuaVM::Instance( ).ExecuteLuaAsset( api );

    if ( IsResourceLoaded( start_up ) )
    {
        LuaVM::Instance( ).ExecuteLuaAsset( start_up );
        LuaVM::Instance( ).ExecuteFunction( "Startup", "" );
    }

    {
        ResourceHandle hStats = ResourceHandle( Id::Create( ) );

        Node *pNode = new Node;
        pNode->Create( );

        IdList uiList; uiList.Create( );
        uiList.Add( "UI" );

        LabelComponent *pLabel = new LabelComponent;
        pLabel->Create( Vector2( w / 4.0f, (float) h ), Math::OneVector( ), NullHandle, default_font_material, default_font_map, default_font_texture, uiList );
        pLabel->SetAlign( TextArea::AlignLeft, TextArea::AlignTop );

        pNode->SetWorldPosition( Vector( w / 2.0f - w / 8.0f, -56 ) );
        pNode->AddComponent( pLabel );
        hStats.Bind( NULL, pNode );

        pNode->AddToScene( );
        pNode->Bind( );

        m_pStats = pLabel;

        uiList.Destroy( );
    }
}

void App::Destroy( void )
{
    RenderWorld::Instance( ).FinishFrame( );
    TaskWorld::Instance( ).Destroy( );

    m_hMainCamera = NullHandle;

    TeardownRenderers( );

    GpuDevice::Instance( ).AppShutdown( );

    {
        ResourceHandle start_up = ResourceHandle::FromAlias( StringRef( "Startup.lua" ) );

        if ( IsResourceLoaded( start_up ) )
            LuaVM::Instance( ).ExecuteFunction( "Shutdown", "" );
    }

    {
        ResourceHandle hCamera = ResourceHandle::FromAlias( StringRef( "MainCamera" ) );

        if ( IsResourceLoaded( hCamera ) )
        {
            Node *pNode = GetResource( hCamera, Node );
            pNode->RemoveFromScene( );
            pNode->Destroy( );
            delete pNode;
        }
    }

    AudioWorld::Instance( ).StopAllSounds( );

    //m_EditorConnection.Destroy( );

    Debug::DisableVisualAssert( );
    Debug::DisableVisualLog( );

    RenderWorld::Instance( ).RemoveWindow( m_WindowId );

    SerializerWorld::Instance( ).DeleteSerializers( );

    TypeRegistry::Destroy( );

    Localization::Instance( ).Destroy( );
    VideoManager::Instance( ).Destroy( );
    AudioWorld::Instance( ).Destroy( );
    GpuProfiler::Instance( ).Destroy( );

#ifdef DIRECTX9
    Dx9::Instance( ).DestroyChannel( );
#elif defined OPENGL
    Gl::Instance( ).DestroyChannel( );
#endif

    DebugGraphics::Instance( ).RemoveFromScene( );

    Database::UnloadDatabases( false );

    {
        ResourceHandleList list;
        list.Create( );

        {
            ResourceType type = ResourceWorld::Instance( ).GetResourceType( StringRef( "Asset" ) );
            ResourceWorld::Instance( ).GetHandles( &list, type );
        }

        {
            ResourceType type = ResourceWorld::Instance( ).GetResourceType( StringRef( "Node" ) );
            ResourceWorld::Instance( ).GetHandles( &list, type );
        }

        int i, size = list.GetSize( );

        for ( i = 0; i < size; i++ )
        {
            if ( true == IsResourceLoaded( list.Get( i ) ) )
            {
                Resource *pResource = GetResource( list.Get( i ), Resource );

                pResource->RemoveFromScene( );
                pResource->Destroy( );

                delete pResource;
            }
        }
        list.Destroy( );
    }

    Component::s_ActiveComponents.Destroy( );

#ifdef DIRECTX9
    Material::s_MaterialHash.Destroy( );
#error Graphics API Undefined
#endif
    Node::s_ActiveNodes.Destroy( );
    Node::s_AllNodes.Destroy( );

    RenderContexts::DestroyContexts( );

    AnimationWorld::Instance( ).Destroy( );
    LuaVM::Instance( ).Destroy( );
    PhysicsWorld::Instance( ).Destroy( );
    DebugGraphics::Instance( ).Destroy( );
    InputSystem::Instance( ).Destroy( );
    SerializerWorld::Instance( ).Destroy( );
    TouchWorld::Instance( ).Destroy( );
    RenderWorld::Instance( ).Destroy( );
    RegistryWorld::Instance( ).Destroy( );
    ResourceWorld::Instance( ).Destroy( );
    ChannelSystem::Instance( ).Destroy( );

    Database::EndInstancing( );

    Id::Registrar::Destroy( );
    Debug::Destroy( );
    StringPool::Instance( ).Destroy( );

    MemoryManager::Instance( ).Destroy( );
}

void App::Update( void )
{
    static float mark;

    float deltaSeconds;
    float leftover = 0;

    static float screen_shot_pending = false;

    //m_EditorConnection.Update( );
    deltaSeconds = ( m_Clock.TestSample( ) - mark );

    static RegistryInt app_interval( "App/interval", 2, 0, 3 );
    int interval = app_interval.GetValue( );

    //count up to our delta seconds
    if ( interval > 0 )
    {
        static int s_prev_interval;

        float fps = 1.0f / 60.0f * interval;

        if ( interval != s_prev_interval )
        {
            GpuDevice::Instance( ).SetPresentInterval( interval );
            s_prev_interval = interval;
        }

        while ( ( deltaSeconds = ( m_Clock.TestSample( ) - mark ) ) < fps )
            Thread::YieldThread( );
    }
    else
    {
        GpuDevice::Instance( ).SetPresentInterval( 0 );
    }

    DeltaSeconds = deltaSeconds;

    mark = m_Clock.TestSample( );

    static float s_setup;
    static float s_updates;
    static float s_gc;
    static float s_get_render_data;
    static float s_render_submit;
    static float s_render_wait;

    static float average;
    static float last_average;
    static int average_count;

    // reset average every N frames
    if ( average_count == 60 )
    {
        average_count = 1;
        average = last_average;
    }

    average_count++;
    average += deltaSeconds;

    last_average = average / average_count;

    {
        static RegistryInt showStats( "Debug/show_stats", 0x7, 0x0, 0xff );

        uint32 renderablesCulled = 0, totalRenderables = 0;
        uint32 lightsCulled = 0, totalLights = 0;

        if ( NULL != m_pCullOpaque )
        {
            renderablesCulled = m_pCullOpaque->RenderablesCulled( );
            totalRenderables = m_pCullOpaque->TotalRenderables( );
            lightsCulled = m_pCullOpaque->LightsCulled( );
            totalLights = m_pCullOpaque->TotalLights( );
        }

        if ( NULL != m_pCullTransparent )
        {
            renderablesCulled += m_pCullTransparent->RenderablesCulled( );
            totalRenderables += m_pCullTransparent->TotalRenderables( );
            lightsCulled = Math::Max( (int) m_pCullTransparent->LightsCulled( ), (int) lightsCulled );
            totalLights = Math::Max( (int) m_pCullTransparent->TotalLights( ), (int) totalLights );
        }

        if ( showStats.GetValue( ) == 0 )
        {
            m_pStats->SetActive( false );
        }
        else
        {
            static RegistryInt hdr_width( "Render/hdr_width" );
            static RegistryInt hdr_height( "Render/hdr_height" );
            static RegistryInt window_width( "Window/width" );
            static RegistryInt window_height( "Window/height" );
            static RegistryBool deferred( "Render/deferred" );

            RenderStats stats[ 2 ];

            stats[ 0 ] = g_pOpaqueRenderer ? g_pOpaqueRenderer->GetStats( ) : RenderStats::Empty;
            stats[ 1 ] = g_pTransparentRenderer ? g_pTransparentRenderer->GetStats( ) : RenderStats::Empty;

            m_pStats->SetActive( true );

            m_pStats->Clear( );

            if ( showStats.GetValue( ) & 0x01 )
            {
                m_pStats->PrintArgs( "Avg Fps: %.2f\n", 1.0f / last_average );
                m_pStats->PrintArgs( "Avg Frametime: %.2f\n\n", last_average * 1000 );
            }

            if ( showStats.GetValue( ) & 0x02 )
            {
                m_pStats->PrintArgs( "Render: %s\n", deferred.GetValue( ) ? "Deferred" : "Forward" );
                m_pStats->PrintArgs( "Scene: %dx%d (%d%%)\n", g_scene_buffer_width, g_scene_buffer_height, (int) ( g_resolution_scale * 100.0f ) );
                m_pStats->PrintArgs( "Present: %dx%d\n\n", window_width.GetValue( ), window_height.GetValue( ) );
            }

            if ( showStats.GetValue( ) & 0x04 )
            {
                m_pStats->PrintArgs( "Pixel Count: %d\n", g_scene_buffer_width * g_scene_buffer_height );
                m_pStats->PrintArgs( "Polys Submitted: %d\n", stats[ 0 ].num_faces + stats[ 1 ].num_faces );
                m_pStats->PrintArgs( "Verts Submitted: %d\n", stats[ 0 ].num_verts + stats[ 1 ].num_verts );
                m_pStats->PrintArgs( "Lights Culled: %d of %d\n", lightsCulled, totalLights );
                m_pStats->PrintArgs( "Items Culled: %d of %d\n\n", renderablesCulled, totalRenderables );
            }

            if ( showStats.GetValue( ) & 0x08 )
            {
                m_pStats->PrintArgs( "frametime: %.4fms\n", deltaSeconds * 1000 );
                m_pStats->PrintArgs( "fps: %.2f\n", 1.0f / deltaSeconds );
                m_pStats->PrintArgs( "setup: %.2fms\n", s_setup * 1000 );
                m_pStats->PrintArgs( "updates: %.2fms\n", s_updates * 1000 );
                m_pStats->PrintArgs( "gc: %.2fms\n", s_gc * 1000 );
                m_pStats->PrintArgs( "get_render_data: %.2fms\n", s_get_render_data * 1000 );
                m_pStats->PrintArgs( "render_submit: %.2fms\n", s_render_submit * 1000 );
                m_pStats->PrintArgs( "render_wait: %.2fms\n\n", s_render_wait * 1000 );
            }

            if ( showStats.GetValue( ) & 0x10 )
            {
                for ( uint32 i = 0; i < m_GpuTimers.GetSize( ); i++ )
                {
                    if ( m_GpuTimers.GetAt(i) )
                        m_pStats->PrintArgs( "%s: %.4fms\n", m_GpuTimers.GetAt( i )->GetName( ), m_GpuTimers.GetAt( i )->GetDuration( ) );
                    else
                        m_pStats->PrintArgs( "\n" );
                }
            }
        }
    }

    Debug::Update( );

    if ( DeltaSeconds > ( 1.0f / 30.0f ) )
        DeltaSeconds = 1.0f / 30.0f;

    deltaSeconds = DeltaSeconds;

    static RegistryFloat ds_update( "App/deltaSeconds", deltaSeconds );
    ds_update.SetValue( deltaSeconds );

    /*
   Ticking Order
   Flush
   Tick
   Flush
   AnimUpdate, AudioUpdate  //each can be threaded
   Flush
   PostTick (final changes after anim before physics)//enforce dependency order
   Flush
   PhysicsTick    //each can be threaded
   Flush
   Final (fix up any results from physics)   //enforce order
   Flush

   //wait for previous render
   GetRenderData
   Render            //start thread
   */
   //m_Game.LoadScenes( );

    {
        static RegistryBool deferred( "Render/deferred" );

        s_setup = m_Clock.TestSample( );
        {
            static RegistryBool visualLog( "Debug/show_log", false );

            if ( visualLog.GetValue( ) )
                Debug::ShowVisualLog( );
            else
                Debug::HideVisualLog( );

            if ( g_RebuildRenderer )
            {
                TeardownRenderers( );
                SetupRenderers( );
            }

            s_setup = m_Clock.TestSample( ) - s_setup;
        }

        s_updates = m_Clock.TestSample( );
        {
            CameraComponent *pMainCamera = GetResource( m_hMainCamera, Node )->GetComponent<CameraComponent>( );

            static RegistryBool show_hemisphere( "hemisphere.enable", false );

            if ( show_hemisphere.GetValue() )
                ShowHemisphere( );

            static RegistryFloat offset_x( "FullRes/offset_x", 0 );
            static RegistryFloat offset_y( "FullRes/offset_y", 0 );

            Vector2 offset = Vector2(
                offset_x.GetValue( ) / g_scene_buffer_width * 2.0f,
                offset_y.GetValue( ) / g_scene_buffer_height * 2.0f );

            Vector rect;
            rect.x = 0;
            rect.y = 0;
            rect.z = 1.0f;
            rect.w = 1.0f;

            pMainCamera->GetCamera( )->SetViewportRect( rect );
            pMainCamera->GetCamera( )->SetJitter( offset );

            ResourceWorld::Instance( ).RunMaintenance( );
            ResourceWorld::Instance( ).BuildTickList( );

            ChannelSystem::Instance( ).Flush( );
            ResourceWorld::Instance( ).Tick( deltaSeconds );

            if ( true == m_LuaApiExists )
                LuaVM::Instance( ).ExecuteFunction( "ExecuteCoroutines", "f", deltaSeconds );

            ChannelSystem::Instance( ).Flush( );

            static RegistryBool animation( StringRef( "Animation/enable" ), false );

            if ( animation.GetValue( ) )
                AnimationWorld::Instance( ).Tick( deltaSeconds );

            AudioWorld::Instance( ).Tick( deltaSeconds );

            ChannelSystem::Instance( ).Flush( );
            ResourceWorld::Instance( ).PostTick( );

            ChannelSystem::Instance( ).Flush( );
            PhysicsWorld::Instance( ).Tick( deltaSeconds );

            ChannelSystem::Instance( ).Flush( );
            ResourceWorld::Instance( ).Final( );

            ChannelSystem::Instance( ).Flush( );
            ParticleWorld::Instance( ).Tick( deltaSeconds );

            ChannelSystem::Instance( ).Flush( );

            //it's ok to unload outside of a render lock
            //because all data to be rendered must be copied
            //which means there won't be any instance data we unload
            //m_Game.UnloadScenes( );

            RenderWorld::Instance( ).AcquireLock( );

            s_updates = m_Clock.TestSample( ) - s_updates;
        }

        s_get_render_data = m_Clock.TestSample( );
        {
            RenderWorld::Instance( ).GetRenderData( );

            m_FrameReady = true;

            RenderWorld::Instance( ).Unlock( );

            DebugGraphics::Instance( ).Clear( );

            s_get_render_data = m_Clock.TestSample( ) - s_get_render_data;
        }

        s_render_wait = m_Clock.TestSample( );
        {


            RenderWorld::Instance( ).FinishFrame( );

            s_render_wait = m_Clock.TestSample( ) - s_render_wait;
        }

        if ( screen_shot_pending )
        {
#ifdef SHADE_PER_SAMPLE
#define APPEND_SAMPLE_TYPE "_sps"
#else
#define APPEND_SAMPLE_TYPE "_spp"
#endif

            char path[ 256 ] = { 0 };
            const char *pFilename = "scene_full";

            BuildDataPath( path, pFilename, sizeof( path ) - 1, false );
            String::Format( path, sizeof( path ), "%s", pFilename );

            char finalPath[ 256 * 2 ];

            int count = 1;

            do
            {
                String::Format( finalPath, sizeof( finalPath ), "%s_%d.bmp", path, count++ );
            } while ( FileStream::FileExists( finalPath ) );

            bool result = m_pFrameGrabber->SaveFile( finalPath );

            if ( true == result )
                Debug::Print( Debug::TypeWarning, "Screenshot Saved: %s\n", finalPath );

            screen_shot_pending = false;
        }

        static RegistryBool screenshot( "App/screenshot", false );

        if ( screenshot.GetValue( ) )
        {
            m_pFrameGrabber->GrabFrame( );

            screenshot.SetValue( false );
            screen_shot_pending = true;
        }

        s_render_submit = m_Clock.TestSample( );
        {
            Render( );

            s_render_submit = m_Clock.TestSample( ) - s_render_submit;
        }

        //maybe move this back to the lua gc thread if our machine has multiple cores
        //but for single cores (e.g. old win32 machine and ipad gen 1) it's better to 
        //call it here after our ticks are done processing
        s_gc = m_Clock.TestSample( );
        {
            LuaVM::Instance( ).GarbageCollect( );

            s_gc = m_Clock.TestSample( ) - s_gc;
        }

        g_frameIndex ^= 1;
    }
}

void App::Render( void )
{
    RenderWorld::Instance( ).AcquireLock( );

    if ( true == m_FrameReady )
        RenderWorld::Instance( ).Render( );

    m_FrameReady = false;

    RenderWorld::Instance( ).Unlock( );
}

void App::BeginTouch(
    int touchId,
    int x,
    int y
)
{

    //TODO: replace with viewport and window dimensions

    //float px, py;

    //int w = (int) GpuDevice::Instance( ).GetSwapChain( )->width;
    //int h = (int) GpuDevice::Instance( ).GetSwapChain( )->height;

    //PhysicalToProjected( &px, &py, x, y, w, h, Mode::Landscape.GetPhysicalLeft(), Mode::Landscape.GetPhysicalTop(), Mode::Landscape.GetPhysicalWidth(), Mode::Landscape.GetPhysicalHeight() );

    //TouchWorld::Instance( ).BeginTouch( touchId, px, py );
}

void App::Touch(
    int touchId,
    int x,
    int y
)
{
    //TODO: replace with viewport and window dimensions

    //float px, py;
    //int w = (int) Mode::Landscape.GetVirtualWidth( );
    //int h = (int) Mode::Landscape.GetVirtualHeight( );

    //PhysicalToProjected( &px, &py, x, y, w, h, Mode::Landscape.GetPhysicalLeft(), Mode::Landscape.GetPhysicalTop(), Mode::Landscape.GetPhysicalWidth(), Mode::Landscape.GetPhysicalHeight() );

    //TouchWorld::Instance( ).Touch( touchId, px, py );
}

void App::EndTouch(
    int touchId,
    int x,
    int y
)
{
    //TODO: replace with viewport and window dimensions

    //float px, py;
    //int w = (int) Mode::Landscape.GetVirtualWidth( );
    //int h = (int) Mode::Landscape.GetVirtualHeight( );

    //PhysicalToProjected( &px, &py, x, y, w, h, Mode::Landscape.GetPhysicalLeft(), Mode::Landscape.GetPhysicalTop(), Mode::Landscape.GetPhysicalWidth(), Mode::Landscape.GetPhysicalHeight() );

    //TouchWorld::Instance( ).EndTouch( touchId, px, py );
}

void App::Hover(
    int x,
    int y
)
{
    //TODO: replace with viewport and window dimensions
    ////relay to active game

    //float px, py;
    //int w = (int) Mode::Landscape.GetVirtualWidth( );
    //int h = (int) Mode::Landscape.GetVirtualHeight( );

    //PhysicalToProjected( &px, &py, x, y, w, h, Mode::Landscape.GetPhysicalLeft(), Mode::Landscape.GetPhysicalTop(), Mode::Landscape.GetPhysicalWidth(), Mode::Landscape.GetPhysicalHeight() );

    //TouchWorld::Instance( ).Hover( px, py );
}

void App::SetupRenderers( void )
{
    m_GpuTimers.Create( );

    RegistryBool deferred( "Render/deferred" );
    RegistryInt hdr_width( "Render/hdr_width" );
    RegistryInt hdr_height( "Render/hdr_height" );
    RegistryBool dof_enable( "dof.enable" );

    int windowWidth = GpuDevice::Instance( ).GetSwapChain( )->width;
    int windowHeight = GpuDevice::Instance( ).GetSwapChain( )->height;

    m_ColorBuffer = NullHandle;
    m_DSBuffer = NullHandle;

    hdr_width.SetValue( windowWidth );

    {
        RegistryInt w( StringRef( "Window/width" ) );
        RegistryInt h( StringRef( "Window/height" ) );

        w.SetValue( windowWidth );
        h.SetValue( windowHeight );
    }

    hdr_height.SetValue( (int) ( hdr_width.GetValue( ) * ( windowHeight / (float) windowWidth ) ) );
    g_resolution_scale = 1;

    g_scene_buffer_width = (int) ( hdr_width.GetValue( ) * g_resolution_scale );
    g_scene_buffer_height = (int) ( hdr_height.GetValue( ) * g_resolution_scale );

    ResourceHandle hMainCamera = ResourceHandle::FromAlias( "MainCamera" );
    ResourceHandle hShadowCamera = ResourceHandle::FromAlias( "ShadowCamera" );
    

    {
        CameraComponent *pCameraComponent = NULL;

        if ( false == IsResourceLoaded( hMainCamera ) )
        {
            hMainCamera = ResourceHandle( Id::Create( ) );

            pCameraComponent = new CameraComponent;
            pCameraComponent->Create( Id::Create( ) );

            Node *pCameraNode = new Node;
            pCameraNode->Create( );
            pCameraNode->AddComponent( pCameraComponent );

            hMainCamera.Bind( "MainCamera", pCameraNode );
            m_hMainCamera = hMainCamera;

            pCameraNode->AddToScene( );
            pCameraNode->Bind( );
        }
    }

    {
        CameraComponent *pCameraComponent = NULL;

        if ( false == IsResourceLoaded( hShadowCamera ) )
        {
            hShadowCamera = ResourceHandle( Id::Create( ) );

            pCameraComponent = new CameraComponent;
            pCameraComponent->Create( Id::Create( ) );

            Node *pCameraNode = new Node;
            pCameraNode->Create( );
            pCameraNode->AddComponent( pCameraComponent );

            hShadowCamera.Bind( "ShadowCamera", pCameraNode );

            pCameraNode->AddToScene( );
            pCameraNode->Bind( );
        }
    }

        for ( int i = 0; i < HiZMipLevels; i++ )
            GpuBuffer::CreateTexture( g_pHiZMipLevels[ i ], GpuBuffer::Format::LinearZ, GpuBuffer::Flags::UnorderedAccess, GpuBuffer::State::UnorderedAccess, HiZMipRes >> i, HiZMipRes >> i);

        for ( int i = 0; i < HdrMipLevels; i++ )
            GpuBuffer::CreateTexture( g_pHdrMipLevels[ i ], GpuBuffer::Format::HDR, GpuBuffer::Flags::UnorderedAccess, GpuBuffer::State::UnorderedAccess, HdrMipRes >> i, HdrMipRes >> i );

        ResourceHandle exposure = GpuBuffer::CreateBuffer( "Exposure", GpuResource::Flags::UnorderedAccess, GpuResource::State::UnorderedAccess, 64 * 1024, sizeof(float) );

        ResourceHandle whitepoint = GpuBuffer::CreateBuffer( "Whitepoint", GpuResource::Flags::UnorderedAccess, GpuResource::State::UnorderedAccess, 4 * sizeof(float), sizeof(float) );

        ResourceHandle ssaoMap = GpuBuffer::CreateTexture( "SSAOMap", GpuResource::Format::SSAO, GpuResource::Flags::UnorderedAccess, 
                                                                GpuResource::State::UnorderedAccess, g_scene_buffer_width, g_scene_buffer_height );

        ResourceHandle ssaoFinal = GpuBuffer::CreateTexture( "SSAOFinal", GpuResource::Format::SSAO, GpuResource::Flags::UnorderedAccess, 
                                                                GpuResource::State::UnorderedAccess, g_scene_buffer_width, g_scene_buffer_height );

        ResourceHandle ssaoBlur = GpuBuffer::CreateTexture( "SSAOBlur", GpuResource::Format::SSAO, GpuResource::Flags::UnorderedAccess, 
                                                                GpuResource::State::UnorderedAccess, g_scene_buffer_width, g_scene_buffer_height );

        ResourceHandle linearZ = GpuBuffer::CreateTexture( "LinearZ", GpuResource::Format::LinearZ, GpuResource::Flags::UnorderedAccess, 
                                                                GpuResource::State::UnorderedAccess, g_scene_buffer_width, g_scene_buffer_height );

        ResourceHandle dofBuffer = GpuBuffer::CreateTexture( "DofBuffer", GpuResource::Format::TransparentBuffer, GpuResource::Flags::UnorderedAccess, 
                                                                GpuResource::State::UnorderedAccess, hdr_width.GetValue( ), hdr_height.GetValue( ) >> 1 );

        ResourceHandle cocBuffer = GpuBuffer::CreateTexture( "CocBuffer", GpuResource::Format::COC, GpuResource::Flags::UnorderedAccess, 
                                                                GpuResource::State::UnorderedAccess, hdr_width.GetValue( ), hdr_height.GetValue( ) );
        
        ResourceHandle dofBlurredBuffer = GpuBuffer::CreateTexture( "DofBlurredBuffer", GpuResource::Format::TransparentBuffer, GpuResource::Flags::UnorderedAccess, 
                                                                GpuResource::State::UnorderedAccess, hdr_width.GetValue( ), hdr_height.GetValue( ) >> 1 );

        ResourceHandle ssr = GpuBuffer::CreateTexture( "SSRTarget", GpuResource::Format::SSR, GpuResource::Flags::UnorderedAccess, 
                                                                GpuResource::State::UnorderedAccess, hdr_width.GetValue( ), hdr_height.GetValue( ) );
        
        ResourceHandle hdrTarget = GpuBuffer::CreateTexture( "MainHDRTarget", GpuResource::Format::HDR, (GpuResource::Flags::Type) (GpuResource::Flags::RenderTarget | GpuResource::Flags::UnorderedAccess), 
                                                                GpuResource::State::RenderTarget, hdr_width.GetValue( ), hdr_height.GetValue( ), 1, &ClearColor  );

        ResourceHandle ldrTarget = GpuBuffer::CreateTexture( "LDRTarget", GpuResource::Format::LDR, (GpuResource::Flags::Type) (GpuResource::Flags::RenderTarget | GpuResource::Flags::UnorderedAccess), 
                                                                GpuResource::State::RenderTarget, windowWidth, windowHeight, 1, &ClearColor );

        ResourceHandle shadowMapTarget = GpuBuffer::CreateTexture( "MainShadowMap", GpuResource::Format::ShadowMap, GpuResource::Flags::RenderTarget, 
                                                                GpuResource::State::RenderTarget, g_shadow_width, g_shadow_height, 1, &Color( INT_MAX, INT_MAX, INT_MAX, INT_MAX ) );

        ResourceHandle shadowMapDepthStencil = GpuBuffer::CreateTexture( "MainShadowMapDS", GpuResource::Format::Depth, GpuResource::Flags::DepthStencil,
                                                                GpuResource::State::DepthWriteResource, g_shadow_width, g_shadow_height, 1, &Color(0, 0, 0, 0) );
        
        ResourceHandle hiZBuffer = GpuBuffer::CreateTexture( "HiZBuffer", GpuResource::Format::LinearZ, GpuResource::Flags::UnorderedAccess, 
                                                                GpuResource::State::UnorderedAccess, HiZMipRes, HiZMipRes, HiZMipLevels );

        ResourceHandle hdrMipBuffer = GpuBuffer::CreateTexture( "HdrMipBuffer", GpuResource::Format::HDR, GpuResource::Flags::UnorderedAccess, 
                                                                GpuResource::State::UnorderedAccess, HdrMipRes, HdrMipRes, HdrMipLevels );

    if ( deferred.GetValue( ) )
        SetupDeferredRenderer( );
    else
        SetupForwardRenderer( );

    //DofDesc dofDesc;
    //CreateDof( &m_GpuTimers, &dofDesc );
    //m_GpuTimers.Add( NULL );

    // UI and post processing render node and tree
    {
        Id renderGroup = Id( "UI" );
        RenderWorld::Instance( ).AddRenderGroup( renderGroup );

        m_pFrameGrabber = new FrameGrabRenderer( ldrTarget );

        // copy to back buffer
        ResourceHandle backBuffer = ResourceHandle::FromAlias( StringRef( "BackBuffer" ) );
        CopyResourceRenderer *pCopyResource = new CopyResourceRenderer( backBuffer, ldrTarget );

        // hdr to ldr, avg lum
        ResourceHandle avgLum[ ] = { ResourceHandle( "A64997F8-792A-41C8-B527-F6AB1C8E5088" ), ResourceHandle( "11054564-6CD5-40A7-8D72-26127C438115" ), NullHandle, };
        ComputeNode *pLdrPostProcess = new ComputeNode( "Tonemapping", avgLum, false, HdrToLdrProc );

        ResourceHandle lumResolve[ ] = { ResourceHandle( "30EB5804-8D0E-4C1C-BDF7-0EDD8F5AE5BA" ), NullHandle, };
        ComputeNode *pLdrPostProcessResolve = new ComputeNode( "Tonemapping", lumResolve, false, TonemapProc );

        m_UIRenderTree.Create( Id("UI Tree") );

        m_UIRenderTree.AddNode( new ConvertToRenderer( ldrTarget, GpuBuffer::State::UnorderedAccess ) );
        m_UIRenderTree.AddNode( new ConvertToRenderer( hdrTarget, GpuBuffer::State::PixelShaderResource ) );
        m_UIRenderTree.AddNode( pLdrPostProcess );
        m_UIRenderTree.AddNode( new ConvertToRenderer( exposure, GpuBuffer::State::PixelShaderResource ) );
        m_UIRenderTree.AddNode( pLdrPostProcessResolve );
        m_UIRenderTree.AddNode( new ConvertToRenderer( ldrTarget, GpuBuffer::State::RenderTarget ) );

        m_UIRenderTree.AddNode( new DefaultRenderer( "UI_Backers", false, &renderGroup, 1 ) );
        m_UIRenderTree.AddNode( new DefaultRenderer( "UI_Opaque", true, &renderGroup, 1 ) );
        m_UIRenderTree.AddNode( m_pFrameGrabber );
        m_UIRenderTree.AddNode( pCopyResource );

        m_UIViewport.Create( windowWidth, windowHeight, NullHandle, &ldrTarget, 1, NullHandle );

        ResourceHandle hCameraNode = ResourceHandle::FromAlias( "UICamera" );
        CameraComponent *pCameraComponent = NULL;

        if ( false == IsResourceLoaded( hCameraNode ) )
        {
            hCameraNode = ResourceHandle( Id::Create( ) );

            pCameraComponent = new CameraComponent;
            pCameraComponent->Create( Id::Create( ) );

            Node *pCameraNode = new Node;
            pCameraNode->Create( );
            pCameraNode->AddComponent( pCameraComponent );

            hCameraNode.Bind( "UICamera", pCameraNode );

            pCameraNode->AddToScene( );
            pCameraNode->Bind( );
        }
        else
            pCameraComponent = GetResource( hCameraNode, Node )->GetComponent<CameraComponent>( );

        Camera *pCamera = pCameraComponent->GetCamera( );
        pCamera->CreateAsOrtho( 1.0f, (float) windowWidth, (float) windowHeight, 0, 1 );
        pCamera->SetViewportRect( Vector( 0, 0, 1, 1 ) );

        m_UIViewport.SetCamera( hCameraNode );
        m_UIViewport.SetClearColor( ClearColor );
        m_UIViewport.SetClearFlags( Viewport::ClearFlagsNone );
        RenderWorld::Instance( ).AddViewport( m_UIViewport, &m_UIRenderTree );
        RenderWorld::Instance( ).BindViewportToWindow( m_WindowId, m_UIViewport.GetId( ), RenderOrder_UI );
    }

    g_RebuildRenderer = false;
}

void App::SetupDeferredRenderer( void )
{
    // setup g buffer and g buffer material
    RegistryBool depth_prepass = RegistryBool( "Render/depth_prepass" );

    RegistryBool dof_enable = RegistryBool( "dof.enable" );
    RegistryBool spec_bloom_enable = RegistryBool( "SpecBloom/enable" );
    RegistryBool ssr_enable = RegistryBool( "SSR/enable" );
    RegistryBool ssao_enable = RegistryBool( "SSAO/enable" );

    // g buffer should resolve to MainHDRTarget so it can still do luminance, etc.
    ResourceHandle hdrTarget = ResourceHandle( Id::Id( "MainHDRTarget" ) );

    m_DSBuffer = GpuBuffer::CreateTexture( "DSBuffer", GpuResource::Format::Depth, GpuResource::Flags::DepthStencil, 
                                                GpuResource::State::DepthWriteResource, g_scene_buffer_width, g_scene_buffer_height, 1, &Color(0, 0, 0, 0) );

    ResourceHandle opaqueRt = GpuBuffer::CreateTexture( "OpaqueRT" , GpuResource::Format::OpaqueBuffer, GpuResource::Flags::RenderTarget, 
                                                            GpuResource::State::RenderTarget, g_scene_buffer_width, g_scene_buffer_height, 1, &ClearColor );

    ResourceHandle lightMaskRt = GpuBuffer::CreateTexture( "LightMaskRT", GpuResource::Format::LightMaskBuffer, GpuResource::Flags::RenderTarget, 
                                                                GpuResource::State::RenderTarget, g_scene_buffer_width, g_scene_buffer_height, 1, &ClearColor );

    ResourceHandle propertiesRt = GpuBuffer::CreateTexture( "PropertiesRT", GpuResource::Format::MatProperties, GpuResource::Flags::RenderTarget, 
                                                                GpuResource::State::RenderTarget, g_scene_buffer_width, g_scene_buffer_height, 1, &ClearColor );

    ResourceHandle specularRt = GpuBuffer::CreateTexture( "SpecularRT", GpuResource::Format::SpecularBuffer, GpuResource::Flags::RenderTarget, 
                                                            GpuResource::State::RenderTarget, g_scene_buffer_width, g_scene_buffer_height, 1, &ClearColor );
    
    ResourceHandle normalRt = GpuBuffer::CreateTexture( "NormalRT", GpuResource::Format::NormalBuffer, GpuResource::Flags::RenderTarget,
                                                            GpuResource::State::RenderTarget, g_scene_buffer_width, g_scene_buffer_height, 1, &ClearColor );
    
    ResourceHandle hMainCamera = ResourceHandle::FromAlias( "MainCamera" );
    CameraComponent *pCameraComponent = GetResource( hMainCamera, Node )->GetComponent<CameraComponent>( );

    Camera *pCamera = pCameraComponent->GetCamera( );
    pCamera->CreateAsPerspective( Math::DegreesToRadians( 90.0f ), g_scene_buffer_width / (float) g_scene_buffer_height, 1.0f, FarClip );
    pCamera->SetViewportRect( Vector( 0, 0, 1, 1 ) );

    RenderWorld::Instance( ).AddRenderGroup( GeometryGroup );
    RenderWorld::Instance( ).AddRenderGroup( DebugGraphicsGroup );


    // shadowmap pass
    {
        ResourceHandle shadowMapTarget( "MainShadowMap" );
        ResourceHandle shadowMapDepthStencil( "MainShadowMapDS" );

        ShadowMapDesc shadowMapDesc;
        CreateShadowMap( shadowMapTarget, &shadowMapDesc );

        // shadowmap render tree
        {
            m_ShadowMapRenderTree.Create( Id::Id( "ShadowMapTree" ) );

            AddShadowMap( &m_ShadowMapRenderTree, shadowMapDesc );

            m_ShadowMapViewport.Create( g_shadow_width, g_shadow_height, NullHandle, &shadowMapTarget, 1, shadowMapDepthStencil );

            ResourceHandle hCameraNode = ResourceHandle::FromAlias( "ShadowCamera" );
            CameraComponent *pCameraComponent = GetResource( hCameraNode, Node )->GetComponent<CameraComponent>( );

            Camera *pCamera = pCameraComponent->GetCamera( );
            pCamera->CreateAsOrtho( 1.25f, 1024.0f, 1024.0f, 1.0f, 5000.0f );
            pCamera->SetViewportRect( Vector( 0, 0, 1, 1 ) );

            m_ShadowMapViewport.SetCamera( hCameraNode );
            m_ShadowMapViewport.SetClearColor( Color( INT_MAX, INT_MAX, INT_MAX, INT_MAX ) );
            m_ShadowMapViewport.SetClearFlags( Viewport::ClearFlagsAll );

            RenderWorld::Instance( ).AddViewport( m_ShadowMapViewport, &m_ShadowMapRenderTree );
            RenderWorld::Instance( ).BindViewportToWindow( m_WindowId, m_ShadowMapViewport.GetId( ), RenderOrder_ShadowMap );
        }
    }

    // depth prepass pass
    if ( depth_prepass.GetValue( ) )
    {
        // Depth Prepass
        DefaultRenderer *pDepthPrepass;

        Id groups[] = { GeometryGroup, DebugGraphicsGroup };
        pDepthPrepass = new DefaultRenderer( "DepthPrepass", true, groups, _countof(groups) );

        IRenderModifier *pModifier = new FrustumCullRenderModifier;
        pDepthPrepass->AddModifier( pModifier );
        
        SSAODesc ssaoDesc;

        if ( ssao_enable.GetValue() )
            CreateSSAO( &ssaoDesc );

        HiZMipDesc hiZMipDesc;
        CreateHiZMips( &hiZMipDesc );


        // main scene render tree
        {
            m_DepthPrepassRenderTree.Create( Id::Id( "DepthPrepassTree" ) );
            m_DepthPrepassRenderTree.AddNode( pDepthPrepass );
            m_DepthPrepassRenderTree.AddNode( new ConvertToRenderer( m_DSBuffer, GpuBuffer::State::ShaderResource ) );

            AddLinearZ( &m_DepthPrepassRenderTree );

            if ( ssao_enable.GetValue() )
                AddSSAO( &m_DepthPrepassRenderTree, ssaoDesc );
            
            AddHiZMips( &m_DepthPrepassRenderTree, hiZMipDesc );

            m_DepthPrepassViewport.Create( g_scene_buffer_width, g_scene_buffer_height, NullHandle, NULL, 0, m_DSBuffer );
            m_DepthPrepassViewport.SetRenderScale( Vector2( g_resolution_scale, g_resolution_scale ) );

            m_DepthPrepassViewport.SetCamera( hMainCamera );
            m_DepthPrepassViewport.SetClearFlags( Viewport::ClearFlagsDepth );

            RenderWorld::Instance( ).AddViewport( m_DepthPrepassViewport, &m_DepthPrepassRenderTree );
            RenderWorld::Instance( ).BindViewportToWindow( m_WindowId, m_DepthPrepassViewport.GetId( ), RenderOrder_DepthPrepass );
        }
    }

    // g buffer pass
    {
        Id treeId = Id::Id( "GBufferTree" );

        ComputeNode *pGBufferResolve;
        {
            ResourceHandle gBufferResolve( "85B3DFC9-4A37-4073-ACAF-7D21762CB652" );
            ResourceHandle computeMaterials[ ] = { gBufferResolve, NullHandle, };
            pGBufferResolve = new ComputeNode( "GBufferResolve", computeMaterials, false, GBufferResolveProc );
        }

        m_SceneRenderTree.Create( treeId );

        // Gbuffer Render Node
        {
            Id groups[] = { GeometryGroup, DebugGraphicsGroup };
            g_pOpaqueRenderer = new DefaultRenderer( "GBuffer", true, groups, _countof(groups) );

            IRenderModifier *pModifier = new FrustumCullRenderModifier;
            m_pCullOpaque = (FrustumCullRenderModifier *) pModifier;
            g_pOpaqueRenderer->AddModifier( pModifier );
        }

        // main scene render tree
        {
            m_SceneRenderTree.AddNode( g_pOpaqueRenderer );

            m_SceneRenderTree.AddNode( new ConvertToRenderer( opaqueRt, GpuBuffer::State::ShaderResource ) );
            m_SceneRenderTree.AddNode( new ConvertToRenderer( lightMaskRt, GpuBuffer::State::ShaderResource ) );
            m_SceneRenderTree.AddNode( new ConvertToRenderer( propertiesRt, GpuBuffer::State::ShaderResource ) );
            m_SceneRenderTree.AddNode( new ConvertToRenderer( specularRt, GpuBuffer::State::ShaderResource ) );
            m_SceneRenderTree.AddNode( new ConvertToRenderer( normalRt, GpuBuffer::State::ShaderResource ) );
            m_SceneRenderTree.AddNode( new ConvertToRenderer( m_DSBuffer, GpuBuffer::State::ShaderResource ) );
            m_SceneRenderTree.AddNode( pGBufferResolve );

            ResourceHandle handles[ ] =
            {
               opaqueRt,
               normalRt,
               specularRt,
               propertiesRt,
               lightMaskRt,
            };

            m_SceneViewport.Create( g_scene_buffer_width, g_scene_buffer_height, NullHandle, handles, _countof( handles ), m_DSBuffer );
            m_SceneViewport.SetRenderScale( Vector2( g_resolution_scale, g_resolution_scale ) );

            m_SceneViewport.SetCamera( hMainCamera );
            m_SceneViewport.SetClearColor( ClearColor );
            m_SceneViewport.SetClearFlags( Viewport::ClearFlagsAll );
            RenderWorld::Instance( ).AddViewport( m_SceneViewport, &m_SceneRenderTree );
            RenderWorld::Instance( ).BindViewportToWindow( m_WindowId, m_SceneViewport.GetId( ), RenderOrder_3D );
        }
    }

    // transparency forward pass
    {
        m_DeferredForwardRenderTree.Create( Id::Id( "DeferredForwardRenderTree" ) );

        // Transparent render node
        {
            Id groups[] = { GeometryGroup, DebugGraphicsGroup };
            g_pTransparentRenderer = new DefaultRenderer( "Transparency", false, groups, _countof(groups) );

            IRenderModifier *pModifier = new FrustumCullRenderModifier;
            m_pCullTransparent = (FrustumCullRenderModifier *) pModifier;
            g_pTransparentRenderer->AddModifier( pModifier );

            pModifier = new TransparentSortRenderModifier;
            g_pTransparentRenderer->AddModifier( pModifier );
        }

        HdrMipDesc hdrMipDesc;
        CreateHdrMips( &m_GpuTimers, &hdrMipDesc );
        m_GpuTimers.Add( NULL );

        DofDesc dofDesc;
        SpecBloomDesc specBloomDesc;
        SsrDesc ssrDesc;

        if ( dof_enable.GetValue( ) )
        {
            CreateDof( &m_GpuTimers, &dofDesc );
            m_GpuTimers.Add( NULL );
        }

        if ( spec_bloom_enable.GetValue( ) )
        {
            CreateSpecBloom( &m_GpuTimers, &specBloomDesc );
            m_GpuTimers.Add( NULL );
        }
        
        if ( ssr_enable.GetValue( ) )
        {
            CreateSsr( &m_GpuTimers, &ssrDesc );
            m_GpuTimers.Add( NULL );
        }

        // transparent render pass
        {
            m_DeferredForwardRenderTree.AddNode( g_pTransparentRenderer );
            m_DeferredForwardRenderTree.AddNode( new BarrierRenderer( hdrTarget, GpuResource::Barrier::Uav ) );

            AddHdrMips( &m_DeferredForwardRenderTree, hdrMipDesc );

            if ( ssr_enable.GetValue( ) )
                AddSsr( &m_DeferredForwardRenderTree, ssrDesc );

            if ( spec_bloom_enable.GetValue( ) )
                AddSpecBloom( &m_DeferredForwardRenderTree, specBloomDesc );

            if ( dof_enable.GetValue( ) )
                AddDof( &m_DeferredForwardRenderTree, dofDesc );

            m_DeferredForwardViewport.Create( g_scene_buffer_width, g_scene_buffer_height, NullHandle, &hdrTarget, 1, m_DSBuffer );
            m_DeferredForwardViewport.SetRenderScale( Vector2( g_resolution_scale, g_resolution_scale ) );

            m_DeferredForwardViewport.SetCamera( hMainCamera );
            m_DeferredForwardViewport.SetClearColor( ClearColor );
            m_DeferredForwardViewport.SetClearFlags( Viewport::ClearFlagsNone );

            RenderWorld::Instance( ).AddViewport( m_DeferredForwardViewport, &m_DeferredForwardRenderTree );
            RenderWorld::Instance( ).BindViewportToWindow( m_WindowId, m_DeferredForwardViewport.GetId( ), RenderOrder_CTB );
        }
    }
}

void App::SetupForwardRenderer( void )
{
    m_pCullOpaque = NULL;
    m_pCullTransparent = NULL;
    g_pTransparentRenderer = NULL;

    int sampleCount = 1;

    ResourceHandle hdrTarget( "MainHDRTarget" );

    static RegistryInt hdr_width( "Render/hdr_width", 0 );
    static RegistryInt hdr_height( "Render/hdr_height", 0 );
    static RegistryBool depth_prepass( "Render/depth_prepass" );

    int windowWidth = GpuDevice::Instance( ).GetSwapChain( )->width;
    int windowHeight = GpuDevice::Instance( ).GetSwapChain( )->height;

    RegistryBool ssr_enable = RegistryBool( "SSR/enable" );
    RegistryBool dof_enable = RegistryBool( "dof.enable" );
    RegistryBool ssao_enable = RegistryBool( "SSAO/enable" );
    RegistryBool spec_bloom_enable = RegistryBool( "SpecBloom/enable" );

    sampleCount = 1;

    RenderWorld::Instance( ).AddRenderGroup( GeometryGroup );
    RenderWorld::Instance( ).AddRenderGroup( DebugGraphicsGroup );

    // buffers
    ResourceHandle matProperties = GpuBuffer::CreateTexture( "PropertiesRT", GpuBuffer::Format::MatProperties, GpuBuffer::Flags::RenderTarget, 
                                                                GpuBuffer::State::RenderTarget, g_scene_buffer_width, g_scene_buffer_height, 1, &ClearColor );

    ResourceHandle normalRt = GpuBuffer::CreateTexture( "NormalRT", GpuBuffer::Format::NormalBuffer, GpuBuffer::Flags::RenderTarget, 
                                                            GpuBuffer::State::RenderTarget, g_scene_buffer_width, g_scene_buffer_height, 1, &ClearColor );

    m_DSBuffer = GpuBuffer::CreateTexture( "DSBuffer", GpuBuffer::Format::Depth, GpuBuffer::Flags::DepthStencil, 
                                            GpuResource::State::DepthWriteResource, g_scene_buffer_width, g_scene_buffer_height, 1, &Color(0, 0, 0, 0) );

    ResourceHandle hMainCamera = ResourceHandle::FromAlias( "MainCamera" );
    CameraComponent *pCameraComponent = GetResource( hMainCamera, Node )->GetComponent<CameraComponent>( );

    Camera *pCamera = pCameraComponent->GetCamera( );
    pCamera->CreateAsPerspective( Math::DegreesToRadians( 90.0f ), g_scene_buffer_width / (float) g_scene_buffer_height, 1.0f, FarClip );
    pCamera->SetViewportRect( Vector( 0, 0, 1, 1 ) );

    // shadowmap pass
    {
        ResourceHandle shadowMapDepthStencil( "MainShadowMapDS" );
        ResourceHandle shadowMapTarget( "MainShadowMap" );

        ShadowMapDesc shadowMapDesc;
        CreateShadowMap( shadowMapTarget, &shadowMapDesc );

        // shadowmap render tree
        {
            m_ShadowMapRenderTree.Create( Id::Id( "ShadowMapTree" ) );

            AddShadowMap( &m_ShadowMapRenderTree, shadowMapDesc );

            m_ShadowMapViewport.Create( g_shadow_width, g_shadow_height, NullHandle, &shadowMapTarget, 1, shadowMapDepthStencil );

            ResourceHandle hCameraNode = ResourceHandle::FromAlias( "ShadowCamera" );
            CameraComponent *pCameraComponent = GetResource( hCameraNode, Node )->GetComponent<CameraComponent>( );

            Camera *pCamera = pCameraComponent->GetCamera( );
            pCamera->CreateAsOrtho( 1.25f, 1024.0f, 1024.0f, 1.0f, 5000.0f );
            pCamera->SetViewportRect( Vector( 0, 0, 1, 1 ) );

            m_ShadowMapViewport.SetCamera( hCameraNode );
            m_ShadowMapViewport.SetClearColor( Color( INT_MAX, INT_MAX, INT_MAX, INT_MAX ) );
            m_ShadowMapViewport.SetClearFlags( Viewport::ClearFlagsAll );

            RenderWorld::Instance( ).AddViewport( m_ShadowMapViewport, &m_ShadowMapRenderTree );
            RenderWorld::Instance( ).BindViewportToWindow( m_WindowId, m_ShadowMapViewport.GetId( ), RenderOrder_ShadowMap );
        }
    }

    // depth prepass pass
    if ( depth_prepass.GetValue( ) )
    {
        // Depth Prepass
        DefaultRenderer *pDepthPrepass;
        {
            Id groups[] = { GeometryGroup, DebugGraphicsGroup };
            pDepthPrepass = new DefaultRenderer( "DepthPrepass", true, groups, _countof(groups) );

            IRenderModifier *pModifier = new FrustumCullRenderModifier;
            pDepthPrepass->AddModifier( pModifier );
        }

        SSAODesc ssaoDesc;
        
        if ( ssao_enable.GetValue() )
            CreateSSAO( &ssaoDesc );

        HiZMipDesc hiZMipDesc;
        CreateHiZMips( &hiZMipDesc );

        // main scene render tree
        {
            m_DepthPrepassRenderTree.Create( Id::Id( "DepthPrepassTree" ) );
            m_DepthPrepassRenderTree.AddNode( pDepthPrepass );
            m_DepthPrepassRenderTree.AddNode( new ConvertToRenderer( m_DSBuffer, GpuBuffer::State::ShaderResource ) );

            AddLinearZ( &m_DepthPrepassRenderTree );

            if ( ssao_enable.GetValue() )
                AddSSAO( &m_DepthPrepassRenderTree, ssaoDesc );
            
            AddHiZMips( &m_DepthPrepassRenderTree, hiZMipDesc );

            m_DepthPrepassViewport.Create( g_scene_buffer_width, g_scene_buffer_height, NullHandle, NULL, 0, m_DSBuffer );
            m_DepthPrepassViewport.SetRenderScale( Vector2( g_resolution_scale, g_resolution_scale ) );

            m_DepthPrepassViewport.SetCamera( hMainCamera );
            m_DepthPrepassViewport.SetClearFlags( Viewport::ClearFlagsDepth );

            RenderWorld::Instance( ).AddViewport( m_DepthPrepassViewport, &m_DepthPrepassRenderTree );
            RenderWorld::Instance( ).BindViewportToWindow( m_WindowId, m_DepthPrepassViewport.GetId( ), RenderOrder_DepthPrepass );
        }
    }

    // geometry pass
    {
        m_SceneRenderTree.Create( Id::Id( "SceneTree" ) );

        // Forward Render Node
        {
            Id groups[] = { GeometryGroup, DebugGraphicsGroup };
            g_pOpaqueRenderer = new DefaultRenderer( "Forward", true, groups, _countof(groups) );

            IRenderModifier *pModifier = new FrustumCullRenderModifier;
            m_pCullOpaque = (FrustumCullRenderModifier *) pModifier;
            g_pOpaqueRenderer->AddModifier( pModifier );
        }

        // Transparent render node
        {
            Id groups[] = { GeometryGroup, DebugGraphicsGroup };
            g_pTransparentRenderer = new DefaultRenderer( "Transparency", false, groups, _countof(groups) );

            IRenderModifier *pModifier = new FrustumCullRenderModifier;
            m_pCullTransparent = (FrustumCullRenderModifier *) pModifier;
            g_pTransparentRenderer->AddModifier( pModifier );

            pModifier = new TransparentSortRenderModifier;
            g_pTransparentRenderer->AddModifier( pModifier );

        }

        HdrMipDesc hdrMipDesc;
        CreateHdrMips( &m_GpuTimers, &hdrMipDesc );
        m_GpuTimers.Add( NULL );

        DofDesc dofDesc;
        SpecBloomDesc specBloomDesc;
        SsrDesc ssrDesc;

        if ( dof_enable.GetValue( ) )
        {
            CreateDof( &m_GpuTimers, &dofDesc );
            m_GpuTimers.Add( NULL );
        }

        if ( spec_bloom_enable.GetValue( ) )
        {
            CreateSpecBloom( &m_GpuTimers, &specBloomDesc );
            m_GpuTimers.Add( NULL );
        }
       
        if ( ssr_enable.GetValue( ) )
        {
            CreateSsr( &m_GpuTimers, &ssrDesc );
            m_GpuTimers.Add( NULL );
        }

        // main scene render tree
        {
            m_SceneRenderTree.AddNode( g_pOpaqueRenderer );
            m_SceneRenderTree.AddNode( g_pTransparentRenderer );
            m_SceneRenderTree.AddNode( new ConvertToRenderer( matProperties, GpuBuffer::State::ShaderResource ) );
            m_SceneRenderTree.AddNode( new ConvertToRenderer( normalRt, GpuBuffer::State::ShaderResource ) );
            m_SceneRenderTree.AddNode( new ConvertToRenderer( hdrTarget, GpuBuffer::State::ShaderResource ) );

            AddHdrMips( &m_SceneRenderTree, hdrMipDesc );

            if ( ssr_enable.GetValue( ) )
                AddSsr( &m_SceneRenderTree, ssrDesc );

            if ( spec_bloom_enable.GetValue( ) )
                AddSpecBloom( &m_SceneRenderTree, specBloomDesc );

            if ( dof_enable.GetValue( ) )
                AddDof( &m_SceneRenderTree, dofDesc );

            ResourceHandle handles[ ] =
            {
               hdrTarget,
               matProperties,
               normalRt,
            };

            m_SceneViewport.Create( g_scene_buffer_width, g_scene_buffer_height, NullHandle, handles, _countof( handles ), m_DSBuffer );
            m_SceneViewport.SetRenderScale( Vector2( g_resolution_scale, g_resolution_scale ) );

            m_SceneViewport.SetCamera( hMainCamera );
            m_SceneViewport.SetClearColor( ClearColor );
            m_SceneViewport.SetClearFlags( Viewport::ClearFlagsAll );
            RenderWorld::Instance( ).AddViewport( m_SceneViewport, &m_SceneRenderTree );
            RenderWorld::Instance( ).BindViewportToWindow( m_WindowId, m_SceneViewport.GetId( ), RenderOrder_3D );
        }
    }
}

void App::TeardownRenderers( void )
{
    RenderWorld::Instance( ).FinishFrame( );

    const char *pTargets[ ] =
    {
        "Exposure",
        "Whitepoint",
        "MainHDRTarget",
        "DSBuffer",
        "LDRTarget",
        "MainShadowMap",
        "MainShadowMapDS",
        "FrameEven",
        "FrameOdd",
        "OpaqueRT",
        "LightMaskRT",
        "PropertiesRT",
        "SpecularRT",
        "NormalRT",
        "SSRTarget",
        "LinearZ",
        "SSAOMap",
        "SSAOBlur",
        "SSAOFinal",
        "DepthStencilRT",
        "HiZBuffer",
        "HdrMipBuffer",
        "DofBlurredBuffer",
        "DofBuffer",
        "CocBuffer",

        NULL,
    };

    int index = 0;

    while ( pTargets[ index ] != NULL )
    {
        ResourceHandle target = ResourceHandle( pTargets[ index ] );
        if ( IsResourceLoaded( target ) )
        {
            GpuResource *pBuffer = GetResource( target, GpuResource );
            pBuffer->RemoveFromScene( );
            pBuffer->Destroy( );
            delete pBuffer;
        }

        ++index;
    }

    for ( int i = 0; i < HdrMipLevels; i++ )
    {
        ResourceHandle target = ResourceHandle( g_pHdrMipLevels[ i ] );
        if ( IsResourceLoaded( target ) )
        {
            GpuResource *pBuffer = GetResource( target, GpuResource );
            pBuffer->RemoveFromScene( );
            pBuffer->Destroy( );
            delete pBuffer;
        }
    }

    for ( int i = 0; i < HiZMipLevels; i++ )
    {
        ResourceHandle target = ResourceHandle( g_pHiZMipLevels[ i ] );
        if ( IsResourceLoaded( target ) )
        {
            GpuResource *pBuffer = GetResource( target, GpuResource );
            pBuffer->RemoveFromScene( );
            pBuffer->Destroy( );
            delete pBuffer;
        }

    }

    m_ColorBuffer = NullHandle;
    m_DSBuffer = NullHandle;

    g_pTransparentRenderer = NULL;
    g_pOpaqueRenderer = NULL;

    RenderWorld::Instance( ).RemoveViewport( m_UIViewport.GetId( ) );
    RenderWorld::Instance( ).RemoveViewport( m_SceneViewport.GetId( ) );
    RenderWorld::Instance( ).RemoveViewport( m_DepthPrepassViewport.GetId( ) );
    RenderWorld::Instance( ).RemoveViewport( m_ShadowMapViewport.GetId( ) );
    RenderWorld::Instance( ).RemoveViewport( m_UpResViewport.GetId( ) );
    RenderWorld::Instance( ).RemoveViewport( m_DeferredForwardViewport.GetId( ) );
    RenderWorld::Instance( ).RemoveViewport( m_BokehViewport.GetId( ) );

    m_BokehViewport.Destroy( );
    m_UpResViewport.Destroy( );
    m_UIViewport.Destroy( );
    m_SceneViewport.Destroy( );
    m_DepthPrepassViewport.Destroy( );
    m_ShadowMapViewport.Destroy( );
    m_DeferredForwardViewport.Destroy( );

    m_UpResRenderTree.Destroy( );
    m_UIRenderTree.Destroy( );
    m_ShadowMapRenderTree.Destroy( );
    m_SceneRenderTree.Destroy( );
    m_BokehRenderTree.Destroy( );
    m_DepthPrepassRenderTree.Destroy( );
    m_DeferredForwardRenderTree.Destroy( );

    m_GpuTimers.Destroy( );
}

void ShowHemisphere( void )
{
    RegistryInt method( "Tonemap/method" );
    method.SetValue(1);

    RegistryBool dof_enable = RegistryBool( "dof.enable" );
    dof_enable.SetValue(false);

    //const int min_inner_iterations = 1;
    //const int max_inner_iterations = 14;
    //const int outer_iterations = 2;
    //const float scale_out = 1.0f;

    //int total = 0;

    //float prev_s, prev_c;

    //char txt[ 4096 * 4 ] = {0};

    //for (int i = 0; i < outer_iterations; i++)
    //{
    //    float scale = (i + 1) / (float) outer_iterations;

    //    int inner_iterations = Math::Lerp( min_inner_iterations, max_inner_iterations, (i + 1) / (float) outer_iterations );

    //    for (int k = 0; k < inner_iterations; k++)
    //    {
    //        float theta = k / (float) inner_iterations * 2.0f * Math::PI();

    //        float s, c;

    //        Math::CosSin( &c, &s, theta );

    //        s = s * scale;
    //        c = c * scale;

    //        if ( k > 0 || i > 0 )
    //            DebugGraphics::Instance( ).RenderLine( Math::IdentityTransform(), Vector(prev_c, prev_s, 0, 1), Vector(c, s, 0, 1), Math::OneVector() );

    //        prev_s = s;
    //        prev_c = c;

    //        String::Format( txt, sizeof(txt), "%sfloat2(%f, %f),\n", txt, c, s );
    //        ++total;
    //    }
    //}

    ////String::Format( txt, sizeof(txt), "%stotal: %d\n", txt, total );
    ////OutputDebugString(txt);

    //Vector v[] = 
    //{
    //    Vector(0.53333336, 0),
    //    Vector(0.3325279, 0.4169768),
    //    Vector(-0.11867785, 0.5199616),
    //    Vector(-0.48051673, 0.2314047),
    //    Vector(-0.48051673, -0.23140468),
    //    Vector(-0.11867763, -0.51996166),
    //    Vector(0.33252785, -0.4169769),
    //    Vector(1, 0),
    //    Vector(0.90096885, 0.43388376),
    //    Vector(0.6234898, 0.7818315),
    //    Vector(0.22252098, 0.9749279),
    //    Vector(-0.22252095, 0.9749279),
    //    Vector(-0.62349, 0.7818314),
    //    Vector(-0.90096885, 0.43388382),
    //    Vector(-1, 0),
    //    Vector(-0.90096885, -0.43388376),
    //    Vector(-0.6234896, -0.7818316),
    //    Vector(-0.22252055, -0.974928),
    //    Vector(0.2225215, -0.9749278),
    //    Vector(0.6234897, -0.7818316),
    //    Vector(0.90096885, -0.43388376),
    //};

    //for (int i = 1; i < _countof(v); i++)
    //    DebugGraphics::Instance( ).RenderLine( Math::IdentityTransform(), Vector(v[i - 1].x + 5, v[i - 1].y, 0, 1), Vector(v[i].x + 5, v[i].y, 0, 1), Math::OneVector() );

    //return;


    //http://www.rorydriscoll.com/2009/01/07/better-sampling/
    //https://damart3d.blogspot.com/2016/03/test-article.html
    //http://www.cse.cuhk.edu.hk/~ttwong/papers/udpoint/udpoint.pdf
 
    const int Samples1 = 8;

    Vector t_origin( 0, 0, 0);
    Vector w_origin( 100, 0, 0);
    Vector v_origin( 200, 0, 0);

    Vector direction( -1, .25, .5 );
    //Vector direction( 0, 1, 0 );
    Math::Normalize( &direction, direction );

    DebugGraphics::Instance( ).RenderArrow( t_origin, direction, Vector(1,0,0, 1), 80.0f );
    DebugGraphics::Instance( ).RenderArrow( v_origin, direction, Vector(1,0,0, 1), 80.0f );

    Transform light_transform = Math::IdentityTransform();
    
    {   
        Vector look = Vector(0, 0, 1);
        Vector up = direction;

        Vector newLook, right;

        Math::CrossProduct( &right, up, look );
        Math::Normalize( &right, right );

        Math::CrossProduct( &newLook, right, up );
        Math::Normalize( &newLook, newLook );

        Math::CrossProduct( &right, up, newLook );

        light_transform.SetRight( right );
        light_transform.SetUp( up );
        light_transform.SetLook( newLook );
    }

    for ( int x = 0; x < Samples1; x++ )
    {
        for ( int y = 0; y < Samples1; y++ )
        {
            // u1 defines X/Z rotation
            // v1 defines Y angle
            float u1 = (x + .5f) / (float) Samples1;
            float v1 = (y + .5f) / (float) Samples1;

            {
                // X and Z full rotation around 
                // based on our U [0,1]
                // (think of looking down at a compass)
                float t = u1 * 2 * Math::PI();

                // depending on how adjacent we want our up vector Sqrt(1 - v1)
                // use this value to normalize the X/Z with respect to Y..
                // in concept 'squishing' them in the farther Y extends out
                float r = Math::Sqrt(1 - v1);

                // 'squish' in the X/Z rays depending on
                // how adjacent the up vector is
                float x_ = Math::Cos(t) * r;
                float z_ = Math::Sin(t) * r;

                // Our up vector based on v1 [0-1]
                float y_ = Math::Sqrt(v1);

                Vector v(x_, y_, z_);
                Math::Rotate(&v, v, light_transform);

                DebugGraphics::Instance( ).RenderArrow( t_origin, v, Vector(196 / 255.0f, 189.0f / 255.0f,67/255.0f, 1), 40.0f );                
            }


            {
                // X and Z full rotation around 
                // based on our U [0,1]
                // (think of looking down at a compass)
                float t = u1 * 2 * Math::PI();

                // depending on how adjacent we want our up vector Sqrt(1 - v1)
                // use this value to normalize the X/Z with respect to Y..
                // in concept 'squishing' them in the farther Y extends out
                float r = Math::Sqrt(1 - v1 * v1);

                // 'squish' in the X/Z rays depending on
                // how adjacent the up vector is
                float x_ = Math::Cos(t) * r;
                float z_ = Math::Sin(t) * r;
                // Our up vector based on v1 [0-1]
                float y_ = v1;

                //assume rand.x is a random number from 0 to 2PI...
                //assume rand.y is a random number from 0 to PI...
                //x_ = Math::Cos(t);
                //y_ = Math::Sin(v1);
                //z_ = Math::Sin(t);


                Vector v(x_, y_, z_);
                //Math::Normalize(&v, v);
                Math::Rotate(&v, v, light_transform);

                DebugGraphics::Instance( ).RenderArrow( v_origin, v, Vector(196 / 255.0f, 189.0f / 255.0f,67/255.0f, 1), 40.0f );                
            }
        }
    }
}
