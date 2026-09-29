// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/Widget.h"

THIRD_PARTY_INCLUDES_START
#include <tinyxml2.h>
THIRD_PARTY_INCLUDES_END

/**
 * 
 */
class FWidgetPyScriptGenerator
{
public:
	static void GenerateWidgetXML(const FAssetData& WidgetAsset, const TArray<UWidget*>& TargetWidgets);

private:
	static void WriteXMLHeader(tinyxml2::XMLDocument& Doc, const FAssetData& WidgetAsset);
	static void WriteXMLWidgets(tinyxml2::XMLDocument& Doc, const TArray<UWidget*>& TargetWidgets);

	static void GenerateScriptExample(FString TemplateFilePath, FString DestDirPath, FString AssetName);
};
