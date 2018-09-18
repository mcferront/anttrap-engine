#include "EnginePch.h"

#include "VertexBuffer.h"
#include "GlShader.h"

void VertexBuffer::Create( void )
{
   memset( m_Attribs, 0, sizeof(m_Attribs) );

   m_NumAttribs = 0;
   m_Stride = 0;
   m_VertexArrayObject = -1;
   m_IndexBuffer = -1;
}

void VertexBuffer::Destroy( void )
{
   glDeleteVertexArraysOES( 1, &m_VertexArrayObject );
   
   pglDeleteBuffers( 1, &m_VertexBuffer );
   pglDeleteBuffers( 1, &m_IndexBuffer );
}

void VertexBuffer::SetVertices(
   const void *pVertices,
   uint32 size
)
{
   glCheckError( "VertexBuffer::SetVertices Should be clear" );

   pglGenBuffers( 1, &m_VertexBuffer );
   glCheckError( "VertexBuffer::pglGenBuffers" );

   pglBindBuffer( GL_ARRAY_BUFFER, m_VertexBuffer );
   glCheckError( "VertexBuffer::pglBindBuffer" );

   pglBufferData( GL_ARRAY_BUFFER, size, pVertices, GL_STATIC_DRAW );
   glCheckError( "VertexBuffer::pglBindBuffer" );
}

void VertexBuffer::SetIndices(
   const void *pIndices,
   uint32 size,
   uint32 type,
   uint32 count
)
{
   glCheckError( "VertexBuffer::SetIndices Should be clear" );

   pglGenBuffers( 1, &m_IndexBuffer );
   glCheckError( "VertexBuffer::pglGenBuffers" );

   pglBindBuffer( GL_ELEMENT_ARRAY_BUFFER, m_IndexBuffer );
   glCheckError( "VertexBuffer::pglBindBuffer" );

   pglBufferData( GL_ELEMENT_ARRAY_BUFFER, size, pIndices, GL_STATIC_DRAW );
   glCheckError( "VertexBuffer::pglBufferData" );

   m_IndexType  = type == StreamDecl::UShort ? GL_UNSIGNED_SHORT : GL_UNSIGNED_INT;
   m_NumIndices = count;
}

void VertexBuffer::AddAttributes( 
   int usage,
   StreamDecl decl
)
{
   GLint stride = 0;
   GLenum type = 0;
   GLuint glIndex = 0;
   
   switch ( decl.dataType )
   {  
      case StreamDecl::Float  : stride = 4 * decl.numElements; type = GL_FLOAT; break;
      case StreamDecl::UShort : stride = 2 * decl.numElements; type = GL_UNSIGNED_SHORT; break;
      case StreamDecl::Byte   : stride = 1 * decl.numElements; type = GL_UNSIGNED_BYTE; break;
      case StreamDecl::Color  : stride = 1 * decl.numElements; type = GL_UNSIGNED_BYTE; break;
      case StreamDecl::UInt   : Debug::Assert(Condition(false), "UInt not supported for Vertex Buffer type"); break;
   }
   
   switch ( usage )
   {
      case StreamDecl::Positions   : glIndex = VertexBuffer::Positions; break;
      case StreamDecl::Normals     : glIndex = VertexBuffer::Normals; break;
      case StreamDecl::Colors      : glIndex = VertexBuffer::Colors; break;
      case StreamDecl::UV0s        : glIndex = VertexBuffer::UV0s; break;
      case StreamDecl::UV1s        : glIndex = VertexBuffer::UV1s; break;
      case StreamDecl::BoneWeights : glIndex = VertexBuffer::BoneWeights; break;
      case StreamDecl::BoneIndices : glIndex = VertexBuffer::BoneIndices; break;
   }

   Debug::Assert( Condition(m_NumAttribs < sizeof(m_Attribs) / sizeof(Attrib)), "Too many attributes for the vertex buffer %d", m_NumAttribs);

   m_Attribs[ m_NumAttribs ].size   = decl.numElements;
   m_Attribs[ m_NumAttribs ].index  = glIndex;
   m_Attribs[ m_NumAttribs ].type   = type;
   m_Attribs[ m_NumAttribs ].offset = decl.offset;
   m_Attribs[ m_NumAttribs ].normalized = StreamDecl::Color == decl.dataType ? 1 : 0;
   
   m_Stride += stride;
   
   ++m_NumAttribs;
}

void VertexBuffer::BindData( void ) const
{
    glGenVertexArraysOES( 1, &m_VertexArrayObject );
    glCheckError( "VertexBuffer::glGenVertexArraysOES" );

    glBindVertexArrayOES( m_VertexArrayObject );
    glCheckError( "VertexBuffer::glBindVertexArrayOES" );
        
    glBindBuffer( GL_ARRAY_BUFFER, m_VertexBuffer );
    glCheckError( "VertexBuffer::glBindBuffer" );
  
    if ( -1 != m_IndexBuffer )
    {
        pglBindBuffer( GL_ELEMENT_ARRAY_BUFFER, m_IndexBuffer );
        glCheckError( "VertexBuffer::glBindBuffer" );
    }

    int i;
    
    for ( i = 0; i < m_NumAttribs; i++ )
    {
        pglVertexAttribPointer( m_Attribs[ i ].index, m_Attribs[ i ].size, m_Attribs[ i ].type, m_Attribs[ i ].normalized, m_Stride, (void *) m_Attribs[ i ].offset );
        glCheckError( "VertexBuffer::pglVertexAttribPointer" );
   
        pglEnableVertexAttribArray( m_Attribs[ i ].index );
        glCheckError( "VertexBuffer::pglEnableVertexAttribArray" );
    }
}

void VertexBuffer::Submit( void ) const
{
   glCheckError( "VertexBuffer::Submit Should be clear" );
   
    if ( -1 == m_VertexArrayObject )
        BindData( );

    glBindVertexArrayOES( m_VertexArrayObject );
    glCheckError( "VertexBuffer::glBindVertexArrayOES" );
   
    glDrawElements( GL_TRIANGLES, m_NumIndices, m_IndexType, (void *) 0 );
    
    glCheckError( "VertexBuffer::glDrawElements" );
   
   glBindVertexArrayOES( 0 );
}
