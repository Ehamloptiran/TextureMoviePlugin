
#pragma once

#include "CodecMovieMp4.generated.h"

/* MP4 Video codec class (Currently does not work) */

UCLASS( )
class UCodecMovieMp4 : public UCodecMovieImf
{
	GENERATED_UCLASS_BODY( )

public:

	/** Returns the codec type */
	ECodecMovie::Type GetCodecType( ) OVERRIDE { return ECodecMovie::CodecMovie_Mp4; }
};
