// Fill out your copyright notice in the Description page of Project Settings.

#include "EditorUtilityWidgetSubsystem.h"

#include "EditorWidgetTreeSerialization.h"
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

namespace
{
	FString NormalizeToFilename(const FString& Path, const FString& InExtension=TEXT(""))
	{
		if (Path.IsEmpty())
		{
			return Path;
		}

		FString PackageName;

		if (FPaths::IsDrive(Path) || !Path.StartsWith(TEXT("/")))
		{
			return FPaths::SetExtension(Path, InExtension);
		}

		// Treat it as an Unreal long package name.
		return FPackageName::LongPackageNameToFilename(Path, InExtension);
	}

	FString NormalizeToPackageName(const FString& Path)
	{
		FString PackageName;

		if (FPackageName::TryConvertFilenameToLongPackageName(Path, PackageName))
		{
			return PackageName;
		}

		if (!FPaths::IsDrive(Path) && Path.StartsWith(TEXT("/")))
		{
			return Path;
		}

		// Otherwise assume it is already a package-style path.
		PackageName = Path;
		PackageName.ReplaceInline(TEXT("\\"), TEXT("/"));

		// Remove whatever extension it has.
		const int32 SlashIndex = PackageName.Find(TEXT("/"), ESearchCase::IgnoreCase, ESearchDir::FromEnd);
		const int32 DotIndex = PackageName.Find(TEXT("."), ESearchCase::IgnoreCase, ESearchDir::FromEnd);

		if (DotIndex > SlashIndex)
		{
			PackageName.LeftInline(DotIndex);
		}

		return FPackageName::FilenameToLongPackageName(PackageName);
	}
}

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

UEditorUtilityWidget* UEditorUtilityWidgetSubsystem::SpawnAndRegisterTab(TSubclassOf<UEditorUtilityWidget> WidgetClass, bool bLoadSerialized)
{
	if (IsRunningCommandlet()) return nullptr;
	ensure(RegisteredWidgets.Contains(WidgetClass));

	UEditorUtilityWidgetBlueprint* TransientBP = nullptr;
	if (CreatedBlueprints.Contains(WidgetClass))
	{
		TransientBP = CreatedBlueprints[WidgetClass];
	}
	else 
	{ 
		if (!bLoadSerialized)
		{
			FString AssetPath = NormalizeToPackageName(RegisteredWidgets[WidgetClass]);
			if (AssetPath.IsEmpty()) return nullptr;

			UPackage* Package = LoadPackage(nullptr, *AssetPath, LOAD_None);
			if (Package)
			{
				UEditorUtilityWidgetBlueprint* WidgetBlueprint = FindObject<UEditorUtilityWidgetBlueprint>(Package, *FPackageName::GetShortName(*AssetPath));
				if (WidgetBlueprint)
				{
					TransientBP = CreateTransientBlueprint(WidgetClass, WidgetBlueprint);
				}
			}
		}

		if (!TransientBP)
		{
			FString SerializedWTFilePath = NormalizeToFilename(RegisteredWidgets[WidgetClass], TEXT(".json"));
			TransientBP = CreateTransientBlueprint(WidgetClass, nullptr, nullptr, SerializedWTFilePath);
		}
	}

	if (TransientBP)
	{
		UEditorUtilitySubsystem* EditorUtilitySubsystem = GEditor->GetEditorSubsystem<UEditorUtilitySubsystem>();
		return EditorUtilitySubsystem->SpawnAndRegisterTab(TransientBP);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Loading Failed: Nonexistent Path %s"), *RegisteredWidgets[WidgetClass]);
	}

	return nullptr;
}

