#include "GamePch.h"
//#include "EditorConnection.h"
//
//void EditorConnection::Create( void )
//{
//    Resource::Create( Id("EditorConnection") );
//
//    m_pUdpStream = NULL;
//    m_pTcpStream = NULL;
//
//    m_Clock.Start( );
//
//    CreateBroadcast( );
//
//    Resource::AddToScene( );
//}
//
//void EditorConnection::Destroy( void )
//{
//    Resource::RemoveFromScene( );
//
//    if ( NULL != m_pUdpStream )
//    {
//        m_pUdpStream->GetChannel( )->RemoveEventListener( EventMap<EditorConnection>("Broadcast", this, &EditorConnection::OnBroadcastReceived) );
//        m_pUdpStream->Destroy( );
//        delete m_pUdpStream;
//        m_pUdpStream = NULL;
//    }
//
//    if ( NULL != m_pTcpStream )
//    {
//        m_LiveUpdate.Destroy( );
//
//        OnConnected( false );
//    
//        m_pTcpStream->Destroy( );
//        delete m_pTcpStream;
//        m_pTcpStream = NULL;
//    }
//
//    Resource::Destroy( );
//}
//
//void EditorConnection::Update( void )
//{
//    if ( NULL != m_pUdpStream )
//    {
//        if ( NULL != m_pTcpStream )
//        {
//            // Destroy udp listener
//            m_pUdpStream->GetChannel( )->RemoveEventListener( EventMap<EditorConnection>("Broadcast", this, &EditorConnection::OnBroadcastReceived) );
//            m_pUdpStream->Destroy( );
//
//            delete m_pUdpStream;
//            m_pUdpStream = NULL;
//        }
//        else
//        {
//            if (m_Clock.TestSample() > 1.0f)
//            {
//                PlatformBroadcast broadcast = { PlatformType };
//                m_pUdpStream->SendStream( "Broadcast", &broadcast, sizeof(broadcast) );
//            
//                m_Clock.Reset( );
//            }
//
//            m_pUdpStream->Update( 1 / 1000.0f );
//        }
//    }
//
//    if ( NULL != m_pTcpStream )
//    {
//        if (false == m_pTcpStream->IsConnected())
//            Disconnect( );
//        else
//        {
//            m_pTcpStream->Update( 1 / 1000.0f );
//            m_LiveUpdate.Update( );
//        }
//    }
//}
//
//void EditorConnection::Disconnect( void )
//{
//    if ( NULL == m_pTcpStream )
//        return;
//   
//    m_LiveUpdate.Destroy( );
//
//    OnConnected( false );
//            
//    m_pTcpStream->Destroy( );
//    delete m_pTcpStream;
//    m_pTcpStream = NULL;
//    CreateBroadcast( );
//}
//
//void EditorConnection::OnBroadcastReceived( 
//    const Channel *pChannel,
//    const char *, 
//    const ArgList &args
//)
//{
//    //Already have a connection and just haven't torn down
//    //the broadcast listener yet
//    if ( NULL != m_pTcpStream )
//        return;
//
//    MemoryStream *pStream;
//    args.GetArg<MemoryStream*>( 0, &pStream );
//
//    EditorBroadcast header;
//    pStream->Read( &header, sizeof(header) );
//
//    // create tcp connection
//    TcpIpSocket *pSocket = new TcpIpSocket;
//    pSocket->Create( ((UdpSocket *)m_pUdpStream->GetSocket())->GetRecvFromAddress().ToString( ), header.port );
//    pSocket->Connect( );
//
//    m_pTcpStream = new PipeStream;
//    m_pTcpStream->Create( false );
//    m_pTcpStream->Start( pSocket );
//    m_pTcpStream->RegisterName( "EditorMode", LiveUpdate::MessageId::EditorMode );
//
//    m_pTcpStream->RegisterName( "AssetModified", LiveUpdate::MessageId::AssetModified );
//    m_pTcpStream->RegisterName( "AssetRequest", LiveUpdate::MessageId::AssetRequest );
//    m_pTcpStream->RegisterName( "AssetPush", LiveUpdate::MessageId::AssetPush );
//
//    m_pTcpStream->RegisterName( "SelectionModified", LiveUpdate::MessageId::SelectionModified );
//
//    m_pTcpStream->RegisterName( "ScenePush", LiveUpdate::MessageId::ScenePush );
//    m_pTcpStream->RegisterName( "SceneRemove", LiveUpdate::MessageId::SceneRemove );
//
//    m_pTcpStream->RegisterName( "NodePush", LiveUpdate::MessageId::NodePush );
//    m_pTcpStream->RegisterName( "NodeRemove", LiveUpdate::MessageId::NodeRemove );
//
//    m_pTcpStream->RegisterName( "PackageRequest", LiveUpdate::MessageId::PackageRequest );
//    m_pTcpStream->RegisterName( "PackagePush", LiveUpdate::MessageId::PackagePush );
//
//    m_pTcpStream->RegisterName( "RunBuild", LiveUpdate::MessageId::RunBuild );
//
//    m_pTcpStream->RegisterName( "Pick", LiveUpdate::MessageId::Pick );
//    m_pTcpStream->RegisterName( "PickingResult", LiveUpdate::MessageId::PickingResult );
//    m_pTcpStream->RegisterName( "FixedUpdate", LiveUpdate::MessageId::FixedUpdate );
//    m_pTcpStream->RegisterName( "Step", LiveUpdate::MessageId::Step );
//    m_pTcpStream->RegisterName( "Pause", LiveUpdate::MessageId::Pause );
//    m_pTcpStream->RegisterName( "Broadcast", LiveUpdate::MessageId::Broadcast );
//
//    ConnectionHeader connectionHeader;
//    String::Copy( connectionHeader.connectionId, header.connectionId, sizeof(connectionHeader.connectionId));
//    String::Copy( connectionHeader.platform, PlatformType, sizeof(connectionHeader.platform));
//
//    m_pTcpStream->SendStream("Broadcast", &connectionHeader, sizeof(connectionHeader));
//    
//    OnConnected( true );
//    
//    m_LiveUpdate.Create( this, SendData );
//}
//
//void EditorConnection::OnMessageReceived( 
//    const Channel *pChannel,
//    const char *pName, 
//    const ArgList &args
//)
//{
//    MemoryStream *pStream;
//    args.GetArg<MemoryStream*>( 0, &pStream );
//
//    if ( 0 == strcmp(pName, "NodePush") )
//        m_LiveUpdate.NodePush( pStream );
//    else if ( 0 == strcmp(pName, "NodeRemove") )
//        m_LiveUpdate.NodeRemove( pStream );
//    else if ( 0 == strcmp(pName, "ScenePush") )
//        m_LiveUpdate.ScenePush( pStream );
//    else if ( 0 == strcmp(pName, "SceneRemove") )
//        m_LiveUpdate.SceneRemove( pStream );
//    else if ( 0 == strcmp(pName, "AssetModified") )
//        m_LiveUpdate.AssetModified( pStream );
//    else if ( 0 == strcmp(pName, "AssetPush") )
//        m_LiveUpdate.AssetPush( pStream );
//    else if ( 0 == strcmp(pName, "RunBuild") )
//        m_LiveUpdate.RunBuild( pStream );
//    else if ( 0 == strcmp(pName, "PackagePush") )
//        m_LiveUpdate.PackagePush( pStream );
//    else if ( 0 == strcmp(pName, "Step") )
//        m_LiveUpdate.Step( pStream );
//    else if ( 0 == strcmp(pName, "Pause") )
//        m_LiveUpdate.Pause( pStream );
//    else if ( 0 == strcmp(pName, "EditorMode") )
//        m_LiveUpdate.EditorMode( pStream );
//}
//
//void EditorConnection::CreateBroadcast( void )
//{
//    m_pUdpStream = new PipeStream;
//    m_pUdpStream->Create( false );
//    m_pUdpStream->RegisterName( "Broadcast", LiveUpdate::MessageId::Broadcast );
//    m_pUdpStream->GetChannel( )->AddEventListener( EventMap<EditorConnection>("Broadcast", this, &EditorConnection::OnBroadcastReceived) );
//
//    UdpSocket *pSocket = new UdpSocket;
//    pSocket->Create( NULL, (uint16) EditorConnection::BroadcastPort );
//    pSocket->Open( );
//
//    m_pUdpStream->Start( pSocket );
//
//    Debug::Print( Debug::TypeInfo, "Broadcasting IP..." );
//}
//
//void EditorConnection::OnConnected( 
//    bool connected
//)
//{
//    if (true == connected)
//    {
//        GetChannel( )->SendEvent( "Connected", ArgList() );
//        m_pTcpStream->GetChannel( )->AddEventListener( EventMap<EditorConnection>("NodePush", this, &EditorConnection::OnMessageReceived) );
//        m_pTcpStream->GetChannel( )->AddEventListener( EventMap<EditorConnection>("NodeRemove", this, &EditorConnection::OnMessageReceived) );
//        m_pTcpStream->GetChannel( )->AddEventListener( EventMap<EditorConnection>("ScenePush", this, &EditorConnection::OnMessageReceived) );
//        m_pTcpStream->GetChannel( )->AddEventListener( EventMap<EditorConnection>("SceneRemove", this, &EditorConnection::OnMessageReceived) );
//        m_pTcpStream->GetChannel( )->AddEventListener( EventMap<EditorConnection>("SelectionModified", this, &EditorConnection::OnMessageReceived) );
//        m_pTcpStream->GetChannel( )->AddEventListener( EventMap<EditorConnection>("AssetModified", this, &EditorConnection::OnMessageReceived) );
//        m_pTcpStream->GetChannel( )->AddEventListener( EventMap<EditorConnection>("AssetPush", this, &EditorConnection::OnMessageReceived) );
//        m_pTcpStream->GetChannel( )->AddEventListener( EventMap<EditorConnection>("PackageLoad", this, &EditorConnection::OnMessageReceived) );
//        m_pTcpStream->GetChannel( )->AddEventListener( EventMap<EditorConnection>("PackagePush", this, &EditorConnection::OnMessageReceived) );
//        m_pTcpStream->GetChannel( )->AddEventListener( EventMap<EditorConnection>("RunBuild", this, &EditorConnection::OnMessageReceived) );
//        m_pTcpStream->GetChannel( )->AddEventListener( EventMap<EditorConnection>("EditorMode", this, &EditorConnection::OnMessageReceived) );
//        m_pTcpStream->GetChannel( )->AddEventListener( EventMap<EditorConnection>("PickingResult", this, &EditorConnection::OnMessageReceived) );
//        m_pTcpStream->GetChannel( )->AddEventListener( EventMap<EditorConnection>("Pick", this, &EditorConnection::OnMessageReceived) );
//        m_pTcpStream->GetChannel( )->AddEventListener( EventMap<EditorConnection>("FixedUpdate", this, &EditorConnection::OnMessageReceived) );
//        m_pTcpStream->GetChannel( )->AddEventListener( EventMap<EditorConnection>("Pause", this, &EditorConnection::OnMessageReceived) );
//        m_pTcpStream->GetChannel( )->AddEventListener( EventMap<EditorConnection>("Step", this, &EditorConnection::OnMessageReceived) );
//    }
//    else
//    {
//        GetChannel( )->SendEvent( "Disconnected", ArgList() );
//        m_pTcpStream->GetChannel( )->RemoveEventListener( EventMap<EditorConnection>("NodePush", this, &EditorConnection::OnMessageReceived) );
//        m_pTcpStream->GetChannel( )->RemoveEventListener( EventMap<EditorConnection>("NodeRemove", this, &EditorConnection::OnMessageReceived) );
//        m_pTcpStream->GetChannel( )->RemoveEventListener( EventMap<EditorConnection>("ScenePush", this, &EditorConnection::OnMessageReceived) );
//        m_pTcpStream->GetChannel( )->RemoveEventListener( EventMap<EditorConnection>("SceneRemove", this, &EditorConnection::OnMessageReceived) );
//        m_pTcpStream->GetChannel( )->RemoveEventListener( EventMap<EditorConnection>("SelectionModified", this, &EditorConnection::OnMessageReceived) );
//        m_pTcpStream->GetChannel( )->RemoveEventListener( EventMap<EditorConnection>("AssetModified", this, &EditorConnection::OnMessageReceived) );
//        m_pTcpStream->GetChannel( )->RemoveEventListener( EventMap<EditorConnection>("AssetPush", this, &EditorConnection::OnMessageReceived) );
//        m_pTcpStream->GetChannel( )->RemoveEventListener( EventMap<EditorConnection>("PackageLoad", this, &EditorConnection::OnMessageReceived) );
//        m_pTcpStream->GetChannel( )->RemoveEventListener( EventMap<EditorConnection>("PackagePush", this, &EditorConnection::OnMessageReceived) );
//        m_pTcpStream->GetChannel( )->RemoveEventListener( EventMap<EditorConnection>("RunBuild", this, &EditorConnection::OnMessageReceived) );
//        m_pTcpStream->GetChannel( )->RemoveEventListener( EventMap<EditorConnection>("EditorMode", this, &EditorConnection::OnMessageReceived) );
//        m_pTcpStream->GetChannel( )->RemoveEventListener( EventMap<EditorConnection>("PickingResult", this, &EditorConnection::OnMessageReceived) );
//        m_pTcpStream->GetChannel( )->RemoveEventListener( EventMap<EditorConnection>("Pick", this, &EditorConnection::OnMessageReceived) );
//        m_pTcpStream->GetChannel( )->RemoveEventListener( EventMap<EditorConnection>("FixedUpdate", this, &EditorConnection::OnMessageReceived) );
//        m_pTcpStream->GetChannel( )->RemoveEventListener( EventMap<EditorConnection>("Pause", this, &EditorConnection::OnMessageReceived) );
//        m_pTcpStream->GetChannel( )->RemoveEventListener( EventMap<EditorConnection>("Step", this, &EditorConnection::OnMessageReceived) );
//    }
//}
//
//void EditorConnection::SendData(
//    uint32 id, 
//    const void *pData, 
//    uint32 size
//)
//{
//    m_pTcpStream->SendStream( id, pData, size );
//}
//
//void EditorConnection::SendData(
//    void *pContext, 
//    uint32 id, 
//    const void *pData, 
//    uint32 size
//)
//{
//    EditorConnection *pEc = (EditorConnection *) pContext;
//    pEc->SendData( id, pData, size );
//}
