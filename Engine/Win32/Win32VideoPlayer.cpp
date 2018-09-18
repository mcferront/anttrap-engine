#include "EnginePch.h"

#include "VideoPlayer.h"
#include "Win32VideoSurface.h"
#include "Win32Messages.h"
#include "Dx9.h"

#include "MaterialAsset.h"
#include "TextureAsset.h"
#include "Viewport.h"

HWND VideoPlayer::s_MainWindow;
char VideoPlayer::s_VideoPath[ MAX_PATH ];

void VideoPlayer::Load(
    const char *pPath,
    uint32 offset,
    uint32 size,
    Channel *pChannel
    )
{
    //unused
    (void) offset;
    (void) size;

    Debug::Assert( Condition( 0 == offset && 0 == size ), "Win32 cannot play videos from an offset" );

    extern List<VideoPlayer*> g_ValidVideoPlayers;
    g_ValidVideoPlayers.Add( this );

    Debug::Assert( Condition( NULL != s_MainWindow ), "A main window must be set before the VideoPlayer can be used" );

    m_LastRecordedFrame = 0;

    ResourceHandle videoTexture( Id::Create( ) );

    String::Copy( m_VideoFile, pPath, sizeof( m_VideoFile ) );

    //build our display sprite, texture
    //i would prefer the sprite wasn't a memeber of this class and this class
    //simply drew to a texture - but we're mimicing iOS behaviour and that renders
    //directly the the back buffer


    Debug::Assert( Condition( false ), "This must be redone to use offline materials" );
    {
        //ResourceHandle videoMaterial( Id::Create() );

        //m_pMaterial = new Material;
        //videoMaterial.Bind( NULL, m_pMaterial );

        //m_pMaterial->Create( videoTexture, ResourceHandle( "Video.shader"), Vector(1,1,1,1), false, false, false, false, true );
        //m_pMaterial->AddToScene( );

        //Debug::Assert( Condition(false), "This will crash until it has a proper component as a parent" );
        //
        //IdList groups; groups.Add(Id("Video"));
        //m_Sprite.Create( NULL, videoMaterial, groups, Vector2((float)Mode::Landscape.GetVirtualWidth(), (float)Mode::Landscape.GetVirtualHeight()) );
    }

    m_pChannel = pChannel;

    Dx9::Instance( ).GetChannel( )->AddEventListener( EventMap<VideoPlayer>( "DeviceLost", this, &VideoPlayer::OnDeviceLost ) );
    Dx9::Instance( ).GetChannel( )->AddEventListener( EventMap<VideoPlayer>( "DeviceRestored", this, &VideoPlayer::OnDeviceRestored ) );

    LoadVMR( );
}

void VideoPlayer::Unload( void )
{
    extern List<VideoPlayer*> g_ValidVideoPlayers;
    g_ValidVideoPlayers.Remove( this );

    UnloadVMR( );

    Dx9::Instance( ).GetChannel( )->RemoveEventListener( EventMap<VideoPlayer>( "DeviceLost", this, &VideoPlayer::OnDeviceLost ) );
    Dx9::Instance( ).GetChannel( )->RemoveEventListener( EventMap<VideoPlayer>( "DeviceRestored", this, &VideoPlayer::OnDeviceRestored ) );

    m_Sprite.RemoveFromScene( );
    m_Sprite.Destroy( );

    m_pMaterial->RemoveFromScene( );
    m_pMaterial->Destroy( );

    delete m_pMaterial;

    m_pChannel = NULL;
    m_pMaterial = NULL;
}

void VideoPlayer::Play( void )
{
    //vmr isn't ready, this must have happened because the d3d device was lost
    //and we can't do anything until it is restored
    if ( NULL == m_pMediaControl ) return;

    HRESULT hr = m_pMediaControl->Run( );
    Debug::Assert( Condition( SUCCEEDED( hr ) ), "Media Control error when playing video stream (0x%08x)", hr );
}

void VideoPlayer::Stop( void )
{
    //vmr isn't ready, this must have happened because the d3d device was lost
    //and we can't do anything until it is restored
    if ( NULL == m_pMediaControl ) return;

    HRESULT hr = m_pMediaControl->Pause( );
    Debug::Assert( Condition( SUCCEEDED( hr ) ), "Media Control error when stopping video stream (0x%08x)", hr );
}

