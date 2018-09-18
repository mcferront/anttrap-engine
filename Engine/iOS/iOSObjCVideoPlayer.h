
#import <UIKit/UIKit.h>
#import <AVFoundation/AVFoundation.h>

@interface VideoPlayerView : UIView 
{
}

@property (nonatomic, retain) AVPlayer *player;
@end

@interface ObjCVideoPlayer : NSObject 
{
   const NSString *ItemStatusContext; 

   bool loaded;
   bool asyncDone;
   bool playRequested;
   bool showRequested;
   float seekRequested;
   
   //AVPlayer *player;
   AVPlayerItem *playerItem;
   AVURLAsset *asset;
   //VideoPlayerView *playerView;
   NSMutableArray *allAudioParams;

   char eventChannelId[ 64 ];
}

-(void) load:(const char *)pPath offset:(unsigned int) offset size:(unsigned int) size eventChannelId:(const char *) pId;
-(void) unload;

-(void) play;
-(void) stop;

-(void) seek:(float) seconds;
-(float) getTime;

-(void) show;
-(void) hide;

+(void) MainView:(UIView *) view;

@end
