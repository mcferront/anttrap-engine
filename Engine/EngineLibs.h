#ifdef WIN32
   #pragma comment ( lib, "ws2_32" )
   #pragma comment ( lib, "gdi32" )
   #pragma comment ( lib, "opengl32" )
   #pragma comment ( lib, "openal32" )
   #pragma comment ( lib, "libvorbis_static" )
   #pragma comment ( lib, "libvorbisfile_static" )
   #pragma comment ( lib, "libogg_static" )
   #pragma comment ( lib, "strmiids" )
   #pragma comment ( lib, "d3d9" )
   #pragma comment ( lib, "psapi" )

   #ifdef _DEBUG
      #pragma comment ( lib, "lua_d" )
      #pragma comment ( lib, "Zlibd" )
   #else
      #pragma comment ( lib, "lua" )
      #pragma comment ( lib, "Zlib" )
   #endif

    #ifdef DIRECTX9
        #ifdef _DEBUG
            #pragma comment ( lib, "d3dx9d" )
        #else
            #pragma comment ( lib, "d3dx9" )
        #endif
    #elif DIRECTX12
        #pragma comment ( lib, "d3dcompiler" )
        #pragma comment ( lib, "d3d12" )
        #pragma comment ( lib, "dxgi" )
    #endif
#endif
