#include "GamePch.h"
#include "Global.h"
#include "App.h"
#include "Window.h"
#include "InputSystem.h"
#include "VideoPlayer.h"
#include "GraphicsApi.h"
#include "Win32Messages.h"
#include "RegistryAsset.h"
#include "Log.h"
#include "resource.h"

#include <Shlobj.h>

#ifdef _DISTRIBUTION
//#define QA_BUILD
#endif

BOOL g_Quit = FALSE;

HDC      g_hDC;
HWND     g_hWnd;
Window   g_Window;
bool     g_ShiftDown;
int      g_BitsPerPixel;

List<VideoPlayer*> g_ValidVideoPlayers;

void CreateAppWindow(
   bool windowed,
   int *pX,
   int *pY,
   int *pWidth,
   int *pHeight
);

void Update( void );

uint32 UpperSymbol( uint32 key )
{
   if ( '1' == key ) return '!';
   if ( '2' == key ) return '@';
   if ( '3' == key ) return '#';
   if ( '4' == key ) return '$';
   if ( '5' == key ) return '%';
   if ( '6' == key ) return '^';
   if ( '7' == key ) return '&';
   if ( '8' == key ) return '*';
   if ( '9' == key ) return '(';
   if ( '0' == key ) return ')';
   if ( '-' == key ) return '_';
   if ( '=' == key ) return '+';
   if ( '\'' == key ) return '\"';
   if ( '\\' == key ) return '|';
   if ( ',' == key ) return '<';
   if ( '.' == key ) return '>';
   if ( '/' == key ) return '?';
   if ( '`' == key ) return '~';
   if ( '[' == key ) return '{';
   if ( ']' == key ) return '}';
   if ( ';' == key ) return ':';

   return key;
}

