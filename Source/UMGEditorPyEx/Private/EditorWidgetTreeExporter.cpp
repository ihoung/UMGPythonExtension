// Fill out your copyright notice in the Description page of Project Settings.

#include "EditorWidgetTreeExporter.h"
#include "UMGEditorPyPluginSettings.h"
#include "ToolMenus.h"
#include "ContentBrowserModule.h"
#include "IContentBrowserSingleton.h"
#include "BaseWidgetBlueprint.h"
#include "Blueprint/UserWidgetBlueprint.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "DesktopPlatformModule.h"
#include "IDesktopPlatform.h"
#include "Framework/Application/SlateApplication.h"
#include "Widgets/Notifications/SNotificationList.h"
#include "Framework/Notifications/NotificationManager.h"
#include "EditorWidgetTreeExporterStyle.h"
#include "EditorWidgetTreeSerialization.h"

#define LOCTEXT_NAMESPACE "FEditorWidgetTreeExporter"

namespace
{
	void ShowNotification(const FText& Text, SNotificationItem::ECompletionState CompletionState)
	{
		FNotificationInfo Info(Text);
		Info.ExpireDuration = 4.0f;
		if (TSharedPtr<SNotificationItem> NotificationItem = FSlateNotificationManager::Get().AddNotification(Info))
		{
			NotificationItem->SetCompletionState(CompletionState);
		}
	}
}

FEditorWidgetTreeExporter::FEditorWidgetTreeExporter()
{
	UToolMenus::RegisterStartupCallback(FSimpleMulticastDelegate::FDelegate::CreateRaw(this, &FEditorWidgetTreeExporter::RegisterMenus));
}

FEditorWidgetTreeExporter::~FEditorWidgetTreeExporter()
{
	UToolMenus::UnRegisterStartupCallback(this);

	UToolMenus::UnregisterOwner(this);

	if (FModuleManager::Get().IsModuleLoaded(TEXT("ContentBrowser")))
	{
		FContentBrowserModule* ContentBrowserModule = FModuleManager::GetModulePtr<FContentBrowserModule>(TEXT("ContentBrowser"));
		if (ContentBrowserModule)
		{
			ContentBrowserModule->GetAllAssetViewContextMenuExtenders().RemoveAll(
				[this](const FContentBrowserMenuExtender_SelectedAssets& Delegate)
				{
					return Delegate.IsBoundToObject(this);
				});
		}
	}
}

void FEditorWidgetTreeExporter::RegisterMenus()
{
	// Owner will be used for cleanup in call to UToolMenus::UnregisterOwner
	FToolMenuOwnerScoped OwnerScoped(this);

	FContentBrowserModule& ContentBrowserModule = FModuleManager::LoadModuleChecked<FContentBrowserModule>(TEXT("ContentBrowser"));
	{
		TArray<FContentBrowserMenuExtender_SelectedAssets>& CBMenuAssetExtenderDelegates = ContentBrowserModule.GetAllAssetViewContextMenuExtenders();
		CBMenuAssetExtenderDelegates.Add(FContentBrowserMenuExtender_SelectedAssets::CreateRaw(this, &FEditorWidgetTreeExporter::AddCBMenuExtender));
	}
}

TSharedRef<FExtender> FEditorWidgetTreeExporter::AddCBMenuExtender(const TArray<FAssetData>& SelectedAssets)
{
	TSharedRef<FExtender> Extender = MakeShared<FExtender>();

	// The context menu entry is only available when Developer Mode is enabled in the plugin settings.
	const UUMGEditorPyPluginSettings* Settings = GetDefault<UUMGEditorPyPluginSettings>();
	if (!Settings || !Settings->bDeveloperMode)
	{
		return Extender;
	}

	bool HasWidgetAsset = false;
	for (auto AssetIt = SelectedAssets.CreateConstIterator(); AssetIt; ++AssetIt)
	{
		const FAssetData& Asset = *AssetIt;
		if (!Asset.IsRedirector() && !(Asset.PackageFlags & PKG_FilterEditorOnly))
		{
			UClass* AssetClass = Asset.GetClass();
			if (AssetClass && AssetClass->IsChildOf(UUserWidgetBlueprint::StaticClass()))
			{
				HasWidgetAsset = true;
				break;
			}
		}
	}

	if (HasWidgetAsset)
	{
		Extender->AddMenuExtension(
			"CommonAssetActions",
			EExtensionHook::After,
			nullptr,
			FMenuExtensionDelegate::CreateRaw(this, &FEditorWidgetTreeExporter::AddCBMenuEntries)
		);
	}

	return Extender;
}

