cls
call copy .\jni\Debug.mk .\jni\Android.mk
call copy .\jni\Debug.application .\jni\Application.mk
call ndk-build NDK_APP_OUT=./jni/builds/debug/obj/ NDK_DEBUG=1 -B
call del .\jni\Android.mk
call del .\jni\Application.mk
pause