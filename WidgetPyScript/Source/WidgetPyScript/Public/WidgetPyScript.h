// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "WidgetPyScriptToolkit.h"
#include "Modules/ModuleManager.h"

class FToolBarBuilder;
class FMenuBuilder;

class FWidgetPyScriptModule : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

private:
	void LoadThirdPartyLib();

	void UnloadThirdPartyLib();

private:
	TSharedPtr<FWidgetPyScriptToolkit> PluginToolkit;

	// Dynamic library
	void* DynamicLibHandle;
};
