#pragma once

#include "EngineGlobal.h"
#include "ThreadLocks.h"
#include "ResourceWorld.h"
#include "HashTable.h"
#include "List.h"
#include "Threads.h"

class Wav;

#ifdef WIN32
   typedef int (__cdecl *PFNALGETENUMVALUE)( const char *szEnumName );
#elif defined IOS || defined Mac || defined ANDROID
   typedef int (*PFNALGETENUMVALUE)( const char *szEnumName );
#else
   #error "Platform not defined"
#endif

const int NUM_STREAMING_BUFFERS = 3;

// Ogg decoding file struct
struct _OggFile
{
   char *pData;
   char *pCurrent;
   unsigned int fileSize;
};

struct OggVorbis_File;
struct vorbis_info;

class AudioWorld
{
private:
   friend class AudioStreamingThread;

   class AudioStreamingThread : public Thread
   {
   public:
      void Create()
      {
      }

      virtual void OnThreadRun( void );
   };

public:
   static AudioWorld &Instance( void );

private:
   Lock     m_Lock;

   struct Buffer
   {
      ALuint            m_BufferIDs[NUM_STREAMING_BUFFERS];
      Id              m_SoundAsset;
      OggVorbis_File   *m_pOVF;
      _OggFile         *m_pOggFile;
      vorbis_info      *m_pVorbisInfo;
      int               m_NumBuffers;
      int               m_RefCount;
   	ALenum            m_OALBufferFormat;
   };

   struct Source
   {
      ALuint            m_SourceID;
      Id              m_SourceHandle;
      Id              m_AssetName;
      float             m_Volume;
      bool              m_Looping;
   };

   struct Group
   {
      char  m_Name[32];
      float m_Volume;
   };

   // Volume fade struct
   struct Fade
   {
      float    m_StartVolume;
      float    m_EndVolume;
      float    m_FadeTime;
      float    m_CurrentTime;
      char     m_GroupName[32];
      ALuint   m_SourceID;
   };

   ALCdevice    *m_pDevice;
   ALCcontext   *m_pContext;

   List<Buffer>   m_Buffers;
   List<Source>   m_Sources;
   List<Source>   m_PlayingSources;
   List<Group>    m_Groups;
   List<Fade>     m_Fades;

   AudioStreamingThread m_Thread;

   HashTable<int, const char *>  *m_pGroupMap;

private:
   ALuint GetBufferID(Id assetName, int index);
   int GetBufferIndex(Id assetName, bool assertOnFail = true);
   int GetBufferIndex(ALuint bufferID);
   int GetBufferSubIndex(ALuint bufferID);
   int GetBufferSubIndex(Buffer *pBuffer, ALuint bufferID);
   ALuint GetSourceID(ResourceHandle owner, Id assetName);
   int GetSourceIndex(ResourceHandle owner, Id assetName);
   int GetSourceIndex(ALuint sourceID);

   int   OALGetSourceInt(ALuint sourceID, ALenum property);
   bool  OALSetSourceInt(ALuint sourceID, ALenum property, int value);
   float OALGetSourceFloat(ALuint sourceID, ALenum property);
   bool  OALSetSourceFloat(ALuint sourceID, ALenum property, float value);
   Vector OALGetSourceVector(ALuint sourceID, ALenum property);
   bool   OALSetSourceVector(ALuint sourceID, ALenum property, Vector value);

   bool IsSourcePlaying(ALuint sourceID, bool requireAssert);
   bool IsSourceInInitialState(ALuint sourceID);
   bool SetSourceVolume(ALuint sourceID, float volume);

   bool CheckOpenALError(const char *pFile, int line, const char *pFunc, ALenum errorCode, bool requireAssert = true);
   bool FillOALBuffer(Buffer *buffer, ALuint bufferID);
   void OnThreadRun( void );

   bool RemoveSource(uint32 index, ALuint sourceID);

public:
   void Create ( void );
   void Destroy( void );

   void Tick(
      float deltaSeconds
   );

   void PostTick( void );
   void Final( void );

   void BeginInterruption( void );
   void EndInterruption  ( void );

   bool StopAllSounds();
   bool PauseAllSounds(bool pause);

   Vector GetListenerPosition();
   bool SetListenerPosition(Vector position);
   Vector GetListenerLook();
   Vector GetListenerUp();
   bool SetListenerOrientation(Vector look, Vector up);
   Vector GetListenerVelocity();
   bool SetListenerVelocity(Vector velocity);

   void AddSoundGroup(const char *groupName, float volume = 1.0f);
   void RemoveSoundGroup(const char *groupName);
   void SetSoundGroupVolume(const char *groupName, float volume);
   float GetSoundGroupVolume(const char *groupName);
   void FadeSoundGroupVolume(const char *groupName, float volume, float fadeTime);

   bool GetWaveALBufferFormat(uint32 numChannels, uint32 bitsPerSample, PFNALGETENUMVALUE pfnGetEnumValue, unsigned long *pulFormat);
   bool LoadBuffer(Wav *pWavAsset, char *pWavData);
   void UnloadBuffer(ResourceHandle wavAsset);

   bool AddSource(ResourceHandle owner, const char *assetName, const char *groupName = "Default", float volume = 1.0f, bool looping = true);
   void RemoveSource(ResourceHandle owner, const char *assetName);

   void SetSourceGroup(ResourceHandle owner, const char *assetName, const char *groupName);
   const char *GetSourceGroup(ResourceHandle owner, const char *assetName);

   bool PlaySource(ResourceHandle owner, const char *assetName);
   bool PauseSource(ResourceHandle owner, const char *assetName);
   bool StopSource(ResourceHandle owner, const char *assetName);

   bool SetSourcePosition(ResourceHandle owner, const char *assetName, Vector position);
   bool SetSourceOrientation(ResourceHandle owner, const char *assetName, Vector look, Vector up);
   bool SetSourceVelocity(ResourceHandle owner, const char *assetName, Vector velocity);

   bool IsSourceInInitialState(ResourceHandle owner, const char *assetName);
   bool IsSourcePlaying(ResourceHandle owner, const char *assetName);
   bool GetSourceLooping(ResourceHandle owner, const char *assetName);
   bool SetSourceLooping(ResourceHandle owner, const char *assetName, bool looping);
   float GetSourceVolume(ResourceHandle owner, const char *assetName);
   bool SetSourceVolume(ResourceHandle owner, const char *assetName, float volume);
   void FadeSourceVolume(ResourceHandle owner, const char *assetName, float volume, float fadeTime);
   float GetSourcePlaybackTime(ResourceHandle owner, const char *assetName);
   bool SetSourcePlaybackTime(ResourceHandle owner, const char *assetName, float time);
   float GetSourceInnerRadius(ResourceHandle owner, const char *assetName);
   bool SetSourceInnerRadius(ResourceHandle owner, const char *assetName, float radius);
   float GetSourceOuterRadius(ResourceHandle owner, const char *assetName);
   bool SetSourceOuterRadius(ResourceHandle owner, const char *assetName, float radius);
};