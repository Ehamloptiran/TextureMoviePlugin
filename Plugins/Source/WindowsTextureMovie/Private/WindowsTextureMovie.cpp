// Copyright 1998-2014 Epic Games, Inc. All Rights Reserved.

#include "WindowsTextureMoviePrivatePCH.h"
#include "WindowsTextureMovie.generated.inl"

class FWindowsTextureMovie : public IWindowsTextureMovie
{
	/** IModuleInterface implementation */
	virtual void StartupModule() OVERRIDE;
	virtual void ShutdownModule() OVERRIDE;
};

IMPLEMENT_MODULE( FWindowsTextureMovie, WindowsTextureMovie )



void FWindowsTextureMovie::StartupModule()
{
}


void FWindowsTextureMovie::ShutdownModule()
{
}



