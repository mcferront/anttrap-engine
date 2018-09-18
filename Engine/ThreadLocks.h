#pragma once

#include "EngineGlobal.h"

class CriticalSection
{
   friend class CriticalSectionScope;

private:
#if defined WIN32
   CRITICAL_SECTION c;
#else
   #error "Platform Undefined"
#endif

public:
   CriticalSection( uint32 spinCount )
   {
#if defined WIN32
      BOOL result = InitializeCriticalSectionAndSpinCount( &c, spinCount );
      Debug::Assert( Condition(TRUE == result), "Could not create Critical Section" );
#else
   #error "Platform Undefined"
#endif
   }

   ~CriticalSection( void )
   {
#if defined WIN32
      DeleteCriticalSection( &c );
#else
   #error "Platform Undefined"
#endif
   }
};

class Lock
{
private:
#if defined WIN32
   HANDLE  m_hMutex;
#elif defined IOS || defined LINUX || defined MAC || defined ANDROID
   pthread_mutex_t m_Mutex;
#else
   #error "Platform Undefined"
#endif

public:
   Lock( void )
   {
   #if defined WIN32
      m_hMutex = CreateMutex( NULL, FALSE, NULL );
   #elif defined IOS || defined LINUX || defined MAC || defined ANDROID
      pthread_mutexattr_t attr;
      pthread_mutexattr_init( &attr );
      pthread_mutexattr_settype( &attr, PTHREAD_MUTEX_RECURSIVE );
      pthread_mutex_init( &m_Mutex, &attr );
   #else
      #error "Platform Undefined"
   #endif
   }

   ~Lock( void )
   {
   #if defined WIN32
      CloseHandle( m_hMutex ); 
   #elif defined IOS || defined LINUX || defined MAC || defined ANDROID
      pthread_mutex_destroy( &m_Mutex );
   #else
      #error "Platform Undefined"
   #endif
   }

   void Acquire( void ) const
   {
   #if defined WIN32
      WaitForSingleObject( m_hMutex, INFINITE );      
   #elif defined IOS || defined LINUX || defined MAC || defined ANDROID
      pthread_mutex_lock( &m_Mutex );
   #else
      #error "Platform Undefined"
   #endif
   }

   void Release( void ) const
   {
   #if defined WIN32
      ReleaseMutex( m_hMutex );
   #elif defined IOS || defined LINUX || defined MAC || defined ANDROID
      pthread_mutex_unlock( &m_Mutex );
   #else
      #error "Platform Undefined"
   #endif
   }
};

class CriticalSectionScope
{
private:
   CriticalSection *pSection;

public:
   CriticalSectionScope(
      CriticalSection &section
      )
   {
      pSection = &section;
      EnterCriticalSection( &pSection->c );
   }

   ~CriticalSectionScope( void )
   {
      LeaveCriticalSection( &pSection->c );
   }
};

class ScopeLock
{
protected:
   const Lock *m_pLock;

public:
   ScopeLock( const Lock &lock )
   {
      m_pLock = &lock;
      m_pLock->Acquire( );
   }

   ~ScopeLock( void )
   {
      m_pLock->Release( );
   }
};


