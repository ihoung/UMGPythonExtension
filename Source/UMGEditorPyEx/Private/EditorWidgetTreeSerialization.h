// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

class UBaseWidgetBlueprint;
class FClassProperty;
/**
 *
 */
class FEditorWidgetTreeSerialization
{
public:
	FEditorWidgetTreeSerialization();
	~FEditorWidgetTreeSerialization();

	/** Serializes the widget tree of a single Widget Blueprint to '<DestinationDir>/<AssetName>.json'.
	 *  Asks the user whether to also serialize the entry Widget Blueprints (ListView 'entryWidgetClass')
	 *  referenced by the widget tree to the same directory. */
	static bool SerializeWidgetBlueprint(UBaseWidgetBlueprint* WidgetBlueprint, const FString& DestinationDir);

	/** Create a Widget Blueprint and load the serialized Json file as its widget tree */
	static bool LoadSerializationToWidgetBlueprint(const FString& FilePath, UBaseWidgetBlueprint* WidgetBlueprint, TFunctionRef<UClass*(FString)> EntryWidgetClassPredicate=[](FString){return nullptr;});

private:
	static bool SerializeWidgetBlueprintInternal(UBaseWidgetBlueprint* WidgetBlueprint, const FString& DestinationDir, bool bAskAboutEntries, TSet<UBaseWidgetBlueprint*>& SerializedBlueprints);
};
