//
//  ObjCVideoPlayer.m
//  Game
//
//  Created by Trapper McFerron on 3/7/11.
//  Copyright 2011 None. All rights reserved.
//

#import "iOSObjCVideoPlayer.h"

#include "Debug.h"
#include "ChannelSystem.h"
#include "Channel.h"

@implementation VideoPlayerView

+ (Class)layerClass
{
    return [AVPlayerLayer class];
}

- (AVPlayer*)player
{
    return [(AVPlayerLayer *)[self layer] player];
}

- (void)setPlayer:(AVPlayer *)player
{
    [(AVPlayerLayer *)[self layer] setPlayer:player];
}

@end


@implementation ObjCVideoPlayer

static UIView *s_MainView;

+ (void) MainView:(UIView *) mainView
{
   s_MainView = mainView;
}

static AVPlayer *player;
static VideoPlayerView *playerView;
static AVPlayerItem *currentPlayerItem;

static int playerCount = 0;
static bool inView = false;
//static Lock s_Lock;

//static NSMutableArray *loadedIds = 0;

-(void) finishLoad
{
   loaded = true;
   
   if ( nil == player )
   {
      currentPlayerItem = playerItem;
      player = [[AVPlayer alloc] initWithPlayerItem:playerItem];
      [playerView setPlayer:player];
   }
   
   if ( seekRequested > 0 )
   {
      [self seek:seekRequested];
   }

   if ( true == playRequested )
   {
      [self play];
   }
   if ( true == showRequested )
   {
      [self show];
   }
}

-(void) load:(const char *)pPath offset:(unsigned int) offset size:(unsigned int) size eventChannelId:(const char *) pId
{
   seekRequested = 0;
   
   strncpy( eventChannelId, pId, sizeof(eventChannelId) - 1 );
   eventChannelId[ sizeof(eventChannelId) - 1 ] = NULL;

   NSString *filename = [NSString stringWithCString:pPath encoding:NSASCIIStringEncoding];
   NSURL *fileURL     = [NSURL fileURLWithPath:filename];

   ++playerCount;

   asset = [[AVURLAsset alloc] initWithURL:fileURL options:nil];
   Debug::Assert( Condition(asset != nil), "video file not found: %s", pPath );

   NSString *tracksKey = @"tracks";

   CGRect rect;

extern UIView *g_RootView;

   rect.origin.x = 0;
   rect.origin.y = 0;
   rect.size.width  = g_RootView.frame.size.height;
   rect.size.height = g_RootView.frame.size.width;
   
   //Debug::Print( Debug::TypeInfo, "%s: %s", __FUNCTION__, eventChannelId );

   if ( nil == playerView )
   {
      playerView = [[VideoPlayerView alloc] initWithFrame:rect];
      playerView.opaque = YES;
      playerView.backgroundColor  = [UIColor blackColor];
      playerView.autoresizingMask = UIViewAutoresizingNone;
   }
   
   asyncDone = false;
   
    [asset loadValuesAsynchronouslyForKeys:[NSArray arrayWithObject:tracksKey] completionHandler:
      ^{
         // Completion handler block.
         NSError *error = nil;
         AVKeyValueStatus status = [asset statusOfValueForKey:tracksKey error:&error];
          
         //Debug::Print( Debug::TypeInfo, "%s: %s (In Load)", __FUNCTION__, eventChannelId );
   
               if (status == AVKeyValueStatusLoaded) 
         {
            NSArray *audioTracks = [asset tracksWithMediaType:AVMediaTypeAudio];
            
            for (AVAssetTrack *track in audioTracks) 
            {
               AVMutableAudioMixInputParameters *audioInputParams =[AVMutableAudioMixInputParameters audioMixInputParameters];
               [audioInputParams setVolume:1.0 atTime:kCMTimeZero];
               [audioInputParams setTrackID:[track trackID]];
               [allAudioParams addObject:audioInputParams];
            }
            
            playerItem = [[AVPlayerItem alloc] initWithAsset:asset];
            [playerItem addObserver:self forKeyPath:@"status" options:0 context:&ItemStatusContext];

            [[NSNotificationCenter defaultCenter] addObserver:self
                                                  selector:@selector(playerItemDidReachEnd:)
                                                  name:AVPlayerItemDidPlayToEndTimeNotification
                                                  object:playerItem];
            
            //ScopeLock lock( s_Lock );

            //if ( nil == loadedIds )
            //{
            //   loadedIds = [[NSMutableArray alloc] init];
            //}
            asyncDone = true;
            //[loadedIds addObject:self];
         }
         else 
         {
            Debug::Assert( Condition(false), "Error loading video file: %s (%s)", pPath, [[error localizedDescription] UTF8String] );
         }
      }
   ];
   
   while ( false == asyncDone )
   {
      Thread::Sleep( 1 );
   }
   
   [self finishLoad];
}  

