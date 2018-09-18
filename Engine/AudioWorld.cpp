#include "EnginePch.h"
#include "Threads.h"

#include "AudioWorld.h"
#include "WavAsset.h"

#include "vorbis/vorbisfile.h"
#include "vorbis/codec.h"

#define CHECKERROR(x,y) CheckOpenALError(__FILE__, __LINE__, __FUNCTION__, x, y)

const int STREAM_BUFFER_SIZE = 4096 * 8;

// Ogg decoding read function
size_t _oggRead(void *buffer, size_t size, size_t count, void *stream)
{
   _OggFile *pBuffer = (_OggFile *)stream;

   // decide how much to copy
   unsigned int amountLeft = pBuffer->fileSize - (pBuffer->pCurrent - pBuffer->pData);
   size_t amountToCopy = amountLeft < (size * count) ? amountLeft : size * count;

   // copy and move the current pointer
   memcpy(buffer, pBuffer->pCurrent, amountToCopy);
   pBuffer->pCurrent += amountToCopy;

   return amountToCopy;
}

// Ogg decoding tell function
long _oggTell(void *stream)
{
   _OggFile *pBuffer = (_OggFile *)stream;
   return (pBuffer->pCurrent - pBuffer->pData);
}

// Ogg decoding seek function
int _oggSeek(void *stream, ogg_int64_t offset, int whence)
{
   _OggFile *pBuffer = (_OggFile *)stream;
   if (whence == SEEK_SET)
   {
      pBuffer->pCurrent = pBuffer->pData + offset;
      return 0;
   }
   else if (whence == SEEK_CUR)
   {
      pBuffer->pCurrent = pBuffer->pCurrent + offset;
      return 0;
   }
   else
   {
      pBuffer->pCurrent = pBuffer->pData + pBuffer->fileSize + offset;
      return 0;
   }

   return -1;
}

void AudioWorld::AudioStreamingThread::OnThreadRun( void )
{
   //while ( ShouldRun( ) )
   //{
   //   AudioWorld::Instance( ).OnThreadRun();

   //   Thread::Sleep( 1 );
   //}
}

bool AudioWorld::CheckOpenALError(const char *pFile, int line, const char *pFunc, ALenum errorCode, bool requireAssert)
{
   ALuint alcErrorCode = alcGetError(m_pDevice);
   switch(alcErrorCode)
   {
      case ALC_INVALID_DEVICE:
         Debug::Print( Debug::TypeWarning, "OpenALError(%s:%d) %s - ALC_INVALID_DEVICE - A bad device was passed to an AL call.\n", pFile, line, pFunc );
         Debug::Assert( Condition( !requireAssert ), "OpenALError(%s:%d) %s - ALC_INVALID_DEVICE - A bad device was passed to an AL call.", pFile, line, pFunc );
         return false;
         break;

      case ALC_INVALID_CONTEXT:
         Debug::Print( Debug::TypeWarning, "OpenALError(%s:%d) %s - ALC_INVALID_CONTEXT - A bad context was passed to an AL call.\n", pFile, line, pFunc );
         Debug::Assert( Condition( !requireAssert ), "OpenALError(%s:%d) %s - ALC_INVALID_CONTEXT - A bad context was passed to an AL call.", pFile, line, pFunc );
         return false;
         break;

      case ALC_INVALID_ENUM:
         Debug::Print( Debug::TypeWarning, "OpenALError(%s:%d) %s - ALC_INVALID_ENUM - Unknown enum value was passed to an AL call.\n", pFile, line, pFunc );
         Debug::Assert( Condition( !requireAssert ), "OpenALError(%s:%d) %s - ALC_INVALID_ENUM - Unknown enum value was passed to an AL call.", pFile, line, pFunc );
         return false;
         break;

      case ALC_INVALID_VALUE:
         Debug::Print( Debug::TypeWarning, "OpenALError(%s:%d) %s - ALC_INVALID_VALUE - Invalid value passed to an AL call.\n", pFile, line, pFunc );
         Debug::Assert( Condition( !requireAssert ), "OpenALError(%s:%d) %s - ALC_INVALID_VALUE - Invalid value passed to an AL call.", pFile, line, pFunc );
         return false;
         break;

      case ALC_OUT_OF_MEMORY:
         Debug::Print( Debug::TypeWarning, "OpenALError(%s:%d) %s - ALC_OUT_OF_MEMORY - The requested operation resulted in OpenAL running out of memory.\n", pFile, line, pFunc );
         Debug::Assert( Condition( !requireAssert ), "OpenALError(%s:%d) %s - ALC_OUT_OF_MEMORY - The requested operation resulted in OpenAL running out of memory.", pFile, line, pFunc );
         return false;
         break;

      case ALC_NO_ERROR:
         break;

      default:
         Debug::Print( Debug::TypeWarning, "OpenALError(%s:%d) %s - Unknown OpenAL context error.\n", pFile, line, pFunc );
         Debug::Assert( Condition( !requireAssert ), "OpenALError(%s:%d) %s - Unknown OpenAL context error.", pFile, line, pFunc );
         return false;
         break;
   }

   switch(errorCode)
   {
      case AL_INVALID_NAME:
         Debug::Print( Debug::TypeWarning, "OpenALError(%s:%d) %s - AL_INVALID_NAME - Invalid Name paramater passed to AL call.\n", pFile, line, pFunc );
         Debug::Assert( Condition( !requireAssert ), "OpenALError(%s:%d) %s - AL_INVALID_NAME - Invalid Name paramater passed to AL call.", pFile, line, pFunc );
         return false;
         break;

      case AL_INVALID_ENUM:
         Debug::Print( Debug::TypeWarning, "OpenALError(%s:%d) %s - AL_INVALID_ENUM - Invalid parameter passed to AL call.\n", pFile, line, pFunc );
         Debug::Assert( Condition( !requireAssert ), "OpenALError(%s:%d) %s - AL_INVALID_ENUM - Invalid parameter passed to AL call.", pFile, line, pFunc );
         return false;
         break;

      case AL_INVALID_VALUE:
         Debug::Print( Debug::TypeWarning, "OpenALError(%s:%d) %s - AL_INVALID_VALUE - Invalid enum parameter value.\n", pFile, line, pFunc );
         Debug::Assert( Condition( !requireAssert ), "OpenALError(%s:%d) %s - AL_INVALID_VALUE - Invalid enum parameter value.", pFile, line, pFunc );
         return false;
         break;

      case AL_INVALID_OPERATION:
         Debug::Print( Debug::TypeWarning, "OpenALError(%s:%d) %s - AL_INVALID_OPERATION - Illegal call.\n", pFile, line, pFunc );
         Debug::Assert( Condition( !requireAssert ), "OpenALError(%s:%d) %s - AL_INVALID_OPERATION - Illegal call.", pFile, line, pFunc );
         return false;
         break;

      case AL_NO_ERROR:
         break;

      default:
         Debug::Print( Debug::TypeWarning, "OpenALError(%s:%d) %s - Unknown OpenAL error.\n", pFile, line, pFunc );
         Debug::Assert( Condition( !requireAssert ), "OpenALError(%s:%d) %s - Unknown OpenAL error.", pFile, line, pFunc );
         return false;
         break;
   }

   return true;
}