LRESULT WINAPI WinProc(
   HWND hwnd,
   UINT uMsg,
   WPARAM wparam,
   LPARAM lparam
)
{
   switch ( uMsg )
   {
   case WM_CREATE:
   {
      g_hDC = GetDC( hwnd );

      g_Window.Create( Id( "Window" ) );
      g_Window.SetHandle( hwnd );

      HCURSOR hCursor = LoadCursor( NULL, IDC_ARROW );
      SetCursor( hCursor );

      g_ShiftDown = false;

      return FALSE;
   }
   case WM_ACTIVATE:
   {
      //active and not minimized
      if ( WA_INACTIVE != LOWORD( wparam ) )
      {
         GpuDevice::Instance( ).ResetDevice( );
         UpdateWindow( hwnd );
      }

      return FALSE;
   }
#ifdef DIRECTX9
   case WM_VIDEOGRAPHNOTIFY:
   {
      VideoPlayer *pPlayer = (VideoPlayer *) lparam;

      if ( true == g_ValidVideoPlayers.Contains( pPlayer ) )
         pPlayer->DoEvents( );

      break;
   }
#endif
   case WM_KEYUP:
   {
      uint32 key = MapVirtualKey( wparam, MAPVK_VK_TO_CHAR );

      //shift
      if ( 0x10 == key )
      {
         g_ShiftDown = false;
      }
      //esc
      else if ( 0x1b == key )
      {
         InputMap mapping( "DebugQuit" );
         mapping.Release( );

         return FALSE;
      }
      //1
      else if ( 0x31 == key )
      {
         InputMap mapping( "DebugBreak" );
         mapping.Release( );

         return FALSE;
      }
      //2
      else if ( 0x32 == key )
      {
         InputMap mapping( "DebugSkip" );
         mapping.Release( );

         return FALSE;
      }
      else if ( VK_UP == LOWORD( wparam ) )
      {
         return FALSE;
      }
      else if ( VK_DOWN == LOWORD( wparam ) )
      {
         return FALSE;
      }
      else if ( VK_LEFT == LOWORD( wparam ) )
      {
         return FALSE;
      }
      else if ( VK_RIGHT == LOWORD( wparam ) )
      {
         return FALSE;
      }
      //w
      else if ( 0x57 == key )
      {
         return FALSE;
      }
      //s
      else if ( 0x53 == key )
      {
         return FALSE;
      }
      //a
      else if ( 0x41 == key )
      {
         return FALSE;
      }
      //d
      else if ( 0x44 == key )
      {
         return FALSE;
      }
      else if ( 'Q' == key )
      {
         return FALSE;
      }
      else if ( 'E' == key )
      {
         return FALSE;
      }
      else if ( 'X' == key )
      {
         return FALSE;
      }
      else if ( 'Y' == key )
      {
         return FALSE;
      }
      else if ( 'Z' == key )
      {
         return FALSE;
      }
      else if ( 'P' == key )
      {
         return FALSE;
      }
      else if ( 'I' == key )
      {
         return FALSE;
      }
      else if ( '`' == key )
      {
         static const char *pKey = StringRef( "Console" );
         InputMap mapping( pKey );
         mapping.Release( );
      }

      break;
   }
   case WM_KEYDOWN:
   {
      uint32 key = MapVirtualKey( wparam, MAPVK_VK_TO_CHAR );

      //shift
      if ( 0x10 == key )
      {
         g_ShiftDown = true;
      }
      //esc
      else if ( 0x1b == key )
      {
         static const char *pKey = StringRef( "DebugQuit" );
         InputMap mapping( pKey );
         mapping.Press( );

         return FALSE;
      }
      //1
      else if ( 0x31 == key )
      {
         static const char *pKey = StringRef( "DebugBreak" );
         InputMap mapping( pKey );
         mapping.Press( );

         return FALSE;
      }
      //2
      else if ( 0x32 == key )
      {
         static const char *pKey = StringRef( "DebugSkip" );
         InputMap mapping( pKey );
         mapping.Press( );

         return FALSE;
      }
      else if ( VK_UP == LOWORD( wparam ) )
      {
         return FALSE;
      }
      else if ( VK_DOWN == LOWORD( wparam ) )
      {
         return FALSE;
      }
      else if ( VK_LEFT == LOWORD( wparam ) )
      {
         return FALSE;
      }
      else if ( VK_RIGHT == LOWORD( wparam ) )
      {
         return FALSE;
      }
      //w
      else if ( 0x57 == key )
      {
         return FALSE;
      }
      //s
      else if ( 0x53 == key )
      {
         return FALSE;
      }
      //a
      else if ( 0x41 == key )
      {
         return FALSE;
      }
      //d
      else if ( 0x44 == key )
      {
         return FALSE;
      }
      else if ( 'Q' == key )
      {
         return FALSE;
      }
      else if ( 'E' == key )
      {
         return FALSE;
      }
      else if ( 0x44 == key )
      {
         return FALSE;
      }
      else if ( 'X' == key )
      {
         return FALSE;
      }
      else if ( 'Y' == key )
      {
         return FALSE;
      }
      else if ( 'Z' == key )
      {
         return FALSE;
      }
      else if ( 'P' == key )
      {
         return FALSE;
      }
      else if ( 'I' == key )
      {
         return FALSE;
      }
      else if ( '`' == key )
      {
         static const char *pKey = StringRef( "Console" );
         InputMap mapping( pKey );
         mapping.Press( );
      }

      break;
   }
   case WM_LBUTTONDOWN:
   {
      //track mouse so we know when it leaves the window
      TRACKMOUSEEVENT trackMouse = { 0 };
      trackMouse.cbSize = sizeof( trackMouse );
      trackMouse.dwFlags = TME_LEAVE;
      trackMouse.hwndTrack = hwnd;

      TrackMouseEvent( &trackMouse );

      POINT point;

      GetCursorPos( &point );
      ScreenToClient( hwnd, &point );

      App::Instance( ).BeginTouch( 0, point.x, point.y );

      return FALSE;
   }
   case WM_MOUSELEAVE:
   {
      POINT point;

      GetCursorPos( &point );
      ScreenToClient( hwnd, &point );

      App::Instance( ).EndTouch( 0, point.x, point.y );

      return TRUE;
   }
   case WM_LBUTTONUP:
   {
      POINT point;

      GetCursorPos( &point );
      ScreenToClient( hwnd, &point );

      App::Instance( ).EndTouch( 0, point.x, point.y );

      return FALSE;
   }
   case WM_RBUTTONDOWN:
   {
      return FALSE;
   }
   case WM_RBUTTONUP:
   {
      return FALSE;
   }
   case WM_SETCURSOR:
   {
      return FALSE;
   }
   case WM_CLOSE:
   {
      // won't be equal if we're recreating the window for fullscreen or windowed mode
      if ( g_hWnd == hwnd )
         g_Quit = TRUE;
      
      return FALSE;
   }
   case WM_DESTROY:
   {
      g_Window.Destroy( );

      ReleaseDC( hwnd, g_hDC );

      // won't be equal if we're recreating the window for fullscreen or windowed mode
      if ( g_hWnd == hwnd )
         PostQuitMessage( 0 );
   
      return FALSE;
   }
   case WM_PAINT:
   {
      return FALSE;
   }
   case WM_ERASEBKGND:
   {
      return FALSE;
   }
   }

   return DefWindowProc( hwnd, uMsg, wparam, lparam );
}

