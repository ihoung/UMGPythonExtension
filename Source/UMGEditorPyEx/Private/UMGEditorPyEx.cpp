// Copyright Epic Games, Inc. All Rights Reserved.

#include "UMGEditorPyEx.h"

#define LOCTEXT_NAMESPACE "FUMGEditorPyExModule"

void FUMGEditorPyExModule::StartupModule()
{
	// This code will execute after our module is loaded into memory; the exact timing is specified in the .uplugin file per-module
	WidgetTreeExporter = MakeUnique<FEditorWidgetTreeExporter>();
}

void FUMGEditorPyExModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
	WidgetTreeExporter.Reset();
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FUMGEditorPyExModule, UMGEditorPyEx)
