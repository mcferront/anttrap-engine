
#include "EnginePch.h"
#include "ResourceMaps.h"
#include "Resource.h"
#include "IOStreams.h"

HashTable<const char *, const TypeRegistry::TypeInfo *> TypeRegistry::m_TypesByIdentifier;
HashTable<const char *, const TypeRegistry::TypeInfo *> TypeRegistry::m_TypesByTypeId;

void TypeRegistry::Create( void )
{
    m_TypesByTypeId.Create( 16, 16, HashFunctions::StringHash, HashFunctions::StringCompare );
    m_TypesByIdentifier.Create( 16, 16, HashFunctions::StringHash, HashFunctions::StringCompare );

    //register basic types
    TypeRegistry_RegisterTypeInfo( int );
    TypeRegistry_RegisterTypeInfo( float );
    TypeRegistry_RegisterTypeInfo( char );
    TypeRegistry_RegisterTypeInfo( bool );
    TypeRegistry_RegisterTypeInfo( uint32 );
    TypeRegistry_RegisterTypeInfo( uint64 );
    TypeRegistry_RegisterTypeInfo( MemoryStream );
    TypeRegistry_RegisterTypeInfo( Vector );
    TypeRegistry_RegisterTypeInfo( Transform );
    TypeRegistry_RegisterTypeInfo( Matrix );
    TypeRegistry_RegisterTypeInfo( Color );
}

void TypeRegistry::Destroy( void )
{
    //register basic types
    List<const TypeInfo*> list;
    list.Create( );

    {
        Enumerator<const char *, const TypeInfo *> e = m_TypesByIdentifier.GetEnumerator( );

        while ( e.EnumNext( ) )
        {
            list.Add( e.Data( ) );
        }
    }

    m_TypesByIdentifier.Clear( );
    m_TypesByTypeId.Clear( );

    uint32 i, size = list.GetSize( );

    for ( i = 0; i < size; i++ )
    {
        delete list.GetAt( i );
    }

    list.Destroy( );

    m_TypesByIdentifier.Destroy( );
    m_TypesByTypeId.Destroy( );
}

const TypeRegistry::TypeInfo *TypeRegistry::GetTypeInfo(
    const char *pIdentifier
    )
{
    const TypeInfo *pInfo;

    if ( true == m_TypesByIdentifier.Get( pIdentifier, &pInfo ) )
    {
        return pInfo;
    }

    Debug::Assert( Condition( false ), "Unregistered type: %s", pIdentifier );
    return NULL;
}

const TypeRegistry::TypeInfo *TypeRegistry::GetTypeInfo(
    const type_info &type
    )
{
    const TypeInfo *pInfo;

    if ( true == m_TypesByTypeId.Get( type.name( ), &pInfo ) )
    {
        return pInfo;
    }

    Debug::Assert( Condition( false ), "Unregistered type: %s", type.name( ) );
    return NULL;
}

void TypeRegistry::RegisterTypeInfo(
    const char *pIdentifier,
    const type_info &type
    )
{
    TypeInfo *pTypeInfo = new TypeInfo( pIdentifier, type );

    m_TypesByIdentifier.Add( pIdentifier, pTypeInfo );
    m_TypesByTypeId.Add( type.name( ), pTypeInfo );
}

class ArgListSerializer : public ISerializer
{
public:
    virtual ISerializable *Instantiate( ) const { return new ArgList; }

    virtual bool Serialize(
        Serializer *pSerializer,
        const ISerializable *pSerializable
        )
    {
        const ArgList *pArgList = (const ArgList *) pSerializable;

        IOutputStream *pStream = pSerializer->GetOutputStream( );

        uint32 count = pArgList->GetArgSig( )->GetCount( );

        pStream->Write( &count, sizeof( count ) );

        uint32 i;

        for ( i = 0; i < count; i++ )
        {
            const TypeRegistry::TypeInfo *pType = TypeRegistry::GetTypeInfo( pArgList->GetArgSig( )->GetType( i ) );

            uint32 length = (uint32) strlen( pType->name );
            pStream->Write( &length, sizeof( length ) );
            pStream->Write( pType->name, length );

            const char *pName = pType->name;

            //send across resource ids to recreate the handles with
            if ( typeid( ResourceHandle ) == pType->type )
            {
                ResourceHandle handle = *(ResourceHandle *) pArgList->GetRawBuffer( i );
                Id id = handle.GetId( );

                Id::Serialize( pStream, id );
            }
            //read system id
            else if ( typeid( Id ) == pType->type )
            {
                Id id;
                pArgList->GetArg( i, &id );
                Id::Serialize( pStream, id );
            }
            //if it's anything else by value, just pack the value staight across
            else if ( NULL == strchr( pName, '*' ) )
            {
                uint32 size = pArgList->GetSize( i );

                pStream->Write( &size, sizeof( size ) );
                pStream->Write( pArgList->GetRawBuffer( i ), size );
            }
            else if ( typeid( char * ) == pType->type )
            {
                //copy char to a temp buffer, and add the pointer of that
                //buffer to the new arg list
                char *pValue;
                pArgList->GetArg( i, &pValue );
                uint32 size = (uint32) strlen( pValue );

                pStream->Write( &size, sizeof( size ) );
                pStream->Write( pValue, size );
            }
            else if ( typeid( const char * ) == pType->type )
            {
                //copy char to a temp buffer, and add the pointer of that
                //buffer to the new arg list
                const char *pValue;
                pArgList->GetArg( i, &pValue );
                uint32 size = (uint32) strlen( pValue );

                pStream->Write( &size, sizeof( size ) );
                pStream->Write( pValue, size );
            }
            else if ( typeid( MemoryStream* ) == pType->type )
            {
                //copy char to a temp buffer, and add the pointer of that
                //buffer to the new arg list
                MemoryStream *pMemoryStream;
                pArgList->GetArg( i, &pMemoryStream );

                uint32 size = pMemoryStream->GetAmountWritten( );

                pStream->Write( &size, sizeof( size ) );
                pStream->Write( pMemoryStream->GetBuffer( ), size );
            }
            else
            {
                Debug::Assert( Condition( false ), "Type %s cannot be serialized in an arglist", pName );
                return false;
            }
        }

        return true;
    }

