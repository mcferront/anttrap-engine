#include "EnginePch.h"

#include "FileStreams.h"
#include "List.h"

HashTable<const char *, FileInputStream::FileDescriptor *> FileInputStream::m_Files;

void FileInputStream::CreateDescriptors ( void )
{
   m_Files.Create( );
}

void FileInputStream::DestroyDescriptors( void )
{
   List<FileDescriptor *> files;

   files.Create( );

   {
      Enumerator<const char *, FileDescriptor *> e = m_Files.GetEnumerator( );

      while ( e.EnumNext() )
      {
         files.Add( e.Data() );
      }
   }

   m_Files.Destroy( );

   {
      List<FileDescriptor *>::Enumerator e = files.GetEnumerator( );

      while ( e.EnumNext() )
      {
         delete e.Data();
      }
   }

   files.Destroy( );
}

void FileInputStream::AddDescriptor(
   const char *pName, 
   int fd, 
   int offset, 
   int size
)
{
   FileDescriptor *pDesc = new FileDescriptor( );

   String::Copy( pDesc->name, pName, sizeof(pDesc->name) );

   pDesc->fd     = fd;
   pDesc->offset = offset;
   pDesc->size   = size;
   pDesc->pFile  = fdopen( pDesc->fd, "rb" );

   m_Files.Remove( pDesc->name );
   m_Files.Add( pDesc->name, pDesc );
}

bool FileInputStream::Open(
   const char *pPath
)
{
   if ( true == m_Files.Get(pPath, &m_pDesc) )
   {
      m_pFile = m_pDesc->pFile;
      fseek( m_pFile, m_pDesc->offset, SEEK_SET );
   }
   else
   {
      m_pDesc = NULL;
      m_pFile = fopen( pPath, "rb" );
   }
   
   return NULL != m_pFile;
}

void FileInputStream::Close( void )
{
   if ( NULL == m_pDesc && m_pFile ) fclose( m_pFile );

   m_pFile = NULL;
}

void FileInputStream::Read(
   void *pBuffer,
   uint32 size,
   uint32 *pBytesRead// = NULL
)
{
   size_t bytesRead = fread( pBuffer, 1, size, m_pFile );
   if ( pBytesRead ) *pBytesRead = (uint32) bytesRead;
}

uint32 FileInputStream::Seek(
   int amount,
   Origin origin
)
{
   int startOffset = m_pDesc ? m_pDesc->offset : 0;

   if ( SeekBegin == origin )
   {
      fseek( m_pFile, amount + startOffset, SEEK_SET );
   }
   else if ( SeekCurrent == origin ) 
   {
      fseek( m_pFile, amount, SEEK_CUR );
   }
   else                              
   {
      if ( NULL != m_pDesc )
      {
         fseek( m_pFile, m_pDesc->size + amount, SEEK_SET );
      }
      else
      {
         fseek( m_pFile, amount, SEEK_END );
      }
   }

   return (uint32) ftell( m_pFile ) - startOffset;
}

bool FileOutputStream::Open(
   const char *pPath
)
{
   m_pFile = fopen( pPath, "wb" );
   
   return NULL != m_pFile;
}

void FileOutputStream::Close( void )
{
   if ( m_pFile ) fclose( m_pFile );

   m_pFile = NULL;
}

void FileOutputStream::Write(
   const void *pBuffer,
   uint32 size,
   uint32 *pBytesWritten//= NULL
)
{
   size_t written = fwrite( pBuffer, 1, size, m_pFile );   
   if ( pBytesWritten ) *pBytesWritten = (uint32) written;
}

uint32 FileOutputStream::Seek(
   int amount,
   Origin origin
)
{
   int seek;

   if ( SeekBegin == origin )        seek = SEEK_SET;
   else if ( SeekCurrent == origin ) seek = SEEK_CUR;
   else                              seek = SEEK_END;

   fseek( m_pFile, amount, seek );

   return (uint32) ftell( m_pFile );
}
