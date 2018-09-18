#include "EnginePch.h"

#include "TextArea.h"
#include "ResourceWorld.h"
#include "MaterialObject.h"
#include "Quad.h"
#include "Viewport.h"
#include "Localization.h"
#include "Node.h"
#include "RenderWorld.h"

void TextArea::Create(
   Component *pComponent,
   const Vector2 &size,
   const Vector &color,
   ResourceHandle back_material,
   ResourceHandle front_material,
   ResourceHandle font_map,
   ResourceHandle font_texture,
   const IdList &renderGroups
   )
{
   m_pComponent = pComponent;
   m_Align = AlignLeft | AlignCenter;
   m_Size = size;
   m_Color = color;
   
   int w = GpuDevice::Instance( ).GetSwapChain( )->width;
   int h = GpuDevice::Instance( ).GetSwapChain( )->height;

   m_ReferenceSize.x = (float) w;
   m_ReferenceSize.y = (float) h;

   m_ActualSize = Math::ZeroVector2( );
   m_ActualPosition = Math::ZeroVector2( );

   m_NumQuads = 0;
   m_pQuads = NULL;
   
   m_Padding = Math::ZeroVector2( );

   m_Strings.Create( );

   m_LocalizedBuffer.pString = NULL;
   m_LocalizedBuffer.maxLength = 0;

   m_ActiveQuadCount = 0;
   m_MaxLines = 1024;

   m_RenderGroups.Create( );
   m_RenderGroups.CopyFrom( renderGroups );

   m_pFrontMaterial = new GraphicsMaterialObject( front_material );
   m_pBackMaterial = new GraphicsMaterialObject( back_material );

   m_Font.Create( font_texture, font_map );
}

void TextArea::Destroy( void )
{
   free( m_LocalizedBuffer.pString );

   m_LocalizedBuffer.pString = 0;
   m_LocalizedBuffer.maxLength = 0;

   delete m_pFrontMaterial;
   delete m_pBackMaterial;

   Clear( );

   m_Font.Destroy( );

   m_Strings.Destroy( );

   free( m_pQuads );

   m_pQuads = NULL;
   m_NumQuads = 0;

   m_RenderGroups.Destroy( );
}

void TextArea::SetBackground(
   ResourceHandle material
)
{
   delete m_pBackMaterial;

   m_pBackMaterial = new GraphicsMaterialObject( material );
}

