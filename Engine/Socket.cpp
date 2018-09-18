#include "EnginePch.h"

#include "Socket.h"
#include "PipeStreams.h"

int Socket::GetLastError( void )
{
    int result;

#ifdef WIN32
    result = WSAGetLastError( );
#elif defined IOS || defined LINUX || defined MAC || defined ANDROID
    result = errno;
#else
#error "Platform not defined"
#endif

    return result;
}

void Socket::Close( void )
{
    if ( INVALID_SOCKET != m_Socket )
        Socket::Cleanup( m_Socket );

    m_Socket = INVALID_SOCKET;
}

void Socket::InitiateGracefulClose( void )
{
    //tell our remote socket we're shutting down
    //and then we'll receive a close from it on the next Read call
    //which will call our Close function
#if defined WIN32
    shutdown( m_Socket, SD_SEND );
#elif defined IOS || defined LINUX || defined MAC || defined ANDROID 
    shutdown( m_Socket, SHUT_WR );
#else
#error "Platform not defined"
#endif
}

void Socket::Cleanup(
    SOCKET socket
    )
{
#if defined WIN32
    shutdown( socket, SD_BOTH );
    closesocket( socket );
#elif defined IOS || defined LINUX || defined MAC || defined ANDROID
    shutdown( socket, SHUT_RDWR );
    close( socket );
#else
#error "Platform not defined"
#endif
}

void TcpIpSocket::Create(
    const char *pHostname,
    uint16 port
    )
{
    m_pHostname = NULL;

    if ( pHostname )
    {
        size_t length = strlen( pHostname ) + 1;
        m_pHostname = (char *) malloc( length );

        String::Copy( m_pHostname, pHostname, length );
    }

    m_Port = port;
    m_Socket = INVALID_SOCKET;

    m_ReadBufferSize = 64 * 1024;
    m_pReadBuffer = malloc( m_ReadBufferSize );
}

void TcpIpSocket::Create(
    SOCKET socket,
    const sockaddr_in &address
    )
{
    m_pHostname   = NULL;
    m_Port        = 0;
    m_Socket      = socket;

    m_ReadBufferSize = 64 * 1024;
    m_pReadBuffer = malloc( m_ReadBufferSize );

    memcpy( &m_Address, &address, sizeof(m_Address) );
}

void TcpIpSocket::Destroy( void )
{
    free( m_pHostname );

    free( m_pReadBuffer );
    
    Close( );
}

void TcpIpSocket::Write(
    IPipeSendStream *pStream
    )
{
    char buffer[ 1024 ];
    uint32 bytesRead;

    pStream->Peek( buffer, sizeof(buffer), &bytesRead );

    if ( bytesRead > 0 )
    {
        int result = send( m_Socket, buffer, (int) bytesRead, 0 );

        if ( SOCKET_ERROR == result )
        {
            result = GetLastError( );

            if ( WSAEWOULDBLOCK != result )
            {
                Close( );
            }

            result = 0;
        }

        pStream->Advance( result );
    }
}

void TcpIpSocket::Read(
    IPipeRecvStream *pStream
    )
{
    int result = recv( m_Socket, (char *) m_pReadBuffer, (int) m_ReadBufferSize, 0 );

    if ( 0 == result )
    {
        Close( );
    }
    else if ( SOCKET_ERROR == result )
    {
        result = GetLastError( );

        if ( WSAEWOULDBLOCK != result )
        {
            Close( );
        }

        result = 0;
    }
    else
    {
        pStream->Write( m_pReadBuffer, result );
    }
}


void TcpIpSocket::Connect( void )
{
    Debug::Assert( Condition(NULL != m_pHostname), "Cannot call Open on a socket with a null hostname" );

    sockaddr_in addr = { 0 };

    ADDRINFOA *pAddrInfo;

    int result = getaddrinfo( m_pHostname, NULL, NULL, &pAddrInfo );
     
    if ( 0 == result && NULL != pAddrInfo->ai_addr )
    {
        char *pIp = inet_ntoa (*(struct in_addr *) pAddrInfo->ai_addr);

        addr.sin_family = AF_INET;
        addr.sin_addr.s_addr = inet_addr( pIp );
        addr.sin_port = htons( m_Port );

        SOCKET newSocket = socket( AF_INET, SOCK_STREAM, IPPROTO_TCP );

        int result = connect( newSocket, (sockaddr *) &addr, sizeof(addr) );

        if ( SOCKET_ERROR == result )
            Socket::Cleanup( newSocket );
        else
        {
            m_Socket = newSocket;

            memcpy( &m_Address, &addr, sizeof(m_Address) );

            //set to non blocking
            u_long nonBlocking = 1;
            ioctlsocket( m_Socket, FIONBIO, &nonBlocking );
        }
    }
}