- (void) lazyShow
{
   if ( false == inView )
   {
      //Debug::Print( Debug::TypeInfo, "%s: %s", __FUNCTION__, eventChannelId );
      inView = true;
      [s_MainView addSubview:playerView];
   }
}

- (void) lazyHide
{
   if ( 1 == playerCount )
   {
      //Debug::Print( Debug::TypeInfo, "%s: %s", __FUNCTION__, eventChannelId );
      [playerView removeFromSuperview];
      inView = false;
   }
}

-(void) lazyBind
{
   if ( true == loaded && playerItem != currentPlayerItem )
   {
      //Debug::Print( Debug::TypeInfo, "%s: %s", __FUNCTION__, eventChannelId );
      currentPlayerItem = playerItem;
      [player replaceCurrentItemWithPlayerItem:playerItem];
   }
}

- (void) unload
{
   //Debug::Print( Debug::TypeInfo, "%s: %s", __FUNCTION__, eventChannelId );
   [self stop];
   [self hide];

   --playerCount;

   if ( playerItem == currentPlayerItem )
   {
      //Debug::Print( Debug::TypeInfo, "%s: %s", __FUNCTION__, eventChannelId );
      currentPlayerItem = nil;
      //don't replace with nil, keep the stale one around so it doesn't go black
      //[player replaceCurrentItemWithPlayerItem:nil];
   }

   [[NSNotificationCenter defaultCenter] removeObserver:self];
   [playerItem removeObserver:self forKeyPath:@"status"];   
   
   [playerItem release];
   [asset release];
   [allAudioParams release];
   
   allAudioParams = nil;
   playerItem = nil;
   asset = nil;

   loaded = false;
}

- (void)playerItemDidReachEnd:(NSNotification *)notification
{
   Channel *pChannel = ChannelSystem::Instance( ).GetChannel( Cwid(eventChannelId) );
   pChannel->QueueEvent( "Finished", ArgList() );
}

- (void) prepareVolume
{
   //Debug::Print( Debug::TypeInfo, "%s: %s", __FUNCTION__, eventChannelId );
   AVMutableAudioMix *audioMix = [AVMutableAudioMix audioMix];
   [audioMix setInputParameters:allAudioParams];
   
   [playerItem setAudioMix:audioMix];
}

- (void) play
{   
   if ( false == loaded ) 
   {
      playRequested = true;
   }
   else 
   {    
      //Debug::Print( Debug::TypeInfo, "%s: %s", __FUNCTION__, eventChannelId );
      [self lazyBind];
      
      playRequested = false;
      [self prepareVolume];
      [player play];
   }
}

- (void) stop
{
   if ( false == loaded ) return;

   [player pause];
}

- (void) seek:(float)seconds
{
   if ( false == loaded ) 
   {
      seekRequested = seconds;
   }
   else
   {   
      //Debug::Print( Debug::TypeInfo, "%s: %s", __FUNCTION__, eventChannelId );
      [self lazyBind];
     
       CMTime time = CMTimeMakeWithSeconds( seconds, 1 );
      [player seekToTime:time toleranceBefore:kCMTimeZero toleranceAfter:kCMTimeZero];
      seekRequested = 0;
   }
}

-(float) getTime
{
   if ( false == loaded ) return 0;
  
   CMTime time = player.currentTime;
   return CMTimeGetSeconds( time );
}

- (void) show
{  
   if ( false == loaded ) 
   {
      showRequested = YES;
   }
   else
   {   
      //Debug::Print( Debug::TypeInfo, "%s: %s", __FUNCTION__, eventChannelId );
      [self lazyBind];
      [self lazyShow];
   
      showRequested = NO;
   }
}

- (void) hide
{
   //Debug::Print( Debug::TypeInfo, "%s: %s", __FUNCTION__, eventChannelId );
   [self lazyHide];
}

- (void)observeValueForKeyPath:(NSString *)keyPath ofObject:(id)object
                        change:(NSDictionary *)change context:(void *)context {
 
    if (context == &ItemStatusContext) {
        return;
    }
    [super observeValueForKeyPath:keyPath ofObject:object
           change:change context:context];
    return;
}

@end
