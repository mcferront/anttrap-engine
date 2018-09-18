#pragma once

#include "EngineGlobal.h"
#include "ResourceMaps.h"
#include "Resource.h"

class Quad;
class ResourceHandle;

class Font
{
private:
   ResourceHandle   m_Texture;
   ResourceHandle   m_FontMap;
   
public:
   Font( void )
   {}

   void Create(
      ResourceHandle texture,
      ResourceHandle fontMap
   );

   void Destroy( void );

   //how many characters will fit
   //within limitX size
   uint32 GetCharacterCount(
      const char *pString,
      float limitX
   ) const;

   //how wide the string will be
   //up to numChars
   float GetStringWidth(
      const char *pString,
      size_t numChars
   ) const;

   //go through the string and find
   //the max width/height of the characters
   void GetMaxCharacterSize(
      const char *pString,
      Vector2 *pSize
   ) const;

   //setup a series of quads with the correct
   //coordinates and UVs to render each letter
   //in the string
   uint32 Draw(
      float x,
      float y,
      const char *pString,
      Quad *pQuads,
      uint32 numQuads
   ) const;

   ResourceHandle GetFontMap( void ) const { return m_FontMap; }
};
