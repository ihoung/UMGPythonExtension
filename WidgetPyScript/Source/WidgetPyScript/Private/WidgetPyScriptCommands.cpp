// Copyright Epic Games, Inc. All Rights Reserved.

#include "WidgetPyScriptCommands.h"

#define LOCTEXT_NAMESPACE "FWidgetPyScriptModule"

void FWidgetPyScriptCommands::RegisterCommands()
{
	UI_COMMAND(PluginAction, "Generate Py", "Generate Python scripts based on UMG variables in Designer", EUserInterfaceActionType::Button, FInputGesture());
}

#undef LOCTEXT_NAMESPACE
