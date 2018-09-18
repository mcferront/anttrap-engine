#include "EnginePch.h"

#include "AndroidVideoPlayer.h"
#include "AndroidEnv.h"

VideoPlayer::VideoPlayer()
{
   m_Object = NULL;
}

VideoPlayer::~VideoPlayer()
{
   Unload();
}

void VideoPlayer::Load(
   const char *pPath,
   uint32 offset,
   uint32 size,
   Channel *pChannel
)
{
   JNIEnv *pEnv = AndroidEnv::GetJNIEnv();
   jstring jPath = pEnv->NewStringUTF((char*)pPath);
   jstring jChannel = pEnv->NewStringUTF((char*)pChannel->GetId().ToString());

   jclass vpClass = pEnv->FindClass("com/thelanguageexpress/thesocialexpress/mydps/VideoPlayer");

   jmethodID vpConstructor = pEnv->GetMethodID(vpClass, "<init>", "()V");
   jobject vpObject = pEnv->NewObject(vpClass, vpConstructor);

   jmethodID method = pEnv->GetMethodID(vpClass, "init", "(Ljava/lang/String;Ljava/lang/String;)V");
   pEnv->CallVoidMethod(vpObject, method, jPath, jChannel);

   m_Object = pEnv->NewGlobalRef(vpObject);
}

void VideoPlayer::Unload( void )
{
   if (m_Object == NULL)
      return;

   JNIEnv *pEnv = AndroidEnv::GetJNIEnv();

   jmethodID method = pEnv->GetMethodID(pEnv->GetObjectClass(m_Object), "uninit", "()V");
   pEnv->CallVoidMethod(m_Object, method);

   pEnv->DeleteGlobalRef(m_Object);
   m_Object = NULL;
}

void VideoPlayer::Play( void )
{
   JNIEnv *pEnv = AndroidEnv::GetJNIEnv();

   jmethodID method = pEnv->GetMethodID(pEnv->GetObjectClass(m_Object), "play", "()V");
   pEnv->CallVoidMethod(m_Object, method);
}

void VideoPlayer::Stop( void )
{
    JNIEnv *pEnv = AndroidEnv::GetJNIEnv();
    
    jmethodID method = pEnv->GetMethodID(pEnv->GetObjectClass(m_Object), "stop", "()V");
    pEnv->CallVoidMethod(m_Object, method);
}

void VideoPlayer::SeekTo( 
   VideoPlayer::Seek from, 
   float seconds
)
{
   if (from == VideoPlayer::SeekCurrent)
      seconds += GetTime();

   JNIEnv *pEnv = AndroidEnv::GetJNIEnv();

   jmethodID method = pEnv->GetMethodID(pEnv->GetObjectClass(m_Object), "seekTo", "(I)V");
   pEnv->CallVoidMethod(m_Object, method, (jint)(seconds * 1000));
}

void VideoPlayer::Show( void )
{
   JNIEnv *pEnv = AndroidEnv::GetJNIEnv();

   jmethodID method = pEnv->GetMethodID(pEnv->GetObjectClass(m_Object), "show", "()V");
   pEnv->CallVoidMethod(m_Object, method);
}

void VideoPlayer::Hide( void )
{
    JNIEnv *pEnv = AndroidEnv::GetJNIEnv();
    
    jmethodID method = pEnv->GetMethodID(pEnv->GetObjectClass(m_Object), "hide", "()V");
    pEnv->CallVoidMethod(m_Object, method);
}

float VideoPlayer::GetTime( void ) 
{
   JNIEnv *pEnv = AndroidEnv::GetJNIEnv();

   jmethodID method = pEnv->GetMethodID(pEnv->GetObjectClass(m_Object), "getDuration", "()I");
   jint duration = pEnv->CallIntMethod(m_Object, method);

   return (float)duration / 1000.0f;
}
