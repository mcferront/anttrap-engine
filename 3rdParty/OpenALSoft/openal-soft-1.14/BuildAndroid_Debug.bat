cls
call openal_cmake_debug .\build_debug_armeabi-v7a armeabi-v7a
call openal_make .\build_debug_armeabi-v7a
call openal_cmake_debug .\build_debug_armeabi armeabi
call openal_make .\build_debug_armeabi
pause