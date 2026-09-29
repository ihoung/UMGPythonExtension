// Copyright Epic Games, Inc. All Rights Reserved.

#include "WidgetPyScript.h"
#include "WidgetPyScriptToolkit.h"
#include "Interfaces/IPluginManager.h"

void FWidgetPyScriptModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module

	LoadThirdPartyLib();

	PluginToolkit = MakeShared<FWidgetPyScriptToolkit>();
}

void FWidgetPyScriptModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.

	PluginToolkit.Reset();

	UnloadThirdPartyLib();
}

void FWidgetPyScriptModule::LoadThirdPartyLib()
{
	const FString BasePluginDir = IPluginManager::Get().FindPlugin("WidgetPyScript")->GetBaseDir();

	const FString LibExamplePath = FPaths::Combine(*BasePluginDir, TEXT("Binaries/ThirdParty/TinyXML2/Win64/tinyxml2.dll"));

	DynamicLibHandle = FPlatformProcess::GetDllHandle(*LibExamplePath);

	if (DynamicLibHandle != nullptr)
	{
		UE_LOG(LogTemp, Log, TEXT("tinyxml2.dll loaded successfully!"));
	}
	else
	{
		UE_LOG(LogTemp, Fatal, TEXT("tinyxml2.dll failed to load!"));
	}
}

void FWidgetPyScriptModule::UnloadThirdPartyLib()
{
	if (DynamicLibHandle != nullptr) 
	{
		FPlatformProcess::FreeDllHandle(DynamicLibHandle);
		DynamicLibHandle = nullptr;
	}
}
	
IMPLEMENT_MODULE(FWidgetPyScriptModule, WidgetPyScript)