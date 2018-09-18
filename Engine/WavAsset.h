#pragma once

#include "EngineGlobal.h"
#include "ResourceWorld.h"
#include "Serializer.h"
#include "Asset.h"

class Wav : public Asset
{
friend class WavSerializer;

public:
   struct Header
   {
      uint32 audioSize;
      uint32 fileSize;
      uint32 dataOffset;
      uint32 numChannels;
      uint32 frequency;
      uint32 bitsPerSample;
      bool compressedOgg;
      bool streaming;
      char id[ 42 ];
   };

private:
   uint32 m_AudioSize;
   uint32 m_FileSize;
   uint32 m_DataOffset;
   uint32 m_NumChannels;
   uint32 m_Frequency;
   uint32 m_BitsPerSample;
   bool m_CompressedOgg;
   bool m_Streaming;
   char *m_pData;

public:
   virtual void Destroy( void );

   DeclareResourceType(Wav);

   uint32 GetAudioSize()      { return m_AudioSize; }
   uint32 GetFileSize()       { return m_FileSize; }
   uint32 GetDataOffset()     { return m_DataOffset; }
   uint32 GetNumChannels()    { return m_NumChannels; }
   uint32 GetFrequency()      { return m_Frequency; }
   uint32 GetBitsPerSample()  { return m_BitsPerSample; }
   bool   IsCompressedOgg()   { return m_CompressedOgg; }
   bool   IsStreaming()       { return m_Streaming; }
};

class WavSerializer : public ISerializer
{
public:
   virtual bool Serialize(
      Serializer *pSerializer,
      const ISerializable *pSerializable
   )
   {
      return false;
   }

   virtual ISerializable *Deserialize(
      Serializer *pSerializer,
      ISerializable *pSerializable
   );

   virtual ISerializable *Instantiate() const { return new Wav; }

   virtual const SerializableType &GetSerializableType( void ) const { return Wav::StaticSerializableType( ); }

   virtual uint32 GetVersion( void ) const { return 1; }
};