void TextArea::Flush( void )
{
   uint32 i, size = m_Strings.GetSize( );
   m_ActiveQuadCount = 0;

   // TODO: these 'actualPosition, actualSize, referenceSize' variables
   // shouldn't be here.  it should be computed at render time
   // based on virtual (0-1) coordinates
   {
      Transform transform;
      m_pComponent->GetParent( )->GetWorldTransform( &transform );

      Vector position;
      transform.GetTranslation( &position );

      int w = GpuDevice::Instance( ).GetSwapChain( )->width;
      int h = GpuDevice::Instance( ).GetSwapChain( )->height;

      m_ActualPosition.x = position.x / m_ReferenceSize.x * w;
      m_ActualPosition.y = position.y / m_ReferenceSize.y * h;

      m_ActualSize.x = round(m_Size.x / m_ReferenceSize.x * (float) w);
      m_ActualSize.y = round(m_Size.y / m_ReferenceSize.y * (float) h);
   }


   //size of biggest character, this is our
   //base for the y increment
   Vector2 maxCharacterSize;
   m_Font.GetMaxCharacterSize( "@", &maxCharacterSize );

   float yLine = 0.0f;

   for ( i = 0; i < size; i++ )
   {
      uint32 currentCount = 0;

      TextString *pString = m_Strings.GetPointer( i );

      //make sure we have enough quads to cover
      //the next string
      size_t length = strlen( pString->pString );

      if ( m_ActiveQuadCount + length > m_NumQuads )
      {
         m_NumQuads += ( m_ActiveQuadCount + length ) * 2;
         m_pQuads = (Quad *) realloc( m_pQuads, m_NumQuads * sizeof( Quad ) );
      }

      uint32 count;

      do
      {
         uint32 quadsUsed;

         //draw as many characters as possible on this line
         count = DrawLine( pString->pString + currentCount, yLine, m_pQuads + m_ActiveQuadCount, m_NumQuads - m_ActiveQuadCount, &quadsUsed );

         currentCount += count;
         m_ActiveQuadCount += quadsUsed;

         //if any characters were drawn, or if the line was blank
         //move us down to the next line
         if ( count || 0 == currentCount )
         {
            yLine -= maxCharacterSize.y;
         }

      } while ( count );
   }

   if ( AlignTop & m_Align )
   {
      //if aligned to the top, move all the quads
      //to the top of our transform
      uint32 i;

      float deltaY = m_ActualSize.y / 2.0f;

      for ( i = 0; i < m_ActiveQuadCount; i++ )
      {
         m_pQuads[ i ].y += deltaY;
         //push away from the top
         m_pQuads[ i ].y -= m_Padding.y;
      }
   }
   else if ( AlignMiddle & m_Align )
   {
      //if aligned to the center, figure out
      //how much to move the quads up
      //so the total height is aligned properly
      uint32 i;

      float deltaY = -yLine / 2.0f;

      for ( i = 0; i < m_ActiveQuadCount; i++ )
      {
         m_pQuads[ i ].y += deltaY;
      }
   }
   else if ( AlignBottom & m_Align )
   {
      //if aligned to the bottom figure out how
      //much space is left to the bottom and
      //pull the quads down theres
      uint32 i;

      float deltaY = -( m_ActualSize.y / 2.0f + yLine );

      for ( i = 0; i < m_ActiveQuadCount; i++ )
      {
         m_pQuads[ i ].y += deltaY;
         //push away from the bottom
         m_pQuads[ i ].y += m_Padding.y;
      }
   }

   uint32 validQuads = 0;

   //clamp to the top and bottom - don't render quads which falls outside of this box + padding area
   //we don't worry about left/right because they'll always be moved to the next line down
   //we should replace this w/ a mask or scissor routine when we have that functionality
   Vector2 halfSize = m_ActualSize * .5f;

   for ( i = 0; i < m_ActiveQuadCount; i++ )
   {
      float height = m_pQuads[ i ].height / 2.0f;

      if ( m_pQuads[ i ].y - height < -halfSize.y + m_Padding.y || m_pQuads[ i ].y + height > halfSize.y - m_Padding.y )
      {
         continue;
      }

      m_pQuads[ validQuads++ ] = m_pQuads[ i ];
   }

   m_ActiveQuadCount = validQuads;
}

void TextArea::GetRenderData(
   RendererDesc *pDesc
) const
{
   Transform transform;
   m_pComponent->GetParent( )->GetWorldTransform( &transform );
   transform.SetTranslation( m_ActualPosition );

   int selected = m_pComponent->GetParent( )->IsSelected( );

   if ( m_pFrontMaterial->Prepare() && m_pFrontMaterial->HasPass(pDesc->pPass) )
   {
      RenderObjectDesc *pRODesc = RenderWorld::Instance( ).AllocRenderObjectDesc( );
      pRODesc->renderContext = m_pFrontMaterial->GetRenderContext( pDesc->pPass, pDesc->viewport.GetContext(), Quad::GetVertexContext( ) );
      pRODesc->pMaterial = m_pFrontMaterial;
      pRODesc->pObject = this;
      pRODesc->renderFunc = TextArea::Render;
      pRODesc->argList = ArgList( transform, m_Color, m_ActualSize, m_pBackMaterial, selected, m_ActiveQuadCount );
      pRODesc->buffer.Write( m_pQuads, sizeof( Quad ) * m_ActiveQuadCount );

      pDesc->renderObjectDescs.Add( pRODesc );
   }

   if ( m_pBackMaterial->Prepare( ) && m_pBackMaterial->HasPass(pDesc->pPass) )
   {
      RenderObjectDesc *pRODesc = RenderWorld::Instance( ).AllocRenderObjectDesc( );
      pRODesc->renderContext = m_pBackMaterial->GetRenderContext( pDesc->pPass, pDesc->viewport.GetContext(), Quad::GetVertexContext( ) );
      pRODesc->pMaterial = m_pBackMaterial;
      pRODesc->pObject = this;
      pRODesc->renderFunc = TextArea::RenderBacker;
      pRODesc->argList = ArgList( transform, m_ActualSize, selected );

      pDesc->renderObjectDescs.Add( pRODesc );
   }
}

void TextArea::PrintArgs(
   const char *pString,
   ...
)
{
   va_list list;

   va_start( list, pString );

   PrintV( pString, list );

   va_end( list );
}

