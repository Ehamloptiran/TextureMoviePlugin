
#pragma once

#include "Engine/CodecMovie.h"
#include "CodecMovieImf.generated.h"

/* Media Foundation Codec class */

UCLASS( )
class UCodecMovieImf : public UCodecMovie
{
	GENERATED_UCLASS_BODY( )

	/* Begin UCodecMovie interface */
	virtual bool IsSupported( ) override { return true; }
	virtual uint32 GetSizeX() override { return Dimensions.X; }
	virtual uint32 GetSizeY() override { return Dimensions.Y; }
	virtual EPixelFormat GetFormat() override;
	virtual float GetFrameRate() override { return 30.f; }
	virtual bool Open(const FString& Filename, uint32 Offset, uint32 Size) override;
	virtual bool Open(void* Source, uint32 Size) override;
	virtual void Close() override;
	virtual void ResetStream() override;
	virtual void GetFrame(FRHICommandListImmediate& RHICmdList, class FTextureMovieResource* InTextureMovieResource) override;
	virtual void Play(bool bLooping, bool bOneFrameOnly, bool bResetOnLastFrame) override;
	virtual void Pause(bool bPause) override;
	virtual void Stop() override;
	virtual void ReleaseDynamicResources() override;
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
