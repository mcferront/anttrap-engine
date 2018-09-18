cls
call openal_cmake_release .\build_release_armeabi-v7a armeabi-v7a
call openal_make .\build_release_armeabi-v7a
call openal_cmake_release .\build_release_armeabi armeabi
call openal_make .\build_release_armeabi
pause