    virtual ISerializable *Deserialize(
        Serializer *pSerializer,
        ISerializable *pSerializable
        )
    {
        if ( NULL == pSerializable ) pSerializable = new ArgList;

        ArgList *pArgList = (ArgList *) pSerializable;

        pArgList->StartPack( );

        IInputStream *pStream = pSerializer->GetInputStream( );

        uint32 count;

        pStream->Read( &count, sizeof( count ) );

        uint32 i;

        for ( i = 0; i < count; i++ )
        {
            char typeName[ 32 ];

            uint32 length;
            pStream->Read( &length, sizeof( length ) );
            Debug::Assert( Condition( length < sizeof( typeName ) ), "typeName is too long to be serialized" );

            pStream->Read( typeName, length );
            typeName[ length ] = NULL;

            const TypeRegistry::TypeInfo *pType = TypeRegistry::GetTypeInfo( typeName );
            if ( NULL == pType ) return false;

            const char *pName = pType->name;

            //read system id and turn it into a resource handle
            if ( typeid( ResourceHandle ) == pType->type )
            {
                Id id = Id::Deserialize( pStream );

                ResourceHandle handle = ResourceWorld::Instance( ).GetHandle( id );
                pArgList->Pack( pType->type, &handle, sizeof( handle ) );
            }
            //read system id
            else if ( typeid( Id ) == pType->type )
            {
                Id id = Id::Deserialize( pStream );
                pArgList->Pack( pType->type, &id, sizeof( id ) );
            }
            //if it's anything else by value, just pack the value staight across
            else if ( NULL == strchr( pName, '*' ) )
            {
                uint32 size;
                char rawBuffer[ 128 ];

                pStream->Read( &size, sizeof( size ) );
                if ( size >= sizeof( rawBuffer ) ) return false;

                pStream->Read( rawBuffer, size );
                pArgList->Pack( pType->type, rawBuffer, size );
            }
            else if ( typeid( char * ) == pType->type || typeid( const char * ) == pType->type )
            {
                uint32 size;

                pStream->Read( &size, sizeof( size ) );

                char *pString = (char *) malloc( size + 1 );
                pStream->Read( pString, size );
                pString[ size ] = NULL;

                //pack the address of the pointer, because that is what is saved
                //in a normal arglist, only this time the address will point
                //to our new buffer
                pArgList->Pack( pType->type, &pString, sizeof( char* ) );
            }
            else if ( typeid( Vector * ) == pType->type || typeid( const Vector * ) == pType->type )
            {
                uint32 size;

                pStream->Read( &size, sizeof( size ) );

                void *pData = (void *) malloc( size );
                pStream->Read( pData, size );

                //pack the address of the pointer, because that is what is saved
                //in a normal arglist, only this time the address will point
                //to our new buffer
                pArgList->Pack( pType->type, &pData, sizeof( void* ) );
            }
            else if ( typeid( MemoryStream* ) == pType->type )
            {
                uint32 size;

                pStream->Read( &size, sizeof( size ) );

                unsigned char *pBuffer = (unsigned char *) malloc( size );
                pStream->Read( pBuffer, size );

                MemoryStream *pStream = new MemoryStream( pBuffer, size, true );

                //pack the address of the pointer, because that is what is saved
                //in a normal arglist, only this time the address will point
                //to our new buffer
                pArgList->Pack( pType->type, &pStream, sizeof( MemoryStream* ) );
            }
            else
            {
                Debug::Assert( Condition( false ), "Type %s cannot be deserialized in an arglist", pName );
                return false;
            }
        }

        return pSerializable;
    }

    virtual const SerializableType &GetSerializableType( void ) const { return ArgList::StaticSerializableType( ); }

    virtual uint32 GetVersion( void ) const { return 1; }
};

DefineSerializableType( ArgList, new ArgListSerializer );

void SerializedArgList::Destroy( void )
{
    int i, size = m_ArgList.GetArgSig( )->GetCount( );

    //release our resource handles
    //which had an extra ref so they didn't go away during the queueing
    for ( i = 0; i < size; i++ )
    {
        if ( typeid( char * ) == m_ArgList.GetArgSig( )->GetType( i ) )
        {
            char *pChar;

            m_ArgList.GetArg( i, &pChar );
            free( (void *) pChar );
        }
        else if ( typeid( const char * ) == m_ArgList.GetArgSig( )->GetType( i ) )
        {
            const char *pChar;

            m_ArgList.GetArg( i, &pChar );
            free( (void *) pChar );
        }
        else if ( typeid( Vector * ) == m_ArgList.GetArgSig( )->GetType( i ) )
        {
            Vector *pData;

            m_ArgList.GetArg( i, &pData );
            free( (void *) pData );
        }
        else if ( typeid( MemoryStream* ) == m_ArgList.GetArgSig( )->GetType( i ) )
        {
            MemoryStream*pStream;

            m_ArgList.GetArg( i, &pStream );

            pStream->Close( );
            delete pStream;
        }
    }
}