void VideoPlayer::SeekTo(
    VideoPlayer::Seek seek,
    float seconds
    )
{
    //vmr isn't ready, this must have happened because the d3d device was lost
    //and we can't do anything until it is restored
    if ( NULL == m_pMediaSeeking ) return;

    //reftime is in 100nanosecond increments
    //1,000,000,000 nanoseconds in 1 second
    //1,000,000,000 / 100 = 10,000,000 refincrements in a second
    //so second * 10,000,000 = correct reftime
    LONGLONG time = (LONGLONG) floorf( seconds * 10000000 );

    HRESULT hr;

    //IMediaSeeking doesn't always support relative seeking
    //so we fall back and always use absolute 
    if ( VideoPlayer::SeekCurrent == seek )
    {
        sint64 frame;

        hr = m_pMediaSeeking->GetCurrentPosition( &frame );
        Debug::Assert( Condition( SUCCEEDED( hr ) ), "Coult not get current position (0x%08x)", hr );

        time += frame;
    }

    hr = m_pMediaSeeking->SetPositions( &time, AM_SEEKING_AbsolutePositioning, NULL, AM_SEEKING_NoPositioning );
    Debug::Assert( Condition( SUCCEEDED( hr ) ), "Could not seek to absolute position (0x%08x)", hr );
}

void VideoPlayer::Show( void )
{
    m_Sprite.AddToScene( );
}

void VideoPlayer::Hide( void )
{
    m_Sprite.RemoveFromScene( );
}

float VideoPlayer::GetTime( void )
{
    //vmr isn't ready, this must have happened because the d3d device was lost
    //and we can't do anything until it is restored
    if ( NULL != m_pMediaSeeking )
    {
        sint64 frame;

        HRESULT hr = m_pMediaSeeking->GetCurrentPosition( &frame );
        Debug::Assert( Condition( SUCCEEDED( hr ) ), "Coult not get current position (0x%08x)", hr );

        m_LastRecordedFrame = frame;
    }

    //reftime is in 100nanosecond increments
    return ( m_LastRecordedFrame / 10000000.0f );
}

void VideoPlayer::DoEvents( void )
{
    //media events can stil possibly fire after the media event
    //is released or the class is destroyed
    if ( NULL == m_pMediaEvent ) return;

    long evCode;
    LONG_PTR param1, param2;

    while ( SUCCEEDED( m_pMediaEvent->GetEvent( &evCode, &param1, &param2, 0 ) ) )
    {
        m_pMediaEvent->FreeEventParams( evCode, param1, param2 );

        switch ( evCode )
        {
        case EC_COMPLETE:
            //case EC_USERABORT:
            //case EC_ERRORABORT:
        {
                            m_pChannel->SendEvent( "Finished", ArgList( ) );
                            return;
        }
        }
    }

}

