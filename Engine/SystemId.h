#pragma once

#include "EngineGlobal.h"
#include "HashTable.h"
#include "IOStreams.h"
#include "Serializer.h"

class Id
{
public:
    class Registrar
    {
        friend class Id;

    public:
        struct IdDesc
        {
            const char *pId;
        };

    private:
        static HashTable<const char *, IdDesc> *m_pIdHash;
        static Lock m_Lock;

    public:
        static void Create( )
        {
            if ( NULL == m_pIdHash )
            {
                m_pIdHash = new HashTable<const char *, IdDesc>( );
                m_pIdHash->Create( );
            }
        }
        static void Destroy( )
        {
            List<IdDesc> list;
            list.Create( );

            {
                Enumerator<const char *, IdDesc> e = m_pIdHash->GetEnumerator( );

                while ( e.EnumNext( ) )
                    list.Add( e.Data( ) );
            }

            m_pIdHash->Destroy( );

            delete m_pIdHash;

            m_pIdHash = NULL;

            for ( uint32 i = 0; i < list.GetSize( ); i++ )
                StringRel( list.GetAt(i).pId );

            list.Destroy( );
        }

    private:
        static IdDesc Register(
            const char *pRequestedId
            )
        {
            IdDesc idDesc;

            if ( NULL == pRequestedId ) 
                idDesc.pId = NULL;
            else
            {
                ScopeLock lock( m_Lock );

                Create( );

                pRequestedId = StringRef(pRequestedId);

                if ( false == m_pIdHash->Get( pRequestedId, &idDesc ) )
                {
                    idDesc.pId = StringRef(pRequestedId);
                    m_pIdHash->Add( idDesc.pId, idDesc );
                }

                StringRel(pRequestedId);
            }

            return idDesc;
        }
    };

public:
    static const Id Empty;

private:
    Registrar::IdDesc idDesc;

public:
    const char *ToString( void ) const { return idDesc.pId; }
    
public:
    Id( void )
    {
        static const char *pNull = StringRef("");
        idDesc.pId = pNull;
    }

    Id( const char *pId
        )
    {
        if ( pId != NULL && 0 != pId[ 0 ] )
            idDesc = Registrar::Register(pId);
        else
            idDesc = Empty.idDesc;
    }

    bool operator == ( const Id &rhs ) const
    {
        return idDesc.pId == rhs.idDesc.pId;
    }

    bool operator != ( const Id &rhs ) const
    {
        return idDesc.pId != idDesc.pId;
    }

private:
    Id(
        const Registrar::IdDesc &_idDesc
        )
    {
        idDesc = _idDesc;
    }

public:
    static Id Create( void )
    {
        static uint32 s_nextId = 1024;

        char id[ 64 ];

        String::Format( id, sizeof(id) -1, "DynamicId_%d", s_nextId );
        id[ sizeof(id) -1 ] = NULL;

        ++s_nextId;

        return Id(Registrar::Register(id));
    }

    static void Serialize(
        IOutputStream *pStream,
        Id id
        )
    {
        int length = strlen( id.ToString( ) );
        pStream->Write( &length, sizeof( length ) );
        pStream->Write( id.ToString( ), length );
    }

    static Id Deserialize(
        IInputStream *pStream
        )
    {
        char idString[ 1024 ];
        
        int length;
        pStream->Read( &length, sizeof( length ) );

        char *pId;

        if ( length > sizeof(idString) )
            pId = (char *) malloc( length + 1 );
        else
            pId = idString;

        pStream->Read( pId, length );

        pId[ length ] = NULL;

        Id id( Registrar::Register(pId) );

        if ( pId != idString ) 
            free( pId );

        return id;
    }

    static void DeserializeList(
        IInputStream *pStream,
        List<Id> *pList
        )
    {
        uint32 count;
        pStream->Read( &count, sizeof( count ) );

        for ( uint32 i = 0; i < count; i++ )
            pList->Add( Id::Deserialize( pStream ) );
    }
};

static inline uint32 IdHash(
    Id id
    )
{
    return HashFunctions::NUIntHash( (nuint) id.ToString( ) );
}

static inline bool IdCompare(
    Id id1,
    Id id2
    )
{
    return id1 == id2;
}
