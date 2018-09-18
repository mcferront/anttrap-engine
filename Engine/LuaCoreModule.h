#pragma once

#include "EngineGlobal.h"
#include "UtilityMath.h"
#include "LuaVM.h"
#include "ScriptComponent.h"
#include "Node.h"
#include "LuaScriptInstance.h"
#include "SplineComponent.h"
#include "ButtonComponent.h"
#include "LabelComponent.h"
#include "MeshRendererComponent.h"
#include "Animation3dComponent.h"
#include "LightComponent.h"
#include "Raycast.h"
#include "InputSystem.h"
#include "RegistryWorld.h"

static Component *GetContextComponent( )
{
   return LuaVM::Instance( ).GetLuaContext( )->GetParentComponent( );
}

static void SetRegistryFloat(
   const char *pPath,
   float value
)
{
   RegistryFloat f( pPath );
   return f.SetValue( value );
}

static void SetRegistryInt(
   const char *pPath,
   int value
)
{
   RegistryInt d( pPath );
   return d.SetValue( value );
}

static void SetRegistryBool(
   const char *pPath,
   bool value
)
{
   RegistryBool b( pPath );
   return b.SetValue( value );
}

static float GetRegistryFloat(
   const char *pPath,
   float defaultValue
)
{
   RegistryFloat f( pPath, defaultValue );
   return f.GetValue();
}

static int GetRegistryInt(
   const char *pPath,
   int defaultValue
)
{
   RegistryInt d( pPath, defaultValue );
   return d.GetValue();
}

static bool GetRegistryBool(
   const char *pPath,
   bool defaultValue
)
{
   RegistryBool d( pPath, defaultValue );
   return d.GetValue();
}

static const char *FindClosestCommand(
   const char *pCommand,
   int lastIndex
)
{
   Enumerator<const char *, RegistryValue*> e = RegistryWorld::Instance( ).Enumerate( );

   int index = 0;

   while ( e.EnumNext( ) )
   {
      if ( String::StartsWith(e.Key( ), pCommand) )
      {
         if (index == lastIndex)
            return e.Key( );

         ++index;
      }
   }

   return NULL;
}

static const char *HandleConsoleCommand(
   const char *pCommand
)
{
   char command[ 256 ];
   String::Copy( command, pCommand, sizeof(command) );

   int numArgs = 0;
   const char *pArgs[ 256 ];

   char *pHead = command;
   char *pSpace = strchr( pHead, ' ' );

   while ( pSpace )
   {
      *pSpace = NULL;
      pArgs[ numArgs++ ] = pHead;

      pHead = pSpace + 1;
      pSpace = strchr( pHead, ' ' );
   }

   if ( *pHead != NULL )
      pArgs[ numArgs++ ] = pHead;

   if ( numArgs > 0 )
   {
      static char result[ 256 ];
      result[0] = 0;

      RegistryValue *pValue = RegistryWorld::Instance( ).GetValue( StringRef(pArgs[0]), false );

      if ( NULL == pValue )
         return NULL;

      if ( pValue->GetType() == RegistryValue::Int )
      {
         RegistryInt v(pValue);

         if ( numArgs > 1 )
         {
            if ( 0 == strncmp( "0x", pArgs[1], 2 ) )
               v.SetValue( strtol(pArgs[1], NULL, 16) );
            else
               v.SetValue( atoi(pArgs[1]) );
         }

         String::Format( result, sizeof(result), "%d", v.GetValue() );
         return result;
      }
      
      if ( pValue->GetType() == RegistryValue::String )
      {
         RegistryString v(pValue) ;
         
         if ( numArgs > 1 )
            v.SetValue( pArgs[1] );
         
         String::Format( result, sizeof(result), "%s", v.GetValue() );
         return result;
      }

      if ( pValue->GetType() == RegistryValue::Float )
      {
         RegistryFloat v(pValue);
         
         if ( numArgs > 1 )
            v.SetValue( (float) atof(pArgs[1]) );

         String::Format( result, sizeof(result), "%f", v.GetValue() );
         return result;
      }
      
      if ( pValue->GetType() == RegistryValue::Bool )
      {
         RegistryBool b(pValue);

         if ( numArgs > 1 )
            b.SetValue( atoi(pArgs[1]) != 0 );
            
         String::Format( result, sizeof(result), "%s", b.GetValue() ? "true" : "false" );
         return result;
      }
      
      if ( pValue->GetType() == RegistryValue::Method )
      {
         RegistryMethod(pValue).GetValue( )( pArgs, numArgs );

         String::Format( result, sizeof(result), "%s(", pArgs[0] );

         int i;

         for ( i = 1; i < numArgs - 1; i++ )
            String::Format( result, sizeof(result), "%s%s, ", result, pArgs[i] );
         
         if ( i == numArgs - 1 )
            String::Format( result, sizeof(result), "%s%s", result, pArgs[i] );

         String::Format( result, sizeof(result), "%s)", result );

         return result;
      }
   }

   return NULL;
}

