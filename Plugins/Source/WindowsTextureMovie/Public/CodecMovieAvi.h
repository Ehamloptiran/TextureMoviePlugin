
#pragma once

#include "CodecMovieAvi.generated.h"

/* AVI Video file codec class */

UCLASS( )
class UCodecMovieAvi : public UCodecMovieImf
{
	GENERATED_UCLASS_BODY( )

public:

	/** Returns the codec type */
	ECodecMovie::Type GetCodecType( ) OVERRIDE { return ECodecMovie::CodecMovie_Avi; }
};
