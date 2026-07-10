// Fill out your copyright notice in the Description page of Project Settings.

#include "EditorUtilityWidgetSubsystem.h"

#include "Blueprint/UserWidget.h"
#include "Blueprint/WidgetTree.h"
#include "LevelEditor.h"
#include "Misc/PackageName.h"
#include "IBlutilityModule.h"
#include "EditorUtilitySubsystem.h"
#include "Components/ListView.h"
#include "Serialization/ArchiveUObject.h"
#include "Kismet2/KismetEditorUtilities.h"
#include "Kismet2/blueprintEditorUtils.h"


UEditorUtilityWidgetSubsystem::UEditorUtilityWidgetSubsystem()
	: UEditorSubsystem()
{

}

void UEditorUtilityWidgetSubsystem::RegisterWidgetWithBluprint(TSubclassOf<UEditorUtilityWidget> WidgetClass, FString BlueprintPath)
{
	if (RegisteredWidgets.Contains(WidgetClass))
	{
		RegisteredWidgets[WidgetClass] = BlueprintPath;
	}
	else
	{
		RegisteredWidgets.Add(WidgetClass, BlueprintPath);
	}

	if (RegisteredEntryWidgets.Contains(WidgetClass))
	{
		RegisteredEntryWidgets.Remove(WidgetClass);
	}
}

