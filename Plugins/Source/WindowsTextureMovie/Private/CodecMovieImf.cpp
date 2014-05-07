
#include "WindowsTextureMoviePrivatePCH.h"

/* Constructor */
UCodecMovieImf::UCodecMovieImf( const class FPostConstructInitializeProperties& PCIP )
	: Super( PCIP )
	, VideoPlayer( NULL )
	, SampleGrabberCallback( NULL )
	, TextureData( )
	, Looping( false )
	, OneFrameOnly( false )
	, ResetOnLastFrame( false )
	, FrameCount( 0 )
{
}

/* Returns the format to use */
EPixelFormat UCodecMovieImf::GetFormat( )
{
	return PF_B8G8R8A8;
}

/* Opens a file from the HDD */
bool UCodecMovieImf::Open( const FString& Filename, uint32 Offset, uint32 Size )
{
	/* Unimplemented */
	return true;
}

/* Opens a file located in memory */
bool UCodecMovieImf::Open( void* Source, uint32 Size )
{
	/* Initialise video player */
	SampleGrabberCallback = new FImfSampleGrabberCallback( TextureData );
	VideoPlayer = new FImfVideoPlayer( );

	/* Open file */
	FIntPoint VideoDimensions = VideoPlayer->OpenFile( Source, Size, GetCodecType( ), SampleGrabberCallback );
	if( VideoDimensions != FIntPoint::ZeroValue )
	{
		/* Prepare buffer */
		TextureData.Empty( );
		TextureData.AddZeroed( VideoDimensions.X * VideoDimensions.Y * GPixelFormats[ PF_B8G8R8A8 ].BlockBytes );

		Dimensions = VideoDimensions;
		return true;
	}

	return false;
}

/* Close file and release resources */
void UCodecMovieImf::Close( )
{
	TextureData.Empty( );

	if( VideoPlayer )
	{
		VideoPlayer->Shutdown( );
		VideoPlayer->Release( );
		VideoPlayer = NULL;
	}

	if( SampleGrabberCallback )
	{
		SampleGrabberCallback->Release( );
		SampleGrabberCallback = NULL;
	}
}

/* Reset stream (Doesn't appear to get called) */
void UCodecMovieImf::ResetStream( )
{
	/* Unimplemented */
}

/* Copies the next frame into the render target ready for viewing */
void UCodecMovieImf::GetFrame( class FTextureMovieResource* InTextureMovieResource )
{
	/* If only rendering one frame */
	if( OneFrameOnly && FrameCount > 1 )
		return;

	/* Is a sample ready? */
	if( InTextureMovieResource && InTextureMovieResource->IsInitialized( ) && SampleGrabberCallback->GetIsSampleReadyToUpdate() )
	{
		/* Copy to render target */
		uint32 Stride;
		uint8* DestTextureData = (uint8*) RHILockTexture2D( InTextureMovieResource->TextureRHI->GetTexture2D( ), 0, RLM_WriteOnly, Stride, false );
		FMemory::Memcpy( DestTextureData, TextureData.GetData( ), TextureData.Num( ) );
		RHIUnlockTexture2D( InTextureMovieResource->TextureRHI->GetTexture2D( ), 0, false );

		/* Wait for next sample */
		SampleGrabberCallback->SetNeedNewSample( );
		FrameCount++;
	}
}

/* Begins playing the movie */
void UCodecMovieImf::Play( bool bLooping, bool bOneFrameOnly, bool bResetOnLastFrame )
{
	FrameCount = 0;
	Looping = bLooping;
	OneFrameOnly = bOneFrameOnly;
	ResetOnLastFrame = bResetOnLastFrame;

	VideoPlayer->SetPlaybackParameters( bLooping, bOneFrameOnly, bResetOnLastFrame );
	VideoPlayer->StartPlayback( );
}

/* Pauses playback and resumes the movie */
void UCodecMovieImf::Pause( bool bPause )
{
	VideoPlayer->PausePlayback( bPause );
}

/* Stops the movie */
void UCodecMovieImf::Stop( )
{
	VideoPlayer->StopPlayback( );
}

/* Releases any dynamically allocated resoueces */
void UCodecMovieImf::ReleaseDynamicResources( )
{
	/* No resources to release here */
}

