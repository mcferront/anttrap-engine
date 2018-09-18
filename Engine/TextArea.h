#pragma once

#include "EngineGlobal.h"
#include "RenderObject.h"
#include "FontAsset.h"
#include "ResourceWorld.h"

class GraphicsMaterialObject;

class TextArea : public RenderObject
{
public:
   enum Align
   {
      AlignLeft = 1 << 0,
      AlignRight = 1 << 1,
      AlignCenter = 1 << 2,
      AlignTop = 1 << 3,
      AlignBottom = 1 << 4,
      AlignMiddle = 1 << 5,

      force32 = 0xffffffff,
   };

private:
   struct TextString
   {
      char *pString;
      size_t maxLength;
   };

private:
   List<TextString> m_Strings;

   Vector2 m_Size;
   Vector2 m_ReferenceSize;
   Vector2 m_ActualSize;
   Vector2 m_ActualPosition;
   Font    m_Font;
   uint32  m_Align;
   uint32  m_NumQuads;
   Vector  m_Color;
   Vector2 m_Padding;
   uint32  m_MaxLines;
   uint32  m_ActiveQuadCount;
   IdList  m_RenderGroups;
   Quad  *m_pQuads;
   Component *m_pComponent;

   GraphicsMaterialObject *m_pFrontMaterial;
   GraphicsMaterialObject *m_pBackMaterial;

   TextString m_LocalizedBuffer;

public:
   void Create(
      Component *pComponent,
      const Vector2 &size,
      const Vector &color,
      ResourceHandle back_material,
      ResourceHandle front_material,
      ResourceHandle font_map,
      ResourceHandle font_texture,
      const IdList &renderGroups
   );

   void Destroy( void );

   void SetBackground(
      ResourceHandle material
   );

   virtual void Flush( void );

   virtual void GetRenderData(
      RendererDesc *pDesc
   ) const;

   void PrintArgs(
      const char *pString,
      ...
   );

   void PrintV(
      const char *pString,
      va_list args
   );

   void Print(
      const char *pString
   );

   void Pad(
      float vertical,
      float horizontal
   );

   void Clear( void );

   bool IsVisible(
      const Frustum &frustum
   ) const;

   const Vector2 &GetSize( void ) const { return m_Size; }

   void SetSize( const Vector2 &size ) { m_Size = size; }

   float GetAlpha( void ) const { return m_Color.w; }
   void SetAlpha( float alpha ) { m_Color.w = alpha; }

   const Vector &GetColor( void ) const { return m_Color; }

   void SetColor( const Vector &color )
   {
      float a = m_Color.w;
      m_Color = color;
      m_Color.w = a;
   }

   void SetAlign( Align horiz_align, Align vert_align ) { m_Align = horiz_align | vert_align; }

   void SetMaxLines(
      uint32 maxLines
   )
   {
      m_MaxLines = maxLines;
   }

   void SetRenderGroups(
      const IdList &renderGroups
   )
   {
      m_RenderGroups.Clear( );
      m_RenderGroups.CopyFrom( renderGroups );
   }

   virtual bool NeedsFlush( void ) const { return true; }

   virtual int GetRenderType( void ) const { return RenderObject::Type::Text; }

   virtual void GetRenderGroups(
      IdList *pGroups
   ) const
   {
      pGroups->CopyFrom( m_RenderGroups );
   }

   virtual void GetWorldTransform(
      Transform *pWorldTransform
   ) const
   {
      m_pComponent->GetParent( )->GetWorldTransform( pWorldTransform );
   }

private:

   float AlignXPosition(
      const char *pString,
      size_t maxCharacters
   );

   uint32 FitStringToLine(
      const char *pString,
      uint32 maxCharacters
   );

   const char *FindLineBreak(
      const char *pString
   );

   uint32 DrawLine(
      const char *pString,
      float yLine,
      Quad *pQuads,
      uint32 quadCount,
      uint32 *pQuadsUsed
   );

private:
   static void RenderBacker(
      const RenderDesc &desc,
      RenderStats *pStats
   );

   static void Render(
      const RenderDesc &desc,
      RenderStats *pStats
   );
};

