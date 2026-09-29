// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

/**
 * 
 */
class FWidgetPyScriptToolkit
{
public:
	FWidgetPyScriptToolkit();
	~FWidgetPyScriptToolkit();

private:
	TSharedPtr<class FUICommandList> PluginCommands;

private:

	/** This function will be bound to Command. */
	void PluginButtonClicked();

	void RegisterMenus();

	TSharedRef<FExtender> AddCBMenuExtender(const TArray<FAssetData>& SelectedAssets);

	void AddCBMenuAction();
};