void UdpSocket::Create(
    const char *pAddress,
    uint16 port
    )
{
    m_Address = Id( pAddress );
    m_RecvFromAddress[ 0 ] = 0;

    m_Port = port;

    m_Socket = INVALID_SOCKET;
    m_pBuffer = NULL;
    m_MaxPacketSize = 0;
}

void UdpSocket::Destroy( void )
{
    Close( );

    free( m_pBuffer );
    m_pBuffer = NULL;

    m_MaxPacketSize = 0;
}

void UdpSocket::Write(
    IPipeSendStream *pStream
    )
{
    uint32 bytesRead;
    
    pStream->Peek( m_pBuffer, m_MaxPacketSize, &bytesRead );
    if ( bytesRead == 0 ) return;

    int result = Write( m_pBuffer, bytesRead );

    if ( result > 0 )
        pStream->Advance( result );

    pStream->Advance( result );
}

void UdpSocket::Read(
    IPipeRecvStream *pStream
    )
{
    sockaddr_in address;
    socklen_t length = sizeof(address);

    int result = recvfrom( m_Socket, (char *) m_pBuffer, m_MaxPacketSize, 0, (sockaddr *) &address, &length );

    if ( 0 == result )
        Close( );
    else if ( SOCKET_ERROR == result )
    {
        result = GetLastError( );

        if ( WSAEWOULDBLOCK != result )
            Close( );

        result = 0;
    }
    else
    {
        char *pAddress = inet_ntoa(((sockaddr_in *)&address)->sin_addr);
        pStream->Write( m_pBuffer, result );

        {
            //TODO: A better way of saving the receive from address
            ScopeLock lock(m_AddressLock);
            String::Copy( m_RecvFromAddress, pAddress, sizeof(m_RecvFromAddress) );
        }
    }
}

void UdpSocket::Open( void )
{
    m_Socket = socket( AF_INET, SOCK_DGRAM, IPPROTO_UDP );
    Debug::Print( Condition(m_Socket != INVALID_SOCKET), Debug::TypeError, "socket() failed to create socket (%d)\n", this->GetLastError() );

    int success;

    //set to non blocking
    u_long nonBlocking = 1;
    success = ioctlsocket( m_Socket, FIONBIO, &nonBlocking );
    Debug::Print( Condition(success != SOCKET_ERROR), Debug::TypeError, "ioctlsocket failed to set to non blocking (%d)\n", this->GetLastError() );

#ifdef WIN32
    int optVal, optLen = sizeof(int);
    success = getsockopt( m_Socket, SOL_SOCKET, SO_MAX_MSG_SIZE, (char *) &optVal, &optLen);
    Debug::Print( Condition(success != SOCKET_ERROR), Debug::TypeError, "getsockopt failed to get max message size (%d)\n", this->GetLastError() );
#else
   int optVal = 64 * 1024;
#endif

    m_MaxPacketSize = optVal;

    m_pBuffer = malloc( m_MaxPacketSize );

    if ( m_Address == Id::Empty )
    {
    #ifdef WIN32
        char val = 1;
    #else
        int val = 1;
    #endif
        success = setsockopt( m_Socket, SOL_SOCKET, SO_BROADCAST, &val, sizeof(val) );
        Debug::Print( Condition(success != SOCKET_ERROR), Debug::TypeError, "setsockopt failed to set broadcast (%d)\n", this->GetLastError() );

        sockaddr_in address;

        memset( &address, 0, sizeof(address) );
        address.sin_family = AF_INET;
        address.sin_port = htons(m_Port + 1);
        address.sin_addr.s_addr = INADDR_ANY;

        success = bind( m_Socket, (sockaddr *) &address, sizeof(address) );
        Debug::Print( Condition(success != SOCKET_ERROR), Debug::TypeError, "bind failed (%d)\n", this->GetLastError() );
    }
}

int UdpSocket::Write(
    const void *pBuffer,
    int size
)
{
    sockaddr_in address;

    memset( &address, 0, sizeof(address) );
    address.sin_family = AF_INET;
    address.sin_port = htons(m_Port);
    
    if (m_Address == Id::Empty)
        address.sin_addr.s_addr = inet_addr("255.255.255.255");
    else
        address.sin_addr.s_addr = inet_addr(m_Address.ToString());

    int result = sendto( m_Socket, (const char *) pBuffer, size, 0, (sockaddr *) &address, sizeof(address) );

    if ( SOCKET_ERROR == result )
    {
        result = GetLastError( );

        if ( WSAEWOULDBLOCK != result )
            Close( );

        result = 0;
    }

    return result;
}