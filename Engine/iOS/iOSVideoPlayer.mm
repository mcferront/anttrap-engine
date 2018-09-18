#import "iOSObjCVideoPlayer.h"

#include "VideoPlayer.h"

void VideoPlayer::Load(
   const char *pPath,
   uint32 offset,
   uint32 size,
   Channel *pChannel
)
{
   player = (int) [ObjCVideoPlayer alloc];   
   [((ObjCVideoPlayer*)player) load:pPath offset:offset size:size eventChannelId:pChannel->GetId().ToString()];
}

void VideoPlayer::Unload( void )
{
   [((ObjCVideoPlayer*)player) unload];
   [((ObjCVideoPlayer*)player) release];
}

void VideoPlayer::Play( void )
{
   [((ObjCVideoPlayer*)player) play];
}

void VideoPlayer::Stop( void )
{
   [((ObjCVideoPlayer*)player) stop];
}

float VideoPlayer::GetTime( void )
{
   return [((ObjCVideoPlayer*)player) getTime];
}

void VideoPlayer::SeekTo( 
   VideoPlayer::Seek from,
   float seconds
)
{
   if ( from == VideoPlayer::SeekCurrent )
   {
      float currentTime = [((ObjCVideoPlayer*)player) getTime];
      seconds += currentTime;
   }

   [((ObjCVideoPlayer*)player) seek:seconds];
}

void VideoPlayer::Show( void )
{
   [((ObjCVideoPlayer*)player) show];
}

void VideoPlayer::Hide( void )
{
   [((ObjCVideoPlayer*)player) hide];
}