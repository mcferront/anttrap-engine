#pragma once

#include "EngineGlobal.h"
#include "IOStreams.h"
#include "HashTable.h"
#include "Resource.h"

class FileStream
{
public:
    static bool Copy(
        const char *pExisting,
        const char *pDestination,
        bool overwriteExisting
    );

    static bool Delete(
        const char *pFile
    );

    static void CreateDirectory(
        const char *pDirectory
    );

    static bool FileExists(
       const char *pFile
    );

    static void FindFiles(
        const char *pPath,
        bool recursive,
        IdList *pFiles
    );

    static uint64 GetFileSize(
        const char *pPath
    );
};

class FileInputStream : public IInputStream
{
private:
   FILE *m_pFile;

public:
   FileInputStream( void )
   {
      m_pFile = NULL;
   }

   ~FileInputStream( void )
   {
      Close( );
   }

   bool Open(
      const char *pPath
   );

   void Close( void );

   virtual void Read(
      void *pBuffer,
      uint32 size,
      uint32 *pBytesRead = NULL
   );

   virtual uint32 Seek(
      int amount,
      SeekOrigin origin
   );

#ifdef ANDROID
public:
   static void CreateDescriptors ( void );
   static void DestroyDescriptors( void );

   static void AddDescriptor(
      const char *pName, 
      int fd, 
      int offset, 
      int size
   );

private:
   struct FileDescriptor
   {
      int fd;
      int offset;
      int size;
      FILE *pFile;
      char name[256];
   };

   static HashTable<const char *, FileDescriptor *> m_Files;

   FileDescriptor *m_pDesc;
#endif
};

class FileOutputStream : public IOutputStream
{
private:
   FILE *m_pFile;

public:
   FileOutputStream( void )
   {
      m_pFile = NULL;
   }

   ~FileOutputStream( void )
   {
      Close( );
   }

   bool Open(
      const char *pPath
   );

   void Close( void );

   virtual void Write(
      const void *pBuffer,
      uint32 size,
      uint32 *pBytesWritten = NULL
   );

   virtual uint32 Seek(
      int amount,
      SeekOrigin origin
   );
};
