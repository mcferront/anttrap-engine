#pragma once

#include "EngineGlobal.h"
#include "SystemId.h"

class IPipeSendStream;
class IPipeRecvStream;
class IMethodMap;
struct hostent;

#if defined IOS || defined LINUX || defined MAC || defined ANDROID
typedef int SOCKET;

#define INVALID_SOCKET (-1)
#define SOCKET_ERROR   (-1)
#define WSAEWOULDBLOCK EWOULDBLOCK
#define ioctlsocket    ioctl
#endif

#ifdef LINUX
#include <arpa/inet.h>  
#endif

#ifdef WIN32
typedef int socklen_t;
#endif

class Socket
{
protected:
    SOCKET m_Socket;

public:
    virtual ~Socket( void ) {};
   
    virtual void Destroy( void ) = 0;

    virtual void Write(
        IPipeSendStream *pStream
        ) = 0;

    virtual void Read(
        IPipeRecvStream *pStream
        ) = 0;

public:
    int GetLastError( void );

    void Close( void );

    void InitiateGracefulClose( void );

    bool IsConnected( void ) const { return INVALID_SOCKET != m_Socket; }

protected:
    static void Cleanup(
        SOCKET socket
        );
};

class TcpIpSocket : public Socket
{
private:
    sockaddr_in  m_Address;
    char        *m_pHostname;
    uint16       m_Port;
    void        *m_pReadBuffer;
    int          m_ReadBufferSize;

public:
    void Create(
        const char *pHostname,
        uint16 port
        );

    void Create(
        SOCKET socket,
        const sockaddr_in &address
        );

    virtual void Destroy( void );

    virtual void Write(
        IPipeSendStream *pStream
        );

    virtual void Read(
        IPipeRecvStream *pStream
        );

    void Connect( void );

#ifdef WIN32
    uint32 GetAddress( void ) const { return m_Address.sin_addr.S_un.S_addr; }
#else
    uint32 GetAddress( void ) const { return m_Address.sin_addr.s_addr; }
#endif
};

class UdpSocket : public Socket
{
private:
    Lock    m_AddressLock;
    Id      m_Address;
    uint16  m_Port;
    void   *m_pBuffer;
    int     m_MaxPacketSize;
    char    m_RemoteAddress[ 64 ];
    char    m_RecvFromAddress[ 64 ];

public:
    void Create(
        const char *pAddress,
        uint16 port
        );

    virtual void Destroy( void );

    virtual void Write(
        IPipeSendStream *pStream
        );

    virtual void Read(
        IPipeRecvStream *pStream
        );

    void Open( void );

    Id GetRecvFromAddress( void )
    { 
        ScopeLock lock(m_AddressLock);
        return Id(m_RecvFromAddress); 
    }

private:
    int Write(
        const void *pBuffer,
        int size
    );
};
