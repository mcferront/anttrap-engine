cd %1
call cmake -G"MinGW Makefiles" -DLIBTYPE="STATIC" -DLIBRARY_OUTPUT_PATH_ROOT="debug" -DANDROID_ABI=%2 -DCMAKE_BUILD_TYPE="Debug" -DCMAKE_TOOLCHAIN_FILE=toolchain.android.cmake -DCMAKE_MAKE_PROGRAM="%ANDROID_NDK%\prebuilt\windows\bin\make.exe" ..
cd ..
