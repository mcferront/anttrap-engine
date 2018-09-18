#pragma once

#include "EngineGlobal.h"

class HashFunctions
{
public:
    static uint32 StringHash(
        const char *pString
        );

    static bool StringCompare(
        const char *pString1,
        const char *pString2
        );

    static uint32 StringRefHash(
        const char *pString
        );

    static bool StringRefCompare(
        const char *pString1,
        const char *pString2
        );


    static uint32 NUIntHash(
        nuint value
        )
    {
        return (uint32) value;
    }

    static bool NUIntCompare(
        nuint value1,
        nuint value2
        )
    {
        return value1 == value2;
    }

    static uint32 UIntHash(
       uint32 value
    )
    {
       return (uint32) value;
    }

    static bool UIntCompare(
       uint32 value1,
       uint32 value2
    )
    {
       return value1 == value2;
    }

    static uint32 IntHash(
        int value
        )
    {
        return (int) value;
    }

    static bool IntCompare(
        int value1,
        int value2
        )
    {
        return value1 == value2;
    }
};