static void AddEventListener(
    Id id,
    const char *pEventName,
    const char *pMethod
    )
{
    Component *pComponent = LuaVM::Instance( ).GetLuaContext( )->GetParentComponent( );
    Debug::Assert( Condition( pComponent->GetType( ) == ScriptComponent::StaticType( ) ), "Fatal programming bug, this should be a ScriptComponent" );

    return ( (ScriptComponent *) pComponent )->AddEventListener( id, StringRef(pEventName), pMethod );
}

static InputMap CreateInputMap(const char *pString)
{
    InputMap map(StringRef(pString));
    return map;
}

static void RemoveEventListener(
    Id id,
    const char *pEventName,
    const char *pMethod
    )
{
    Component *pComponent = LuaVM::Instance( ).GetLuaContext( )->GetParentComponent( );
    Debug::Assert( Condition( pComponent->GetType( ) == ScriptComponent::StaticType( ) ), "Fatal programming bug, this should be a ScriptComponent" );

    return ( (ScriptComponent *) pComponent )->RemoveEventListener( id, StringRef(pEventName), pMethod );
}

static Component *GetComponentFromId( Id id )
{
    return Component::GetComponent( id );
}

static Node *CreateNode( Id id, const char *pAlias )
{
    Node *pNode = new Node;
    pNode->Create( );

    ResourceHandle handle( id );
    handle.Bind( pAlias, pNode );

    return pNode;
}

static Component *CreateComponent( Node *pOwner, const Id &id, const char *pType )
{
    pType = StringRef( pType );

    ISerializer *pISerializer = SerializerWorld::Instance( ).GetISerializer( pType );

    if ( NULL == pISerializer )
        return NULL;

    Component *pComponent = (Component *) pISerializer->Instantiate( );
    pComponent->SetId( id );
    pOwner->AddComponent( pComponent );

    StringRel( pType );

    return pComponent;
}

static void DeleteComponent( Node *pOwner, const Id &id )
{
   pOwner->DeleteComponent( id );
}

static Node *GetNodeFromAlias( const char *pAlias )
{
    pAlias = StringRef( pAlias );

    ResourceHandle h = ResourceHandle::FromAlias(pAlias);

    Node *pNode;

    if ( IsResourceLoaded(h) )
      pNode = GetResource( h, Node );
    else
      pNode = NULL;

    StringRel( pAlias );

    return pNode;
}

static ResourceHandle GetResourceHandleFromAlias( const char *pAlias )
{
    pAlias = StringRef( pAlias );

    ResourceHandle handle = ResourceWorld::Instance( ).GetHandleFromAlias( pAlias );

    StringRel( pAlias );

    return handle;
}

