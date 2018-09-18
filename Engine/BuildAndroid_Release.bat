cls
call copy .\jni\Release.mk .\jni\Android.mk
call copy .\jni\Release.application .\jni\Application.mk
call ndk-build NDK_APP_OUT=./jni/builds/release/obj/ NDK_DEBUG=0
call del .\jni\Android.mk
call del .\jni\Application.mk
pause