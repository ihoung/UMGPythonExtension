// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Templates/SubclassOf.h"
#include "Templates/SharedPointer.h"
#include "EditorSubsystem.h"
#include "PyEditorUtilityEntryWidget.h"
#include "EditorUtilityWidget.h"
#include "EditorUtilityWidgetBlueprint.h"
#include "Widgets/Docking/SDockTab.h"
#include "Framework/Docking/TabManager.h"
#include "EditorUtilityWidgetSubsystem.generated.h"

/**
 * 
 */
UCLASS(NotBlueprintable, MinimalAPI)
class UEditorUtilityWidgetSubsystem : public UEditorSubsystem
{
	GENERATED_BODY()
	
public:
	UEditorUtilityWidgetSubsystem();

	UFUNCTION(BlueprintCallable, Category = "Development|Editor")
	void RegisterWidgetWithBluprint(TSubclassOf<UEditorUtilityWidget> WidgetClass, FString Path);

	UFUNCTION(BlueprintCallable, Category = "Development|Editor")
	void UnregisterWidget(TSubclassOf<UEditorUtilityWidget> WidgetClass);

	UFUNCTION(BlueprintCallable, Category = "Development|Editor")
	void RegisterEntryWidgetClasses(TSubclassOf<UEditorUtilityWidget> WidgetClass, TMap<FString, TSubclassOf<UPyEditorUtilityEntryWidget>> EntryWidgetClasses);

	UFUNCTION(BlueprintCallable, Category = "Development|Editor")
	UEditorUtilityWidget* SpawnAndRegisterTab(TSubclassOf<UEditorUtilityWidget> WidgetClass, bool bLoadSerialized=false);

protected:
	TMap<TSubclassOf<UEditorUtilityWidget>, TMap<FString, TSubclassOf<UPyEditorUtilityEntryWidget>>> RegisteredEntryWidgets;
	TMap<TSubclassOf<UEditorUtilityWidget>, FString> RegisteredWidgets;

private:
	inline UEditorUtilityWidgetBlueprint* CreateTransientBlueprint(TSubclassOf<UEditorUtilityWidget> InWidgetClass, UEditorUtilityWidgetBlueprint* InBlueprint, UObject* Outer=nullptr, FString SerializedWidgetTreePath=TEXT(""));

	inline void ModifyBlueprintInternalReference(UEditorUtilityWidgetBlueprint* InBlueprint, TSubclassOf<UEditorUtilityWidget> InWidgetClass);

	TMap<TSubclassOf<UEditorUtilityWidget>, UEditorUtilityWidgetBlueprint*> CreatedBlueprints;
};
