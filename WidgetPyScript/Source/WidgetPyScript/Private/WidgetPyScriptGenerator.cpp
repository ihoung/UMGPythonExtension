// Fill out your copyright notice in the Description page of Project Settings.


#include "WidgetPyScriptGenerator.h"
#include "Interfaces/IPluginManager.h"
#include "Misc/FileHelper.h"

using namespace tinyxml2;

void FWidgetPyScriptGenerator::GenerateWidgetXML(const FAssetData& WidgetAsset, const TArray<UWidget*>& TargetWidgets)
{
	FString PackagePath = WidgetAsset.PackagePath.ToString();
	TArray<FString> PackagePathParsedArray;
	PackagePath.ParseIntoArray(PackagePathParsedArray, TEXT("/"), true);
	FString RootPath = PackagePathParsedArray[0];
	FString AssetName = WidgetAsset.AssetName.ToString();

	// Copy dependencies
	FString DependenciesDir;
	if (RootPath.Equals(TEXT("Game")))
	{
		DependenciesDir = FPaths::Combine(FPaths::ProjectContentDir(), TEXT("Python"), TEXT("Dependencies")) + TEXT("/");
	}
	else
	{
		DependenciesDir = FPaths::Combine(IPluginManager::Get().FindPlugin(RootPath)->GetBaseDir(), TEXT("Content"), TEXT("Python"), TEXT("Dependencies")) + TEXT("/");
	}
	IPlatformFile& FileManager = FPlatformFileManager::Get().GetPlatformFile();
	if (!FileManager.DirectoryExists(*DependenciesDir))
	{
		FileManager.CreateDirectoryTree(*DependenciesDir);
	}
	FString TemplatesRoot = IPluginManager::Get().FindPlugin("WidgetPyScript")->GetBaseDir() / TEXT("Templates");
	FString DependenciesSrc = TemplatesRoot / TEXT("Dependencies");
	FileManager.CopyDirectoryTree(*DependenciesDir, *DependenciesSrc, false);

	// Find destination directory
	FString DestDirPath;
	if (RootPath.Equals(TEXT("Game")))
	{
		DestDirPath = FPaths::Combine(FPaths::ProjectContentDir(), TEXT("Python"), AssetName) + TEXT("/");
	}
	else
	{
		DestDirPath = FPaths::Combine(IPluginManager::Get().FindPlugin(RootPath)->GetBaseDir(), TEXT("Content"), TEXT("Python"), AssetName) + TEXT("/");
	}
	if (!FileManager.DirectoryExists(*DestDirPath))
	{
		FileManager.CreateDirectoryTree(*DestDirPath);
	}
	FString DestFilePath = FPaths::Combine(DestDirPath, AssetName + TEXT(".xml"));

	// Create XML File
	XMLDocument XMLDoc;
	WriteXMLHeader(XMLDoc, WidgetAsset);
	WriteXMLWidgets(XMLDoc, TargetWidgets);
	XMLDoc.SaveFile(TCHAR_TO_UTF8(*DestFilePath));

	FString ScriptExamplePath = TemplatesRoot / TEXT("example.py");
	GenerateScriptExample(ScriptExamplePath, DestDirPath, AssetName);
}

void FWidgetPyScriptGenerator::WriteXMLHeader(XMLDocument& Doc, const FAssetData& WidgetAsset)
{
	// Declaration
	XMLDeclaration* declaration = Doc.NewDeclaration();
	Doc.InsertFirstChild(declaration);

	XMLElement* header = Doc.NewElement("header");
	Doc.InsertEndChild(header);

	// Asset name
	FString AssetName = WidgetAsset.AssetName.ToString();
	XMLElement* name = header->InsertNewChildElement("asset_name");
	name->SetText(TCHAR_TO_UTF8(*AssetName));

	// Type
	FString WidgetType = WidgetAsset.AssetClass.ToString();
	XMLElement* type = header->InsertNewChildElement("asset_type");
	type->SetText(TCHAR_TO_UTF8(*WidgetType));

	// Asset path
	FString WidgetPath = WidgetAsset.ObjectPath.ToString();
	if (WidgetPath.IsEmpty())
	{
		WidgetPath = *FPaths::Combine(WidgetAsset.PackagePath.ToString(), AssetName + "." + AssetName);
	}
	XMLElement* path = header->InsertNewChildElement("asset_path");
	path->SetText(TCHAR_TO_UTF8(*WidgetPath));
}

void FWidgetPyScriptGenerator::WriteXMLWidgets(XMLDocument& Doc, const TArray<UWidget*>& TargetWidgets)
{
	XMLElement* body = Doc.NewElement("body");
	Doc.InsertEndChild(body);

	for (UWidget* TargetWidget : TargetWidgets)
	{
		XMLElement* widget = body->InsertNewChildElement("widget_element");

		FString WidgetName = TargetWidget->GetName();
		widget->SetAttribute("name", TCHAR_TO_UTF8(*WidgetName));

		FString WidgetType = TargetWidget->GetClass()->GetName();
		XMLElement* widgetType = widget->InsertNewChildElement("class");
		widgetType->SetText(TCHAR_TO_UTF8(*WidgetType));
	}
}

void FWidgetPyScriptGenerator::GenerateScriptExample(FString TemplateFilePath, FString DestDirPath, FString AssetName)
{
	IPlatformFile& FileManager = FPlatformFileManager::Get().GetPlatformFile();
	if (!FileManager.DirectoryExists(*DestDirPath))
	{
		FileManager.CreateDirectoryTree(*DestDirPath);
	}
	
	FString DestFile = DestDirPath / TEXT("example.py");
	if (FileManager.CopyFile(*DestFile, *TemplateFilePath))
	{
		FString FileContent;
		FFileHelper::LoadFileToString(FileContent, *DestFile);
		FileContent = FileContent.Replace(*FString("ExampleWidget"), *AssetName, ESearchCase::CaseSensitive);
		FFileHelper::SaveStringToFile(FileContent, *DestFile);
	}
}


