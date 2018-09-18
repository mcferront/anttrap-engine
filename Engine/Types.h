#pragma once

#define MAX_UINT 0xffffffff

typedef unsigned int     uint32;
typedef signed int       sint32;
typedef unsigned short   uint16;

#ifdef WIN32
   typedef __w64 unsigned int nuint;
   typedef unsigned __int64 uint64;

   typedef signed __int64 sint64;
   
   typedef int ThreadId;
   typedef BYTE byte;
#endif

#if defined IOS || defined LINUX || defined MAC || defined ANDROID
   typedef unsigned char BYTE;
   typedef unsigned char byte;
   typedef unsigned long nuint;
   typedef unsigned long long uint64;
   typedef signed   long long sint64;

   typedef pthread_t ThreadId;
   
   #ifndef TRUE
      #define TRUE 1
   #endif
   
   #ifndef FALSE
      #define FALSE 0
   #endif
#endif

