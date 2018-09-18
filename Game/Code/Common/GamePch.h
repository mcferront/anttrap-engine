#include "BuildOptions.h"

#define CompanyName     ""
#define ApplicationName "Anttrap"
#define BuildNumber     "0.0.1"
#define QAEmail         "trapper@trapzz.com"

#define CompanyIdentifier  "com.trapzz"

#if defined IOS || defined MAC
    #include <sys/sem.h>
    #include <sys/types.h>
    #include <sys/stat.h>
    #include <sys/socket.h>
    #include <sys/types.h> 
    #include <sys/ioctl.h>
    #include <arpa/inet.h>  
    #include <netdb.h>
    #include <unistd.h>
    #include <pthread.h>
    #include <errno.h>
#endif

#ifdef WIN32
   #include <windows.h>
   #include <crtdbg.h>
   #include <typeinfo>

   #ifdef OPENGL
      #define GL_GLEXT_PROTOTYPES
      #include <gl\gl.h>
      #include <gl\glext.h>
      #include <gl\glu.h>
   #endif

   #ifdef DIRECTX9
      #include <d3dx9shader.h>
   #endif

   #ifdef DIRECTX12
      #include <D3D12.h>   
      #include <DXGI1_4.h>
   #endif

   //for video playback to texture
   #ifdef DIRECTX9
      #include <d3d9.h>
      #include <dshow.h>
      #include <vmr9.h>
   #endif
   
   #include "al.h"
   #include "alc.h"
#endif

#include <limits.h>
#include <stdlib.h>
#include <memory.h>
#include <math.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <time.h>

#ifdef IOS
    #include <OpenGLES/ES2/gl.h>
    #include <OpenGLES/ES2/glext.h>
    #include "OpenAL/al.h"
    #include "OpenAL/alc.h"
    #import <Availability.h>
    #ifdef __OBJC__
        #import <Foundation/Foundation.h>
        #import <UIKit/UIKit.h>
    #endif
#endif

#ifdef ANDROID
    #include <jni.h>
    #include <sys/types.h>
    #include <sys/stat.h>
    #include <sys/socket.h>
    #include <sys/types.h> 
    #include <sys/ioctl.h>
    #include <sys/time.h>
    #include <arpa/inet.h>  
    #include <pthread.h>
    #include <unistd.h>
    #include <netdb.h>
    #include <netinet/in.h>
    #include <net/if.h>
    #include <typeinfo>
    #include <GLES2/gl2.h>
    #include <GLES2/gl2ext.h>
    #include "al.h"
    #include "alc.h"
    #include <errno.h>
#endif


#ifdef MAC
    #include <mach/mach.h> 
    #include <mach/mach_host.h>
#ifdef OPENGL
    #define GL_GLEXT_PROTOTYPES
    #include <OpenGL/OpenGL.h>
    #include <OpenGL/GLExt.h>
#endif
    #include <OpenAL/MacOSX_OALExtensions.h>
#endif
