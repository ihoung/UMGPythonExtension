// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Modules/ModuleManager.h"
#include "EditorWidgetTreeExporter.h"

class FUMGEditorPyExModule : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

private:
	TUniquePtr<FEditorWidgetTreeExporter> WidgetTreeExporter;
};
