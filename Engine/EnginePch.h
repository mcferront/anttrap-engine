#include "BuildOptions.h"

#ifdef WIN32
   #define _WINSOCK_DEPRECATED_NO_WARNINGS
   #include <winsock2.h>   
   #include <Ws2tcpip.h>
   #include <windows.h>
   #include <fcntl.h>
   #include <io.h>
   #include <ios>
   #include <psapi.h>
   #include <direct.h>
   

   #ifdef OPENGL
      #define GL_GLEXT_PROTOTYPES
      #include <gl\gl.h>
      #include <gl\glext.h>
      #include <gl\glu.h>
      #include <typeinfo>
   #endif

   #ifdef DIRECTX9
      #include <d3dx9shader.h>
   #endif

   #ifdef DIRECTX12
      #include <D3D12.h>   
      #include <DXGI1_4.h>
      #include <D3Dcompiler.h>
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

#if defined IOS || defined LINUX || defined MAC
   #include <sys/sem.h>
   #include <sys/types.h>
   #include <sys/stat.h>
   #include <sys/socket.h>
   #include <sys/types.h> 
   #include <sys/ioctl.h>
   #include <sys/time.h>
   #include <arpa/inet.h>  
   #include <netdb.h>
   #include <unistd.h>
   #include <pthread.h>
   #include <errno.h>
#endif

#if defined ANDROID
   #include <jni.h>
   #include <android/log.h>
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

#ifdef IOS
   #include <mach/mach.h> 
   #include <mach/mach_host.h>
   #include <OpenGLES/ES2/gl.h>
   #include <OpenGLES/ES2/glext.h>
   #include "OpenAL/al.h"
   #include "OpenAL/alc.h"
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
   //#include <OpenAL/al.h>
   //#include <OpenAL/alc.h>
#endif

#include <limits.h>
#include <float.h>
#include <stdarg.h>
#include <stdio.h>
#include <math.h>
#include <stdarg.h>
#include <time.h>
#include <memory.h>
#include <stdlib.h>
#include <string.h>
