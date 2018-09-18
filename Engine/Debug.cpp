#include "EnginePch.h"

#include "Debug.h"
#include "Log.h"
#include "ResourceWorld.h"
#include "HashTable.h"
#include "Node.h"

#ifndef LINUX
#include "TextArea.h"
#include "RenderWorld.h"
#include "InputSystem.h"
#endif

LogCallback g_pLogCallback = NULL;

struct VisualAssert
{
   Node    *m_pNode;
   ResourceHandle m_FontMap;
   ResourceHandle m_Background;
   ResourceHandle m_Material;
   ResourceHandle m_FontTexture;
   float          m_Width;
   float          m_Height;

   bool m_Enable;
};

struct VisualLog
{
   struct Message
   {
      char m[64];
   };

   Node    *m_pNode;
   TextArea m_TextArea;
   List<Message> m_ThreadedMessages;
   Lock     m_Lock;

   bool     m_IsShowing;
   bool     m_Enable;
};

void ProcessMessages( void );

VisualAssert Debug::m_VisualAssert;
VisualLog    Debug::m_VisualLog;

int Debug::m_AllowFlags = Debug::TypeAll;

bool Debug::m_Created;
bool Debug::m_Abort;

uint32 DescHash(
   const Debug::Desc desc
)
{
   return HashFunctions::StringHash( desc.pFile );
}

bool DescCompare(
   const Debug::Desc desc1,
   const Debug::Desc desc2
)
{
   return desc1.pFile == desc2.pFile &&
      desc1.line == desc2.line;
}

#ifdef ENABLE_ASSERT
static HashTable<Debug::Desc, bool> m_SkipLine;
#endif

void Debug::Create( void )
{
#ifdef ENABLE_ASSERT
   m_SkipLine.Create( 16, 16, DescHash, DescCompare );
#endif
   m_Created = true;
}

void Debug::Destroy( void )
{
   m_Created = false;
#ifdef ENABLE_ASSERT
   m_SkipLine.Destroy( );
#endif
}

void Debug::Quit( void )
{
   m_Abort = true;
   exit( 1 );
}

#ifndef LINUX

void Debug::Update( void )
{
   if ( m_VisualLog.m_Enable )
   {
      ScopeLock lock( m_VisualLog.m_Lock );

      for ( uint32 i = 0; i < m_VisualLog.m_ThreadedMessages.GetSize(); i++ )
         m_VisualLog.m_TextArea.Print( m_VisualLog.m_ThreadedMessages.GetPointer(i)->m );

      m_VisualLog.m_ThreadedMessages.Clear( );
   }
}

void Debug::EnableVisualAssert(
   ResourceHandle back_material,
   ResourceHandle front_material,
   ResourceHandle font_map,
   ResourceHandle font_texture,
   float width,
   float height
)
{
   m_VisualAssert.m_pNode = new Node;
   m_VisualAssert.m_pNode->Create( );

   ResourceHandle handle( Id::Create( ) );
   handle.Bind( NULL, m_VisualAssert.m_pNode );

   Component *pComponent = new Component;
   pComponent->Create( Id::Create( ) );
   m_VisualAssert.m_pNode->AddComponent( pComponent );

   m_VisualAssert.m_Background = back_material;
   m_VisualAssert.m_FontMap = font_map;
   m_VisualAssert.m_Material = front_material;
   m_VisualAssert.m_FontTexture = font_texture;
   m_VisualAssert.m_Width = width;
   m_VisualAssert.m_Height = height;

   m_VisualAssert.m_Enable = true;
}

void Debug::EnableVisualLog(
   ResourceHandle back_material,
   ResourceHandle front_material,
   ResourceHandle font_map,
   ResourceHandle font_texture,
   float x,
   float y,
   float width,
   float height
)
{
   m_VisualLog.m_Enable = true;
   m_VisualLog.m_IsShowing = false;

   m_VisualLog.m_pNode = new Node;
   m_VisualLog.m_pNode->Create( );
   m_VisualLog.m_ThreadedMessages.Create( );

   ResourceHandle handle( Id::Create( ) );
   handle.Bind( NULL, m_VisualLog.m_pNode );

   Component *pComponent = new Component;
   pComponent->Create( Id::Create( ) );
   m_VisualLog.m_pNode->AddComponent( pComponent );

   IdList groups; groups.Create( ); groups.Add( Id( "UI" ) );
   m_VisualLog.m_TextArea.Create( pComponent, Vector2( width, height ),
      Vector( 1, 1, 1, 1 ), back_material, front_material, font_map, font_texture, groups );

   m_VisualLog.m_TextArea.SetAlign( TextArea::AlignLeft, TextArea::AlignBottom );
   m_VisualLog.m_TextArea.SetMaxLines( 1024 );

   Transform transform = Math::IdentityTransform( );
   transform.SetTranslation( Vector( x, y ) );
   m_VisualLog.m_pNode->SetWorldTransform( transform );

   groups.Destroy( );
}

