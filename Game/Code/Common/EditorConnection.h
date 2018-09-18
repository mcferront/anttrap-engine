//#pragma once
//
//#include "Global.h"
//
//#include "ResourceWorld.h"
//#include "TcpIpStream.h"
//#include "LiveUpdate.h"
//#include "UtilityClock.h"
//
//class EditorConnection : public Resource
//{
//public:
//    struct PlatformBroadcast
//    {
//        char platform[ 8 ];
//    };
//
//    struct EditorBroadcast
//    {
//        short port;
//        char connectionId[ 42 ];
//    };
//
//    struct ConnectionHeader
//    {
//        char connectionId[ 42 ];
//        char platform[ 8 ];
//    };
//
//    struct DependencyDesc
//    {
//        DependencyDesc()
//        {
//            dependencies.Create();
//        }
//
//        ~DependencyDesc()
//        {
//            dependencies.Destroy();
//        }
//
//        Resource *pResource;
//        ResourceHandleList dependencies;
//    };
//
//    static const uint16 BroadcastPort = 10001;
//
//private:
//    PipeStream *m_pUdpStream;
//    PipeStream *m_pTcpStream;
//    LiveUpdate  m_LiveUpdate;
//    Clock       m_Clock;
//
//public:
//    void Create( void );
//    void Destroy( void );
//    
//    void Update( void );
//
//    void Disconnect( void );
//
//private:
//    void OnBroadcastReceived( 
//        const Channel *pChannel,
//        const char *, 
//        const ArgList &args
//    );
//
//    void OnMessageReceived( 
//        const Channel *pChannel,
//        const char *, 
//        const ArgList &args
//    );
//
//    void CreateBroadcast( void );
//
//    void OnConnected( 
//        bool connected
//    );
//   
//    void SendData(
//        uint32 id, 
//        const void *pData, 
//        uint32 size
//    );
//
//private:
//    static void SendData(
//        void *pContext, 
//        uint32 id, 
//        const void *pData, 
//        uint32 size
//    );
//
//};