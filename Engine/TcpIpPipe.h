#pragma once

#include "EngineGlobal.h"
#include "StringPool.h"
#include "Socket.h"

class Channel;

class Pipe
{
private:
    struct NameLookup
    {
        char name[ MaxNameLength ];
    };

private:
    static const uint32 MaxLookups = 256;

    const char *m_pNameIdLookup[ MaxLookups ];
    HashTable<const char *, uint32> m_NameIdHash;

    Channel *m_pChannel;

public:
    virtual ~Pipe( void ) {};

    virtual void Create ( void );
    virtual void Destroy( void );

public:
    virtual void SetMaxMessageSize(
        uint32 maxSize
        ) = 0;

    virtual void Update(
        float maxTimeSlice
        ) = 0;

    virtual void SendStream(
        const char *pName,
        const void *pBuffer,
        uint32 size
        ) = 0;

    virtual void SendStream(
        int id,
        const void *pBuffer,
        uint32 size
        ) = 0;

    virtual bool IsConnected( void ) const = 0;

public:
    uint32 NameToId(
        const char *pName
        ) const
    {
        uint32 id = 0xffffffff;

        m_NameIdHash.Get(pName, &id);

        return id;
    }

    const char *IdToName(
        uint32 id
        )
    {
        Debug::Assert( Condition(id < MaxLookups), "Network id %d is too high", id );
        return m_pNameIdLookup[ id ];
    }

    void RegisterName(
        const char *pName,
        uint32 id
        )
    {
        Debug::Assert( Condition(NULL == m_pNameIdLookup[id]), "Network id %d is already registered", id );
        Debug::Assert( Condition(id < MaxLookups), "Network id %d is too high", id );

        m_pNameIdLookup[ id ] = StringRef( pName );

        m_NameIdHash.Add( m_pNameIdLookup[id], id );
    }

    Channel *GetChannel( void ) { return m_pChannel; }
};
