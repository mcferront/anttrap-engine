#pragma once

#include "EngineGlobal.h"
#include "ResourceType.h"

struct ComponentType
{
private:
    const char *m_pType;

public:
    ComponentType( )
    {
        m_pType = NULL;
    }

    ComponentType(
        const char *pType
        )
    {
        m_pType = StringRef(pType);
    }

    bool operator == ( const ComponentType &rhs ) const
    {
        return 0 == memcmp( this, &rhs, sizeof( rhs ) );
    }

    bool operator != ( const ComponentType &rhs ) const
    {
        return 0 != memcmp( this, &rhs, sizeof( rhs ) );
    }

    const char *ToString( ) const { return m_pType; }
};