void UEditorUtilityWidgetSubsystem::UnregisterWidget(TSubclassOf<UEditorUtilityWidget> WidgetClass)
{
	if (RegisteredWidgets.Contains(WidgetClass))
	{
		RegisteredWidgets.Remove(WidgetClass);

		if (RegisteredEntryWidgets.Contains(WidgetClass))
		{
			RegisteredEntryWidgets.Remove(WidgetClass);
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Widget Class has never been registered: %s"), *(WidgetClass->GetFName().ToString()));
	}
}

void UEditorUtilityWidgetSubsystem::RegisterEntryWidgetClasses(TSubclassOf<UEditorUtilityWidget> WidgetClass, TMap<FString, TSubclassOf<UPyEditorUtilityEntryWidget>> EntryWidgetClasses)
{
	if (RegisteredEntryWidgets.Contains(WidgetClass))
	{
		RegisteredEntryWidgets[WidgetClass] = EntryWidgetClasses;
	}
	else
	{
		RegisteredEntryWidgets.Add(WidgetClass, EntryWidgetClasses);
	}
}

UEditorUtilityWidget* UEditorUtilityWidgetSubsystem::SpawnAndRegisterTab(TSubclassOf<UEditorUtilityWidget> WidgetClass)
{
	if (IsRunningCommandlet()) return nullptr;
	ensure(RegisteredWidgets.Contains(WidgetClass));

	FString AssetPath = RegisteredWidgets[WidgetClass];
	if (AssetPath.IsEmpty()) return nullptr;

	UPackage* Package = LoadPackage(nullptr, *AssetPath, LOAD_None);
	if (!Package)
	{
		UE_LOG(LogTemp, Error, TEXT("Loading Failed: Nonexistent Path %s"), *AssetPath);
		return nullptr;
	}
	UEditorUtilityWidgetBlueprint* WidgetBlueprint = FindObject<UEditorUtilityWidgetBlueprint>(Package, *FPackageName::GetShortName(*AssetPath));
	if (!WidgetBlueprint)
	{
		UE_LOG(LogTemp, Error, TEXT("Loading Failed: Nonexistent Path %s"), *AssetPath);
		return nullptr;
	}

	UEditorUtilityWidgetBlueprint* TransientBP = CreatedBlueprints.Contains(WidgetClass) ? CreatedBlueprints[WidgetClass] : CreateTransientBlueprint(WidgetBlueprint, WidgetClass);
	if (TransientBP)
	{
		UEditorUtilitySubsystem* EditorUtilitySubsystem = GEditor->GetEditorSubsystem<UEditorUtilitySubsystem>();
		return EditorUtilitySubsystem->SpawnAndRegisterTab(TransientBP);
	}

	return nullptr;
}

inline UEditorUtilityWidgetBlueprint* UEditorUtilityWidgetSubsystem::CreateTransientBlueprint(UEditorUtilityWidgetBlueprint* InBlueprint, TSubclassOf<UEditorUtilityWidget> InWidgetClass, UObject* Outer)
{
	if (InBlueprint && InWidgetClass)
	{
		FName TransientBPName = FName(*FString::Printf(TEXT("%s_%s"), *InBlueprint->GetFName().ToString(), *InWidgetClass->GetFName().ToString()));
		FString TransientBPPath = Outer ? FString::Printf(TEXT("%s:%s"), *Outer->GetPathName(), *TransientBPName.ToString()) : 
			FString::Printf(TEXT("%s.%s"), *GetTransientPackage()->GetPathName(), *TransientBPName.ToString());
		TSoftObjectPtr<UEditorUtilityWidgetBlueprint> ExistingTransientBPPtr = TSoftObjectPtr<UEditorUtilityWidgetBlueprint>(FSoftObjectPath(TransientBPPath));
		UEditorUtilityWidgetBlueprint* ExistingTransientBP = ExistingTransientBPPtr.Get();
		if (IsValid(ExistingTransientBP))
		{
			if (!Outer) CreatedBlueprints.Contains(InWidgetClass) ? CreatedBlueprints[InWidgetClass] = ExistingTransientBP : CreatedBlueprints.Add(InWidgetClass, ExistingTransientBP);
			return ExistingTransientBP;
		}

		UEditorUtilityWidgetBlueprint* TransientBP = Cast<UEditorUtilityWidgetBlueprint>(
			StaticDuplicateObject(InBlueprint, Outer ? Outer : GetTransientPackage(), TransientBPName, RF_Transient, UEditorUtilityWidgetBlueprint::StaticClass())
		);

		if (TransientBP)
		{
			ModifyBlueprintInternalReference(TransientBP, InWidgetClass);

			TransientBP->ParentClass = InWidgetClass;
			FBlueprintEditorUtils::MarkBlueprintAsStructurallyModified(TransientBP);
			FKismetEditorUtilities::CompileBlueprint(TransientBP, EBlueprintCompileOptions::SkipGarbageCollection);

			FString InBPPath = InBlueprint->GetPackage()->GetPathName();

			return TransientBP;
		}
	}

	return nullptr;
}

inline void UEditorUtilityWidgetSubsystem::ModifyBlueprintInternalReference(UEditorUtilityWidgetBlueprint* InBlueprint, TSubclassOf<UEditorUtilityWidget> InWidgetClass)
{
	if (!InBlueprint) return;

	if (RegisteredEntryWidgets.Contains(InWidgetClass))
	{
		TMap<FString, TSubclassOf<UPyEditorUtilityEntryWidget>> EntryWidgetClassMap = RegisteredEntryWidgets[InWidgetClass];
		UWidgetTree* BPWidgetTree = InBlueprint->WidgetTree;
		BPWidgetTree->ForEachWidgetAndDescendants([this, InBlueprint, &EntryWidgetClassMap](UWidget* Content)
			{
				if (UListView* ListViewContent = Cast<UListView>(Content))
				{
					if (TSubclassOf<UUserWidget> OriginalEntryWidgetClass = ListViewContent->GetEntryWidgetClass())
					{
						UEditorUtilityWidgetBlueprint* OriginalEntryWidgetBP = Cast<UEditorUtilityWidgetBlueprint>(OriginalEntryWidgetClass->ClassGeneratedBy);
						FString OriginalEntryWidgetBPPath = OriginalEntryWidgetBP ? OriginalEntryWidgetBP->GetPackage()->GetPathName() : FString();
						TSubclassOf<UEditorUtilityWidget> RegisteredEntryWidgetClass = EntryWidgetClassMap.Contains(OriginalEntryWidgetBPPath) ? EntryWidgetClassMap[OriginalEntryWidgetBPPath] : nullptr;
						if (RegisteredEntryWidgetClass)
						{
							UEditorUtilityWidgetBlueprint* TransientEntryWidgetBP = CreateTransientBlueprint(OriginalEntryWidgetBP, RegisteredEntryWidgetClass, InBlueprint);
							if (TransientEntryWidgetBP)
							{
								FProperty* Property = ListViewContent->GetClass()->FindPropertyByName(TEXT("EntryWidgetClass"));
								if (FClassProperty* ClassProperty = CastField<FClassProperty>(Property))
								{
									void* VariablePtr = ClassProperty->ContainerPtrToValuePtr<void>(ListViewContent);
									if (TSubclassOf<UUserWidget> TransientEntryWidgetBGClass = Cast<UClass>(TransientEntryWidgetBP->GeneratedClass))
									{
										ClassProperty->SetPropertyValue(VariablePtr, TransientEntryWidgetBGClass);
									}
								}
							}
						}
					}
				}
			}
		);
	}
}
