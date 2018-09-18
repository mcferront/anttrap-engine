#include "EnginePch.h"

#include "VideoComponent.h"
#include "Database.h"

//a video manager is used as our buffer layer.  The video manager
//manages how many videos can play at one time and handles loading/unloading videos
//behind the scenes so the component can assume they are always loaded and interactable

DefineComponentType(VideoComponent, NULL);

void VideoComponent::Create(
    const char *pPath,
    Channel *pEventChannel
    )
{
    m_pVideo = VideoManager::Instance( ).LoadVideo( pPath, pEventChannel ); 
}

void VideoComponent::Destroy( void )
{  
    VideoManager::Instance( ).UnloadVideo( m_pVideo ); 
}

void VideoComponent::Play( void )
{
    VideoManager::Instance( ).PlayVideo( m_pVideo ); 
}

void VideoComponent::Stop( void )
{
    VideoManager::Instance( ).StopVideo( m_pVideo ); 
}

void VideoComponent::Seek( 
    VideoPlayer::Seek seek,
    float seconds
    )
{
    VideoManager::Instance( ).SeekVideo( m_pVideo, seek, seconds ); 
}

void VideoComponent::AddToScene( void )
{
    if ( false == GetParent()->IsInScene() ) return;
    VideoManager::Instance( ).ShowVideo( m_pVideo ); 
}

void VideoComponent::RemoveFromScene( void )
{
    VideoManager::Instance( ).HideVideo( m_pVideo ); 
}

float VideoComponent::GetTime( void )
{
    return VideoManager::Instance( ).GetVideoTime( m_pVideo ); 
}

VideoManager &VideoManager::Instance( void )
{
    static VideoManager s_instance;
    return s_instance;
}

void VideoManager::Create(
    const char *pVideoPackFile
    )
{
    m_AccessId  = 1;
    m_pDatabase = NULL;

    m_Videos.Create( );

    if ( NULL != pVideoPackFile )
    {
        Debug::Assert( Condition(false), "Vide Pack File not supported" );
        //String::Copy( m_VideoPackFile, pVideoPackFile, sizeof(m_VideoPackFile) );

        //m_pDatabase = Database::Create( );
        //m_pDatabase->LoadHeaders( m_VideoPackFile );
    }
}

void VideoManager::Destroy( void )
{
    if ( NULL != m_pDatabase )
        Database::Destroy( m_pDatabase );

    UnloadAll( );
    m_Videos.Destroy( );
}

VideoDesc *VideoManager::LoadVideo(
    const char *pPath,
    Channel *pChannel
    )
{
    uint32 offset = 0;
    uint32 size   = 0;

    //if mac define then look up file and offset in the pack video file
    //if windows or ios then just proceed with 0 offset
    if ( NULL != m_pDatabase )
    {
        bool result = m_pDatabase->GetResourceLocation( Id(pPath), &offset, &size );
        pPath  = m_VideoPackFile;

        Debug::Assert( Condition(true == result), "Could not find %s in the video pack file", pPath );
        (void) result;
    }

    //lookup path and offset here?
    VideoDesc *pDesc = new VideoDesc( pPath, offset, size, pChannel );
    m_Videos.Add( pDesc );

    return pDesc;
}

void VideoManager::UnloadVideo(
    VideoDesc *pDesc
    )
{
    pDesc->Unload( );

    m_Videos.Remove( pDesc );

    delete pDesc;
}

void VideoManager::PlayVideo(
    VideoDesc *pDesc
    )
{
    if ( true == pDesc->m_Visible )
    {
        pDesc->ShowVideo( );
    }

    pDesc->Play( );
    pDesc->m_AccessId = m_AccessId++;

    UnloadLowIds( pDesc );
}

void VideoManager::StopVideo(
    VideoDesc *pDesc
    )
{
    pDesc->Stop( );
}

void VideoManager::SeekVideo(
    VideoDesc *pDesc,
    VideoPlayer::Seek seek,
    float seconds
    )
{
    pDesc->Seek( seek, seconds );
    pDesc->m_AccessId = m_AccessId++;

    UnloadLowIds( pDesc );
}

void VideoManager::ShowVideo(
    VideoDesc *pDesc
    )
{
    pDesc->ShowVideo( );
    pDesc->m_AccessId = m_AccessId++;

    UnloadLowIds( pDesc );
}

void VideoManager::HideVideo(
    VideoDesc *pDesc
    )
{
    pDesc->HideVideo( );
}

float VideoManager::GetVideoTime(
    VideoDesc *pDesc
    )
{
    return pDesc->GetTime( );
}

void VideoManager::UnloadLowIds(
    VideoDesc *pDesc
    )
{
    uint32 highestCutoff = 0;

    List<VideoDesc*>::Enumerator e = m_Videos.GetEnumerator( );

    //find the 2 highest cutoffs
    while ( e.EnumNext( ) )
    {
        if ( e.Data( ) == pDesc ) continue;

        if ( e.Data( )->m_AccessId > highestCutoff )
        {
            highestCutoff = e.Data( )->m_AccessId;
        }
    }

    //unload the rest
    e = m_Videos.GetEnumerator( );

    while ( e.EnumNext( ) )
    {
        if ( e.Data( ) == pDesc ) continue;

        if ( e.Data( )->m_AccessId < highestCutoff )
        {
            e.Data( )->Unload( );
        }
    }
}

void VideoManager::UnloadAll( void )
{
    List<VideoDesc*>::Enumerator e = m_Videos.GetEnumerator( );

    while ( e.EnumNext( ) )
    {
        e.Data( )->Unload( );
    }
}

VideoDesc::VideoDesc(
    const char *pPath,
    uint32 offset,
    uint32 size,
    Channel *pChannel
    )
{
    String::Copy( m_Path, pPath, sizeof(m_Path) );
    m_pChannel = pChannel;

    m_Visible     = false;
    m_pPlayer     = NULL;
    m_CurrentTime = 0.0f;
    m_AccessId    = 0;
    m_Offset      = offset;
    m_Size        = size;
}

void VideoDesc::Unload( void )
{
    if ( m_pPlayer )
    {
        m_CurrentTime = GetTime( );

        m_pPlayer->Unload( );
        delete m_pPlayer;

        m_pPlayer = NULL;
    }
}


void VideoDesc::Play( void )
{
    LoadPlayer( );
    m_pPlayer->Play( );
}

void VideoDesc::Stop( void )
{
    if ( NULL != m_pPlayer )
    {
        m_pPlayer->Stop( );
    }
}

void VideoDesc::Seek(
    VideoPlayer::Seek seek,
    float seconds
    )
{
    LoadPlayer( );
    m_pPlayer->SeekTo( seek, seconds );
}

void VideoDesc::ShowVideo( void )
{   
    m_Visible = true;

    LoadPlayer( );

    //force to top
    m_pPlayer->Hide( );
    m_pPlayer->Show( );
}

void VideoDesc::HideVideo( void )
{   
    if ( NULL != m_pPlayer )
    {
        m_pPlayer->Hide( );
    }

    m_Visible = false;
}

float VideoDesc::GetTime( void )
{
    if ( NULL != m_pPlayer )
    {
        m_CurrentTime = m_pPlayer->GetTime( );
    }

    return m_CurrentTime;
}


void VideoDesc::LoadPlayer( void )
{  
    if ( NULL == m_pPlayer )
    {
        m_pPlayer = new VideoPlayer;
        m_pPlayer->Load( m_Path, m_Offset, m_Size, m_pChannel );
        m_pPlayer->SeekTo( VideoPlayer::SeekBegin, m_CurrentTime );
    }
}
