// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "EditorUtilityWidget.h"
#include "Blueprint/IUserObjectListEntry.h"
#include "PyEditorUtilityEntryWidget.generated.h"

/**
 * 
 */
UCLASS(Abstract, MinimalAPI)
class UPyEditorUtilityEntryWidget : public UEditorUtilityWidget, public IUserObjectListEntry
{
	GENERATED_BODY()

public:
	UPyEditorUtilityEntryWidget(const FObjectInitializer& ObjectInitializer);
};
