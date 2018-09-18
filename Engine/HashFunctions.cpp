#include "EnginePch.h"

#include "HashFunctions.h"
#include "StringPool.h"

uint32 HashFunctions::StringRefHash(
    const char *pString
    )
{
    StringRefValidate( pString );
    return NUIntHash( (nuint) pString );

    //hash routine found here
    //http://www.azillionmonkeys.com/qed/hash.html

    //uint32 hash, temp;
    //const char *pData;
    //int remainder;

    //int length = (int) strlen( pString );

    //hash = length;
    //pData = pString;

    //remainder = length % 4;
    //length >>= 2;

    //while ( length > 0 )
    //{
    //    hash += *(uint16 *) pData;
    //    temp = ( *(uint16 *) ( pData + 2 ) << 11 ) ^ hash;
    //    hash = ( hash << 16 ) ^ temp;
    //    pData += 2 * sizeof ( uint16 );
    //    hash += hash >> 11;

    //    --length;
    //}

    //switch ( remainder )
    //{
    //case 3:
    //    hash += *(uint16 *) pData;
    //    hash ^= hash << 16;
    //    hash ^= pData[ sizeof ( uint16 ) ] << 18;
    //    hash += hash >> 11;
    //    break;

    //case 2:
    //    hash += *(uint16 *) pData;
    //    hash ^= hash << 11;
    //    hash += hash >> 17;
    //    break;

    //case 1:
    //    hash += *pData;
    //    hash ^= hash << 10;
    //    hash += hash >> 1;
    //}

    //hash ^= hash << 3;
    //hash += hash >> 5;
    //hash ^= hash << 4;
    //hash += hash >> 17;
    //hash ^= hash << 25;
    //hash += hash >> 6;

    //return hash;
}

bool HashFunctions::StringRefCompare(
    const char *pString1,
    const char *pString2
    )
{
    return StringRefEqual( pString1, pString2 );
}

uint32 HashFunctions::StringHash(
    const char *pString
    )
{
    //hash routine found here
    //http://www.azillionmonkeys.com/qed/hash.html

    uint32 hash, temp;
    const char *pData;
    int remainder;

    int length = (int) strlen( pString );

    hash = length;
    pData = pString;

    remainder = length % 4;
    length >>= 2;

    while ( length > 0 )
    {
        hash += *(uint16 *) pData;
        temp = ( *(uint16 *) ( pData + 2 ) << 11 ) ^ hash;
        hash = ( hash << 16 ) ^ temp;
        pData += 2 * sizeof ( uint16 );
        hash += hash >> 11;

        --length;
    }

    switch ( remainder )
    {
    case 3:
        hash += *(uint16 *) pData;
        hash ^= hash << 16;
        hash ^= pData[ sizeof ( uint16 ) ] << 18;
        hash += hash >> 11;
        break;

    case 2:
        hash += *(uint16 *) pData;
        hash ^= hash << 11;
        hash += hash >> 17;
        break;

    case 1:
        hash += *pData;
        hash ^= hash << 10;
        hash += hash >> 1;
    }

    hash ^= hash << 3;
    hash += hash >> 5;
    hash ^= hash << 4;
    hash += hash >> 17;
    hash ^= hash << 25;
    hash += hash >> 6;

    return hash;
}

bool HashFunctions::StringCompare(
    const char *pString1,
    const char *pString2
    )
{
    return 0 == strcmp(pString1, pString2);
}
