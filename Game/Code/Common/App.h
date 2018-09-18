#pragma once

#include "Global.h"
#include "UtilityClock.h"
#include "Database.h"
#include "Viewport.h"
#include "Renderer.h"
#include "Button.h"
#include "ShaderAsset.h"
#include "RenderTree.h"
#include "EditorConnection.h"
#include "InputSystem.h"

class GpuTimer;
class Window;
class RenderTree;
class FrameGrabRenderer;
class DefaultRenderer;
class LabelComponent;
class CameraComponent;
class FrustumCullRenderModifier;
class ExecuteIndirect;

bool ImportRegistry(
   const char *pFilename
);

void ExportRegistry(
   const char *pKey,
   const char *pFilename
);

class App
{
private:
   Clock      m_Clock;
   Viewport   m_UIViewport;
   Viewport   m_SceneViewport;
   Viewport   m_ShadowMapViewport;
   Viewport   m_BokehViewport;
   Viewport   m_UpResViewport;
   Viewport	  m_DepthPrepassViewport;
   Viewport   m_DeferredForwardViewport;
   Id         m_WindowId;

   //EditorConnection m_EditorConnection;
   bool       m_LuaApiExists;

   RenderTree   m_UIRenderTree;
   RenderTree   m_SceneRenderTree;
   RenderTree   m_BokehRenderTree;
   RenderTree   m_ShadowMapRenderTree;
   RenderTree   m_UpResRenderTree;
   RenderTree   m_DepthPrepassRenderTree;
   RenderTree   m_DeferredForwardRenderTree;

   FrameGrabRenderer *m_pFrameGrabber;
   LabelComponent  *m_pStats;
   ExecuteIndirect *m_pDofIndirect;

   FrustumCullRenderModifier *m_pCullOpaque;
   FrustumCullRenderModifier *m_pCullTransparent;

   ResourceHandle m_ColorBuffer;
   ResourceHandle m_DSBuffer;
   ResourceHandle m_hMainCamera;

   List<GpuTimer*> m_GpuTimers;

   char m_ConfigPath[ 256 ];
   char m_DataPath[ 256 ];

   bool m_FrameReady;

public:
   static App &Instance( void );

public:
   App( void );

   ~App( void );

   void Create(
      const Window &window
   );

   void Destroy( void );

   void Update( void );

   void Render( void );

   void AbortFrame( void ) { m_FrameReady = false; }

   void BeginTouch(
      int touchId,
      int x,
      int y
   );

   void Touch(
      int touchId,
      int x,
      int y
   );

   void EndTouch(
      int touchId,
      int x,
      int y
   );

   void Hover(
      int x,
      int y
   );

   void SetPaths(
      const char *pData,
      const char *pConfig
   )
   {
      String::Copy( m_DataPath, pData, sizeof( m_DataPath ) );
      String::Copy( m_ConfigPath, pConfig, sizeof( m_ConfigPath ) );

      String::Replace( m_DataPath, '\\', '/' );
      String::Replace( m_ConfigPath, '\\', '/' );

      Debug::Print( Debug::TypeInfo, "Data Path: %s\n", m_DataPath );
      Debug::Print( Debug::TypeInfo, "Config Path: %s\n", m_ConfigPath );
   }

   const char *GetConfigPath( void ) const { return m_ConfigPath; }
   const char *GetDataPath( void ) const { return m_DataPath; }


   void SetupRenderers( void );
   void SetupDeferredRenderer( void );
   void SetupForwardRenderer( void );
   void TeardownRenderers( void );
   
   void DeferredCheckerboardUpdate( 
      CameraComponent *pMainCamera
   );
   
   void ForwardCheckerboardUpdate(
      CameraComponent *pMainCamera
   );
};