bool AudioWorld::FillOALBuffer(Buffer *buffer, ALuint bufferID)
{
   char data[STREAM_BUFFER_SIZE];
   int  size = 0;
   int  section;
   int  result;
   int  index = GetBufferSubIndex(buffer, bufferID);

   while (size < STREAM_BUFFER_SIZE)
   {
      result = ov_read(buffer->m_pOVF, data + size, STREAM_BUFFER_SIZE - size, 0, 2, 1, &section);

      if (result > 0)
      {
         size += result;
      }
      else
      {
         if (result < 0)
         {
            Debug::Assert( Condition( false ), "%s", result );
         }
         else
         {
            break;
         }
      }
   }

   if (size == 0) return false;
 
   alBufferData(buffer->m_BufferIDs[index], buffer->m_OALBufferFormat, data, size, buffer->m_pVorbisInfo->rate);
   if (!CHECKERROR(alGetError(), true))
   {
      return false;
   }

   return true;
}

void AudioWorld::OnThreadRun( void )
{
   uint32 i;
   ScopeLock lock( m_Lock );

   for (i = 0; i < m_PlayingSources.GetSize(); i++)
   {
      // for each source...
      Source *pSource = m_PlayingSources.GetPointer(i);
      if (false == IsSourcePlaying(pSource->m_SourceID, false))
      {
         m_PlayingSources.RemoveAt(i--);
         continue;
      }
      
      {
         // if it's playing...
         Buffer *pBuffer = m_Buffers.GetPointer(GetBufferIndex(pSource->m_AssetName));
         Debug::Assert( Condition(pSource->m_AssetName == pBuffer->m_SoundAsset), "AudioWorld::OnThreadRun - got a buffer back that doesn't match." );
         if (pBuffer->m_NumBuffers > 1)
         {
            // and it's using streaming buffers...
            int processed;
            int queued;
            bool active = true;

            alGetSourcei(pSource->m_SourceID, AL_BUFFERS_PROCESSED, &processed);
            if (!CHECKERROR(alGetError(), false))
            {
               continue;
            }
            alGetSourcei(pSource->m_SourceID, AL_BUFFERS_QUEUED, &queued);
            if (!CHECKERROR(alGetError(), false))
            {
               continue;
            }

            while(processed--)
            {
               ALuint bufferID;

               alSourceUnqueueBuffers(pSource->m_SourceID, 1, &bufferID);
               if (!CHECKERROR(alGetError(), false))
               {
                  continue;
               }

               active = FillOALBuffer(pBuffer, bufferID);
               if (!active)
               {
                  Debug::Print( Debug::TypeInfo, "Stream ended - '%s'\n", pSource->m_AssetName.ToString() );
                  if (pSource->m_Looping)
                  {
                     // if we're supposed to loop, start over from the beginning
                     ov_time_seek(pBuffer->m_pOVF, 0.0);
                     active = FillOALBuffer(pBuffer, bufferID);
                     Debug::Assert( Condition( active ), "AudioWorld::OnThreadRun - Failed to restart looping sound - '%s'", pSource->m_AssetName.ToString() );
                  }
                  else
                  {
                     continue;
                  }
               }

               alSourceQueueBuffers(pSource->m_SourceID, 1, &bufferID);
               if (!CHECKERROR(alGetError(), true))
               {
                  continue;
               }
            }         
         }
      }
   }
}

AudioWorld &AudioWorld::Instance( void )
{
   static AudioWorld s_instance;
   return s_instance;
}

void AudioWorld::Create( void )
{
   m_pGroupMap = new HashTable<int, const char *>;
   m_pGroupMap->Create(16, 16, HashFunctions::IntHash, HashFunctions::IntCompare);
   
   m_Fades.Create( );
   m_Groups.Create( );
   m_Buffers.Create( );
   m_Sources.Create( );
   m_PlayingSources.Create( );
   
   // Initialization
#ifdef WIN32
   m_pDevice = alcOpenDevice((ALchar*)"DirectSound3D");
#else
   m_pDevice = alcOpenDevice(NULL); // select the "preferred device"
#endif
   if (m_pDevice) 
   {
      // use the device to make a context
      m_pContext = alcCreateContext(m_pDevice, NULL);
      // set my context to the currently active one
      alcMakeContextCurrent(m_pContext);
	}

   m_Thread.Create( );
   m_Thread.Run( );
}

void AudioWorld::Destroy( void )
{
   m_Thread.Stop( );

   m_Fades.Clear();
   m_Fades.Destroy( );

   m_Groups.Clear();

   ScopeLock lock( m_Lock );
   Debug::Assert( Condition( m_Sources.GetSize() == 0 ), "AudioWorld::Destroy - AudioWorld being destroyed, but all sources have not been removed. Some object added a source that it did not remove." );
   Debug::Assert( Condition( m_PlayingSources.GetSize() == 0 ), "AudioWorld::Destroy - AudioWorld being destroyed, but all sources have not been removed. Some object added a source that it did not remove." );
   m_Sources.Clear();
   m_PlayingSources.Clear();
   m_pGroupMap->Clear();

   Debug::Assert( Condition( m_Buffers.GetSize() == 0 ), "AudioWorld::Destroy - AudioWorld being destroyed, but all buffers have not been unloaded. This means there are unloaded WavAssets." );
   m_Buffers.Clear();

   m_Groups.Destroy();
   m_Sources.Destroy( );
   m_PlayingSources.Destroy( );
   m_Buffers.Destroy( );

   // destroy the context
   alcDestroyContext(m_pContext);
   alcMakeContextCurrent(NULL);

   // close the device
   alcCloseDevice(m_pDevice);

   m_pGroupMap->Destroy( );
   delete m_pGroupMap;
}

