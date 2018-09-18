#pragma once

#include "EngineGlobal.h"
#include "Resource.h"
#include "List.h"

class Pipe;

#pragma pack(1)
struct ProxyHeader
{
   enum MessageType
   {
      SendEvent,
      ExecuteMethod,
      SetProperty,
      AddEventListener,

      force32 = 0xffffffff
   };

   MessageType type;
   char        id[ 42 ];
   char        command[ MaxNameLength ];
};
#pragma pack()

class ProxyChannel : public Channel
{
private:
   Pipe *m_pPipe;

public:
   virtual void ExecuteMethod( 
      const char *pMethod, 
      const ArgList &list
   );

   virtual void SetProperty( 
      const char *pProperty, 
      const ArgList &list
   );

   virtual void GetProperty( 
      const char *pProperty, 
      ArgList &list
   );

   virtual void AddEventListener(
      const IEventMap &eventMap
   );

   virtual void AddMethod(
      const IMethodMap &methodMap
   );

   virtual void RemoveMethod(
      const IMethodMap &methodMap
   );

   virtual void AddGetProperty(
      const IGetPropertyMap &propertyMap
   );

   virtual void AddSetProperty(
      const ISetPropertyMap &propertyMap
   );

   void SetPipe(
      Pipe *pPipe
   )
   {
      m_pPipe = pPipe;
   }
};

class ProxyResource : public Resource
{
public:
   DeclareResourceType(ProxyResource);
   
private:
   virtual Channel *CreateChannel( void );
};
