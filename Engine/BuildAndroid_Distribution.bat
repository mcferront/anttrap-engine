cls
call copy .\jni\Distribution.mk .\jni\Android.mk
call copy .\jni\Distribution.application .\jni\Application.mk
call ndk-build NDK_APP_OUT=./jni/builds/distribution/obj/ NDK_DEBUG=0
call del .\jni\Android.mk
call del .\jni\Application.mk
pause