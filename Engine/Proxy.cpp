#include "EnginePch.h"

#include "Proxy.h"
#include "TcpIpPipe.h"
#include "ResourceMaps.h"
#include "MemoryStreams.h"
#include "ChannelSystem.h"

DefineResourceType(ProxyResource, Resource, NULL);

void ProxyChannel::GetProperty( 
   const char *pProperty, 
   ArgList &list
)
{
   Debug::Assert( Condition(false), "ProxyChannels cannot use this function" );
}

void ProxyChannel::AddMethod(
   const IMethodMap &methodMap
)
{
   Debug::Assert( Condition(false), "ProxyChannels cannot use this function" );
}

void ProxyChannel::RemoveMethod(
   const IMethodMap &methodMap
)
{
   Debug::Assert( Condition(false), "ProxyChannels cannot use this function" );
}

void ProxyChannel::AddGetProperty(
   const IGetPropertyMap &propertyMap
)
{
   Debug::Assert( Condition(false), "ProxyChannels cannot use this function" );
}

void ProxyChannel::AddSetProperty(
   const ISetPropertyMap &propertyMap
)
{
   Debug::Assert( Condition(false), "ProxyChannels cannot use this function" );
}

void ProxyChannel::ExecuteMethod( 
   const char *pMethod, 
   const ArgList &list
)
{
   ProxyHeader proxyHeader;

   //write the return instance's addresss
   //and the arg list to our network
   String::Copy( proxyHeader.command, pMethod, sizeof(proxyHeader.command) );
   String::Copy( proxyHeader.id, GetId( ).ToString(), sizeof(proxyHeader.id) );

   proxyHeader.type      = ProxyHeader::ExecuteMethod;

   MemoryStream stream;   
   stream.Write( &proxyHeader, sizeof(proxyHeader) );

   Serializer serializer( stream.GetOutputStream() );
   serializer.Serialize( &list );

   //Debug::Print( Debug::TypeInfo, "ExecuteMethod (Remote): %s", pMethod );

   m_pPipe->SendStream( "GameHub", stream.GetBuffer( ), stream.GetAmountWritten( ) );
}

void ProxyChannel::SetProperty( 
   const char *pProperty, 
   const ArgList &list
)
{
   ProxyHeader proxyHeader;

   //write the return instance's addresss
   //and the arg list to our network
   String::Copy( proxyHeader.command, pProperty, sizeof(proxyHeader.command) );
   String::Copy( proxyHeader.id, GetId( ).ToString(), sizeof(proxyHeader.id) );

   proxyHeader.type       = ProxyHeader::SetProperty;

   MemoryStream stream;   
   stream.Write( &proxyHeader, sizeof(proxyHeader) );

   Serializer serializer( stream.GetOutputStream() );
   serializer.Serialize( &list );

   //Debug::Print( Debug::TypeInfo, "SetProperty (Remote): %s", pProperty );

   m_pPipe->SendStream( "GameHub", stream.GetBuffer( ), stream.GetAmountWritten( ) );
}

void ProxyChannel::AddEventListener(
   const IEventMap &eventMap
)
{
   ProxyHeader proxyHeader;

   //write the return instance's addresss
   //and the arg list to our network
   String::Copy( proxyHeader.command, eventMap.GetName( ), sizeof(proxyHeader.command) );
   String::Copy( proxyHeader.id, GetId( ).ToString(), sizeof(proxyHeader.id) );

   proxyHeader.type       = ProxyHeader::AddEventListener;

   MemoryStream stream;
   stream.Write( &proxyHeader, sizeof(proxyHeader) );

   m_pPipe->SendStream( "GameHub", stream.GetBuffer( ), stream.GetAmountWritten( ) );

   Channel::AddEventListener( eventMap );
}

Channel *ProxyResource::CreateChannel( void )
{
   ProxyChannel *pChannel = new ProxyChannel;
   pChannel->Create( Id( GetId() ), GetType().GetChannel() );

   return pChannel;
}
