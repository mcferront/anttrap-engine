#pragma once

#include "EngineGlobal.h"
#include "Geometry.h"

class VertexBuffer
{
public:
   static const GLuint Positions = 0;
   static const GLuint Normals   = 1;
   static const GLuint Colors    = 2;
   static const GLuint UV0s      = 3;
   static const GLuint UV1s      = 4;
   static const GLuint BoneWeights = 5;
   static const GLuint BoneIndices = 6;

   struct Attrib
   {
      GLuint index;
      GLint size;
      GLint offset;
      GLenum type;
      GLboolean normalized;
   };

private:
   unsigned int m_VertexBuffer;
   unsigned int m_IndexBuffer;

   Attrib m_Attribs[8];
   uint32 m_Stride;
   uint32 m_NumAttribs;
   uint32 m_NumIndices;
   uint32 m_IndexType;

   mutable GLuint m_VertexArrayObject;

public:
   void Create ( void );
   void Destroy( void );

   void SetVertices(
      const void *pVertices,
      uint32 size
   );

   void SetIndices(
      const void *pIndices,
      uint32 size,
      uint32 type,
      uint32 count
   );

   void AddAttributes(
      int usage,
      StreamDecl decl
   );

   void BindData( void ) const;
   void Submit( void ) const;
};