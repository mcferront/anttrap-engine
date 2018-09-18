#pragma once

#include "EngineGlobal.h"
#include "List.h"
#include "HashTable.h"
#include "SystemId.h"

class ArgList;
class Channel;

class ChannelSystem
{
public:
   static ChannelSystem &Instance( );

public:
   typedef List<Channel *> ChannelList;
   typedef HashTable<const char *, Channel *> ChannelHash;

private:
   ChannelList m_ChannelList;
   ChannelHash m_ChannelHash;

public:
   void Create ( void ) 
   {
      m_ChannelList.Create( );
      m_ChannelHash.Create( );
   }

   void Destroy( void )
   {
      m_ChannelList.Destroy( );
      m_ChannelHash.Destroy( );
   }

   Channel *GetChannel(
      Id id
   );

   Channel *TryGetChannel(
      Id id
   );

   void Add(
      Channel *pChannel
   );
   
   void Remove(
      Channel *pChannel
   );

   //executes the method on any channel
   //which has us in the inheritance chain.
   void ExecuteTypeMethod(
      const char *pName,
      const ArgList &list,
      const Channel *pBaseChannel
   );

   //executes the method on any channel
   //which has us in the inheritance chain.
   void SetTypeProperty(
      const char *pName,
      const ArgList &list,
      const Channel *pBaseChannel
   );


   void Flush( void );
};