void AudioWorld::Tick(
   float deltaSeconds                        
)
{
   ScopeLock lock( m_Lock );
   MainThreadCheck;

   // process fades, remove them from the list as they complete
   int i;
   for (i = m_Fades.GetSize() - 1; i >= 0; i--)
   {
      Fade *pFade = m_Fades.GetPointer(i);
      pFade->m_CurrentTime += deltaSeconds;
      float newVolume = Math::Max(0.0f, Math::Min(1.0f, Math::Lerp(pFade->m_StartVolume, pFade->m_EndVolume, pFade->m_CurrentTime / pFade->m_FadeTime)));

      if (pFade->m_GroupName[0] != '\0')
      {
         // fading a group volume
         SetSoundGroupVolume(pFade->m_GroupName, newVolume);
      }
      else
      {
         // fading a single source volume
         SetSourceVolume(pFade->m_SourceID, newVolume);
      }

      if (pFade->m_CurrentTime >= pFade->m_FadeTime)
      {
         // fade is complete, remove ourselves from the list
         m_Fades.RemoveAt(i);
      }
   }
}

void AudioWorld::PostTick( void )
{
   MainThreadCheck;
}

void AudioWorld::Final( void )
{
}

void AudioWorld::BeginInterruption( void )
{
   ALenum err;
   
   alcMakeContextCurrent(NULL);
   err = alGetError();
   
   alcSuspendContext(m_pContext);
   err = alGetError();
}

void AudioWorld::EndInterruption( void )
{
   ALenum err;

   alcMakeContextCurrent(m_pContext);
   err = alGetError();

   alcProcessContext(m_pContext);
   err = alGetError();
}

bool AudioWorld::StopAllSounds()
{
   uint32 i;
   ScopeLock lock( m_Lock );

   bool result = true;

   for (i = 0; i < m_Sources.GetSize(); i++)
   {
      Source *pSource = m_Sources.GetPointer(i);
      alSourceStop(pSource->m_SourceID);
      if (!CHECKERROR(alGetError(), true))
      {
         result = false;
      }
   }

   m_PlayingSources.Clear( );

   return result;
}

bool AudioWorld::PauseAllSounds(bool pause)
{
   uint32 i;
   ScopeLock lock( m_Lock );

   for (i = 0; i < m_Sources.GetSize(); i++)
   {
      Source *pSource = m_Sources.GetPointer(i);
      if (pause)
      {
         alSourcePause(pSource->m_SourceID);
         if (!CHECKERROR(alGetError(), true))
         {
            return false;
         }
      }
      else
      {
         alSourcePlay(pSource->m_SourceID);
         if (!CHECKERROR(alGetError(), true))
         {
            return false;
         }
      }
   }

   return true;
}

Vector AudioWorld::GetListenerPosition()
{
   bool success = true;
   ALfloat vector[3];

   alGetListener3f( AL_POSITION, &vector[0], &vector[1], &vector[2] );
   if (!CHECKERROR(alGetError(), true))
   {
      success = false;
   }

   return Vector(vector[0], vector[1], -vector[2]);
}

bool AudioWorld::SetListenerPosition(Vector position)
{
   bool success = true;

   alListener3f( AL_POSITION, position.x, position.y, -position.z );
   if (!CHECKERROR(alGetError(), true))
   {
      success = false;
   }

   return success;
}

Vector AudioWorld::GetListenerLook()
{
   bool success = true;
   ALfloat vectors[6];

   alGetListenerfv( AL_ORIENTATION, vectors );
   if (!CHECKERROR(alGetError(), true))
   {
      success = false;
   }

   return Vector(vectors[0], vectors[1], -vectors[2]);
}

Vector AudioWorld::GetListenerUp()
{
   bool success = true;
   ALfloat vectors[6];

   alGetListenerfv( AL_ORIENTATION, vectors );
   if (!CHECKERROR(alGetError(), true))
   {
      success = false;
   }

   return Vector(vectors[3], vectors[4], -vectors[5]);
}

bool AudioWorld::SetListenerOrientation(Vector look, Vector up)
{
   bool success = true;
   ALfloat vectors[6];

   vectors[0] = look.x;
   vectors[1] = look.y;
   vectors[2] = -look.z;
   vectors[3] = up.x;
   vectors[4] = up.y;
   vectors[5] = -up.z;

   alListenerfv( AL_ORIENTATION, vectors );
   if (!CHECKERROR(alGetError(), true))
   {
      success = false;
   }

   return success;
}

Vector AudioWorld::GetListenerVelocity()
{
   bool success = true;
   ALfloat vector[3];

   alGetListener3f( AL_VELOCITY, &vector[0], &vector[1], &vector[2] );
   if (!CHECKERROR(alGetError(), true))
   {
      success = false;
   }

   return Vector(vector[0], vector[1], -vector[2]);
}

bool AudioWorld::SetListenerVelocity(Vector velocity)
{
   bool success = true;

   alListener3f( AL_VELOCITY, velocity.x, velocity.y, -velocity.z );
   if (!CHECKERROR(alGetError(), true))
   {
      success = false;
   }

   return success;
}

void AudioWorld::AddSoundGroup(const char *groupName, float volume /* = 1.0f */)
{
   Group newGroup;

   String::Copy(newGroup.m_Name, groupName, sizeof(newGroup.m_Name));
   newGroup.m_Volume = volume;

   m_Groups.Add(newGroup);
}

void AudioWorld::RemoveSoundGroup(const char *groupName)
{
   uint32 i;
   for (i = 0; i < m_Groups.GetSize(); i++)
   {
      if (strcmp(groupName, m_Groups.GetAt(i).m_Name) == 0)
      {
         m_Groups.RemoveAt(i);
         return;
      }
   }
}

