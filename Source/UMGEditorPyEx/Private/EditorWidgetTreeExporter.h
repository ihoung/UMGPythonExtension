// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

/**
 * Registers a Content Browser context menu entry (for EditorUtilityWidget Blueprint assets)
 * that serializes the Editor Utility Widget Blueprint Design.
 * The menu entry is only visible when Developer Mode is enabled in the plugin settings.
 */
class FEditorWidgetTreeExporter
{
public:
	FEditorWidgetTreeExporter();
	~FEditorWidgetTreeExporter();
private:
	void RegisterMenus();

	TSharedRef<FExtender> AddCBMenuExtender(const TArray<FAssetData>& SelectedAssets);

	void AddCBMenuEntries(FMenuBuilder& MenuBuilder);

	/** Serializes the widget trees of all selected User Widget Blueprints to JSON files. */
	void SerializeWidgetTree(bool AsDefault);
};
