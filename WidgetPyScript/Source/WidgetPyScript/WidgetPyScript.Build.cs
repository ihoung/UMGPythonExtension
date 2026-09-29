// Copyright Epic Games, Inc. All Rights Reserved.

using System.IO;
using UnrealBuildTool;

public class WidgetPyScript : ModuleRules
{
	public WidgetPyScript(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
		
		PublicIncludePaths.AddRange(
			new string[] {
				// ... add public include paths required here ...
				Path.Combine(ModuleDirectory, "..", "ThirdParty", "TinyXML2", "include"),
            }
			);
				
		
		PrivateIncludePaths.AddRange(
			new string[] {
				// ... add other private include paths required here ...
			}
			);
			
		
		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
				// ... add other public dependencies that you statically link with here ...
				"TinyXML2",
            }
			);
			
		
		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"Projects",
				"InputCore",
				"UnrealEd",
				"ToolMenus",
                "ContentBrowser",
                "CoreUObject",
				"Engine",
				"Slate",
				"SlateCore",
				"UMGEditor",
                "Blutility",
				"Kismet",
				"UMG"
				// ... add private dependencies that you statically link with here ...	
			}
			);
		
		
		DynamicallyLoadedModuleNames.AddRange(
			new string[]
			{
				// ... add any modules that your module loads dynamically here ...
			}
			);

		//if (Target.Platform == UnrealTargetPlatform.Win64)
		//{
		//	// Add the import library
		//	PublicAdditionalLibraries.Add(Path.Combine(ModuleDirectory, "../ThirdParty", "Win64", "tinyxml2.lib"));

  //          // Put the library along with the executable
  //          RuntimeDependencies.Add("$(PluginDir)/Binaries/ThirdParty/TinyXML2/Win64/tinyxml2.dll");

  //          // Delay-load the DLL, so we can load it from the right place first
  //          PublicDelayLoadDLLs.Add("tinyxml2.dll");
		//}
	}
}
