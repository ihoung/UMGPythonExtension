// Fill out your copyright notice in the Description page of Project Settings.


#include "EditorWidgetTreeExporterStyle.h"
#include "Interfaces/IPluginManager.h"

//#include "Styling/SlateTypes.h"
//#include "Styling/CoreStyle.h"
//#include "Styling/AppStyle.h"
#include "Styling/SlateStyleMacros.h"

FName FEditorWidgetTreeExporterStyle::StyleName("EditorWidgetTreeExporterStyle");

FEditorWidgetTreeExporterStyle& FEditorWidgetTreeExporterStyle::Get()
{
	static FEditorWidgetTreeExporterStyle Inst;
	return Inst;
}

FEditorWidgetTreeExporterStyle::FEditorWidgetTreeExporterStyle()
	: FSlateStyleSet(StyleName)
{
	const FVector2D IconSize(16.0f, 16.0f);

	SetContentRoot(IPluginManager::Get().FindPlugin("CGPythonExtension")->GetBaseDir() / TEXT("Resources"));

	Set("Icons.WidgetTreeSerialization", new IMAGE_BRUSH("JsonSerialization", IconSize));

	FSlateStyleRegistry::RegisterSlateStyle(*this);
}

FEditorWidgetTreeExporterStyle::~FEditorWidgetTreeExporterStyle()
{
	FSlateStyleRegistry::UnRegisterSlateStyle(*this);
}
