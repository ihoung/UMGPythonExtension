// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Framework/Commands/Commands.h"
#include "WidgetPyScriptStyle.h"

class FWidgetPyScriptCommands : public TCommands<FWidgetPyScriptCommands>
{
public:

	FWidgetPyScriptCommands()
		: TCommands<FWidgetPyScriptCommands>(TEXT("WidgetPyScript"), NSLOCTEXT("Contexts", "WidgetPyScript", "WidgetPyScript Plugin"), NAME_None, FWidgetPyScriptStyle::GetStyleSetName())
	{
	}

	// TCommands<> interface
	virtual void RegisterCommands() override;

public:
	TSharedPtr< FUICommandInfo > PluginAction;
};
