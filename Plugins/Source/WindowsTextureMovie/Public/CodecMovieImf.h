
#pragma once

#include "CodecMovieImf.generated.h"

/* Media Foundation Codec class */

UCLASS( )
class UCodecMovieImf : public UCodecMovie
{
	GENERATED_UCLASS_BODY( )

	/* Begin UCodecMovie interface */
	virtual bool IsSupported( ) OVERRIDE { return true; }
	virtual uint32 GetSizeX( ) OVERRIDE { return Dimensions.X; }
	virtual uint32 GetSizeY( ) OVERRIDE { return Dimensions.Y; }
	virtual EPixelFormat GetFormat( ) OVERRIDE;
	virtual float GetFrameRate( ) OVERRIDE { return 30.f; }
	virtual bool Open( const FString& Filename, uint32 Offset, uint32 Size ) OVERRIDE;
	virtual bool Open( void* Source, uint32 Size ) OVERRIDE;
	virtual void Close( ) OVERRIDE;
	virtual void ResetStream( ) OVERRIDE;
	virtual void GetFrame( class FTextureMovieResource* InTextureMovieResource ) OVERRIDE;
	virtual void Play( bool bLooping, bool bOneFrameOnly, bool bResetOnLastFrame ) OVERRIDE;
	virtual void Pause( bool bPause ) OVERRIDE;
	virtual void Stop( ) OVERRIDE;
	virtual void ReleaseDynamicResources( ) OVERRIDE;
	/* End UCodecMovie interface */

	/** Returns the codec type */
	virtual ECodecMovie::Type GetCodecType( ) { return ECodecMovie::CodecMovie_Null; }

private:

	/** Video player for playing movie in the background */
	FImfVideoPlayer* VideoPlayer;

	/** Sample grabber callback */
	FImfSampleGrabberCallback* SampleGrabberCallback;

	/** Stores texture data */
	TArray<uint8> TextureData;

	/** Stores the width/height of the movie */
	FIntPoint Dimensions;

	/** Various playback flags */
	bool Looping;
	bool OneFrameOnly;
	bool ResetOnLastFrame;

	/** Frame counter */
	uint32 FrameCount;
};
