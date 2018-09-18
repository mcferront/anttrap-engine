
#import <Cocoa/Cocoa.h>

typedef void (*MovieEnd)( unsigned long );

@class QTMovie;
@class QTRefs;

@interface ObjCVideoPlayer : NSObject 
{
   QTMovie *m_movie;
   QTRefs  *m_refs;
   MovieEnd m_callback;
   unsigned long  m_userParam;
}

-(void) load:(const char *)pPath offset:(unsigned int) offset size:(unsigned int) size movieEnd:(MovieEnd) callback param:(unsigned long) userParam;
-(void) unload;

-(void) play;
-(void) stop;

-(void) seek:(float) seconds;
-(float) getTime;

-(void) getImage:(GLuint *) pTarget texture: (GLuint *) pTexture;

+ (void) GLContext:(CGLContextObj) context;
+ (void) PixelFormat:(CGLPixelFormatObj) pixelFormat;

@end