// Fill out your copyright notice in the Description page of Project Settings.


#include "WidgetPyScriptToolkit.h"
#include "WidgetPyScriptStyle.h"
#include "WidgetPyScriptCommands.h"
#include "WidgetPyScriptFunctionLibrary.h"
#include "Misc/MessageDialog.h"
#include "ToolMenus.h"
#include "ContentBrowserModule.h"
#include "IContentBrowserSingleton.h"
#include "EditorUtilityWidgetBlueprint.h"

#define LOCTEXT_NAMESPACE "FWidgetPyScriptModule"

FWidgetPyScriptToolkit::FWidgetPyScriptToolkit()
{
	FWidgetPyScriptStyle::Initialize();
	FWidgetPyScriptStyle::ReloadTextures();

	FWidgetPyScriptCommands::Register();

	PluginCommands = MakeShareable(new FUICommandList);

	PluginCommands->MapAction(
		FWidgetPyScriptCommands::Get().PluginAction,
		FExecuteAction::CreateRaw(this, &FWidgetPyScriptToolkit::PluginButtonClicked),
		FCanExecuteAction());

	UToolMenus::RegisterStartupCallback(FSimpleMulticastDelegate::FDelegate::CreateRaw(this, &FWidgetPyScriptToolkit::RegisterMenus));
}

FWidgetPyScriptToolkit::~FWidgetPyScriptToolkit()
{
	UToolMenus::UnRegisterStartupCallback(this);

	UToolMenus::UnregisterOwner(this);

	FWidgetPyScriptStyle::Shutdown();

	FWidgetPyScriptCommands::Unregister();
}

void FWidgetPyScriptToolkit::PluginButtonClicked()
{
	FText DialogText = FText(
		LOCTEXT("PluginButtonDialogText",
			"Not supported yet.\n Please locate the asset in Content Browser and generate Python scripts from RMB clicked menu.")
	);
	FMessageDialog::Open(EAppMsgType::Ok, DialogText);
}

void FWidgetPyScriptToolkit::RegisterMenus()
{
	// Owner will be used for cleanup in call to UToolMenus::UnregisterOwner
	FToolMenuOwnerScoped OwnerScoped(this);

	//{
	//	UToolMenu* Menu = UToolMenus::Get()->ExtendMenu("LevelEditor.MainMenu.Window");
	//	{
	//		FToolMenuSection& Section = Menu->FindOrAddSection("WindowLayout");
	//		Section.AddMenuEntryWithCommandList(FWidgetPyScriptCommands::Get().PluginAction, PluginCommands);
	//	}
	//}

	{
		UToolMenu* ToolbarMenu = UToolMenus::Get()->ExtendMenu("AssetEditor.BlueprintEditor.ToolBar.DesignerName");
		{
			FToolMenuSection& Section = ToolbarMenu->FindOrAddSection("Compile");
			{
				FToolMenuEntry& Entry = Section.AddEntry(FToolMenuEntry::InitToolBarButton(FWidgetPyScriptCommands::Get().PluginAction));
				Entry.SetCommandList(PluginCommands);
			}
		}
	}

	FContentBrowserModule& ContentBrowserModule = FModuleManager::LoadModuleChecked<FContentBrowserModule>(TEXT("ContentBrowser"));
	{
		TArray<FContentBrowserMenuExtender_SelectedAssets>& CBMenuAssetExtenderDelegates = ContentBrowserModule.GetAllAssetViewContextMenuExtenders();
		CBMenuAssetExtenderDelegates.Add(FContentBrowserMenuExtender_SelectedAssets::CreateRaw(this, &FWidgetPyScriptToolkit::AddCBMenuExtender));
	}
}

TSharedRef<FExtender> FWidgetPyScriptToolkit::AddCBMenuExtender(const TArray<FAssetData>& SelectedAssets)
{
	TSharedRef<FExtender> Extender = MakeShared<FExtender>();

	bool HasWidgetAsset = false;
	for (auto AssetIt = SelectedAssets.CreateConstIterator(); AssetIt; ++AssetIt)
	{
		const FAssetData& Asset = *AssetIt;
		if (!Asset.IsRedirector() && Asset.AssetClass != NAME_Class && !(Asset.PackageFlags & PKG_FilterEditorOnly))
		{
			if (Asset.GetClass()->IsChildOf(UEditorUtilityWidgetBlueprint::StaticClass()))
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
			PluginCommands,
			FMenuExtensionDelegate::CreateLambda([&](FMenuBuilder& MenuBuilder)
				{
					MenuBuilder.BeginSection("Python", LOCTEXT("ASSET_CONTEXT", "Python"));
					{
						// Add Menu Entry Here
						MenuBuilder.AddMenuEntry(
							LOCTEXT("GeneratePy", "Generate Python Scripts"),
							LOCTEXT("GeneratePy ToolTip", "Generate Python Scripts for the associated Widget"),
							FSlateIcon(FWidgetPyScriptStyle::GetStyleSetName(), "WidgetPyScript.MenuIcon"),
							FUIAction(FExecuteAction::CreateRaw(this, &FWidgetPyScriptToolkit::AddCBMenuAction)),
							NAME_None,
							EUserInterfaceActionType::Button
						);
					}
					MenuBuilder.EndSection();
				}
			)
		);
	}

	return Extender;
}

void FWidgetPyScriptToolkit::AddCBMenuAction()
{
	FContentBrowserModule& ContentBrowserModule = FModuleManager::LoadModuleChecked<FContentBrowserModule>(TEXT("ContentBrowser"));
	TArray<FAssetData> SelectedAssets;
	IContentBrowserSingleton& ContentBrowserSingleton = ContentBrowserModule.Get();
	ContentBrowserSingleton.GetSelectedAssets(SelectedAssets);
	UWidgetPyScriptFunctionLibrary::GeneratePyFromWidgets(SelectedAssets);
}

#undef LOCTEXT_NAMESPACE