void TextArea::PrintV(
   const char *pString,
   va_list args
)
{
   //add string to our lists of
   //strings to render this frame
   char string[ 256 ];

   String::FormatV( string, sizeof( string ), pString, args );

   Print( string );
}

void TextArea::Print(
   const char *pString
)
{
   size_t required = Localization::Instance( ).GetString( NULL, pString, 0 ) + 1;

   if ( m_LocalizedBuffer.maxLength < required )
   {
      m_LocalizedBuffer.maxLength = required;
      m_LocalizedBuffer.pString = (char *) realloc( m_LocalizedBuffer.pString, m_LocalizedBuffer.maxLength );
   }

   Localization::Instance( ).GetString( m_LocalizedBuffer.pString, pString, m_LocalizedBuffer.maxLength );

   char *pStart = m_LocalizedBuffer.pString;
   char *pNewline = strchr( pStart, '\n' );

   TextString text;

   while ( pNewline )
   {
      *pNewline = NULL;

      text.maxLength = ( pNewline - pStart ) + 1;
      text.pString = (char *) malloc( text.maxLength + 1 );

      String::Copy( text.pString, pStart, text.maxLength );
      m_Strings.Add( text );

      pStart = pNewline + 1;
      pNewline = strchr( pStart, '\n' );
   }

   if ( *pStart != 0 )
   {
      text.maxLength = strlen( pStart ) + 1;
      text.pString = (char *) malloc( text.maxLength + 1 );

      String::Copy( text.pString, pStart, text.maxLength );

      m_Strings.Add( text );
   }

   if ( m_Strings.GetSize( ) > m_MaxLines )
   {
      free( m_Strings.GetAt( 0 ).pString );
      m_Strings.RemoveSorted( 0 );
   }
}

void TextArea::Pad(
   float vertical,
   float horizontal
)
{
   m_Padding.y = vertical;
   m_Padding.x = horizontal;
}

void TextArea::Clear( void )
{
   {
      List<TextString>::Enumerator e = m_Strings.GetEnumerator( );

      while ( e.EnumNext( ) )
      {
         free( e.Data( ).pString );
      }
   }

   m_Strings.Clear( );
}

bool TextArea::IsVisible(
   const Frustum &frustum
) const
{
   Transform t = m_pComponent->GetParent( )->GetWorldTransform( );
   t.SetTranslation( m_ActualPosition );

   return BoxInFrustum( Math::ZeroVector( ), m_ActualSize * .5f, t, frustum );
}

float TextArea::AlignXPosition(
   const char *pString,
   size_t maxCharacters
)
{
   //return x position which takes into account
   //string width and alignment

   float stringWidth = m_Font.GetStringWidth( pString, maxCharacters );

   float x = 0.0f;

   //we have a string width which fits our line
   //so now we just figure out where to make it start
   if ( AlignCenter & m_Align )
   {
      //first letter starts half line width to the left
      x = -stringWidth / 2.0f;

      //if we violated the padding then push back out
      //to the padding amount
      float leftBorder = -m_ActualSize.x / 2.0f + m_Padding.x;
      if ( x < leftBorder ) x = leftBorder;
   }
   else if ( AlignLeft & m_Align )
   {
      //first letter starts all the way on the
      //left of the text area
      x = -m_ActualSize.x / 2.0f;

      //push away from the left border
      x += m_Padding.x;
   }
   else if ( AlignRight & m_Align )
   {
      //first letter starts the line width from the
      //right of the text area
      x = m_ActualSize.x / 2.0f - stringWidth;

      //push away from the right border
      x -= m_Padding.x;

      //if too close to the left border move back in towards center
      float leftBorder = -m_ActualSize.x / 2.0f + m_Padding.x;
      if ( x < leftBorder ) x = leftBorder;
   }

   return x;
}