LONG WINAPI UnhandledException(
   struct _EXCEPTION_POINTERS *pInfo
)
{
   char finalError[ 256 ];

   if ( pInfo )
   {
      String::Format( finalError, sizeof( finalError ), "Unhandled Exception: Code 0x%x Address: 0x%x", pInfo->ExceptionRecord->ExceptionCode, pInfo->ExceptionRecord->ExceptionAddress );
   }
   else
   {
      String::Copy( finalError, "Unhandled Exception: Exception Info Unavailable", sizeof( finalError ) );
   }

   LOG( finalError );


#ifdef QA_BUILD
   Log::Instance( ).Show( );

   MessageBox( NULL, "We are very sorry, " ApplicationName " encountered a serious error and had to close.\n\n"
      "I opened a crash log, please save or copy this and "
      "email it to " QAEmail ".", ApplicationName, MB_ICONINFORMATION | MB_OK );
#else
   MessageBox( GetForegroundWindow( ), "We are very sorry, " ApplicationName " encountered a serious error and had to close.  "
      "Please contact " CompanyName " if you need assistance.", "" ApplicationName "", MB_ICONINFORMATION | MB_OK );
#endif

   Log::Instance( ).Destroy( );

   exit( 1 );

   return EXCEPTION_EXECUTE_HANDLER;
}

