#pragma once

#include <stdio.h>

class String
{
public:
    static wchar_t *ToWideChar(
        const char *pString
    )
    {
        // TODO: remove when we go full unicode
        static wchar_t wideChar[ 256 ];
        MultiByteToWideChar( CP_UTF8, MB_PRECOMPOSED, pString, -1, wideChar, _countof(wideChar) );

        return wideChar;
    }

    static inline int FormatV(
        char *pString,
        size_t length,
        const char *pFormat,
        va_list args
        )
    {
        if ( length > 0 )
        {
            int written = vsnprintf( pString, length - 1, pFormat, args);
            pString[ length - 1 ] = NULL;
            return written;
        }

        return 0;
    }

    static inline void Format(
        char *pString,
        size_t length,
        const char *pFormat,
        ...
        )
    {
        va_list args;

        va_start( args, pFormat );

        FormatV( pString, length, pFormat, args );

        va_end( args ); 
    }

    static inline void Replace(
        char *pString,
        char oldChar,
        char newChar
        )
    {
        while ( *pString )
        {
            if ( *pString == oldChar ) 
                *pString = newChar;

            ++pString;
        }
    }

    static inline void Copy(
        char *pDest,
        const char *pSource,
        size_t size
        )
    {
        if ( size > 0 )
        {
            strncpy( pDest, pSource, size - 1 );
            pDest[ size - 1 ] = NULL;
        }
    }

    static inline void UnsafeCopy(
        char *pDest,
        const char *pSource,
        size_t size
        )
    {
        if ( size > 0 )
        {
            strncpy( pDest, pSource, size );
        }
    }

    static inline bool StartsWith(
        const char *pString,
        const char *pStart
        )
    {
        size_t length      = strlen( pString );
        size_t startLength = strlen( pStart );

        // end is longer than the string we're checking
        if ( length < startLength )
            return false;

        return 0 == strncmp( pString, pStart, startLength );
    }

    static inline bool EndsWith(
        const char *pString,
        const char *pEnd
        )
    {
        size_t length    = strlen( pString );
        size_t endLength = strlen( pEnd );

        // end is longer than the string we're checking
        if ( length < endLength )
            return false;

        const char *pStartFrom = pString + length - endLength;
        return 0 == strcmp( pStartFrom, pEnd );
    }
};