void AudioWorld::SetSoundGroupVolume(const char *groupName, float volume)
{
   uint32 i;
   ScopeLock lock( m_Lock );

   for (i = 0; i < m_Groups.GetSize(); i++)
   {
      if (strcmp(groupName, m_Groups.GetAt(i).m_Name) == 0)
      {
         // update group structure
         Group *pGroup = m_Groups.GetPointer(i);
         pGroup->m_Volume = volume;

         // update all members of the group
         Enumerator<int, const char *> e = m_pGroupMap->GetEnumerator();
         while ( e.EnumNext( ) )
         {
            if (strcmp(e.Data( ), groupName) == 0)
            {
               Source *pSource = m_Sources.GetPointer( GetSourceIndex(e.Key( )) );
               OALSetSourceFloat(pSource->m_SourceID, AL_GAIN, volume * pSource->m_Volume);
            }
         }
         return;
      }
   }
}

float AudioWorld::GetSoundGroupVolume(const char *groupName)
{
   uint32 i;
   for (i = 0; i < m_Groups.GetSize(); i++)
   {
      if (strcmp(groupName, m_Groups.GetAt(i).m_Name) == 0)
      {
         return m_Groups.GetAt(i).m_Volume;
      }
   }

   return 0.0f;
}

void AudioWorld::FadeSoundGroupVolume(const char *groupName, float volume, float fadeTime)
{
   ScopeLock lock( m_Lock );
   Fade newFade;

   newFade.m_StartVolume = GetSoundGroupVolume(groupName);
   newFade.m_CurrentTime = 0.0f;
   newFade.m_FadeTime    = fadeTime;
   newFade.m_EndVolume   = volume;
   String::Copy(newFade.m_GroupName, groupName, sizeof(newFade.m_GroupName));

   m_Fades.Add(newFade);
}

bool AudioWorld::GetWaveALBufferFormat(uint32 numChannels, uint32 bitsPerSample, PFNALGETENUMVALUE pfnGetEnumValue, unsigned long *pulFormat)
{
   bool success = true;

	if (pfnGetEnumValue && pulFormat)
	{
		*pulFormat = 0;

		if (numChannels == 1)
		{
			switch (bitsPerSample)
			{
			case 4:
				*pulFormat = pfnGetEnumValue("AL_FORMAT_MONO_IMA4");
				break;
			case 8:
				*pulFormat = pfnGetEnumValue("AL_FORMAT_MONO8");
				break;
			case 16:
				*pulFormat = pfnGetEnumValue("AL_FORMAT_MONO16");
				break;
			}
		}
		else if (numChannels == 2)
		{
			switch (bitsPerSample)
			{
			case 4:
				*pulFormat = pfnGetEnumValue("AL_FORMAT_STEREO_IMA4");
				break;
			case 8:
				*pulFormat = pfnGetEnumValue("AL_FORMAT_STEREO8");
				break;
			case 16:
				*pulFormat = pfnGetEnumValue("AL_FORMAT_STEREO16");
				break;
			}
		}
		else if ((numChannels == 4) && (bitsPerSample == 16))
      {
			*pulFormat = pfnGetEnumValue("AL_FORMAT_QUAD16");
      }

      if (*pulFormat == 0)
      {
			success = false;
      }
	}
	else
	{
		success = false;
	}

	return success;
}

ALuint AudioWorld::GetBufferID(Id id, int index)
{
   uint32 i;
   for (i = 0; i < m_Buffers.GetSize(); i++)
   {
      Buffer *pBuffer = m_Buffers.GetPointer(i);
      if (pBuffer->m_SoundAsset == id)
      {
         Debug::Assert( Condition(index < pBuffer->m_NumBuffers), "AudioWorld::GetBufferID - trying to get buffer %d, but there are only %d buffers for %s", index, pBuffer->m_NumBuffers, pBuffer->m_SoundAsset.ToString() );
         return pBuffer->m_BufferIDs[index];
      }
   }

   Debug::Assert( Condition(false), "AudioWorld::GetBufferID - cannot find a buffer for %s", id.ToString());
   return -1;
}

int AudioWorld::GetBufferIndex(Id id, bool assertOnFail /*= true*/)
{
   uint32 i;
   for (i = 0; i < m_Buffers.GetSize(); i++)
   {
      if (m_Buffers.GetAt(i).m_SoundAsset == id)
      {
         return i;
      }
   }

   if ( true == assertOnFail )
   {
      Debug::Assert( Condition(false), "AudioWorld::GetBufferIndex - cannot find a buffer for %s", id.ToString());
   }
   
   return -1;
}

int AudioWorld::GetBufferIndex(ALuint bufferID)
{
   uint32 i;
   for (i = 0; i < m_Buffers.GetSize(); i++)
   {
      int j;
      Buffer buffer = m_Buffers.GetAt(i);
      for (j = 0; j < buffer.m_NumBuffers; j++)
      {
         if (buffer.m_BufferIDs[j] == bufferID)
         {
            return i;
         }
      }
   }

   Debug::Assert( Condition(false), "AudioWorld::GetBufferIndex - cannot find a buffer for %d", bufferID);
   return -1;
}

int AudioWorld::GetBufferSubIndex(ALuint bufferID)
{
   uint32 i;
   for (i = 0; i < m_Buffers.GetSize(); i++)
   {
      int j;
      Buffer buffer = m_Buffers.GetAt(i);
      for (j = 0; j < buffer.m_NumBuffers; j++)
      {
         if (buffer.m_BufferIDs[j] == bufferID)
         {
            return j;
         }
      }
   }

   Debug::Assert( Condition(false), "AudioWorld::GetBufferSubIndex - cannot find a buffer for %d", bufferID);
   return -1;
}

int AudioWorld::GetBufferSubIndex(Buffer *pBuffer, ALuint bufferID)
{
   int i;
   for (i = 0; i < pBuffer->m_NumBuffers; i++)
   {
      if (pBuffer->m_BufferIDs[i] == bufferID)
      {
         return i;
      }
   }

   Debug::Assert( Condition(false), "AudioWorld::GetBufferSubIndex - cannot find a buffer for %d", bufferID);
   return -1;
}