void FEditorWidgetTreeExporter::AddCBMenuEntries(FMenuBuilder& MenuBuilder)
{
	MenuBuilder.BeginSection("WidgetDesignExport", LOCTEXT("WidgetDesignExportSection", "Widget Design Export"));
	{
		// Add Menu Entry Here
		MenuBuilder.AddMenuEntry(
			LOCTEXT("SerializeWidgetTree", "Serialize Widget Tree"),
			LOCTEXT("SerializeWidgetTree ToolTip", 
				"Serialize the Widget tree of the selected Editor Utility Widget (or User Widget) Blueprint to a Json file in the same-level directory with the same name"),
			FSlateIcon(FEditorWidgetTreeExporterStyle::Get().GetStyleSetName(), "Icons.WidgetTreeSerialization"),
			FUIAction(FExecuteAction::CreateRaw(this, &FEditorWidgetTreeExporter::SerializeWidgetTree, true)),
			NAME_None,
			EUserInterfaceActionType::Button
		);
		MenuBuilder.AddMenuEntry(
			LOCTEXT("SerializeWidgetTree", "Serialize Widget Tree As ..."),
			LOCTEXT("SerializeWidgetTree ToolTip",
				"Serialize the Widget tree of the selected Editor Utility Widget (or User Widget) Blueprint to a Json file in the specified directory"),
			FSlateIcon(FEditorWidgetTreeExporterStyle::Get().GetStyleSetName(), "Icons.WidgetTreeSerialization"),
			FUIAction(FExecuteAction::CreateRaw(this, &FEditorWidgetTreeExporter::SerializeWidgetTree, false)),
			NAME_None,
			EUserInterfaceActionType::Button
		);
	}
	MenuBuilder.EndSection();
}

void FEditorWidgetTreeExporter::SerializeWidgetTree(bool AsDefault)
{
	// Gather selected User Widget Blueprints (Editor Utility Widget Blueprints included)
	FContentBrowserModule& ContentBrowserModule = FModuleManager::LoadModuleChecked<FContentBrowserModule>(TEXT("ContentBrowser"));
	TArray<FAssetData> SelectedAssets;
	ContentBrowserModule.Get().GetSelectedAssets(SelectedAssets);

	TArray<UBaseWidgetBlueprint*> WidgetBlueprints;
	for (const FAssetData& SelectedAsset : SelectedAssets)
	{
		if (UBaseWidgetBlueprint* WidgetBlueprint = Cast<UBaseWidgetBlueprint>(SelectedAsset.GetAsset()))
		{
			WidgetBlueprints.Add(WidgetBlueprint);
		}
	}

	if (WidgetBlueprints.IsEmpty())
	{
		return;
	}

	FString DestinationDir;
	if (AsDefault)
	{
		DestinationDir = FPackageName::LongPackageNameToFilename(FPaths::GetPath(WidgetBlueprints[0]->GetPathName()));
	}
	else
	{
		// Ask the user where to save; the directory of the chosen file is used for all selected blueprints
		IDesktopPlatform* DesktopPlatform = FDesktopPlatformModule::Get();
		if (!DesktopPlatform)
		{
			ShowNotification(LOCTEXT("SerializeNoDesktopPlatform", "Failed to open the save dialog: DesktopPlatform module is unavailable."), SNotificationItem::CS_Fail);
			return;
		}

		TArray<FString> OutFiles;
		const bool bPicked = DesktopPlatform->SaveFileDialog(
			FSlateApplication::Get().FindBestParentWindowHandleForDialogs(nullptr),
			LOCTEXT("SaveSerializationDialogTitle", "Save Widget Tree Serialization").ToString(),
			FPaths::ProjectDir(),
			WidgetBlueprints[0]->GetName() + TEXT(".json"),
			TEXT("JSON Files (*.json)|*.json"),
			0,
			OutFiles);
		if (!bPicked || OutFiles.IsEmpty())
		{
			return;
		}
		DestinationDir = FPaths::GetPath(OutFiles[0]);
		IPlatformFile & FileManager = FPlatformFileManager::Get().GetPlatformFile();
		if (!FileManager.DirectoryExists(*DestinationDir))
		{
			FileManager.CreateDirectoryTree(*DestinationDir);
		}
	}

	int32 NumSaved = 0;
	for (UBaseWidgetBlueprint* WidgetBlueprint : WidgetBlueprints)
	{
		if (FEditorWidgetTreeSerialization::SerializeWidgetBlueprint(WidgetBlueprint, DestinationDir))
		{
			++NumSaved;
		}
	}

	if (NumSaved > 0)
	{
		ShowNotification(
			FText::Format(LOCTEXT("SerializeWidgetTreeSuccess", "Serialized {0} widget tree(s) to {1}"), NumSaved, FText::FromString(DestinationDir)),
			SNotificationItem::CS_Success);
	}
	else
	{
		ShowNotification(LOCTEXT("SerializeWidgetTreeFailed", "Failed to serialize widget tree(s). See Log for details."), SNotificationItem::CS_Fail);
	}
}

#undef LOCTEXT_NAMESPACE
