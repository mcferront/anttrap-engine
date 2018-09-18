#include "EnginePch.h"

#include "FileStreams.h"

#ifdef ANDROID
   #error AndroidFileStreams.cpp should be compiled instead of FileStreams.cpp
#endif

bool FileStream::Copy(
    const char *pExisting,
    const char *pDestination,
    bool overwriteExisting
)
{
#ifdef WIN32
    return TRUE == CopyFile( pExisting, pDestination, false == overwriteExisting );
#elif defined IOS
    Debug::Assert( Condition(false), "Not Implemented" );
    return false;
#else
    #error Not Implemented
#endif
}


#ifdef WIN32
bool FileStream::Delete(
    const char *pFile
)
{
    return TRUE == DeleteFile( pFile );
}

void FileStream::CreateDirectory(
    const char *pDirectory
)
{
    char path[ 260 ];
    String::Copy( path, pDirectory, sizeof(path) );

    char *pSlash = strchr( path, '/' );

    while ( pSlash )
    {
        *pSlash = 0;
       
      _mkdir(path);

      *pSlash = '/';
       pSlash = strchr( pSlash + 1, '/' );
    }

   _mkdir(path);
}

bool FileStream::FileExists(
   const char *pFile
)
{
   DWORD result = GetFileAttributes( pFile );
   
   return INVALID_FILE_ATTRIBUTES != result || 
            GetLastError() != ERROR_FILE_NOT_FOUND;
}

void FileStream::FindFiles(
    const char *pPath,
    bool recursive,
    IdList *pFiles
)
{
    char fullPath[ MAX_PATH ];
    char subPath[ MAX_PATH ];

    String::Format( fullPath, sizeof(fullPath), "%s/*", pPath );

    WIN32_FIND_DATA data;
    HANDLE handle = FindFirstFile( fullPath, &data );

    if ( handle != INVALID_HANDLE_VALUE )
    {    
        while ( 1 )
        {
            if ( 0 == (data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) )
            {
                String::Format( subPath, sizeof(subPath), "%s/%s", pPath, data.cFileName );
                pFiles->Add( Id(subPath) );
            }
            else
            {
                if ( true == recursive )
                {
                    if ( 0 != strcmp(".", data.cFileName) &&
                         0 != strcmp("..", data.cFileName) )
                    {
                        String::Format( subPath, sizeof(subPath), "%s/%s", pPath, data.cFileName );
                        FindFiles( subPath, true, pFiles );
                    }
                }
            }

            BOOL result = FindNextFile( handle, &data );
            if ( FALSE == result) break;
        }

        FindClose( handle );
    }
}

uint64 FileStream::GetFileSize(
    const char *pPath
)
{
    WIN32_FILE_ATTRIBUTE_DATA attribData;

    GetFileAttributesEx( pPath, GetFileExInfoStandard, &attribData );

    return ((uint64) attribData.nFileSizeHigh << 32) | attribData.nFileSizeLow;
}

#endif

bool FileInputStream::Open(
   const char *pPath
)
{
   m_pFile = fopen( pPath, "rb" );
   
   return NULL != m_pFile;
}

void FileInputStream::Close( void )
{
   if ( m_pFile ) fclose( m_pFile );

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
   SeekOrigin origin
)
{
   int seek;

   if ( SeekBegin == origin )        seek = SEEK_SET;
   else if ( SeekCurrent == origin ) seek = SEEK_CUR;
   else                              seek = SEEK_END;

   fseek( m_pFile, amount, seek );

   return (uint32) ftell( m_pFile );
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
   SeekOrigin origin
)
{
   int seek;

   if ( SeekBegin == origin )        seek = SEEK_SET;
   else if ( SeekCurrent == origin ) seek = SEEK_CUR;
   else                              seek = SEEK_END;

   fseek( m_pFile, amount, seek );

   return (uint32) ftell( m_pFile );
}
