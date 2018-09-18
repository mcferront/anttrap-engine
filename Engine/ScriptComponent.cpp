#include "EnginePch.h"

#include "ScriptComponent.h"
#include "LuaScriptInstance.h"
#include "LuaCoreModule.h"

DefineComponentType( ScriptComponent, new ScriptComponentSerializer );

void ScriptComponent::Create(
   ResourceHandle script
)
{
   m_ScriptHandle = script;

   m_pScript = NULL;

   m_Events.Create( 4, 4, EventBindingHash, EventBindingCompare );

   m_ScriptHandle.GetChannel( )->AddEventListener( EventMap<ScriptComponent>( "Reloaded", this, &ScriptComponent::OnLuaScriptReloaded ) );
}

void ScriptComponent::Bind( void )
{
   if ( NULL != m_pScript )
      return;

   LoadScript( );

   if ( NULL != m_pScript )
   {
      m_Stream.Seek( 0, SeekBegin );

      m_pScript->Load( );

      ScriptComponentSerializer sc;
      sc.DeserializeVariables( this, &m_Stream );

      m_pScript->Bind( );
   }
}

void ScriptComponent::Tick(
   float deltaSeconds
)
{
   if ( NULL != m_pScript ) m_pScript->Tick( deltaSeconds );
}

void ScriptComponent::PostTick( void )
{
   if ( NULL != m_pScript ) m_pScript->PostTick( );
}

void ScriptComponent::Final( void )
{
   if ( NULL != m_pScript ) m_pScript->Final( );
}

void ScriptComponent::EditorRender( void )
{
   if ( NULL != m_pScript ) m_pScript->EditorRender( );
}

void ScriptComponent::Destroy( void )
{
   m_ScriptHandle.GetChannel( )->RemoveEventListener( EventMap<ScriptComponent>( "Reloaded", this, &ScriptComponent::OnLuaScriptReloaded ) );

   if ( NULL != m_pScript )
   {
      m_pScript->Unload( );
      m_Stream.Close( );
   }

   delete m_pScript;
   m_pScript = NULL;

   m_ScriptHandle = NullHandle;

   {
      Enumerator<EventBinding, EventDesc> e = m_Events.GetEnumerator( );

      while ( e.EnumNext( ) )
      {

         ResourceHandle resource( e.Data( ).sender );
         resource.GetChannel( )->RemoveEventListener( EventMap<ScriptComponent>( e.Key( ).sender.ToString( ), this, &ScriptComponent::OnEvent ) );
      }
   }

   m_Events.Destroy( );
}

void ScriptComponent::AddEventListener(
   Id id,
   const char *pEventName,
   const char *pMethod
)
{
   pEventName = StringRef( pEventName );

   EventBinding eb = { id, StringRef( pEventName ) };

   if ( false == m_Events.Contains( eb ) )
   {
      ResourceHandle resource( id );
      resource.GetChannel( )->AddEventListener( EventMap<ScriptComponent>( pEventName, this, &ScriptComponent::OnEvent ) );

      EventDesc desc;
      desc.sender = id;
      desc.method = Id( pMethod );

      m_Events.Add( eb, desc );
   }
}

void ScriptComponent::RemoveEventListener(
   Id id,
   const char *pEventName,
   const char *pMethod
)
{
   pEventName = StringRef( pEventName );

   EventBinding eb = { id, pEventName };

   if ( true == m_Events.Contains( eb ) )
   {
      ResourceHandle resource( id );
      resource.GetChannel( )->RemoveEventListener( EventMap<ScriptComponent>( pEventName, this, &ScriptComponent::OnEvent ) );

      m_Events.Remove( eb );
   }
}

void ScriptComponent::LoadScript( )
{
   if ( m_pScript != NULL )
      return;

   if ( IsResourceLoaded( m_ScriptHandle ) )
   {
      const char *pExt = strrchr( m_ScriptHandle.GetName( ), '.' );

      if ( NULL != pExt )
      {
         if ( 0 == strcmp( pExt, ".lua" ) ) m_pScript = new LuaScriptInstance( this, m_ScriptHandle );

         else Debug::Print( Debug::TypeError, "Unrecognized script type: %s\n", pExt );
      }
   }
}

