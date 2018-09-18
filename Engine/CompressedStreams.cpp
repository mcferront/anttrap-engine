#include "EnginePch.h"

#include "CompressedStreams.h"
#include "MemoryManager.h"

typedef voidpf (*alloc_func) OF((voidpf opaque, uInt items, uInt size));
typedef void   (*free_func)  OF((voidpf opaque, voidpf address));

voidpf zLibAlloc(voidpf opaque, uInt items, uInt size)
{
   MemoryHeap *pHeap = (MemoryHeap *) opaque;

   #ifdef ENABLE_DEBUGMEMORY
      return pHeap->Alloc( size * items, __FILE__, __LINE__ );
   #else
      return pHeap->Alloc( size * items );   
   #endif
}

void zLibFree(voidpf opaque, voidpf address)
{
   MemoryHeap *pHeap = (MemoryHeap *) opaque;

   #ifdef ENABLE_DEBUGMEMORY
      return pHeap->Free( address, __FILE__, __LINE__ );
   #else
      return pHeap->Free( address );   
   #endif
}

CompressedInputStream::CompressedInputStream(
   IInputStream *pRawStream,
   uint32 upperLimit //= UINT_MAX
)
{
   m_pRawStream = NULL;

   m_Buffer.Create( 32 * 1024 );

   Bind( pRawStream, upperLimit );
}

void CompressedInputStream::Bind(
   IInputStream *pRawStream,
   uint32 upperLimit //= UINT_MAX
)
{
   CleanupInflate( );

   m_TotalRead  = 0;
   m_pRawStream = pRawStream;
   m_UpperLimit = upperLimit;

   m_Buffer.Reset( );

   if ( NULL != m_pRawStream )
   {
      memset( &m_Stream, 0, sizeof(m_Stream) );

      m_Stream.zalloc  = zLibAlloc;
      m_Stream.zfree   = zLibFree;
      m_Stream.opaque  = MemoryManager::Instance( ).GetHeap("System");
      m_Stream.avail_in= 0;
      m_Stream.next_in = Z_NULL;

      int ret = inflateInit( &m_Stream );
      Debug::Assert( Condition(Z_OK == ret), "inflateInit failed: %d", ret );
   }
}

CompressedInputStream::~CompressedInputStream( void )
{
   m_Buffer.Destroy( );

   CleanupInflate( );
}

void CompressedInputStream::Read(
   void *pBuffer,
   uint32 size,
   uint32 *pBytesRead// = NULL
)
{
   BYTE *pDest = (BYTE *) pBuffer;

   uint32 totalBytesRead;

   //see if we have enough in our uncompressed overflow buffer
   //to just read out of that
   m_Buffer.Read( pDest, size, &totalBytesRead );

   pDest += totalBytesRead;

   //if we didn't have enough in our uncompressed overflow buffer
   if ( size - totalBytesRead > 0 )
   {
      BYTE bufferIn [ 64 * 1024 ];
      BYTE bufferOut[ 64 * 1024 ];

      uint32 bytesRead;
      
      //we have no idea how much compressed data to read
      //so we'll just continue reading until 'size' has been fullfilled
      uint32 readCompressedAmount = m_UpperLimit - m_TotalRead;

      //break read up into blocks and any remaining
      int blocks    = readCompressedAmount / sizeof(bufferIn);
      int remainder = readCompressedAmount % sizeof(bufferIn);
   
      int i;

      for ( i = 0; i < blocks; i++ )
      {
         uint32 amountIn;

         //read compressed data
         m_pRawStream->Read( bufferIn, sizeof(bufferIn), &amountIn );         
         m_TotalRead += amountIn;

         //set our z info
         m_Stream.avail_in  = amountIn;
         m_Stream.next_in   = (Bytef *) bufferIn;

         do
         {
            m_Stream.avail_out = sizeof(bufferOut);
            m_Stream.next_out  = (Bytef *) bufferOut;

            int ret = inflate( &m_Stream, Z_NO_FLUSH );
            Debug::Assert( Condition(Z_OK == ret || Z_STREAM_END == ret), "inflate failed: %d", ret );      

            //write the uncompressed to our uncompressed buffer
            //we can't write directly to the pBuffer because
            //it might need to uncompress more than the requested amount
            m_Buffer.Write( bufferOut, sizeof(bufferOut) - m_Stream.avail_out );

         } while ( 0 == m_Stream.avail_out );

         //read the uncompressed data, from our uncompressed buffer
         //and it will hold any overflow for the next read
         m_Buffer.Read( pDest, size - totalBytesRead, &bytesRead );

         pDest += bytesRead;
         totalBytesRead += bytesRead;

         if ( totalBytesRead == size ) break;
      }

      //any left over? then duplicate the functionality above
      if ( size - totalBytesRead > 0 && remainder > 0 )
      {
         uint32 amountIn;

         m_pRawStream->Read( bufferIn, remainder, &amountIn );
         m_TotalRead += amountIn;

         m_Stream.avail_in  = amountIn;
         m_Stream.next_in   = (Bytef *) bufferIn;
      
         do
         {
            m_Stream.avail_out = sizeof(bufferOut);
            m_Stream.next_out  = (Bytef *) bufferOut;

            int ret = inflate( &m_Stream, Z_NO_FLUSH );
            Debug::Assert( Condition(Z_OK == ret || Z_STREAM_END == ret), "inflate failed: %d", ret );      

            m_Buffer.Write( bufferOut, sizeof(bufferOut) - m_Stream.avail_out );

         } while ( 0 == m_Stream.avail_out );

         //read the uncompressed data, from our uncompressed buffer
         //and it will hold any overflow for the next read
         m_Buffer.Read( pDest, size - totalBytesRead, &bytesRead );

         pDest += bytesRead;
         totalBytesRead += bytesRead;
      }   
   }

   if ( NULL != pBytesRead ) *pBytesRead = totalBytesRead;
}
   
uint32 CompressedInputStream::Seek(
   int amount,
   SeekOrigin origin
)
{
   Debug::Assert( Condition(false), "Seek not implemented for compressed streams" );
   return 0;
}

void CompressedInputStream::CleanupInflate( void )
{   
   if ( NULL != m_pRawStream )
   {
      int ret = inflateEnd( &m_Stream );
      Debug::Assert( Condition(Z_OK == ret), "inflateEnd failed: %d", ret );
   }
}