inline UEditorUtilityWidgetBlueprint* UEditorUtilityWidgetSubsystem::CreateTransientBlueprint(TSubclassOf<UEditorUtilityWidget> InWidgetClass, UEditorUtilityWidgetBlueprint* InBlueprint, UObject* Outer, FString SerializedWidgetTreePath)
{
	if (InWidgetClass)
	{
		FName TransientBPName = FName(*FString::Printf(TEXT("%s_%s"), InBlueprint ? *InBlueprint->GetFName().ToString() : *FPackageName::GetShortName(FPaths::GetBaseFilename(SerializedWidgetTreePath)), *InWidgetClass->GetFName().ToString()));
		FString TransientBPPath = Outer ? FString::Printf(TEXT("%s:%s"), *Outer->GetPathName(), *TransientBPName.ToString()) : 
			FString::Printf(TEXT("%s.%s"), *GetTransientPackage()->GetPathName(), *TransientBPName.ToString());
		TSoftObjectPtr<UEditorUtilityWidgetBlueprint> ExistingTransientBPPtr = TSoftObjectPtr<UEditorUtilityWidgetBlueprint>(FSoftObjectPath(TransientBPPath));
		UEditorUtilityWidgetBlueprint* ExistingTransientBP = ExistingTransientBPPtr.Get();
		if (IsValid(ExistingTransientBP))
		{
			if (!Outer) CreatedBlueprints.Contains(InWidgetClass) ? CreatedBlueprints[InWidgetClass] = ExistingTransientBP : CreatedBlueprints.Add(InWidgetClass, ExistingTransientBP);
			return ExistingTransientBP;
		}

		UEditorUtilityWidgetBlueprint* TransientBP;
		if (InBlueprint)
		{
			TransientBP = Cast<UEditorUtilityWidgetBlueprint>(
				StaticDuplicateObject(InBlueprint, Outer ? Outer : GetTransientPackage(), TransientBPName, RF_Transient, UEditorUtilityWidgetBlueprint::StaticClass())
			);
			if (TransientBP) ModifyBlueprintInternalReference(TransientBP, InWidgetClass);

		}
		else
		{
			TransientBP = NewObject< UEditorUtilityWidgetBlueprint>(
				Outer ? Outer : GetTransientPackage(),
				MakeUniqueObjectName(GetTransientPackage(), UEditorUtilityWidgetBlueprint::StaticClass(), TransientBPName),
				RF_Transient
			);
			if (!FEditorWidgetTreeSerialization::LoadSerializationToWidgetBlueprint(SerializedWidgetTreePath, TransientBP, [&](FString SerializedEntryWidgetTreePath) -> UClass*
				{
					// FindRef (not operator[]) so an unregistered widget class yields an empty map instead of
					// asserting: the predicate now also runs when the original entry asset could be resolved.
					TMap<FString, TSubclassOf<UPyEditorUtilityEntryWidget>> EntryWidgetClassMap = RegisteredEntryWidgets.FindRef(InWidgetClass);
					FString RegisteredEntryWidgetBPPath = EntryWidgetClassMap.Contains(SerializedEntryWidgetTreePath) ? SerializedEntryWidgetTreePath : NormalizeToPackageName(SerializedEntryWidgetTreePath);
					if (EntryWidgetClassMap.Contains(RegisteredEntryWidgetBPPath))
					{
						TSubclassOf<UEditorUtilityWidget> RegisteredEntryWidgetClass = EntryWidgetClassMap[RegisteredEntryWidgetBPPath];
						UEditorUtilityWidgetBlueprint* TransientEntryWidgetBP = CreateTransientBlueprint(RegisteredEntryWidgetClass, nullptr, InBlueprint ? TransientBP : nullptr, SerializedEntryWidgetTreePath);
						if (TransientEntryWidgetBP)
						{	
							return TransientEntryWidgetBP->GeneratedClass;
						}
					}
					return nullptr;
				}))
			{
				UE_LOG(LogTemp, Error, TEXT("Loading Serialized Widget Tree to Transient Blueprint Failed: Path %s"), *SerializedWidgetTreePath);
				return nullptr;
			}
		}

		if (TransientBP)
		{
			TransientBP->ParentClass = InWidgetClass;
			FBlueprintEditorUtils::MarkBlueprintAsStructurallyModified(TransientBP);
			FKismetEditorUtilities::CompileBlueprint(TransientBP, EBlueprintCompileOptions::SkipGarbageCollection);

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
						FString OriginalEntryWidgetBPPath = OriginalEntryWidgetBP ? OriginalEntryWidgetBP->GetPackage()->GetPathName() : TEXT("");
						TSubclassOf<UEditorUtilityWidget> RegisteredEntryWidgetClass = EntryWidgetClassMap.Contains(OriginalEntryWidgetBPPath) ? EntryWidgetClassMap[OriginalEntryWidgetBPPath] : nullptr;
						if (RegisteredEntryWidgetClass)
						{
							UEditorUtilityWidgetBlueprint* TransientEntryWidgetBP = CreateTransientBlueprint(RegisteredEntryWidgetClass, OriginalEntryWidgetBP, InBlueprint);
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