void VideoPlayer::LoadVMR( void )
{
    HRESULT hr;

    hr = CoCreateInstance( CLSID_FilterGraph, NULL, CLSCTX_INPROC_SERVER, IID_IGraphBuilder, (void **) &m_pGraph );
    Debug::Assert( Condition( SUCCEEDED( hr ) ), "Query Graph error when building video stream (0x%08x)", hr );

    hr = CoCreateInstance( CLSID_VideoMixingRenderer9, NULL, CLSCTX_INPROC_SERVER, IID_IBaseFilter, (void **) &m_pVMR );
    Debug::Assert( Condition( SUCCEEDED( hr ) ), "Query VMR error when building video stream (0x%08x)", hr );

    //set renderless mode
    {
        IVMRFilterConfig9 *pConfig;

        hr = m_pVMR->QueryInterface( IID_IVMRFilterConfig9, (void**) &pConfig );
        Debug::Assert( Condition( SUCCEEDED( hr ) ), "Query Config error when building video stream (0x%08x)", hr );

        hr = pConfig->SetRenderingMode( VMR9Mode_Renderless );
        Debug::Assert( Condition( SUCCEEDED( hr ) ), "SetRenderingMode error when building video stream (0x%08x)", hr );

        hr = pConfig->SetNumberOfStreams( 1 );
        Debug::Assert( Condition( SUCCEEDED( hr ) ), "SetNumberOfStreams error when building video stream (0x%08x)", hr );

        pConfig->Release( );
    }

    //set surface to update each frame
    {
        IVMRSurfaceAllocatorNotify9 *lpIVMRSurfAllocNotify;

        hr = m_pVMR->QueryInterface( IID_IVMRSurfaceAllocatorNotify9, reinterpret_cast<void**>( &lpIVMRSurfAllocNotify ) );
        Debug::Assert( Condition( SUCCEEDED( hr ) ), "Query SurfaceAllocNotify error when building video stream (0x%08x)", hr );

        // create our surface allocator
        m_pVideoSurface = new VideoSurface;

        Debug::Assert( Condition( false ), "This must be redone to use offline materials" );
        //m_pVideoSurface->Create( m_pMaterial->GetTexture( ), m_pChannel );
        m_pVideoSurface->AddRef( );

        // let the allocator and the notify know about each other
        hr = lpIVMRSurfAllocNotify->AdviseSurfaceAllocator( 1, m_pVideoSurface );
        Debug::Assert( Condition( SUCCEEDED( hr ) ), "AdviseSurfaceAllocator error when building video stream (0x%08x)", hr );

        m_pVideoSurface->AdviseNotify( lpIVMRSurfAllocNotify );

        lpIVMRSurfAllocNotify->Release( );
    }

    hr = m_pGraph->AddFilter( m_pVMR, L"Video" );
    Debug::Assert( Condition( SUCCEEDED( hr ) ), "Add Video filter error when building video stream (0x%08x)", hr );

    hr = m_pGraph->QueryInterface( IID_IMediaControl, (void **) &m_pMediaControl );
    Debug::Assert( Condition( SUCCEEDED( hr ) ), "Query Media Control error when building video stream (0x%08x)", hr );

    hr = m_pGraph->QueryInterface( IID_IMediaSeeking, (void **) &m_pMediaSeeking );
    Debug::Assert( Condition( SUCCEEDED( hr ) ), "Query Media Seeking error when building video stream (0x%08x)", hr );

    hr = m_pGraph->QueryInterface( IID_IMediaEventEx, (void **) &m_pMediaEvent );
    Debug::Assert( Condition( SUCCEEDED( hr ) ), "Query Media EventEx error when building video stream (0x%08x)", hr );

    //render out the video graph
    {
        char windowsPath[ MAX_PATH ];
#ifdef _DISTRIBUTION
        //distrubtion uses the installed path
        String::Format( windowsPath, sizeof( windowsPath ), "%s\\%s", s_VideoPath, m_VideoFile );
#else
        //developer uses the same directory as the executable
        String::Copy( windowsPath, m_VideoFile, sizeof( windowsPath ) );
#endif

        WCHAR wPath[ 256 ];
        MultiByteToWideChar( CP_UTF8, NULL, windowsPath, -1, wPath, sizeof( wPath ) / sizeof( WCHAR ) );

        hr = m_pGraph->RenderFile( wPath, NULL );
        Debug::Assert( Condition( SUCCEEDED( hr ) ), "RenderFile error when building video (%s) (0x%08x)", windowsPath, hr );
    }

    hr = m_pMediaEvent->SetNotifyFlags( 0 );
    Debug::Assert( Condition( SUCCEEDED( hr ) ), "Set Notify Flags error when building video stream (0x%08x)", hr );

    hr = m_pMediaEvent->SetNotifyWindow( (OAHWND) s_MainWindow, WM_VIDEOGRAPHNOTIFY, ( LONG_PTR ) this );
    Debug::Assert( Condition( SUCCEEDED( hr ) ), "Set Notify Window error when building video stream (0x%08x)", hr );

    //calling pause tells the video to render the first frame of the image
    //rather than staying at a black screen
    //wait until the true return so we know all filters have completed
    while ( S_FALSE == m_pMediaControl->Pause( ) )
    {
        Thread::Sleep( 1 );
    }

    //refresh material so the uv offsets can be updated
    m_Sprite.SetMaterial( ResourceHandle( m_pMaterial->GetId( ) ) );
}

void VideoPlayer::UnloadVMR( )
{
    m_pVideoSurface->Destroy( );

    m_pMediaEvent->SetNotifyWindow( NULL, 0, 0 );

    m_pMediaEvent->Release( );
    m_pMediaSeeking->Release( );
    m_pMediaControl->Release( );
    m_pVMR->Release( );
    m_pVideoSurface->Release( );
    m_pGraph->Release( );

    //we have to set this to null
    //incase DoEvents runs after Unload
    m_pMediaEvent = NULL;
    m_pMediaSeeking = NULL;
    m_pMediaControl = NULL;
    m_pVMR = NULL;
    m_pGraph = NULL;
    m_pVideoSurface = NULL;
}

void VideoPlayer::OnDeviceLost(
    const Channel *pSender,
    const char *pName,
    const ArgList &list
    )
{
    //force a record of the last frame
    //so we can restore it when coming back
    GetTime( );

    UnloadVMR( );
}

void VideoPlayer::OnDeviceRestored(
    const Channel *pSender,
    const char *pName,
    const ArgList &list
    )
{
    LoadVMR( );

    m_pMediaSeeking->SetPositions( &m_LastRecordedFrame, AM_SEEKING_AbsolutePositioning, NULL, AM_SEEKING_NoPositioning );
}
