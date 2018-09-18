#pragma once


#include "BuildOptions.h"
#include "MemoryAllocation.h"

#include <math.h>
#include <float.h>

#include "UtilityMath.h"
#include "UtilityString.h"
#include "Types.h"
#include "Debug.h"
#include <typeinfo>

#ifdef IOS
   using namespace std;
#elif defined ANDROID
   using namespace std;
#elif defined LINUX
   using namespace std;
#elif defined MAC
   using namespace std;
#elif defined WIN32
#else
   #error Platform not defined
#endif

#define NullHandle (int)0
#define BreakIf(v)\
    if ( v )\
        break;

#define Align(value, alignment) (((value) + ((alignment) - 1)) / (alignment) * (alignment))

//We want everything to be node but that causes havoc with visual studio debugging
//so I've called it AnttrapNode
#define Node AnttrapNode

//32 + . + 8 extension + null
static const uint32 MaxNameLength = 42;

extern float DeltaSeconds;

class Id;

// Can be implemented differently for application
void BuildDataPath(
    char *pOut, 
    const char *pFilename, 
    uint32 length,
    bool createDirectories
    );

void BuildConfigPath(
    char *pOut, 
    const char *pFilename, 
    uint32 length,
    bool createDirectories
    );

void BuildPlatformConfigPath(
    char *pOut, 
    const char *pFilename, 
    uint32 length,
    bool createDirectories
    );

void BuildPlatformDataPath(
    char *pOut, 
    const char *pFilename, 
    uint32 length,
    bool createDirectories
    );

typedef uint32 PipelineContext;
typedef uint32 VertexContext;
typedef uint32 ViewportContext;