bool AudioWorld::LoadBuffer(Wav *pWavAsset, char *pWavData)
{
   ScopeLock lock( m_Lock );

   //if it already exists just increment the refcount
   //because our sources reference buffers by name
   //and duplicate names messes up the references
   
   //if sources need unique buffers it'll take more memory
   //and we can have them refer to a buffer pointer instead
   //of a wav asset name
   int index = GetBufferIndex(pWavAsset->GetId( ).ToString(), false);

   if ( index >= 0 )
   {
      m_Buffers.GetPointer(index )->m_RefCount++;
      return true;
   }

   uint32 fileSize = pWavAsset->GetFileSize();
   uint32 audioSize = pWavAsset->GetAudioSize();
   uint32 dataOffset = pWavAsset->GetDataOffset();
   uint32 numChannels = pWavAsset->GetNumChannels();
   uint32 frequency = pWavAsset->GetFrequency();
   uint32 bitsPerSample = pWavAsset->GetBitsPerSample();
   bool isCompressedOgg = pWavAsset->IsCompressedOgg();
   bool isStreaming = pWavAsset->IsStreaming();
   const char *pAudioData = &pWavData[dataOffset];
   char *pcmout = new char[audioSize];
   Buffer newBuffer;
	ALenum eBufferFormat;
   bool success = true;

   newBuffer.m_SoundAsset = pWavAsset->GetHandle().GetId();

   if (!GetWaveALBufferFormat(numChannels, bitsPerSample, &alGetEnumValue, (unsigned long*)&eBufferFormat))
   {
      success = false;
   }
   else
   {
      newBuffer.m_OALBufferFormat = eBufferFormat;

      if ( !isStreaming )
      {
         // Generate an AL Buffer
         alGenBuffers( 1, &newBuffer.m_BufferIDs[0] );
         if (!CHECKERROR(alGetError(), true))
         {
            success = false;
         }
         newBuffer.m_NumBuffers = 1;

         if (isCompressedOgg)
         {
            // setup ogg file data
            OggVorbis_File vf;
            _OggFile oggFile;
            oggFile.pData = (char *)pWavData;
            oggFile.pCurrent = (char *)pWavData;
            oggFile.fileSize = fileSize;

            // decompress ogg data
            ov_callbacks callbacks;
            callbacks.close_func = NULL;
            callbacks.read_func  = _oggRead;
            callbacks.seek_func  = _oggSeek;
            callbacks.tell_func  = _oggTell;

            // tell the ogg lib about our file reading functions
            int result = ov_open_callbacks((void *)&oggFile, &vf, NULL, 0, callbacks);
            if (result < 0)
            {
               Debug::Print( Debug::TypeInfo, "Audio decompression fail: input does not appear to be a valid Ogg bitstream.\n" );
               return false;
            }

            {
               // ogg comment info
               char **ptr = ov_comment(&vf, -1)->user_comments;
               vorbis_info *vi = ov_info(&vf, -1);
               vi = vi; //remove unused variable warning
               while (*ptr)
               {
//                  Debug::Print( Debug::TypeInfo, "Ogg comment: %s\n", *ptr );
                  ++ptr;
               }
//               Debug::Print( Debug::TypeInfo, "Ogg Bitstream is %d channel, %ldHz\n", vi->channels, vi->rate );
//               Debug::Print( Debug::TypeInfo, "Decoded length: %ld samples\n", (long)ov_pcm_total(&vf, -1) );
//               Debug::Print( Debug::TypeInfo, "Encoded by: %s\n\n",ov_comment(&vf, -1)->vendor );
            }

            int eof = 0;
            long decodedSize = 0;
            char *pPCMout = &pcmout[0];
            while (!eof)
            {
               int current_section;
               long ret = ov_read(&vf, pPCMout, audioSize - decodedSize, 0, 2, 1, &current_section);
               decodedSize += ret;
               if (ret == 0)
               {
                  // EOF
                  eof = 1;
               }
               else if (ret < 0)
               {
                  // error in the stream.  Not a problem, just reporting it in case we (the app) cares.  In this case, we don't.
               }
               else
               {
                  // we don't bother dealing with sample rate changes, etc, but you'll have to
                  pPCMout += ret;
               }
            }

            // cleanup 
            ov_clear(&vf);

            // set beginning of pcm audio data buffer
            pAudioData = &pcmout[0];
         }

         // create open al buffer
         alBufferData(newBuffer.m_BufferIDs[0], newBuffer.m_OALBufferFormat, pAudioData, audioSize, frequency);
         if (!CHECKERROR(alGetError(), true))
         {
            success = false;
         }
      }
      else
      {
         // Generate NUM_STREAMING_BUFFERS AL Buffers for streaming
         alGenBuffers( NUM_STREAMING_BUFFERS, newBuffer.m_BufferIDs );
         if (!CHECKERROR(alGetError(), true))
         {
            success = false;
         }
         newBuffer.m_NumBuffers = NUM_STREAMING_BUFFERS;

         if (isCompressedOgg)
         {
            // setup ogg file data
            newBuffer.m_pOggFile = new _OggFile;
            newBuffer.m_pOggFile->pData = (char *)pWavData;
            newBuffer.m_pOggFile->pCurrent = (char *)pWavData;
            newBuffer.m_pOggFile->fileSize = fileSize;

            // decompress ogg data
            ov_callbacks callbacks;
            callbacks.close_func = NULL;
            callbacks.read_func  = _oggRead;
            callbacks.seek_func  = _oggSeek;
            callbacks.tell_func  = _oggTell;

            // tell the ogg lib about our file reading functions
            newBuffer.m_pOVF = new OggVorbis_File;
            if(ov_open_callbacks((void *)newBuffer.m_pOggFile, newBuffer.m_pOVF, NULL, 0, callbacks) < 0)
            {
               Debug::Print( Debug::TypeInfo, "Audio decompression fail: input does not appear to be a valid Ogg bitstream.\n" );
               return false;
            }

            {
               // ogg comment info
               char **ptr = ov_comment(newBuffer.m_pOVF, -1)->user_comments;
               newBuffer.m_pVorbisInfo = ov_info(newBuffer.m_pOVF, -1);
               while (*ptr)
               {
//                  Debug::Print( Debug::TypeInfo, "Ogg comment: %s\n", *ptr );
                  ++ptr;
               }
//               Debug::Print( Debug::TypeInfo, "Ogg Bitstream is %d channel, %ldHz\n", newBuffer.m_pVorbisInfo->channels, newBuffer.m_pVorbisInfo->rate );
//               Debug::Print( Debug::TypeInfo, "Decoded length: %ld samples\n", (long)ov_pcm_total(&vf, -1) );
//               Debug::Print( Debug::TypeInfo, "Encoded by: %s\n\n",ov_comment(&vf, -1)->vendor );
            }
         }
         else
         {
            newBuffer.m_pOggFile = NULL;
         }
      }
   }

   newBuffer.m_RefCount = 1;
   m_Buffers.Add(newBuffer);

   delete [] pcmout;

   return success;
}