void Debug::PositionVisualLog(
   float x,
   float y,
   float width,
   float height
)
{
   if ( m_VisualLog.m_Enable )
   {
      Transform transform = Math::IdentityTransform( );
      transform.SetTranslation( Vector( x, y ) );
      m_VisualLog.m_pNode->SetWorldTransform( transform );

      m_VisualLog.m_TextArea.SetSize( Vector2(width, height) );
   }
}

void Debug::ShowVisualLog( void )
{
   if ( true == m_VisualLog.m_Enable && 
        false == m_VisualLog.m_IsShowing )
   {
      m_VisualLog.m_IsShowing = true;
      m_VisualLog.m_TextArea.AddToScene( );
   }
}

void Debug::HideVisualLog( void )
{
   if ( true == m_VisualLog.m_Enable && 
        true == m_VisualLog.m_IsShowing )
   {
      m_VisualLog.m_IsShowing = false;
      m_VisualLog.m_TextArea.RemoveFromScene( );
   }
}

void Debug::DisableVisualAssert( void )
{
   if ( true == m_VisualAssert.m_Enable )
   {
      m_VisualAssert.m_Background = NullHandle;
      m_VisualAssert.m_FontMap = NullHandle;
      m_VisualAssert.m_Material = NullHandle;
      m_VisualAssert.m_FontTexture = NullHandle;

      m_VisualAssert.m_pNode->Destroy( );
      delete m_VisualAssert.m_pNode;

      m_VisualAssert.m_Enable = false;
   }
}

void Debug::DisableVisualLog( void )
{
   if ( true == m_VisualLog.m_Enable )
   {
      m_VisualLog.m_TextArea.Destroy( );
      m_VisualLog.m_ThreadedMessages.Destroy( );
      m_VisualLog.m_pNode->Destroy( );
      delete m_VisualLog.m_pNode;

      m_VisualLog.m_Enable = false;
   }
}

#endif


#ifdef ENABLE_ASSERT

Lock s_AssertLock;