int WINAPI WinMain( HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd )
{
    unsigned int current_word = 0;  
   _controlfp_s( &current_word, _DN_FLUSH, _MCW_DN );


#ifndef _DEBUG
   char command[ MAX_PATH ] = { 0 };
   strncpy_s( command, sizeof( command ) - 1, GetCommandLine( ) + 1, MAX_PATH );

   char *pPath = strrchr( command, '\\' );
   *( pPath ) = NULL;

   SetCurrentDirectory( command );
#endif

   _CrtSetDbgFlag( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );
   //_CrtSetBreakAlloc( 25892 );

   DEVMODE devMode;
   EnumDisplaySettings( NULL, ENUM_CURRENT_SETTINGS, &devMode );

   g_BitsPerPixel = devMode.dmBitsPerPel;

   WNDCLASSEX wc =
   {
       sizeof( WNDCLASSEX ), 0, WinProc, 0, 0,

       GetModuleHandle( NULL ), LoadIcon( GetModuleHandle( NULL ), MAKEINTRESOURCE( IDI_ICON1 ) ), LoadCursor( NULL, IDC_ARROW ), NULL, NULL,

       "AnttrapCls", NULL
   };

   RegisterClassEx( &wc );

   bool windowed = true;

   int x, y, appWidth, appHeight;
   CreateAppWindow( windowed, &x, &y, &appWidth, &appHeight );

   //set up config paths regardless of user
   //this is for logging and videos
   char configPath[ MAX_PATH ] = { 0 };
   SHGetFolderPath( g_hWnd, CSIDL_COMMON_APPDATA, NULL, SHGFP_TYPE_CURRENT, configPath );

   strncat( configPath, "\\" CompanyName, sizeof( configPath ) - strlen( configPath ) - 1 );
   CreateDirectory( configPath, NULL );

   strncat( configPath, "\\" ApplicationName, sizeof( configPath ) - strlen( configPath ) - 1 );
   CreateDirectory( configPath, NULL );

   Log::Instance( ).Create( configPath );

#ifdef DIRECTX9
   String::Format( VideoPlayer::s_VideoPath, sizeof( VideoPlayer::s_VideoPath ) - 1, "%s\\Video", configPath );
#endif

   //set up registry and other app paths on a per user basis
   SHGetFolderPath( g_hWnd, CSIDL_APPDATA, NULL, SHGFP_TYPE_CURRENT, configPath );

   strncat( configPath, "\\" CompanyName, sizeof( configPath ) - strlen( configPath ) - 1 );
   CreateDirectory( configPath, NULL );

   strncat( configPath, "\\" ApplicationName, sizeof( configPath ) - strlen( configPath ) - 1 );
   CreateDirectory( configPath, NULL );

   char dir[ 256 ];
   GetCurrentDirectory( sizeof( dir ) - 1, dir );
   dir[ sizeof( dir ) - 1 ] = NULL;

   App::Instance( ).SetPaths( dir, configPath );


   SetUnhandledExceptionFilter( UnhandledException );


   WSADATA wsaData;
   WSAStartup( MAKEWORD( 2, 2 ), &wsaData );

   CoInitialize( NULL );

   Thread::InitMainThread( );

   LOG( "This is a "ApplicationName" log, if you see it please send these contents to "QAEmail );
   LOG( "" );
   LOG( "" );

   LOG( "Creating DX" );

   bool result = GpuDevice::Instance( ).Create( g_hWnd, windowed, appWidth, appHeight );

   if ( false == result )
   {
      MessageBox( GetForegroundWindow( ), GRAPHICS_API_SZ" is required.", "" ApplicationName "", MB_ICONERROR | MB_OK );
      return 0;
   }

   LOG( "Creating App" );

   App::Instance( ).Create( g_Window );

   g_ValidVideoPlayers.Create( 4, 4 );

   RegistryInt xPos( StringRef( "Win32/Placement/X" ) );
   RegistryInt yPos( StringRef( "Win32/Placement/Y" ) );

   xPos.SetValue( x );
   yPos.SetValue( y );

   if ( true == windowed )
   {
      ImportRegistry( "Win32.registry" );

      x = xPos.GetValue( );
      y = yPos.GetValue( );

      RECT rect;
      GetClientRect( GetDesktopWindow(), &rect );

      if ( x > rect.right || x < 0 )  x = 0;
      if ( y > rect.bottom  || y < 0 ) y = 0;

      SetWindowPos( g_hWnd, NULL, x, y, 0, 0, SWP_NOZORDER | SWP_NOSIZE );
   }
   else
   {
      //keep cursor within our fullscreen
      RECT rect;
      GetWindowRect( g_hWnd, &rect );
      ClipCursor( &rect );
   }

   //LOG( "Showing Window" );

   ShowWindow( g_hWnd, SW_SHOWNORMAL );

   {
      MSG message;

      while ( FALSE == g_Quit )
      {
         if ( PeekMessage( &message, NULL, NULL, NULL, PM_REMOVE ) )
         {
            if ( WM_QUIT == message.message )
               g_Quit = TRUE;
            else
            {
               TranslateMessage( &message );
               DispatchMessage( &message );
            }
         }

         bool hasDevice = GpuDevice::Instance( ).HasDevice( );

         if ( true == hasDevice )
            Update( );
         //don't update if we can't render.. 
         //this mimics iOS going to sleep or suspending the app
         else
         {
            ResourceHandle handle( "GlobalEntity" );
            handle.GetChannel( )->SendEvent( "GoingToSleep", ArgList( ) );
         }
      };

   };

   RECT rect;
   GetWindowRect( g_hWnd, &rect );

   if ( true == windowed )
   {
      xPos.SetValue( rect.left );
      yPos.SetValue( rect.top );

      ExportRegistry( "Win32", "Win32.registry" );
   }

   //ExportRegistry( "", "LocalPaths.registry" );

   g_ValidVideoPlayers.Destroy( );
   App::Instance( ).Destroy( );

   GpuDevice::Instance( ).Destroy( );

   CoUninitialize( );

   WSACleanup( );

   DestroyWindow( g_hWnd );

   UnregisterClass( "AnttrapCls", GetModuleHandle( NULL ) );

   Log::Instance( ).Destroy( );

   return 0;
}