void ScriptComponent::OnEvent(
   const Channel *pSender,
   const char *pEvent,
   const ArgList &list
)
{
   if ( m_pScript == NULL ) return;

   EventDesc desc;

   pEvent = StringRef( pEvent );

   EventBinding eb = { pSender->GetId( ), pEvent };

   if ( m_Events.Get( eb, &desc ) )
      m_pScript->OnEvent( desc.method.ToString( ), pSender, pEvent, list );
}

void ScriptComponent::OnLuaScriptReloaded(
   const Channel *pSender,
   const char *pName,
   const ArgList &list
)
{
   if ( NULL != m_pScript )
   {
      m_pScript->Unload( );
      m_pScript->Load( );

      ScriptComponentSerializer sc;

      m_Stream.Seek( 0, SeekBegin );
      sc.DeserializeVariables( this, &m_Stream );

      m_pScript->Bind( );
   }
}

ISerializable *ScriptComponentSerializer::Deserialize(
   Serializer *pSerializer,
   ISerializable *pSerializable
)
{
   if ( NULL == pSerializable ) pSerializable = new ScriptComponent;

   ScriptComponent *pScriptComponent = (ScriptComponent *) pSerializable;

   Id id = Id::Deserialize( pSerializer->GetInputStream( ) );

   Id assetId = Id::Deserialize( pSerializer->GetInputStream( ) );
   ( (Identifiable *) pScriptComponent )->SetId( id );
   pScriptComponent->Create( ResourceHandle( assetId ) );

   int variableStream;
   pSerializer->GetInputStream( )->Read( &variableStream, sizeof( variableStream ) );

   pScriptComponent->m_Stream.Copy( pSerializer->GetInputStream( ), variableStream );

   return pSerializable;
}

void ScriptComponentSerializer::DeserializeVariables(
   ScriptComponent *pComponent,
   IInputStream *pStream
)
{
   int variableCount = 0;
   pStream->Read( &variableCount, sizeof( variableCount ) );

   for ( int i = 0; i < variableCount; i++ )
   {
      const char *pName = Id::Deserialize( pStream ).ToString( );

      int type;
      pStream->Read( &type, sizeof( type ) );

      if ( 0 == type )
      {
         int value;
         pStream->Read( &value, sizeof( value ) );
         pComponent->SetInt( value, pName );
      }
      else if ( 1 == type )
      {
         float value;
         pStream->Read( &value, sizeof( value ) );
         pComponent->SetFloat( value, pName );
      }
      else if ( 2 == type )
      {
         Vector2 value;
         pStream->Read( &value, sizeof( value ) );
         pComponent->SetVector2( value, pName );
      }
      else if ( 3 == type )
      {
         Vector value;
         pStream->Read( &value, sizeof( value ) );
         pComponent->SetVector( value, pName );
      }
      else if ( 4 == type )
      {
         Vector value;
         pStream->Read( &value, sizeof( value ) );
         pComponent->SetVector( value, pName );
      }
      else if ( 5 == type )
      {
         Matrix value;
         pStream->Read( &value, sizeof( value ) );
         pComponent->SetMatrix( value, pName );
      }
      else if ( 6 == type )
      {
         Quaternion q;
         Vector t, s;
         pStream->Read( &q, sizeof( q ) );
         pStream->Read( &t, sizeof( t ) );
         pStream->Read( &s, sizeof( s ) );

         Transform value( q, t, s );
         pComponent->SetTransform( value, pName );
      }
      else if ( 7 == type )
      {
         Id value = Id::Deserialize( pStream );
         pComponent->SetId( value, pName );
      }
      else if ( 8 == type )
      {
         Id string = Id::Deserialize( pStream );
         pComponent->SetString( string.ToString( ), pName );
      }
      else if ( 9 == type )
      {
         byte value;
         pStream->Read( &value, sizeof( value ) );
         pComponent->SetBool( 0 != value, pName );
      }
      else if ( 10 == type )
      {
         Color color;
         pStream->Read( &color, sizeof( color ) );
         pComponent->SetColor( color, pName );
      }
   }
}
