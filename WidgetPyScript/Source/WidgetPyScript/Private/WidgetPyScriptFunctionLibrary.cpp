// Fill out your copyright notice in the Description page of Project Settings.


#include "WidgetPyScriptFunctionLibrary.h"
#include "WidgetPyScriptGenerator.h"
#include "BlueprintEditor.h"
#include "EditorUtilityWidgetBlueprint.h"
#include "Components/Widget.h"
#include "Interfaces/IPluginManager.h"

//UObject* UWidgetPyScriptFunctionLibrary::GetCurrentEditingWidget()
//{
//    UAssetEditorSubsystem* AssetEditorSubsystem = GEditor->GetEditorSubsystem<UAssetEditorSubsystem>();
//    TArray<UObject*> Assets = AssetEditorSubsystem->GetAllEditedAssets();
//    TArray<IAssetEditorInstance*> AssetEditors;
//    for (UObject* Asset : Assets)
//    {
//        if (Asset->GetClass()->IsChildOf(UEditorUtilityWidgetBlueprint::StaticClass()))
//        {
//            IAssetEditorInstance* AssetEditor = AssetEditorSubsystem->FindEditorForAsset(Asset, false);
//            AssetEditors.Add(AssetEditor);
//            UE_LOG(LogTemp, Warning, TEXT("asset: %s"), *StaticCast<FBlueprintEditor*>(AssetEditor)->GetBlueprintObj()->GetFName().ToString());
//        }
//    }
//    return nullptr;
//
//    return nullptr;
//}

//TArray<UObject*> UWidgetPyScriptFunctionLibrary::GetAllObjects()
//{
//    TArray<UObject*> Array;
//    for (TObjectIterator<UObject> Itr; Itr; ++Itr)
//    {
//        Array.Add(*Itr);
//    }
//    return Array;
//}

void UWidgetPyScriptFunctionLibrary::GeneratePyFromWidgets(const TArray<FAssetData>& Assets)
{
	for (auto AssetIt = Assets.CreateConstIterator(); AssetIt; ++AssetIt)
	{
		const FAssetData& Asset = *AssetIt;
		if (!Asset.IsRedirector() && Asset.AssetClass != NAME_Class && !(Asset.PackageFlags & PKG_FilterEditorOnly))
		{
			if (Asset.GetClass()->IsChildOf(UEditorUtilityWidgetBlueprint::StaticClass()))
			{
				UWidgetPyScriptFunctionLibrary::GeneratePyFromWidget(Asset);
			}
		}
	}
}

void UWidgetPyScriptFunctionLibrary::GeneratePyFromWidget(const FAssetData& Asset)
{
	// Load asset
	FString WidgetPath = Asset.ObjectPath.ToString();
	if (WidgetPath.IsEmpty())
	{
		FString AssetName = Asset.AssetName.ToString();
		WidgetPath = *FPaths::Combine(Asset.PackagePath.ToString(), AssetName + "." + AssetName);
	}
	UWidgetBlueprint* WidgetObject = FindObject<UWidgetBlueprint>(nullptr, *WidgetPath);
	if (WidgetObject == nullptr)
	{
		WidgetObject = LoadObject<UWidgetBlueprint>(nullptr, *WidgetPath);
	}
	TArray<UWidget*> VariableWidgets;
	WidgetObject->ForEachSourceWidget([&](UWidget* InWidget)
		{
			if (InWidget->bIsVariable == 1)
			{
				//UE_LOG(LogTemp, Warning, TEXT("widget name: %s"), *InWidget->GetName());
				VariableWidgets.Emplace(InWidget);
			}
		}
	);

	FWidgetPyScriptGenerator::GenerateWidgetXML(Asset, VariableWidgets);
}
