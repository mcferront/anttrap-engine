#pragma once

#include "EngineGlobal.h"
#include "List.h"

class Thread
{
private:
   static pthread_t s_pMainThreadId;

public:
   static void InitMainThread( void )
   {
      s_pMainThreadId = pthread_self( );
   }

   static bool IsMainThread( void )
   {
      return s_pMainThreadId == pthread_self( );
   }

private:
   pthread_t       m_ThreadId;
   pthread_attr_t  m_Attr;

   bool m_Run;
   bool m_IsValid;
   
public:
   Thread( void )
   {
      m_IsValid = false;
   }

   virtual ~Thread( void ) {}

   void Run( void )
   {
      m_Run = true;

      pthread_attr_init( &m_Attr );	
      pthread_attr_setstacksize( &m_Attr, 64 * 1024 );
      pthread_create( &m_ThreadId, &m_Attr, (void *(*)(void *))ThreadProc, this );
   };

   void Stop( void ) 
   { 
      m_Run = false; 
         
      while ( true == m_IsValid )
      {
         Sleep( 1 );
      }
   }

   bool ShouldRun( void ) { return m_Run; }

   virtual void OnThreadRun( void ) = 0;

public:
   static void YieldThread( void )
   {
      sched_yield( );
   }

   static void Sleep(
      int milliseconds
   )
   {
      struct timespec timeOut,remains;

      timeOut.tv_sec = milliseconds / 1000;

      int remaining = (int) (milliseconds - (timeOut.tv_sec * 1000));

      timeOut.tv_nsec = remaining * 1000000;

      while ( -1 == nanosleep(&timeOut, &remains) )
      {
         timeOut = remains;
      } 
   }

   static ThreadId GetCurrentThreadId( void ) { return pthread_self( ); }
   
private:
   static void ThreadProc(
      void *pParam
   )
   {
      ((Thread *)pParam)->m_IsValid = true;
      ((Thread *)pParam)->OnThreadRun( );
      ((Thread *)pParam)->m_IsValid = false;

      pthread_exit( 0 );
   }
};