static Component *GetScriptComponent(
    const Node *pNode,
    const char *pRequestedType
    )
{
    Component *pUserComponent = NULL;

    ComponentList list;
    list.Create( );

    pNode->GetComponents( &list );

    const char *pName = NULL;

    for ( uint32 i = 0; i < list.GetSize( ); i++ )
    {
        Component *pComponent = list.GetAt( i );
        if ( pComponent->GetType( ) == ScriptComponent::StaticType( ) )
        {
            ScriptComponent *pScript = (ScriptComponent *) pComponent;

            ResourceHandle script = pScript->GetScript( );
            const char *pType = strrchr( script.GetId( ).ToString( ), '/' ) + 1;

            if ( 0 == strcmp( pType, pRequestedType ) )
            {
                pUserComponent = pScript;
                break;
            }
        }
    }

    list.Destroy( );

    return pUserComponent;
}

static const char *GetLuaObjectName(
    const Component *pComponent
    )
{
    if ( pComponent->GetType( ) == ScriptComponent::StaticType( ) )
    {
        ScriptComponent *pScript = (ScriptComponent *) pComponent;
        return pScript->GetObjectName( );
    }

    return NULL;
}

static IdList *CreateIdList( const char *pIds )
{
    IdList *pList = new IdList( ); pList->Create( );

    const char *pId = pIds;
    const char *pNext = strchr( pId, ',' );

    char id[ 256 ];

    while ( NULL != pNext )
    {
        String::Copy( id, pId, min( sizeof( id ), pNext - pId + 1 ) );
        
        if ( 0 != id[0] )
            pList->Add( id );

        pId = pNext;
        pId++;

        pNext = strchr( pId, ',' );
    }

    if ( NULL != pId && 0 != *pId )
        pList->Add( pId );

    return pList;
}

static void DestroyIdList( IdList *pList )
{
    pList->Destroy( );
    delete pList;
}

static ResourceHandleList *CreateResourceHandleList_FromAlias( const char *pAliases )
{
    ResourceHandleList *pList = new ResourceHandleList( ); pList->Create( );

    const char *pAlias = pAliases;
    const char *pNext = strchr( pAlias, ',' );

    char alias[ 256 ];

    while ( NULL != pNext )
    {
        String::Copy( alias, pAlias, min( sizeof( alias ), pNext - pAlias + 1 ) );
        
        if ( 0 != alias[0] )
        {
            const char *pRef = StringRef(alias);
            pList->Add( ResourceHandle::FromAlias(pRef) );
            StringRel(pRef);
        }

        pAlias = pNext;
        pAlias++;

        pNext = strchr( pAlias, ',' );
    }

    if ( NULL != pAlias && 0 != *pAlias )
    {
        const char *pRef = StringRef(pAlias);
        pList->Add( ResourceHandle::FromAlias(pRef) );
        StringRel(pRef);
    }

    return pList;
}

static void DestroyResourceHandleList( ResourceHandleList *pList )
{
    pList->Destroy( );
    delete pList;
}

static LabelComponent *ToLabelComponent( Component *pComponent )
{
    Debug::Assert( Condition( pComponent->GetType( ) == LabelComponent::StaticType( ) ), "Cannot cast %s to LabelComponent", pComponent->GetType( ).ToString( ) );
    return (LabelComponent *) pComponent;
}

static ScriptComponent *ToScriptComponent( Component *pComponent )
{
    Debug::Assert( Condition( pComponent->GetType( ) == ScriptComponent::StaticType( ) ), "Cannot cast %s to ScriptComponent", pComponent->GetType( ).ToString( ) );
    return (ScriptComponent *) pComponent;
}

static ButtonComponent *ToButtonComponent( Component *pComponent )
{
    Debug::Assert( Condition( pComponent->GetType( ) == ButtonComponent::StaticType( ) ), "Cannot cast %s to ButtonComponent", pComponent->GetType( ).ToString( ) );
    return (ButtonComponent *) pComponent;
}

static SplineComponent *ToSplineComponent( Component *pComponent )
{
    Debug::Assert( Condition( pComponent->GetType( ) == SplineComponent::StaticType( ) ), "Cannot cast %s to SplineComponent", pComponent->GetType( ).ToString( ) );
    return (SplineComponent *) pComponent;
}

