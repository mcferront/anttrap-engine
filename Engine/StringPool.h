#pragma once

#include "EngineGlobal.h"
#include "UtilityString.h"
#include "HashTable.h"
#include "IOStreams.h"
#include "List.h"
#include "Threads.h"
#include "Debug.h"

#ifdef ENABLE_STRINGPOOL_VALIDATION
    #define StringRefValidate(stringRef)\
        Debug::Assert( Condition(StringPool::Instance().Validate(stringRef)), "%s was not part of String Pool", stringRef);

    #define StringRefEqual(stringRef1,stringRef2)\
        StringPool::Instance( ).Compare( __FILE__, __LINE__, stringRef1, stringRef2 )

    #define StringRef(string)\
        StringPool::Instance( ).Alloc( __FILE__, __LINE__, string )

    #define StringRel(string)\
        StringPool::Instance( ).Free( __FILE__, __LINE__, string )
#else
    #define StringRefValidate(string)

    #define StringRef(string)\
        StringPool::Instance( ).Alloc( string )

    #define StringRel(string)\
        StringPool::Instance( ).Free( string )
    
    #define StringRefEqual(stringRef1,stringRef2)\
        stringRef1 == stringRef2
#endif

class StringPool
{
public:
    static StringPool &Instance( void );

private:
    static const int Magic = 0x0000feed;

    struct StringRef
    {
        int magic;
        uint32 refCount;
    };

private:
    HashTable<const char *, StringRef *> m_Hash;
    Lock m_Lock;

public:
    StringPool( void )
    {
        m_Hash.Create(1024, 1024, HashFunctions::StringHash, HashFunctions::StringCompare);
    }

    #ifdef ENABLE_STRINGPOOL_VALIDATION
        bool Compare(
            const char *pFile,
            int line,
            const char *pString1,
            const char *pString2
            )
        {
            StringRefValidate(pString1);
            StringRefValidate(pString2);

            return pString1 == pString2;
        }
    #endif

    const char *Alloc(
    #ifdef ENABLE_STRINGPOOL_VALIDATION
        const char *pFile,
        int line,
    #endif
        const char *pString
        )
    {
        if ( pString )
        {
            StringRef *pStringRef = (StringRef *) (pString - sizeof(StringRef));
    
            // Is it already a ref which we can inc the refcount?
            if (Magic != pStringRef->magic)
            {
                ScopeLock lock( m_Lock );
    
               #ifdef ENABLE_STRINGPOOL_VALIDATION
                  Debug::Print( Debug::TypeInfo, "%s(%d): StringRef \"%s\"\r\n", pFile, line, pString );
               #endif
               
                //if not, see if it's in the hash table
                if ( false == m_Hash.Get( pString, &pStringRef ) )
                {
                    //if not, create a new one
                    size_t length = strlen( pString ) + 1;
                
                    char *pMemory = (char *) malloc( length + sizeof(StringRef) );

                    pStringRef = (StringRef *) pMemory;
                    pStringRef->magic = StringPool::Magic;
                    pStringRef->refCount = 0;
                
                    pMemory += sizeof(StringRef);

                    String::Copy( pMemory, pString, length );

                    m_Hash.Add( pMemory, pStringRef );
                }
            }

            AtomicIncrement(&pStringRef->refCount);

            return (const char *)pStringRef + sizeof(StringRef);
        }

        return NULL;
    }

    void Free(
    #ifdef ENABLE_STRINGPOOL_VALIDATION
        const char *pFile,
        int line,
    #endif
        const char *pString
        )
    {
        if ( pString )
        {
            StringRefValidate(pString);

            #ifdef ENABLE_STRINGPOOL_VALIDATION
               Debug::Print( Debug::TypeInfo, "%s(%d): StringRel \"%s\"\r\n", pFile, line, pString );
            #endif

            StringRef *pStringRef = (StringRef *) (pString - sizeof(StringRef));

            Debug::Assert( Condition( pStringRef->refCount > 0 ), "StringRef refcount is 0 but still in string pool" );

            AtomicDecrement(&pStringRef->refCount);

            //if ( 0 == pStringRef->refCount )
            //{
            //    m_Hash.Remove( pString );
            //    free( pStringRef );
            //}
        }
    }

    bool Validate(const char *pString)
    {
        StringRef *pStringRef = (StringRef *) (pString - sizeof(StringRef));
        return (pStringRef->magic == Magic);
    }

    void Destroy( void )
    {
        List<StringRef *> refs;
        refs.Create( );

        {
            Enumerator<const char *, StringRef *> e = m_Hash.GetEnumerator( );

            while ( e.EnumNext( ) )
            {
                refs.Add( e.Data( ) );
            }

        }

        uint32 i;

        for ( i = 0; i < refs.GetSize( ); i++ )
            free( refs.GetAt( i ) );

        refs.Destroy( );
        m_Hash.Destroy( );
    }

public:
    static const char *Deserialize(
        IInputStream *pStream
        )
    {
        char idString[ 1024 ];

        int length;
        pStream->Read( &length, sizeof( length ) );

        char *pId;

        if ( length > sizeof( idString ) )
            pId = (char *) malloc( length + 1 );
        else
            pId = idString;

        pStream->Read( pId, length );

        pId[ length ] = NULL;

        const char *pStringRef = StringRef( pId );

        if ( pId != idString )
            free( pId );

        return pStringRef;
    }
};
