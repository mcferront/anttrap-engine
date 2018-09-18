#pragma once

#include "EngineGlobal.h"

#if defined (ENABLE_ASSERT) || defined (ENABLE_DEBUGPRINT)
#define Condition(x) Debug::Desc(x, __FILE__, __FUNCTION__, __LINE__)
#else
#define Condition(x) Debug::Desc()
#endif

typedef void( *LogCallback )( const char *, int );

class ResourceHandle;
class Id;

struct VisualAssert;
struct VisualLog;

class Debug
{
private:
    static int  m_AllowFlags;
    static bool m_Created;
    static bool m_Abort;

    static VisualAssert  m_VisualAssert;
    static VisualLog     m_VisualLog;

public:
    enum Type
    {
        TypeNone = 0,

        TypeInfo = 1 << 0,
        TypeWarning = 1 << 1,
        TypeError = 1 << 2,
        TypeScript = 1 << 3,

        TypeAll = 0xffffffff,
    };

    class Desc
    {
    public:
        const char *pFile;
        const char *pFunc;
        int   line;
        bool  condition;

    public:
        Desc( )
        {}

        Desc( bool c, const char *pF, const char *pFunction, int l )
        {
            condition = c;
            pFile = pF;
            line = l;
            pFunc = pFunction;
        }
    };

    static void Create( void );
    static void Destroy( void );
    static void Update( void );

    static void EnableVisualAssert(
        ResourceHandle back_material,
        ResourceHandle front_material,
        ResourceHandle font_map,
        ResourceHandle font_texture,
        float width,
        float height
        );

    static void EnableVisualLog(
        ResourceHandle back_material,
        ResourceHandle front_material,
        ResourceHandle font_map,
        ResourceHandle font_texture,
        float x,
        float y,
        float width,
        float height
        );

    static void PositionVisualLog(
       float x,
       float y,
       float width,
       float height
    );

    static void ShowVisualLog( void );
    static void HideVisualLog( void );

    static void DisableVisualAssert( void );
    static void DisableVisualLog( void );

    static void SetAllowFlags(
        int types
        )
    {
        m_AllowFlags = types;
    }

public:
    static void Quit( void );
    static void ProcessMessages( void );

    static void Assert(
        const Desc &desc,
        const char *pMessage,
        ...
        );

    static void Print(
        const Desc &desc,
        Type type,
        const char *pMessage,
        ...
        );

    static void Print(
        Type type,
        const char *pMessage,
        ...
        );

private:
    static void Break( void )
    {
#ifdef WIN32
        _asm
        {
            int 3;
        }
#elif defined ANDROID
        Debug::Print( TypeError, "Debug::Break - HALTING EXECUTION\n" );
        exit( 1 );
#endif
    }
};
