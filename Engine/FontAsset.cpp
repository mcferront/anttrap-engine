#include "EnginePch.h"

#include "FontAsset.h"
#include "RenderWorld.h"
#include "ResourceWorld.h"
#include "UtilityString.h"
#include "FontMapAsset.h"
#include "TextureAsset.h"
#include "Quad.h"

void Font::Create(
   ResourceHandle texture,
   ResourceHandle fontMap
)
{
   m_Texture = texture;
   m_FontMap = fontMap;
}

void Font::Destroy( void )
{
   m_Texture = NullHandle;
   m_FontMap = NullHandle;
}

uint32 Font::GetCharacterCount(
   const char *pString,
   float limitX
) const
{
   //how many characters will fit
   //within limitX size

   if ( false == IsResourceLoaded(m_FontMap) ) return 0;
   if ( false == IsResourceLoaded(m_Texture) ) return 0;

   FontMap *pFontMap = GetResource( m_FontMap, FontMap );
   Texture *pTexture = GetResource( m_Texture, Texture );

   const char *pCharacter = pString;

   int spacingWidth = pFontMap->GetSpacingWidth( );

   while ( *pCharacter )
   {
      limitX -= pFontMap->GetCharacter( *pCharacter - 32 )->width * pTexture->GetWidth( ) + spacingWidth;
      
      if ( limitX < 0.0f ) 
      {
         return (uint32) (pCharacter - pString);
      }

      ++pCharacter;
   }

   return (uint32) (pCharacter - pString);
}

float Font::GetStringWidth(
   const char *pString,
   size_t maxChars
) const
{
   //how wide the string will be
   //up to numChars

   if ( false == IsResourceLoaded(m_FontMap) ) return 0;
   if ( false == IsResourceLoaded(m_Texture) ) return 0;

   FontMap *pFontMap = GetResource( m_FontMap, FontMap );
   Texture *pTexture = GetResource( m_Texture, Texture );

   int spacingWidth = pFontMap->GetSpacingWidth( );

   const char *pCharacter = pString;
   
   float width = 0.0f;

   while ( (size_t)(pCharacter - pString) < maxChars && *pCharacter )
   {
      width += pFontMap->GetCharacter( *pCharacter - 32 )->width * pTexture->GetWidth( ) + spacingWidth;
      ++pCharacter;
   }
   
   return width;
}

void Font::GetMaxCharacterSize(
   const char *pString,
   Vector2 *pSize
) const
{
   //go through the string and find
   //the max width/height of the characters

   Vector2 size;

   *pSize = Math::ZeroVector2( );

   if ( false == IsResourceLoaded(m_FontMap) ) return;
   if ( false == IsResourceLoaded(m_Texture) ) return;

   FontMap *pFontMap = GetResource( m_FontMap, FontMap );
   Texture *pTexture = GetResource( m_Texture, Texture );

   int spacingWidth = pFontMap->GetSpacingWidth ( );
   int spacingHeight= pFontMap->GetSpacingHeight( );

   const char *pCharacter = pString;

   pSize->x = 0;
   pSize->y = 0;

   while ( *pCharacter )
   {
      size.x = pFontMap->GetCharacter( *pCharacter - 32 )->width  * pTexture->GetWidth( ) + spacingWidth;
      size.y = pFontMap->GetCharacter( *pCharacter - 32 )->height * pTexture->GetHeight( )+ spacingHeight;
      
      Math::Max( pSize, size, *pSize );
      
      ++pCharacter;
   }
}

uint32 Font::Draw(
   float startX,
   float startY,
   const char *pString,
   Quad *pQuads,
   uint32 numQuads
) const
{
   //setup a series of quads with the correct
   //coordinates and UVs to render each letter
   //in the string

   uint32 i;

   if ( false == IsResourceLoaded(m_FontMap) ) return 0;
   if ( false == IsResourceLoaded(m_Texture) ) return 0;

   FontMap *pFontMap = GetResource( m_FontMap, FontMap );
   Texture *pTexture = GetResource( m_Texture, Texture );
   
   int spacingWidth = pFontMap->GetSpacingWidth ( );

   size_t length = strlen(pString);

   length = Math::Min( (int) length, (int) numQuads );

   float x = startX, y = startY;

   //because fonts use point sampling
   //force uv coordinates to round down to the pixel instead of choosing nearest.
   //this more closely matches bi-linear results and vastly decreases the 'wavy' appearance of small characters in a line
   float halfStepX = 0;
   float halfStepY = - 0.5f / pTexture->GetHeight( );

   for ( i = 0; i < length; i++ )
   {
      FontMap::CharacterDesc *pDesc = pFontMap->GetCharacter( pString[i] - 32 );

      float charWidth  = pDesc->width  * pTexture->GetWidth( );
      float charHeight = pDesc->height * pTexture->GetHeight( );

      y = startY - (charHeight);

      Vector uvs(pDesc->x + halfStepX, pDesc->y + halfStepY, pDesc->x + pDesc->width + halfStepX, pDesc->y + pDesc->height + halfStepY);
          
      pQuads[ i ].x = x + (charWidth  / 2);
      pQuads[ i ].y = y + (charHeight / 2);
      pQuads[ i ].width  = charWidth;
      pQuads[ i ].height = charHeight;

      pQuads[ i ].uvs = uvs;

      x += charWidth + spacingWidth;
   }

   return (uint32) length;
}
