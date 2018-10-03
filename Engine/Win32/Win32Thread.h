#pragma once

#include "EngineGlobal.h"
#include "List.h"

struct ThreadEvent
{
   HANDLE t;
};

struct Semaphore
{
   HANDLE s;
};

class Thread
{
private:
   static DWORD s_MainThreadId;

public:
   static void InitMainThread( void )
   {
      s_MainThreadId = GetCurrentThreadId( );
   }

   static bool IsMainThread( void )
   {
      return s_MainThreadId == GetCurrentThreadId( );
   }

   static ThreadEvent CreateEvent( bool manualReset = false, bool startSignaled = false )
   {
      return ThreadEvent { ::CreateEvent( NULL, manualReset ? TRUE : FALSE, startSignaled ? TRUE : FALSE, NULL ) };
   }

   static Semaphore CreateSemaphore( int initialCount, int maxCount )
   {
      return Semaphore { ::CreateSemaphore( NULL, initialCount, maxCount, NULL ) };
   }

   static void DeleteEvent( ThreadEvent t )
   {
      CloseHandle( t.t );
   }

   static void DeleteSemaphore( Semaphore s )
   {
      CloseHandle( s.s );
   }

   static void Signal( ThreadEvent t )
   {
      SetEvent( t.t );
   }

   static void Signal( Semaphore s )
   {
      ReleaseSemaphore( s.s, 1, NULL );
   }

   static bool Wait( ThreadEvent t, sint32 timeout = INFINITE )
   {
      return Wait( t.t, timeout );
   }

   static bool Wait( Semaphore s, sint32 timeout = INFINITE )
   {
      return Wait( s.s, timeout );
   }

   static bool Wait( HANDLE t, sint32 timeout )
   {
      //if ( timeout != INFINITE )
         return WAIT_OBJECT_0 == WaitForSingleObject( t, timeout );

      //DWORD result;

      //do
      //{
      //   result = WaitForSingleObject( t, 10 );

      //}
      //while ( result == WAIT_TIMEOUT );

      //return result == WAIT_OBJECT_0;
   }

private:
   HANDLE m_hThread;
   DWORD  m_ThreadId;
   bool   m_Run;

public:
   Thread( void )
   {
      m_hThread = INVALID_HANDLE_VALUE;
   }

   virtual ~Thread( void ) {}

   void Run( 
      uint32 logicalProcessor = 0xffffffff 
      )
   {
      m_Run = true;

      m_hThread = CreateThread( NULL, 128 * 1024, ThreadProc, this, 0, &m_ThreadId );
      
      if ( logicalProcessor != 0xffffffff )
      {
         DWORD result = SetThreadAffinityMask( m_hThread, 1 << logicalProcessor );
         Debug::Assert( Condition(result != 0), "Failed to SetThreadAffinityMask %d", GetLastError() );
      }
   };

   void Stop(
       bool block = true
   ) 
   { 
      m_Run = false; 
      
      if ( false == block ) return;

      while ( INVALID_HANDLE_VALUE != m_hThread )
      {
         Sleep( 1 );
      }
   }

   void Join( void )
   { 
      while ( INVALID_HANDLE_VALUE != m_hThread )
         Sleep( 1 );
   }

   bool ShouldRun( void ) const { return m_Run; }

   ThreadId GetThreadId( void ) const { return m_ThreadId; }

   void Terminate( void )
   {
      TerminateThread( m_hThread, 0 );
   }

   virtual void OnThreadRun( void ) = 0;

public:
   static void YieldThread( void )
   {
      ::Sleep( 0 );
   }

   static void Sleep(
      int milliseconds
   )
   {
      ::Sleep( milliseconds );
   }

   static ThreadId GetCurrentThreadId( void )
   {
      return ::GetCurrentThreadId( );
   }

private:
   static DWORD WINAPI ThreadProc(
      void *pParam
   )
   {
      ((Thread *)pParam)->OnThreadRun( );

      HANDLE handle = ((Thread *)pParam)->m_hThread;

      ((Thread *)pParam)->m_hThread = INVALID_HANDLE_VALUE;
      
      CloseHandle( handle );
      
      return TRUE;
   }
};

inline uint64 AtomicIncrement( uint64 *v )
{
   return (uint64) InterlockedIncrement64( (sint64 *) v );
}

inline uint64 AtomicDecrement( uint64 *v )
{
   return (uint64) InterlockedDecrement64( (sint64 *) v );
}

inline uint32 AtomicIncrement( uint32 *v )
{
   return InterlockedIncrement( v );
}

inline uint32 AtomicDecrement( uint32 *v )
{
   return InterlockedDecrement( v );
}

inline uint32 AtomicExchange( uint32 *v, uint32 nv )
{
    return InterlockedExchange( v, nv );
}
