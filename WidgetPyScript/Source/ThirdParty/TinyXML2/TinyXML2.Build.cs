// Fill out your copyright notice in the Description page of Project Settings.

using System.IO;
using UnrealBuildTool;

public class TinyXML2 : ModuleRules
{
	public TinyXML2(ReadOnlyTargetRules Target) : base(Target)
	{
		Type = ModuleType.External;

		if (Target.Platform == UnrealTargetPlatform.Win64)
		{
			// Add the import library
			PublicAdditionalLibraries.Add(Path.Combine(ModuleDirectory, "Win64", "tinyxml2.lib"));

            // Ensure that the DLL is staged along with the executable
            RuntimeDependencies.Add("$(PluginDir)/Binaries/ThirdParty/TinyXML2/Win64/tinyxml2.dll");

            // Delay-load the DLL, so we can load it from the right place first
            PublicDelayLoadDLLs.Add("tinyxml2.dll");
        }
  //      else if (Target.Platform == UnrealTargetPlatform.Mac)
  //      {
  //          PublicDelayLoadDLLs.Add(Path.Combine(ModuleDirectory, "Mac", "Release", "libExampleLibrary.dylib"));
  //          RuntimeDependencies.Add("$(PluginDir)/Source/ThirdParty/WidgetPyThirdPartyLibLibrary/Mac/Release/libExampleLibrary.dylib");
  //      }
  //      else if (Target.Platform == UnrealTargetPlatform.Linux)
		//{
		//	string ExampleSoPath = Path.Combine("$(PluginDir)", "Binaries", "ThirdParty", "WidgetPyThirdPartyLibLibrary", "Linux", "x86_64-unknown-linux-gnu", "libExampleLibrary.so");
		//	PublicAdditionalLibraries.Add(ExampleSoPath);
		//	PublicDelayLoadDLLs.Add(ExampleSoPath);
		//	RuntimeDependencies.Add(ExampleSoPath);
		//}
	}
}
