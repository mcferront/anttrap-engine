#include "EnginePch.h"

#include "Log.h"
#include "UtilityString.h"

Log &Log::Instance( void )
{
   static Log s_log;
   return s_log;
}

void Log::Create(
   const char *pPath
)
{
#ifdef WIN32
   String::Format( m_Fullpath, sizeof(m_Fullpath), "%s\\AppLog.txt", pPath );
#else
   String::Format( m_Fullpath, sizeof(m_Fullpath), "%s/AppLog.txt", pPath );   
#endif
   m_pFile = fopen( m_Fullpath, "w" );
}

void Log::Destroy( void )
{
   if ( NULL != m_pFile ) fclose( m_pFile );

   m_pFile = NULL;
}

void Log::Show( void )
{
   Flush( );

#ifdef WIN32
   ShellExecute( NULL, "open", m_Fullpath, NULL, NULL, SW_SHOWNORMAL );
#endif
}

void Log::Write( 
   const char *pFile, 
   int line, 
   const char *pMessage 
)
{
   if ( NULL != m_pFile ) fprintf( m_pFile, "%s(%d): %s\n", pFile, line, pMessage );
   Debug::Print( Debug::TypeInfo, "%s(%d): %s\n", pFile, line, pMessage );
}

void Log::Flush( void )
{
   if ( NULL != m_pFile ) fflush( m_pFile );
}


