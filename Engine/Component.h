#pragma once

#include "EngineGlobal.h"
#include "ComponentType.h"
#include "Serializer.h"
#include "Identifiable.h"

//helper macros to create standard Component type declarations
//and register with Serializer loaders
#define DeclareComponentType(className)\
virtual const ComponentType &GetType( void ) const { return className::StaticType( ); }\
static const ComponentType &StaticType( void )\
{\
   return _internal_ComponentType;\
}\
virtual const SerializableType &GetSerializableType( void ) const { return className::StaticSerializableType( ); }\
static const ComponentType &LoadComponentType( void );\
static const SerializableType &LoadSerializableType( void );\
static void Register( void );\
static ComponentType _internal_ComponentType;\
static SerializableType _internal_SerializableType;\
static const SerializableType &StaticSerializableType( void )\
{\
   return _internal_SerializableType;\
}

//copy serializerAddr here incase it is a memory allocation (new ISerializer)
#define DefineComponentType(className, serializerAddr)\
ComponentType className::_internal_ComponentType;\
SerializableType className::_internal_SerializableType;\
void className::Register( void )\
{\
   TypeRegistry_RegisterTypeInfo(className);\
   ISerializer *pSerializer = serializerAddr;\
   _internal_ComponentType = LoadComponentType( );\
   _internal_SerializableType = LoadSerializableType( );\
   if ( pSerializer ) SerializerWorld::Instance( ).AddISerializer( TypeRegistry::GetTypeInfo(typeid(className*)), pSerializer );\
}\
const SerializableType &className::LoadSerializableType( void )\
{\
   static SerializableType s_Type( #className );\
   return s_Type;\
}\
const ComponentType &className::LoadComponentType( void )\
{\
   static ComponentType s_Type( #className );\
   return s_Type;\
}

#include "Node.h"

class Component : public Identifiable, public ISerializable
{
private:
   friend class Node;

public:
   DeclareComponentType( Component );

private:
   Node *m_pParent;
   bool m_IsActive;

public:
   Component( void )
   {
      m_pParent = NULL;
      m_IsActive = true;
   }

   virtual ~Component( void ) {}

   Node *GetParent( void ) const { return m_pParent; }

   virtual void AddToScene( void )
   {
      if ( false == s_ActiveComponents.Contains( GetId( ) ) )
         s_ActiveComponents.Add( GetId( ), this );
   };

   virtual void RemoveFromScene( void )
   {
      s_ActiveComponents.Remove( GetId( ) );
   };

   virtual void Bind( void ) {}

   virtual void Tick(
      float deltaSeconds
      ) {}

   virtual void PostTick( void ) {}
   virtual void Final( void ) {}

   virtual void EditorRender( void ) {}

   void SetActive(
      bool active
      )
   {
      if ( active == m_IsActive )
         return;

      m_IsActive = active;

      if ( m_IsActive && m_pParent->IsInScene( ) )
         AddToScene( );
      else if ( false == m_IsActive )
         RemoveFromScene( );
   }

   bool IsActive( void ) const { return m_IsActive; }

public:
   static Component *GetComponent(
      Id id
      )
   {
      Component *pComponent;

      if ( true == s_ActiveComponents.Get( id, &pComponent ) )
         return pComponent;

      return NULL;
   }

public:
   static HashTable<Id, Component *> s_ActiveComponents;
};
