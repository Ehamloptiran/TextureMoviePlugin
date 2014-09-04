#include "WindowsTextureMovieEditorPrivatePCH.h"
#include "WindowsTextureMovieEditorModule.h"
//////////////////////////////////////////////////////////////////////////
// FWindowsTextureMovieEditorModule

class FWindowsTextureMovieEditorModule : public IWindowsTextureMovieEditorModule
{
public:
	virtual void StartupModule() override
	{
	}

	virtual void ShutdownModule() override
	{
	}
};

//////////////////////////////////////////////////////////////////////////

IMPLEMENT_MODULE(FWindowsTextureMovieEditorModule, WindowsTextureMovieEditor);
//DEFINE_LOG_CATEGORY(LogWindowsTextureMovieEditor);