void CreateAppWindow(
   bool windowed,
   int *pX,
   int *pY,
   int *pWidth,
   int *pHeight
)
{
   DWORD style = 0;
   DWORD styleEx = 0;

   int screenWidth = GetSystemMetrics( SM_CXSCREEN );
   int screenHeight = GetSystemMetrics( SM_CYSCREEN );

   int appWidth;
   int appHeight;

   if ( windowed )
   {
      style   = WS_POPUP;
      styleEx = WS_EX_APPWINDOW;

      appWidth = screenWidth;
      appHeight = screenHeight;

      appWidth = 1920;
      appHeight = 1080;

      //appWidth = 4096;
      //appHeight = 2160;

      //appWidth = 1280;
      //appHeight = 720;
   }
   else
   {
      style   = WS_POPUP;
      styleEx = WS_EX_APPWINDOW;

      appWidth = screenWidth;
      appHeight = screenHeight;
   }

   int windowWidth, windowHeight;

   if ( true == windowed )
   {
      style = WS_OVERLAPPEDWINDOW & ~WS_MAXIMIZEBOX;
      style = style & ~WS_THICKFRAME;

      RECT rect = { 0, 0, appWidth, appHeight };
      AdjustWindowRectEx( &rect, style, FALSE, styleEx );

      windowWidth = rect.right - rect.left;
      windowHeight = rect.bottom - rect.top;
   }
   else
   {
      windowWidth = appWidth;
      windowHeight = appHeight;
   }

   int x = ( screenWidth - windowWidth ) / 2;
   int y = 0;

   g_hWnd = CreateWindowEx( styleEx, "AnttrapCls", "", style, x, y, windowWidth, windowHeight, NULL, NULL, GetModuleHandle( NULL ), NULL );
   
   *pX = x;
   *pY = y;
   *pWidth = appWidth;
   *pHeight = appHeight;
}

