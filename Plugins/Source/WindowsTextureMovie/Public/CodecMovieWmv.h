
#pragma once

#include "CodecMovieWmv.generated.h"

/* WMV Video codec class */

UCLASS( )
class UCodecMovieWmv : public UCodecMovieImf
{
	GENERATED_UCLASS_BODY( )

public:

	/** Returns the codec type */
	ECodecMovie::Type GetCodecType( ) OVERRIDE { return ECodecMovie::CodecMovie_Wmv; }
};
