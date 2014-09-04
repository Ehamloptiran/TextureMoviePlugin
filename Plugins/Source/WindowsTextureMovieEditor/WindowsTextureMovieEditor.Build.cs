// Copyright 1998-2014 Epic Games, Inc. All Rights Reserved.

namespace UnrealBuildTool.Rules
{
	public class WindowsTextureMovieEditor : ModuleRules
	{
		public WindowsTextureMovieEditor(TargetInfo Target)
		{
			PublicDependencyModuleNames.Add("WindowsTextureMovie");
			
			PrivateIncludePaths.AddRange(
				new string[] {
					"WindowsTextureMovieEditor/Private",
				}
				);

			PublicDependencyModuleNames.AddRange(
				new string[]
				{
					"Core",
					"CoreUObject",
                    "Engine",
                    "RenderCore",
                    "ShaderCore",
                    "RHI"
					
				}
				);
				

            PublicDependencyModuleNames.Add("UnrealEd");

            if (Target.Platform == UnrealTargetPlatform.Win64 || Target.Platform == UnrealTargetPlatform.Win32)
            {
                PublicDelayLoadDLLs.Add("shlwapi.dll");
                PublicDelayLoadDLLs.Add("mf.dll");
                PublicDelayLoadDLLs.Add("mfplat.dll");
                PublicDelayLoadDLLs.Add("mfplay.dll");
                PublicDelayLoadDLLs.Add("mfuuid.dll");
            }
		}
	}
}