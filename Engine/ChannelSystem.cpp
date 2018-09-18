#include "EnginePch.h"

#include "ChannelSystem.h"
#include "Channel.h"

ChannelSystem &ChannelSystem::Instance( )
{
   static ChannelSystem s_instance;
   return s_instance;
}

Channel *ChannelSystem::GetChannel(
   Id id
)
{
   Channel *pChannel = TryGetChannel( id );
   Debug::Assert( Condition(NULL != pChannel), "Channel %s does not exist", id.ToString() );

   return pChannel;
}

Channel *ChannelSystem::TryGetChannel(
   Id id
)
{
   Channel *pChannel;
   
   if ( m_ChannelHash.Get(id.ToString( ), &pChannel) )
   {
      return pChannel;
   }

   return NULL;
}

void ChannelSystem::Add(
   Channel *pChannel
)
{
   Debug::Assert( Condition(false == m_ChannelHash.Contains(pChannel->GetId().ToString())), "Channel %s already exists", pChannel->GetId().ToString() );
 
   m_ChannelHash.Add( pChannel->GetId( ).ToString( ), pChannel );
   m_ChannelList.Add( pChannel );
}

void ChannelSystem::Remove(
   Channel *pChannel
)
{
   m_ChannelList.Remove( pChannel );
   m_ChannelHash.Remove( pChannel->GetId( ).ToString( ) );
}

void ChannelSystem::ExecuteTypeMethod(
   const char *pName,
   const ArgList &list,
   const Channel *pChannel
)
{
   List<Channel *>::Enumerator e = m_ChannelList.GetEnumerator( );

   //this will go through all channels
   //and examine their heirarchy chain.  if there is a channel
   //in their chain which matches the incoming channel
   //then the method is sent across the current channel being enum'd
   
   //CLARITY NOTE:
   //all the channels may be checked multiple times
   //(because we go through the flat list plus the inheritance chain)
   //but executemethod will only be called one time for each
   //because we only call it on the current channel being enum'd

   while ( e.EnumNext( ) )
   {
      //don't resend to us
      if ( e.Data( ) == pChannel ) continue;

      //for each channel see if they point to us somewhere in
      //their inheritance chain
      Channel *pBase = e.Data( );

      while ( pBase )
      {
         //if one of our base channels is the incoming channel
         //then send the method across the top level channel
         if ( pBase == pChannel )
         {
            e.Data( )->ExecuteMethod( pName, list );
            break;
         }

         pBase = pBase->GetBaseChannel( );
      }
   }
}

void ChannelSystem::SetTypeProperty(
   const char *pName,
   const ArgList &list,
   const Channel *pChannel
)
{
   List<Channel *>::Enumerator e = m_ChannelList.GetEnumerator( );

   //this will go through all channels
   //and examine their heirarchy chain.  if there is a channel
   //in their chain which matches the incoming channel
   //then the method is sent across the current channel being enum'd
   
   //CLARITY NOTE:
   //all the channels may be checked multiple times
   //(because we go through the flat list plus the inheritance chain)
   //but SetProperty will only be called one time for each
   //because we only call it on the current channel being enum'd

   while ( e.EnumNext( ) )
   {
      //don't resend to us
      if ( e.Data( ) == pChannel ) continue;

      //for each channel see if they point to us somewhere in
      //their inheritance chain
      Channel *pBase = e.Data( );

      while ( pBase )
      {
         //if one of our base channels is the incoming channel
         //then send the method across the top level channel
         if ( pBase == pChannel )
         {
            e.Data( )->SetProperty( pName, list );
            break;
         }

         pBase = pBase->GetBaseChannel( );
      }
   }
}

void ChannelSystem::Flush( void )
{
   uint32 i, size = m_ChannelList.GetSize( );

   for ( i = 0; i < size; i++ )
   {
      m_ChannelList.GetAt( i )->Flush( );
   }
}