void Debug::Assert(
   const Desc &desc,
   const char *pMessage,
   ...
)
{
   if ( false == desc.condition && false == m_Abort )
   {
      ScopeLock lock( s_AssertLock );

      if ( false == m_Created )
      {
         Debug::Print( Debug::TypeError, pMessage );
         Debug::Break( );
      }

      bool skip;
      if ( m_SkipLine.Get( desc, &skip ) )
         if ( true == skip ) return;

      static bool reentrant = false;

      char message[ 1024 ];

      va_list args;

      va_start( args, pMessage );

      String::FormatV( message, sizeof( message ), pMessage, args );

      Debug::Print( Debug::TypeError, "%s(%d): %s\n", desc.pFile, desc.line, message );
      LOG_FLUSH( message );

      va_end( args );

      if ( true == reentrant )
         Debug::Print( Debug::TypeError, "Assert is reentrant, halting\n" );

#ifndef LINUX
      if ( false == reentrant && true == m_VisualAssert.m_Enable )
      {
         RenderWorld::Instance( ).AcquireLock( );

         reentrant = true;

         IdList groups; groups.Create( ); groups.Add( Id( "UI" ) );
         TextArea textArea;
         textArea.Create( m_VisualAssert.m_pNode->GetComponent<Component>( ), Vector2( m_VisualAssert.m_Width, m_VisualAssert.m_Height ),
            Vector( 1, 1, 1, 1 ), m_VisualAssert.m_Background, m_VisualAssert.m_Material, m_VisualAssert.m_FontMap, m_VisualAssert.m_FontTexture, groups );

         groups.Destroy( );

         textArea.SetAlign( TextArea::AlignLeft, TextArea::AlignTop );

         textArea.PrintArgs( message );
         textArea.PrintArgs( "" );
         textArea.PrintArgs( "Func: %s", desc.pFunc );
         textArea.PrintArgs( "File: %s", desc.pFile );
         textArea.PrintArgs( "Line: %d", desc.line );
         textArea.PrintArgs( "" );
         textArea.PrintArgs( "" );
         textArea.PrintArgs( "1: Break" );
         textArea.PrintArgs( "2: Skip" );
         textArea.PrintArgs( "Esc: Quit" );

         textArea.AddToScene( );

         while ( 1 )
         {
            RenderWorld::Instance( ).GetRenderData( );
            RenderWorld::Instance( ).Render( );
            Thread::YieldThread( );

            ProcessMessages( );

            {
               static const char *pKey = StringRef( "DebugBreak" );
               InputMap mapping( pKey );

               if ( mapping.IsPressed( ) )
               {
                  Debug::Break( );
               }
            }

            {
               static const char *pKey = StringRef( "DebugSkip" );
               InputMap mapping( pKey );

               if ( mapping.IsPressed( ) )
               {
                  m_SkipLine.Remove( desc );
                  m_SkipLine.Add( desc, true );
                  break;
               }
            }

            {
               static const char *pKey = StringRef( "DebugQuit" );
               InputMap mapping( pKey );

               if ( mapping.IsPressed( ) )
               {
                  Debug::Quit( );
                  break;
               }
            }
         }

         textArea.RemoveFromScene( );
         textArea.Destroy( );

         RenderWorld::Instance( ).Unlock( );
      }
      else
      {
         Debug::Break( );
      }
#else
      while ( 1 )
      {
      }
#endif

      reentrant = false;
   }
}
#else //ENABLE_ASSERT
void Debug::Assert(
   const Desc &desc,
   const char *pMessage,
   ...
)
{}
#endif

#ifdef ENABLE_DEBUGPRINT
void Debug::Print(
   const Desc &desc,
   Type type,
   const char *pMessage,
   ...
)
{
   if ( true == desc.condition ) return;

   int allow = 0;

   allow |= ( type & m_AllowFlags );

   if ( allow )
   {
      char message[ 1024 ];

      va_list args;

      va_start( args, pMessage );

      String::FormatV( message, sizeof( message ), pMessage, args );

      Debug::Print( type, message );

      va_end( args );
   }
}
void Debug::Print(
   Type type,
   const char *pMessage,
   ...
)
{
   int allow = 0;

   allow |= ( type & m_AllowFlags );

   if ( 0 == allow ) return;

   char message[ 1024 ];

   va_list args;

   va_start( args, pMessage );

   String::FormatV( message, sizeof( message ), pMessage, args );

   extern LogCallback g_pLogCallback;

   printf( "%s", message );
   if ( NULL != g_pLogCallback ) g_pLogCallback( message, (int) type );

#ifdef ANDROID
   int t = ANDROID_LOG_INFO;

   if ( type == TypeWarning ) t = ANDROID_LOG_WARN;
   else if ( type == TypeError ) t = ANDROID_LOG_ERROR;

   __android_log_print( t, "CWP", message );
#endif

#ifdef WIN32
   OutputDebugString( message );
#endif

   va_end( args );

   if ( true == m_VisualLog.m_Enable )
   {
      if ( Thread::IsMainThread( ) )
         m_VisualLog.m_TextArea.Print( message );
      else
      {
         ScopeLock lock( m_VisualLog.m_Lock );

         VisualLog::Message m;
         String::Copy( m.m, message, sizeof(m.m) );
         m_VisualLog.m_ThreadedMessages.Add( m );
      }
   }
}
#else
void Debug::Print(
   const Desc &desc,
   Type type,
   const char *pMessage,
   ...
)
{}
void Debug::Print(
   Type type,
   const char *pMessage,
   ...
)
{}
#endif


#ifdef WIN32
void Debug::ProcessMessages( void )
{
   MSG message;

   if ( PeekMessage( &message, NULL, NULL, NULL, PM_REMOVE ) )
   {
      TranslateMessage( &message );
      DispatchMessage( &message );
   }
}
#elif defined IOS
void Debug::ProcessMessages( void )
{
}
#elif defined MAC
void Debug::ProcessMessages( void )
{
}
#elif defined ANDROID
void Debug::ProcessMessages( void )
{
}
#else
#error Platform not defined
#endif