void AudioWorld::UnloadBuffer(ResourceHandle wavAsset)
{
   uint32 i;
   ScopeLock lock( m_Lock );

   for (i = 0; i < m_Buffers.GetSize(); i++)
   {
      Buffer *pBuffer = m_Buffers.GetPointer(i);
      if (pBuffer->m_SoundAsset == wavAsset.GetId())
      {
         if (pBuffer->m_NumBuffers > 1)
         {
            ov_clear(pBuffer->m_pOVF);
            delete pBuffer->m_pOVF;
            delete pBuffer->m_pOggFile;
         }
         
         // make sure all sources using this buffer are detached, otherwise OpenAL will error
         bool detached = true;
         
         //I don't think we need to do this now that the buffers
         //are refcounted
         //uint32 j;
         //const char *pAssetName = wavAsset.GetId().ToString();
         //for (j = 0; j < m_Sources.GetSize(); j++)
         //{
         //   Source *pSource = m_Sources.GetPointer(j);
         //   if (strcmp(pAssetName, pSource->m_AssetName) == 0)
         //   {
         //      detached = false;
         //      break;
         //   }
         //}

         if (detached)
         {
            m_Buffers.GetPointer(i)->m_RefCount--;

            if ( 0 == m_Buffers.GetPointer(i)->m_RefCount )
            {
               int k;
               for (k = 0; k < pBuffer->m_NumBuffers; k++)
               {
                  ALuint id = pBuffer->m_BufferIDs[k];
                  alDeleteBuffers(1, &id);
                  if (!CHECKERROR(alGetError(), true))
                  {
                     continue;
                  }
               }

               m_Buffers.RemoveAt(i);
            }
            return;
         }
      }
   }
}

ALuint AudioWorld::GetSourceID(ResourceHandle owner, Id assetName)
{
   uint32 i;
   ScopeLock lock( m_Lock );

   for (i = 0; i < m_Sources.GetSize(); i++)
   {
      Source *pSource = m_Sources.GetPointer(i);
      if (pSource->m_SourceHandle == owner.GetId() && pSource->m_AssetName == assetName)
      {
         return pSource->m_SourceID;
      }
   }

   Debug::Assert( Condition(false), "AudioWorld::GetSourceID - cannot find a source for %s, %s", owner.GetId().ToString(), assetName.ToString());
   return -1;
}

int AudioWorld::GetSourceIndex(ResourceHandle owner, Id assetName)
{
   uint32 i;
   ScopeLock lock( m_Lock );

   for (i = 0; i < m_Sources.GetSize(); i++)
   {
      Source source = m_Sources.GetAt(i);
      if (source.m_SourceHandle == owner.GetId() && source.m_AssetName == assetName)
      {
         return i;
      }
   }

   Debug::Assert( Condition(false), "AudioWorld::GetSourceIndex - cannot find a source for %s, %s", owner.GetId().ToString(), assetName.ToString());
   return -1;
}

int AudioWorld::GetSourceIndex(ALuint sourceID)
{
   uint32 i;
   ScopeLock lock( m_Lock );

   for (i = 0; i < m_Sources.GetSize(); i++)
   {
      Source source = m_Sources.GetAt(i);
      if (source.m_SourceID == sourceID)
      {
         return i;
      }
   }

   Debug::Assert( Condition(false), "AudioWorld::GetSourceIndex - cannot find a source for %d", sourceID);
   return -1;
}

int AudioWorld::OALGetSourceInt(ALuint sourceID, ALenum property)
{
   bool success = true;
   ALint value = -1;

   alGetSourcei( sourceID, property, &value );
   if (!CHECKERROR(alGetError(), true))
   {
      success = false;
   }

   return value;
}

bool  AudioWorld::OALSetSourceInt(ALuint sourceID, ALenum property, int value)
{
   bool success = true;

   alSourcei( sourceID, property, value );
   if (!CHECKERROR(alGetError(), true))
   {
      success = false;
   }

   return success;
}

float AudioWorld::OALGetSourceFloat(ALuint sourceID, ALenum property)
{
   bool success = true;
   ALfloat value = -1.0f;

   alGetSourcef( sourceID, property, &value );
   if (!CHECKERROR(alGetError(), true))
   {
      success = false;
   }

   return value;
}

bool  AudioWorld::OALSetSourceFloat(ALuint sourceID, ALenum property, float value)
{
   bool success = true;

   alSourcef( sourceID, property, value );
   if (!CHECKERROR(alGetError(), true))
   {
      success = false;
   }

   return success;
}

Vector AudioWorld::OALGetSourceVector(ALuint sourceID, ALenum property)
{
   bool success = true;
   ALfloat value[3];

   alGetSource3f( sourceID, property, &value[0], &value[1], &value[2] );
   if (!CHECKERROR(alGetError(), true))
   {
      success = false;
   }

   return Vector(value[0], value[1], value[2]);
}

bool  AudioWorld::OALSetSourceVector(ALuint sourceID, ALenum property, Vector value)
{
   bool success = true;

   alSource3f( sourceID, property, value.x, value.y, value.z );
   if (!CHECKERROR(alGetError(), true))
   {
      success = false;
   }

   return success;
}

