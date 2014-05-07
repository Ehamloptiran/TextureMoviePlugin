// Copyright 1998-2014 Epic Games, Inc. All Rights Reserved.

#include "CoreUObject.h"
#include "Engine.h"

#if WITH_EDITOR
#include "UnrealEd.h"
#endif

#include "IWindowsTextureMovie.h"

#include "ImfVideoPlayer.h"
#include "CodecMovieImf.h"
#include "CodecMovieWmv.h"
#include "CodecMovieMp4.h"
#include "CodecMovieAvi.h"

#if WITH_EDITOR
#include "TextureMovieFactory.h"
#endif