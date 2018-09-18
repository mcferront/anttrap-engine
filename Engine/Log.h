#pragma once

#include "EngineGlobal.h"

#define LOG(x) Log::Instance( ).Write( __FILE__, __LINE__, (x) )
#define LOG_FLUSH(x) Log::Instance( ).Write( __FILE__, __LINE__, (x) ); Log::Instance( ).Flush( )

class Log
{
private:
   FILE *m_pFile;
   char m_Fullpath[ 256 ];

public:
   static Log &Instance( void );

public:
   void Create(
      const char *pPath
   );

   void Destroy( void );

   void Show( void );

   void Write( 
      const char *pFile, 
      int line, 
      const char *pMessage 
   );

   void Flush( void );
};
