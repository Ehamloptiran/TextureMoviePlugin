// Copyright 1998-2014 Epic Games, Inc. All Rights Reserved.

namespace UnrealBuildTool.Rules
{
	public class WindowsTextureMovie : ModuleRules
	{
		public WindowsTextureMovie(TargetInfo Target)
		{
			PrivateIncludePaths.AddRange(
				new string[] {
					"WindowsTextureMovie/Private"
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

            if (UEBuildConfiguration.bBuildEditor)
            {
                PublicDependencyModuleNames.Add("UnrealEd");
            }

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