static MeshRendererComponent *ToMeshRendererComponent( Component *pComponent )
{
    Debug::Assert( Condition( pComponent->GetType( ) == MeshRendererComponent::StaticType( ) ), "Cannot cast %s to MeshRendererComponent", pComponent->GetType( ).ToString( ) );
    return (MeshRendererComponent *) pComponent;
}

static AmbientLightComponent *ToAmbientLightComponent( Component *pComponent )
{
   Debug::Assert( Condition( pComponent->GetType( ) == AmbientLightComponent::StaticType( ) ), "Cannot cast %s to AmbientLightComponent", pComponent->GetType( ).ToString( ) );
   return (AmbientLightComponent *) pComponent;
}

static DirectionalLightComponent *ToDirectionalLightComponent( Component *pComponent )
{
   Debug::Assert( Condition( pComponent->GetType( ) == DirectionalLightComponent::StaticType( ) ), "Cannot cast %s to DirectionalLightComponent", pComponent->GetType( ).ToString( ) );
   return (DirectionalLightComponent *) pComponent;
}

static PointLightComponent *ToPointLightComponent( Component *pComponent )
{
   Debug::Assert( Condition( pComponent->GetType( ) == PointLightComponent::StaticType( ) ), "Cannot cast %s to PointLightComponent", pComponent->GetType( ).ToString( ) );
   return (PointLightComponent *) pComponent;
}

static SpotLightComponent *ToSpotLightComponent( Component *pComponent )
{
   Debug::Assert( Condition( pComponent->GetType( ) == SpotLightComponent::StaticType( ) ), "Cannot cast %s to SpotLightComponent", pComponent->GetType( ).ToString( ) );
   return (SpotLightComponent *) pComponent;
}

static AnimationComponent *ToAnimationComponent( Component *pComponent )
{
    Debug::Assert( Condition( pComponent->GetType( ) == AnimationComponent::StaticType( ) ), "Cannot cast %s to AnimationComponent", pComponent->GetType( ).ToString( ) );
    return (AnimationComponent *) pComponent;
}

static void DisableCollisionLayers(
    const char *pLayerA,
    const char *pLayerB
    )
{
    int layerA = PhysicsWorld::Instance( ).LayerToBitFlag( Id( pLayerA ) );
    int layerB = PhysicsWorld::Instance( ).LayerToBitFlag( Id( pLayerB ) );

    PhysicsWorld::Instance( ).DisableCollisionLayers( layerA, layerB );
}

static void ExecuteLuaAsset(
    const char *pAlias
    )
{
    pAlias = StringRef( pAlias );

    ResourceHandle rh = ResourceHandle::FromAlias( pAlias );

    if ( true == IsResourceLoaded( rh ) )
    {
        LuaVM::Instance( ).ExecuteLuaAsset( rh );
        LuaVM::Instance( ).AddChunkRef( rh );
    }

    StringRel( pAlias );
}

static void RemoveLuaAsset(
    const char *pFilename
    )
{
    LuaVM::Instance( ).RemoveChunkRef( ResourceHandle( pFilename ) );
}

static ResourceHandle GetTypeHandle(
    const char *pTypeString
    )
{
    return ResourceWorld::Instance( ).GetTypeHandle( pTypeString );
}

bool CastRay(
    RaycastResult *pResult,
    const Vector &start,
    const Vector &direction,
    float length,
    const char *pLayer,
    bool render
    );

void RenderTransform(
    const Vector &position,
    const Vector &up,
    const Vector &look,
    float size
    );

void FindAllByCone(
    NodeList *pList,
    const Transform &transform,
    float angle,
    float distance
    );

void FindAllBySphere(
    NodeList *pList,
    const Vector &position,
    float radius
    );

int LoadResources(
    const char *pName
    );

void UnloadResources(
    int handle
    );

void AddMeshes(
    int handle
);

void PlaceMeshes(
    Node *pParent,
    const char *mappingAlias
);
