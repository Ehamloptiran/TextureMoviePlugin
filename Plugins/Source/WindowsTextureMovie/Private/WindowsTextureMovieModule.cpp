// Copyright 1998-2014 Epic Games, Inc. All Rights Reserved.

#include "WindowsTextureMoviePrivatePCH.h"

class FWindowsTextureMovie : public IWindowsTextureMovie
{
	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
};

IMPLEMENT_MODULE( FWindowsTextureMovie, WindowsTextureMovie )



void FWindowsTextureMovie::StartupModule()
{
}


void FWindowsTextureMovie::ShutdownModule()
{
}



