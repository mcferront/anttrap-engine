#import "MacObjCVideoPlayer.h"

#import <QuickTime/QuickTime.h>
#import <QTKit/QTKit.h>

@interface QTRefs : NSObject 
{
@public
   CVOpenGLTextureRef currentFrame;
   QTVisualContextRef contextRef;
}

@end

@implementation QTRefs

@end

@implementation ObjCVideoPlayer

static CGLContextObj s_GlContext;
static CGLPixelFormatObj s_PixelFormat;

+ (void) GLContext:(CGLContextObj) context
{
   s_GlContext = context;
}

+ (void) PixelFormat:(CGLPixelFormatObj) pixelFormat
{
   s_PixelFormat = pixelFormat;
}

-(void) load:(const char *)pPath offset:(unsigned int) offset size:(unsigned int) size movieEnd:(MovieEnd) callback param:(unsigned long) userParam
{
   NSString *filename = [NSString stringWithCString:pPath encoding:NSASCIIStringEncoding];
 
   NSDictionary *attributes  = [NSDictionary dictionaryWithObjectsAndKeys:
      filename, QTMovieFileNameAttribute,
      [NSNumber numberWithLongLong:offset], QTMovieFileOffsetAttribute,
      nil
   ];


   NSError *nsError;

   m_movie = [[QTMovie alloc] initWithAttributes:attributes error:&nsError];      
   m_refs  = [[QTRefs alloc] init];

  
   // Create our quicktimee visual context
   OSStatus error;
   
   error = QTOpenGLTextureContextCreate(NULL,
                                        s_GlContext,
                                        s_PixelFormat,
                                        NULL,
                                        &m_refs->contextRef);
  
   //Associate it with our movie.
   SetMovieVisualContext( [m_movie quickTimeMovie], m_refs->contextRef );
 
   NSNotificationCenter *notificationCenter = [NSNotificationCenter defaultCenter];

   [notificationCenter addObserver:self
                selector:@selector(movieEnd:)
                name:QTMovieDidEndNotification
                object:m_movie];
                
   m_callback = callback;
   m_userParam= userParam;
}

- (void)movieEnd:(NSNotification*)notification
{
   m_callback( m_userParam );
}

- (void) unload
{
   [[NSNotificationCenter defaultCenter] removeObserver:self];
   
   [m_movie release];
}

- (void) play
{
   [m_movie play];
}

- (void) stop
{
   [m_movie stop];
}

- (void) seek:(float)seconds
{
   QTTime time = QTMakeTime( (int) (seconds * 1000.0f), 1000 );
   
   if ( NSOrderedDescending == QTTimeCompare(time, m_movie.duration) )
   {
      [m_movie gotoEnd];
   }
   else
   {
      [m_movie setCurrentTime:time];
   }
}

-(float) getTime
{
   TimeRecord currentTime;
   
   QTGetTimeRecord( [m_movie currentTime], &currentTime );

   float seconds;
   
   seconds  = currentTime.value.hi / (float) currentTime.scale;
   seconds += currentTime.value.lo / (float) currentTime.scale;

   return seconds;
}

-(void) getImage:(GLuint *) pTarget texture: (GLuint *) pTexture 
{
    GLfloat                 lowerLeft[2];
    GLfloat                 lowerRight[2];
    GLfloat                 upperRight[2];
    GLfloat                 upperLeft[2];

   QTVisualContextTask( m_refs->contextRef );

   // Check to see if a new frame (image) is ready to be draw at
   // the time specified.
   if( QTVisualContextIsNewImageAvailable(m_refs->contextRef, NULL) )
   {
      // Release the previous frame
      CVOpenGLTextureRelease( m_refs->currentFrame );
      
      // Copy the current frame into our image buffer
      QTVisualContextCopyImageForTime( m_refs->contextRef,
                                      NULL,
                                      NULL,
                                      &m_refs->currentFrame );
      
      // Returns the texture coordinates for the 
      // part of the image that should be displayed

      //we really return these for the texture and shader
      //instead of them being hardcoded to 1024x768
      CVOpenGLTextureGetCleanTexCoords(m_refs->currentFrame, 
                                       lowerLeft, 
                                       lowerRight, 
                                       upperRight, 
                                       upperLeft);

   }

   *pTexture = CVOpenGLTextureGetName  ( m_refs->currentFrame );
   *pTarget  = CVOpenGLTextureGetTarget( m_refs->currentFrame );
}

@end

/*
 - (BOOL)findAtomInFile:(NSString*)moviePath atomType:(long)searchAtomType fileOffset:(unsigned int) offset
 atomOffset:(long*)atomOffsetPrt atomSize:(long*)atomSizePtr
 atomData:(NSData**)atomDataPtr
 {
    NSFileHandle *docFileHandle = [NSFileHandle
 fileHandleForReadingAtPath:moviePath];
    if(!docFileHandle) return NO;
 
    long    atomHeader[2];
    long    atomHeaderSize = sizeof(atomHeader);
    long    atomSize = 0L;
    OSType    atomType = 0L;
    BOOL    atomFound = NO;
 
   [docFileHandle seekToFileOffset:offset];

    while(!atomFound){
        NSAutoreleasePool    *pool = [[NSAutoreleasePool alloc] init];
 
        // We memorize the offset, before reading the atomHeader chunk
        unsigned long long    offsetInFile = [docFileHandle offsetInFile];
 
        // We read the header
        NSData *dataChunk = [docFileHandle
 readDataOfLength:atomHeaderSize];
        if([dataChunk length] != atomHeaderSize){    // We reached the end of file, so we give up
            [pool release];
            break;
        }
 
        [dataChunk getBytes:atomHeader length:atomHeaderSize];
        atomSize = EndianU32_BtoN(atomHeader[0]);
        atomType = EndianU32_BtoN(atomHeader[1]);
        //NSLog(@"atomType '%@'", OSTypeString(atomType));
 
        if(atomType == searchAtomType) // We have found an atom of the desidered type
        {
            // We return the atom data, if requested
            if(atomDataPtr != NULL)
            {
                // We go back at the beginning of the atom and read the whole atom using its atomSize
                [docFileHandle seekToFileOffset:offsetInFile];
                *atomDataPtr = [docFileHandle readDataOfLength:atomSize];
                if([*atomDataPtr length] != atomSize){    // We reached the end of file, so we give up
                    *atomDataPtr = [NSData data];        // We empty the data
                    [pool release];
                    break;
                }
            }
 
            // We return the atom offset, if requested
            if(atomOffsetPrt){
                *atomOffsetPrt = offsetInFile;
            }
 
            // We return the atom size, if requested
            if(atomSizePtr){
                *atomSizePtr = atomSize;
            }
 
            atomFound = true;
        }
        else{
            // We haven't found an atom of the desidered type, we retry
            [docFileHandle seekToFileOffset:offsetInFile + atomSize];
        }
 
        [pool release];
    }
 
 
    return atomFound;
 }
*/