void Update( void )
{
   static RegistryBool app_fullscreen( "App/fullscreen", false );
   static bool prev_fullscreen = app_fullscreen.GetValue( );

   if ( prev_fullscreen != app_fullscreen.GetValue() )
   {
      App::Instance( ).TeardownRenderers( );

         GpuDevice::Instance( ).DestroySwapChain( );

         HWND hWnd = g_hWnd;
         g_hWnd = NULL;
         DestroyWindow( hWnd );

         int x, y, width, height;
         CreateAppWindow( app_fullscreen.GetValue( ) == false, &x, &y, &width, &height );

         GpuDevice::Instance( ).CreateSwapChain( g_hWnd, width, height, g_Window.GetHandle() );

         prev_fullscreen = app_fullscreen.GetValue( );

         ShowWindow( g_hWnd, SW_SHOWNORMAL );

      App::Instance( ).SetupRenderers( );
   }

   static POINT prevCursorPos;
   
   bool isActive = GetForegroundWindow( ) == g_hWnd;

   POINT point;
   GetCursorPos( &point );
   ScreenToClient( g_hWnd, &point );

   bool left_button = 0 != ( GetAsyncKeyState( VK_LBUTTON ) & 0x8000 );

   static const char *pYaw = StringRef( "Yaw" );
   static const char *pPitch = StringRef( "Pitch" );

   if ( true == isActive )
   {
      if ( true == left_button )
      {
         App::Instance( ).Touch( 0, point.x, point.y );

         InputMap yaw( pYaw );
         yaw.SetValue( point.x - (float) prevCursorPos.x );

         InputMap pitch( pPitch );
         pitch.SetValue( point.y - (float) prevCursorPos.y );
      }
      else
      {
         App::Instance( ).Hover( point.x, point.y );

         InputMap yaw( pYaw );
         yaw.SetValue( 0 );

         InputMap pitch( pPitch );
         pitch.SetValue( 0 );
      }
   }

   prevCursorPos = point;

   static SHORT prevKeys[ 256 ];
   static bool needsRelease = false;
   SHORT keys[ 256 ];

   char modifiers[ 64 ];
   char *pModifiers = modifiers;

   int lshift = GetAsyncKeyState( VK_LSHIFT );
   int rshift = GetAsyncKeyState( VK_RSHIFT );
   int lctrl = GetAsyncKeyState( VK_LCONTROL );
   int rctrl = GetAsyncKeyState( VK_RCONTROL );

   if ( lshift & 0x8000 ) *pModifiers = '1';
   else *pModifiers = '0';
   *pModifiers++;

   if ( rshift & 0x8000 ) *pModifiers = '1';
   else *pModifiers = '0';
   *pModifiers++;

   if ( lctrl & 0x8000 ) *pModifiers = '1';
   else *pModifiers = '0';
   *pModifiers++;

   if ( rctrl & 0x8000 ) *pModifiers = '1';
   else *pModifiers = '0';
   *pModifiers++;

   *pModifiers = NULL;

   struct UserMaps
   {
      const char *pMap;
      int prev_state;
      byte key;
   };

   static UserMaps user_maps[ ] =
   {
         { StringRef( "Forward" ), 0, 'W' },
         { StringRef( "Backward" ), 0, 'S' },
         { StringRef( "Right" ), 0, 'D' },
         { StringRef( "Left" ), 0, 'A' },
         { StringRef( "Up" ), 0, 'E' },
         { StringRef( "Down" ), 0, 'Q' },
         { StringRef( "Animation" ), 0, '1' },
         { StringRef( "Slow" ), 0, 16 }, //shift
   };

   if ( isActive )
   {
      //First go through all of the user mappings and
      //dish out their release
      for ( int i = 0; i < sizeof( user_maps ) / sizeof( user_maps[ 0 ] ); i++ )
      {
         int k = GetAsyncKeyState( user_maps[ i ].key );

         if ( ( k & 0x8000 ) != ( user_maps[ i ].prev_state & 0x8000 ) )
         {
            InputMap map( user_maps[ i ].pMap );

            if ( k & 0x8000 )
               map.Press( );
            else
               map.Release( );

            user_maps[ i ].prev_state = k;
         }
      }

      // Now go through all the keys and send the appropriate signals
      for ( int i = 1; i < 256; i++ )
      {
         //have to use this, Winproc and GetKeyboardState 
         //are queued and not instantaneous
         keys[ i ] = GetAsyncKeyState( i );

         if ( ( keys[ i ] & 0x8000 ) != ( prevKeys[ i ] & 0x8000 ) )
         {
            uint32 key = i << 8 | MapVirtualKey( i, MAPVK_VK_TO_CHAR );

            if ( keys[ i ] & 0x8000 )
               InputSystem::Instance( ).KeyDown( key, modifiers );
            else
               InputSystem::Instance( ).KeyUp( key, modifiers );
         }

         prevKeys[ i ] = keys[ i ];
      }

      needsRelease = true;
   }
   else if ( true == needsRelease )
   {
      //First go through all of the user mappings and
      //dish out their release
      for ( int i = 0; i < sizeof( user_maps ) / sizeof( user_maps[ 0 ] ); i++ )
      {
         if ( user_maps[ i ].prev_state & 0x8000 )
         {
            InputMap map( user_maps[ i ].pMap );
            map.Release( );

            user_maps[ i ].prev_state = 0;
         }
      }

      for ( int i = 1; i < 256; i++ )
      {
         if ( prevKeys[ i ] & 0x8000 )
         {
            uint32 key = i << 8 | MapVirtualKey( i, MAPVK_VK_TO_CHAR );
            InputSystem::Instance( ).KeyUp( key, modifiers );
         }

         prevKeys[ i ] = 0;
      }

      needsRelease = false;
   }

   App::Instance( ).Update( );
}