uint32 TextArea::FitStringToLine(
   const char *pString,
   uint32 maxCharacters
)
{
   //return amount of characters before
   //an embedded or forced line break

   uint32 length = (uint32) strlen( pString );

   if ( length <= maxCharacters ) return length;

   const char *pBreak = FindLineBreak( pString + 1 );

   //couldn't find a line break, they'll have to
   //break in the middle of a string
   if ( NULL == pBreak ) return maxCharacters;

   //we are going to break
   //on this one, so we include it in the length
   ++pBreak;

   //couldn't find a line break within the character count
   if ( (uint32) ( pBreak - pString ) > maxCharacters ) return maxCharacters;

   while ( 1 )
   {
      const char *pNextBreak = FindLineBreak( pBreak + 1 );

      //if there is no next break, return
      //the last one found
      if ( NULL == pNextBreak ) return (uint32) ( pBreak - pString );

      //we are going to break
      //on this one, so we include it in the length
      ++pNextBreak;

      //if the next break we found is too far,
      //return the previous break
      if ( (uint32) ( pNextBreak - pString ) > maxCharacters ) return (uint32) ( pBreak - pString );

      pBreak = pNextBreak;
   }

   //never reached
   return NULL;
}

const char *TextArea::FindLineBreak(
   const char *pString
)
{
   const char *pSpace = strchr( pString, ' ' );
   const char *pNewLine = strchr( pString, '\n' );

   //no line break
   if ( NULL == pSpace && NULL == pNewLine ) return NULL;

   //pSpace is null - so return pNewLine
   //it will either be NULL also or a valid line break
   if ( NULL == pSpace )  return pNewLine;
   //space is valid but newline is null - so use space as the line break
   if ( NULL == pNewLine ) return pSpace;

   //both a space and newline exist, return the one closest
   //to the the start of the string
   if ( pSpace - pString < pNewLine - pString ) return pSpace;

   return pNewLine;
}

uint32 TextArea::DrawLine(
   const char *pString,
   float yLine,
   Quad *pQuads,
   uint32 quadCount,
   uint32 *pQuadsUsed
)
{
   //render as many characters as possible
   //to this line inside the textbox width 

   *pQuadsUsed = 0;

   const char *pStart = pString;

   //if we're not at the beginning of the string
   //(but we are at the beginning of a line because of a forced line break)
   //then skip any starting space
   if ( *pString == ' ' )
   {
      ++pString;
   }

   //figure out how many characters we can fit on this line
   //before we hit a line break or the edge of the text box
   //size is subtraced by padding on the left and the right
   uint32 charactersFit = m_Font.GetCharacterCount( pString, m_ActualSize.x - ( m_Padding.x + m_Padding.x ) );
   if ( 0 == charactersFit ) return 0;

   charactersFit = FitStringToLine( pString, charactersFit );

   //if the line break ends in a space, don't bother rendering it or using it for
   //the alignment
   bool skipEndingSpace = ' ' == pString[ charactersFit - 1 ];
   if ( true == skipEndingSpace ) --charactersFit;

   float startX = AlignXPosition( pString, charactersFit );

   Debug::Assert( Condition( quadCount >= charactersFit ), "Not enough quads available to draw the string" );

   *pQuadsUsed = m_Font.Draw( startX, yLine, pString, pQuads, charactersFit );

   //return total amount of characters rendered or skipped
   return charactersFit + (uint32) ( pString - pStart );
}

void TextArea::RenderBacker(
   const RenderDesc &desc,
   RenderStats *pStats
   )
{
   Transform transform;
   Vector2 areaSize;
   Quad quad;
   int selected;

   desc.pDesc->argList.GetArg( 0, &transform );
   desc.pDesc->argList.GetArg( 1, &areaSize );
   desc.pDesc->argList.GetArg( 2, &selected );

   Vector uv = GetMaterialUV( desc.pDesc->pMaterial );

   quad.Create( areaSize.x, areaSize.y, uv.x, uv.y );
   quad.Render( selected != 0, Vector( 1, 1, 1, 1 ), transform, desc, pStats );
}

void TextArea::Render(
   const RenderDesc &desc,
   RenderStats *pStats
)
{
   Transform transform;
   Vector color;
   Vector2 areaSize;
   GraphicsMaterialObject *pBackMaterial;
   uint32 quadCount;
   int selected;

   desc.pDesc->argList.GetArg( 0, &transform );
   desc.pDesc->argList.GetArg( 1, &color );
   desc.pDesc->argList.GetArg( 2, &areaSize );
   desc.pDesc->argList.GetArg( 3, &pBackMaterial );
   desc.pDesc->argList.GetArg( 4, &selected );
   desc.pDesc->argList.GetArg( 5, &quadCount );

   Quad *pQuads = (Quad *) desc.pDesc->buffer.Read( 0 );
   Quad::Render( pQuads, quadCount, selected != 0, color, transform, desc, pStats );
}
