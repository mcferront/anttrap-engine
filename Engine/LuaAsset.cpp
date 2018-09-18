#include "EnginePch.h"

#include "LuaAsset.h"
#include "IOStreams.h"
#include "LuaVM.h"

DefineResourceType( Lua, Asset, new LuaSerializer );

void Lua::AddToScene( void )
{
    Asset::AddToScene( );
    LuaVM::Instance( ).LoadChunk( GetHandle( ), m_pData, m_Size );
}

void Lua::RemoveFromScene( void )
{
    LuaVM::Instance( ).UnloadChunk( GetHandle( ) );
    Asset::RemoveFromScene( );
}

void Lua::Destroy( void )
{
    free( m_pData );

    Resource::Destroy( );
}

ISerializable *LuaSerializer::Deserialize(
    Serializer *pSerializer,
    ISerializable *pSerializable
    )
{
    struct Header
    {
        uint32 size;
    };

    Header header;

    if ( NULL == pSerializable ) pSerializable = new Lua;

    Lua *pLua = (Lua *) pSerializable;

    pSerializer->GetInputStream( )->Read( &header, sizeof( header ), NULL );

    char *pData = (char *) malloc( header.size + 1 );
    memset( pData, 0, header.size + 1 );
    pSerializer->GetInputStream( )->Read( pData, header.size, NULL );

    pLua->m_pData = pData;
    pLua->m_Size = header.size;

    return pSerializable;
}
