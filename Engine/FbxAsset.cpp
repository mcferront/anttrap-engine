#include "EnginePch.h"

#include "FbxAsset.h"
#include "MeshAsset.h"
#include "AnimAsset.h"
#include "ColliderAsset.h"
#include "SplineAsset.h"

DefineResourceType(Fbx, Asset, new FbxSerializer);

ISerializable *FbxSerializer::Deserialize(
   Serializer *pSerializer,
   ISerializable *pSerializable
)
{
   struct FbxHeader
   {
      unsigned int version;
      char type[8];
   };


   const uint32 CurrentVersion = 2;

   FbxHeader header;

   pSerializer->GetInputStream( )->Read( &header, sizeof(header), NULL );
   Debug::Assert( Condition(header.version == CurrentVersion), "Header version was %d but I was expecting %d", header.version, CurrentVersion );

   if ( 0 == strcmp(header.type, "mesh") )
   {
      MeshSerializer meshSerializer;
      return meshSerializer.Deserialize( pSerializer, pSerializable );
   }
   else if ( 0 == strcmp(header.type, "anim") )
   {
      AnimSerializer animSerializer;
      return animSerializer.Deserialize( pSerializer, pSerializable );
   }
   //else if ( 0 == strcmp(header.type, "spline") )
   //{
   //   SplineSerializer splineSerializer;
   //   return splineSerializer.Deserialize( pSerializer, pSerializable );
   //}

   return NULL;
}
   