bool AudioWorld::IsSourcePlaying(ALuint sourceID, bool requireAssert)
{
   ALenum state;
   alGetSourcei(sourceID, AL_SOURCE_STATE, &state);
   if (!CHECKERROR(alGetError(), requireAssert))
   {
      return false;
   }

   return AL_PLAYING == state;
}

bool AudioWorld::IsSourceInInitialState(ALuint sourceID)
{
   ALenum state;
   alGetSourcei(sourceID, AL_SOURCE_STATE, &state);
   if (!CHECKERROR(alGetError(), true))
   {
      return false;
   }

   return AL_INITIAL == state;
}

bool AudioWorld::AddSource(ResourceHandle owner, const char *assetName, const char *groupName /* = "Default" */, float volume /* = 1.0f */, bool looping /* = true */)
{
   ScopeLock lock( m_Lock );

   Source newSource;
   bool success = true;
   
   int bufferIndex = GetBufferIndex(assetName);
   Buffer *pBuffer = m_Buffers.GetPointer(bufferIndex);

   // Generate an AL Source
   alGenSources( 1, &newSource.m_SourceID );
   if (!CHECKERROR(alGetError(), true))
   {
      success = false;
   }

   if (pBuffer->m_NumBuffers == 1)
   {
      alSourcei(newSource.m_SourceID, AL_BUFFER, GetBufferID(assetName, 0));
      if (!CHECKERROR(alGetError(), true))
      {
         success = false;
      }
   }
   else
   {
      if (pBuffer->m_pOggFile)
      {
         // reset to beginning of buffer
         ov_time_seek(pBuffer->m_pOVF, 0.0);

         // decompress first NUM_STREAMING_BUFFERS buffers
         int i;
         for (i = 0; i < NUM_STREAMING_BUFFERS; i++)
         {
            if (!FillOALBuffer(pBuffer, pBuffer->m_BufferIDs[i]))
            {
               return false;
            }
         }
      }

      // queue open al buffers
      alSourceQueueBuffers(newSource.m_SourceID, NUM_STREAMING_BUFFERS, pBuffer->m_BufferIDs);
      if (!CHECKERROR(alGetError(), true))
      {
         success = false;
      }
   }

   newSource.m_SourceHandle = owner.GetId();
   newSource.m_Volume = volume;
   newSource.m_Looping = looping;
   newSource.m_AssetName = Id(assetName);
   
   float groupVolume = GetSoundGroupVolume(groupName);
   OALSetSourceFloat(newSource.m_SourceID, AL_GAIN, volume * groupVolume);
   if (pBuffer->m_NumBuffers == 1)
   {
      OALSetSourceInt(newSource.m_SourceID, AL_LOOPING, looping);
   }

   //ScopeLock lock( m_Lock );
   m_Sources.Add(newSource);
   m_pGroupMap->Add(newSource.m_SourceID, groupName);

   return success;
}

bool AudioWorld::RemoveSource(uint32 index, ALuint sourceID)
{
   int queued;
   ScopeLock lock( m_Lock );
   alGetSourcei(sourceID, AL_BUFFERS_QUEUED, &queued);
   if (CHECKERROR(alGetError(), true) && queued > 1)
   {
      while(queued--)
      {
         ALuint buffer;
         alSourceUnqueueBuffers(sourceID, 1, &buffer);
         if (!CHECKERROR(alGetError(), false))
         {
            continue;
         }
      }
   }

   // make sure all fades using this source are removed, otherwise OpenAL will error
   int i;
   for (i = m_Fades.GetSize() - 1; i >= 0; i--)
   {
      Fade *pFade = m_Fades.GetPointer(i);
      if (pFade->m_SourceID == sourceID)
      {
         m_Fades.RemoveAt(i);
      }
   }

   alDeleteSources(1, &sourceID);
   if (!CHECKERROR(alGetError(), true))
   {
      return false;
   }

   return true;
}

void AudioWorld::RemoveSource(ResourceHandle owner, const char *assetName)
{
   int sourceIndex = GetSourceIndex(owner, assetName);
   ALuint sourceID = GetSourceID(owner, assetName);
   ScopeLock lock( m_Lock );

   for (uint32 i = 0; i < m_PlayingSources.GetSize(); i++)
   {
      if (m_PlayingSources.GetAt(i).m_SourceID == sourceID)
      {
         m_PlayingSources.RemoveAt(i--);
         break;
      }
   }

   if (RemoveSource(sourceIndex, sourceID))
   {
      m_Sources.RemoveAt(sourceIndex);
      m_pGroupMap->Remove(sourceID);
   }
}

void AudioWorld::SetSourceGroup(ResourceHandle owner, const char *assetName, const char *groupName)
{
   ALuint sourceID = GetSourceID(owner, assetName);
   m_pGroupMap->Remove(sourceID);
   m_pGroupMap->Add(sourceID, groupName);
}

const char *AudioWorld::GetSourceGroup(ResourceHandle owner, const char *assetName)
{
   ALuint sourceID = GetSourceID(owner, assetName);
   const char *pGroupName;

   m_pGroupMap->Get(sourceID, &pGroupName);

   // return the one in the list because it's on the heap
   uint32 i;
   for (i = 0; i < m_Groups.GetSize(); i++)
   {
      Group *pGroup = m_Groups.GetPointer(i);
      if (strcmp(pGroup->m_Name, pGroupName) == 0)
      {
         return pGroup->m_Name;
      }
   }

   return NULL;
}

bool AudioWorld::PlaySource(ResourceHandle owner, const char *assetName)
{
   ALuint sourceID = GetSourceID(owner, assetName);
   bool success = true;

   alSourcePlay( sourceID );
   if (!CHECKERROR(alGetError(), true))
   {
      success = false;
   }

   return success;
}

bool AudioWorld::PauseSource(ResourceHandle owner, const char *assetName)
{
   ALuint sourceID = GetSourceID(owner, assetName);
   bool success = true;

   alSourcePause( sourceID );
   if (!CHECKERROR(alGetError(), true))
   {
      success = false;
   }

   return success;
}

bool AudioWorld::StopSource(ResourceHandle owner, const char *assetName)
{
   ALuint sourceID = GetSourceID(owner, assetName);
   bool success = true;

   alSourceStop( sourceID );
   if (!CHECKERROR(alGetError(), true))
   {
      success = false;
   }

   return success;
}

