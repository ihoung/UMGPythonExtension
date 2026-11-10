// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Templates/SubclassOf.h"
#include "Templates/SharedPointer.h"
#include "EditorSubsystem.h"
#include "EditorUtilityWidget.h"
#include "EditorUtilityWidgetBlueprint.h"
#include "Components/ListView.h"
#include "UObject/StrongObjectPtr.h"
#include "EditorUtilityWidgetSubsystem.generated.h"

/**
 *
 */
UENUM(NotBlueprintable)
enum class EEditorUtilityWidgetSpawnMode : uint8
{
	MultiSpawn = 0,
	Respawn = 1,
	KeepIfExists = 2,
};


UCLASS(NotBlueprintable, MinimalAPI)
class UEditorUtilityWidgetSubsystem : public UEditorSubsystem
{
	GENERATED_BODY()

public:
	UEditorUtilityWidgetSubsystem();

	UFUNCTION(BlueprintCallable, Category = "Development|Editor")
	void RegisterWidgetWithBlueprint(TSubclassOf<UEditorUtilityWidget> WidgetClass, FString Path, bool bKeepBpVariables = false);

	UFUNCTION(BlueprintCallable, Category = "Development|Editor")
	void RegisterWidgetsWithBlueprints(TMap<TSubclassOf<UEditorUtilityWidget>, FString> WidgetClassBpMap, bool bKeepBpVariables = false);

	UFUNCTION(BlueprintCallable, Category = "Development|Editor")
	void UnregisterWidget(TSubclassOf<UEditorUtilityWidget> WidgetClass);

	UFUNCTION(BlueprintCallable, Category = "Development|Editor")
	void UnregisterAllWidgets();

	UFUNCTION(BlueprintCallable, Category = "Development|Editor")
	void RegisterReferencedWidgetClasses(TSubclassOf<UEditorUtilityWidget> WidgetClass, TArray<TSubclassOf<UEditorUtilityWidget>> ReferencedWidgetClasses);

	UFUNCTION(BlueprintCallable, Category = "Development|Editor")
	UEditorUtilityWidget* SpawnAndRegisterTab(TSubclassOf<UEditorUtilityWidget> WidgetClass, FText Title = FText(), EEditorUtilityWidgetSpawnMode SpawnMode = EEditorUtilityWidgetSpawnMode::Respawn, bool bLoadSerialized = false);

protected:
	TMap<TSubclassOf<UEditorUtilityWidget>, TMap<FString, TSubclassOf<UEditorUtilityWidget>>> RegisteredReferencedWidgets;
	TMap<TSubclassOf<UEditorUtilityWidget>, FString> RegisteredWidgets;
	TMap<TSubclassOf<UEditorUtilityWidget>, bool> RegisteredWidgetBPVariablesCache;

private:
	UEditorUtilityWidgetBlueprint* CreateTransientBlueprint(TSubclassOf<UEditorUtilityWidget> InWidgetClass, UEditorUtilityWidgetBlueprint* InBlueprint, FString SerializedWidgetTreePath = TEXT(""));

	inline void ModifyBlueprintInternalReference(UEditorUtilityWidgetBlueprint* InBlueprint, TSubclassOf<UEditorUtilityWidget> InWidgetClass);

	inline void ModifyEntryWidgetReference(UListView* ListView, const TMap<FString, TSubclassOf<UEditorUtilityWidget>>& ReferencedWidgetClassMap);

	inline void ModifyNamedSlotReference(UWidget* Content, const TMap<FString, TSubclassOf<UEditorUtilityWidget>>& ReferencedWidgetClassMap);

	inline void SetTabSpawnerEntryDisplayName(const FName& TabId, const FText& DisplayName);

	UPROPERTY()
	TMap<TSubclassOf<UEditorUtilityWidget>, TObjectPtr<UEditorUtilityWidgetBlueprint>> CreatedBlueprints;
};
