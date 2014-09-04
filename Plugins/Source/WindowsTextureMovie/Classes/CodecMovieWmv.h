
#pragma once

#include "CodecMovieWmv.generated.h"

/* WMV Video codec class */

UCLASS( MinimalAPI )
class UCodecMovieWmv : public UCodecMovieImf
{
	GENERATED_UCLASS_BODY( )

public:

	/** Returns the codec type */
	ECodecMovie::Type GetCodecType() override { return ECodecMovie::CodecMovie_Wmv; }
};