bool AudioWorld::SetSourcePosition(ResourceHandle owner, const char *assetName, Vector position)
{
   return OALSetSourceVector(GetSourceID(owner, assetName), AL_POSITION, Vector(position.x, position.y, -position.z));
}

bool AudioWorld::SetSourceOrientation(ResourceHandle owner, const char *assetName, Vector look, Vector up)
{
   return OALSetSourceVector(GetSourceID(owner, assetName), AL_DIRECTION, Vector(look.x, look.y, -look.z));
}

bool AudioWorld::SetSourceVelocity(ResourceHandle owner, const char *assetName, Vector velocity)
{
   return OALSetSourceVector(GetSourceID(owner, assetName), AL_VELOCITY, Vector(velocity.x, velocity.y, -velocity.z));
}

bool AudioWorld::IsSourceInInitialState(ResourceHandle owner, const char *assetName)
{
   return IsSourceInInitialState(GetSourceID(owner, assetName));
}

bool AudioWorld::IsSourcePlaying(ResourceHandle owner, const char *assetName)
{
   return IsSourcePlaying(GetSourceID(owner, assetName), true);
}

bool AudioWorld::GetSourceLooping(ResourceHandle owner, const char *assetName)
{
   return m_Sources.GetAt(GetSourceIndex(owner, assetName)).m_Looping;
}

bool AudioWorld::SetSourceLooping(ResourceHandle owner, const char *assetName, bool looping)
{
   ScopeLock lock( m_Lock );
   ALuint sourceID = GetSourceID(owner, assetName);
   m_Sources.GetPointer(GetSourceIndex(owner, assetName))->m_Looping = looping;

   // updated structure, now tell oal about it
   return OALSetSourceInt(sourceID, AL_LOOPING, (int)looping);
}

float AudioWorld::GetSourceVolume(ResourceHandle owner, const char *assetName)
{
   return m_Sources.GetAt(GetSourceIndex(owner, assetName)).m_Volume;
}

bool AudioWorld::SetSourceVolume(ALuint sourceID, float volume)
{
   ScopeLock lock( m_Lock );
   const char *pGroupName;
   m_pGroupMap->Get(sourceID, &pGroupName);
   float groupVolume = GetSoundGroupVolume(pGroupName);
   m_Sources.GetPointer(GetSourceIndex(sourceID))->m_Volume = volume;

   // updated structure, now tell oal about it
   return OALSetSourceFloat(sourceID, AL_GAIN, volume * groupVolume);
}

bool AudioWorld::SetSourceVolume(ResourceHandle owner, const char *assetName, float volume)
{
   return SetSourceVolume(GetSourceID(owner, assetName), volume);
}

void AudioWorld::FadeSourceVolume(ResourceHandle owner, const char *assetName, float volume, float fadeTime)
{
   ScopeLock lock( m_Lock );
   Fade newFade;

   newFade.m_StartVolume  = GetSourceVolume(owner, assetName);
   newFade.m_CurrentTime  = 0.0f;
   newFade.m_FadeTime     = fadeTime;
   newFade.m_EndVolume    = volume;
   newFade.m_GroupName[0] = '\0';
   newFade.m_SourceID     = GetSourceID(owner, assetName);

   m_Fades.Add(newFade);
}

float AudioWorld::GetSourcePlaybackTime(ResourceHandle owner, const char *assetName)
{
   return OALGetSourceFloat(GetSourceID(owner, assetName), AL_SEC_OFFSET);
}

bool AudioWorld::SetSourcePlaybackTime(ResourceHandle owner, const char *assetName, float time)
{
   ScopeLock lock( m_Lock );

   // if this is a streaming sound, we need to update buffers, too
   ALuint sourceID = GetSourceID(owner, assetName);
   Buffer *pBuffer = m_Buffers.GetPointer(GetBufferIndex(assetName));
   bool wasPlaying = IsSourcePlaying(sourceID, false);
   if (pBuffer->m_NumBuffers > 1)
   {
      if (wasPlaying)
      {
         // stop the sound first
         alSourceStop(sourceID);
         if (!CHECKERROR(alGetError(), false))
         {
            return false;
         }
      }

      // detach all buffers
      alSourcei(sourceID, AL_BUFFER, 0);
      if (!CHECKERROR(alGetError(), true))
      {
         return false;
      }

      // seek
      if (pBuffer->m_pOggFile)
      {
         // reset to beginning of buffer
         ov_time_seek(pBuffer->m_pOVF, time);

         // decompress first NUM_STREAMING_BUFFERS buffers
         int i;
         for (i = 0; i < NUM_STREAMING_BUFFERS; i++)
         {
            if (!FillOALBuffer(pBuffer, pBuffer->m_BufferIDs[i]))
            {
               return false;
            }
         }
      }

      // queue open al buffers
      alSourceQueueBuffers(sourceID, NUM_STREAMING_BUFFERS, pBuffer->m_BufferIDs);
      if (!CHECKERROR(alGetError(), true))
      {
         return false;
      }

      if (wasPlaying)
      {
         // restart the sound
         alSourcePlay(sourceID);
         if (!CHECKERROR(alGetError(), false))
         {
            return false;
         }
      }
   }

   return OALSetSourceFloat(GetSourceID(owner, assetName), AL_SEC_OFFSET, time);
}

float AudioWorld::GetSourceInnerRadius(ResourceHandle owner, const char *assetName)
{
   return OALGetSourceFloat(GetSourceID(owner, assetName), AL_REFERENCE_DISTANCE);
}

bool AudioWorld::SetSourceInnerRadius(ResourceHandle owner, const char *assetName, float radius)
{
   return OALSetSourceFloat(GetSourceID(owner, assetName), AL_REFERENCE_DISTANCE, radius);
}

float AudioWorld::GetSourceOuterRadius(ResourceHandle owner, const char *assetName)
{
   return OALGetSourceFloat(GetSourceID(owner, assetName), AL_MAX_DISTANCE);
}

bool AudioWorld::SetSourceOuterRadius(ResourceHandle owner, const char *assetName, float radius)
{
   return OALSetSourceFloat(GetSourceID(owner, assetName), AL_MAX_DISTANCE, radius);
}
