// Fill out your copyright notice in the Description page of Project Settings.

#include "EditorUtilityWidgetSubsystem.h"

#include "EditorWidgetTreeSerialization.h"
#include "Blueprint/UserWidget.h"
#include "Blueprint/WidgetTree.h"
#include "LevelEditor.h"
#include "Misc/PackageName.h"
#include "IBlutilityModule.h"
#include "EditorUtilitySubsystem.h"
#include "Components/NamedSlotInterface.h"
#include "Components/PanelSlot.h"
#include "Serialization/ArchiveUObject.h"
#include "Dom/JsonObject.h"
#include "Kismet2/KismetEditorUtilities.h"
#include "Kismet2/blueprintEditorUtils.h"
#include "UObject/StrongObjectPtr.h"
#include "UObject/UnrealType.h"
#include "WidgetBlueprintEditorUtils.h"

namespace
{
	FString NormalizeToFilename(const FString& Path, const FString& InExtension = TEXT(""))
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

	// Called before reparenting to a Python wrapper class: discards blueprint-authored members
	// (variables, function/event graphs, timelines, property bindings) and keeps only the
	// WidgetTree and its derived content, so no name conflicts with Python class members survive
	// into the compile.
	void StripBlueprintAuthoredMembers(UBlueprint* InBlueprint, bool bKeepVariables = false)
	{
		if (!InBlueprint)
		{
			return;
		}

		// Member variables (each compiles into a same-named FProperty; primary conflict source)
		if (!bKeepVariables) InBlueprint->NewVariables.Reset();
		// Timelines (also compile into same-named properties)
		InBlueprint->Timelines.Reset();
		// Function graphs and ubergraph pages (including delegate signature graphs;
		// the cleared graph objects lose their references and get collected by GC)
		InBlueprint->FunctionGraphs.Reset();
		InBlueprint->UbergraphPages.Reset();
		// UMG property bindings reference the removed binding functions, clear them as well
		if (UWidgetBlueprint* WidgetBlueprint = Cast<UWidgetBlueprint>(InBlueprint))
		{
			WidgetBlueprint->Bindings.Reset();
		}
	}
}

UEditorUtilityWidgetSubsystem::UEditorUtilityWidgetSubsystem()
	: UEditorSubsystem()
{

}

void UEditorUtilityWidgetSubsystem::RegisterWidgetWithBlueprint(TSubclassOf<UEditorUtilityWidget> WidgetClass, FString BlueprintPath, bool bKeepBpVariables)
{
	if (CreatedBlueprints.Contains(WidgetClass))
	{
		CreatedBlueprints.Remove(WidgetClass);
	}

	if (RegisteredReferencedWidgets.Contains(WidgetClass))
	{
		RegisteredReferencedWidgets.Remove(WidgetClass);
	}

	if (RegisteredWidgets.Contains(WidgetClass))
	{
		RegisteredWidgets[WidgetClass] = BlueprintPath;
		RegisteredWidgetBPVariablesCache[WidgetClass] = bKeepBpVariables;
	}
	else
	{
		RegisteredWidgets.Add(WidgetClass, BlueprintPath);
		RegisteredWidgetBPVariablesCache.Add(WidgetClass, bKeepBpVariables);
	}
}

void UEditorUtilityWidgetSubsystem::RegisterWidgetsWithBlueprints(TMap<TSubclassOf<UEditorUtilityWidget>, FString> WidgetClassBpMap, bool bKeepBpVariables)
{
	for (auto Pair : WidgetClassBpMap)
	{
		if (CreatedBlueprints.Contains(Pair.Key))
		{
			CreatedBlueprints.Remove(Pair.Key);
		}

		if (RegisteredReferencedWidgets.Contains(Pair.Key))
		{
			RegisteredReferencedWidgets.Remove(Pair.Key);
		}

		if (RegisteredWidgets.Contains(Pair.Key))
		{
			RegisteredWidgets[Pair.Key] = Pair.Value;
			RegisteredWidgetBPVariablesCache[Pair.Key] = bKeepBpVariables;
		}
		else
		{
			RegisteredWidgets.Add(Pair.Key, Pair.Value);
			RegisteredWidgetBPVariablesCache.Add(Pair.Key, bKeepBpVariables);
		}
	}
}

void UEditorUtilityWidgetSubsystem::UnregisterWidget(TSubclassOf<UEditorUtilityWidget> WidgetClass)
{
	if (RegisteredWidgets.Contains(WidgetClass))
	{
		if (CreatedBlueprints.Contains(WidgetClass))
		{
			CreatedBlueprints.Remove(WidgetClass);
		}

		if (RegisteredReferencedWidgets.Contains(WidgetClass))
		{
			RegisteredReferencedWidgets.Remove(WidgetClass);
		}

		if (RegisteredWidgetBPVariablesCache.Contains(WidgetClass))
		{
			RegisteredWidgetBPVariablesCache.Remove(WidgetClass);
		}

		RegisteredWidgets.Remove(WidgetClass);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Widget Class has never been registered: %s"), *(WidgetClass->GetFName().ToString()));
	}
}

void UEditorUtilityWidgetSubsystem::UnregisterAllWidgets()
{
	CreatedBlueprints.Reset();
	RegisteredReferencedWidgets.Reset();
	RegisteredWidgetBPVariablesCache.Reset();
	RegisteredWidgets.Reset();
}

void UEditorUtilityWidgetSubsystem::RegisterReferencedWidgetClasses(TSubclassOf<UEditorUtilityWidget> WidgetClass, TArray<TSubclassOf<UEditorUtilityWidget>> ReferencedWidgetClasses)
{
	TMap<FString, TSubclassOf<UEditorUtilityWidget>> ReferencedWidgetClassMap;
	for (auto ReferencedWidgetClass : ReferencedWidgetClasses)
	{
		if (RegisteredWidgets.Contains(ReferencedWidgetClass))
		{
			ReferencedWidgetClassMap.Add(RegisteredWidgets[ReferencedWidgetClass], ReferencedWidgetClass);
		}
	}
	if (RegisteredReferencedWidgets.Contains(WidgetClass))
	{
		RegisteredReferencedWidgets[WidgetClass] = ReferencedWidgetClassMap;
	}
	else
	{
		RegisteredReferencedWidgets.Add(WidgetClass, ReferencedWidgetClassMap);
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
		if (!IsValid(TransientBP))
		{
			CreatedBlueprints.Remove(WidgetClass);
			TransientBP = nullptr;
		}
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
			TransientBP = CreateTransientBlueprint(WidgetClass, nullptr, SerializedWTFilePath);
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

inline UEditorUtilityWidgetBlueprint* UEditorUtilityWidgetSubsystem::CreateTransientBlueprint(TSubclassOf<UEditorUtilityWidget> InWidgetClass, UEditorUtilityWidgetBlueprint* InBlueprint, FString SerializedWidgetTreePath)
{
	if (InWidgetClass)
	{
		FName TransientBPName = FName(*FString::Printf(TEXT("%s_%s"), InBlueprint ? *InBlueprint->GetFName().ToString() : *FPackageName::GetShortName(FPaths::GetBaseFilename(SerializedWidgetTreePath)), *InWidgetClass->GetFName().ToString()));
		FString TransientBPPath = FString::Printf(TEXT("%s.%s"), *GetTransientPackage()->GetPathName(), *TransientBPName.ToString());

		UEditorUtilityWidgetBlueprint* TransientBP;
		TStrongObjectPtr<UEditorUtilityWidgetBlueprint> TransientBPGuard;
		// The widget class's own properties and the member variables' default values only find a CDO once the
		// Blueprint is compiled below: they are parsed during the load and applied after the compile
		// (see LoadWidgetClassPropertiesToCDO / ApplyVariableDefaultValuesToCDO).
		TSharedPtr<FJsonObject> PendingWidgetClassProperties;
		TMap<FName, FString> PendingVariableDefaultValues;
		if (InBlueprint)
		{
			FBlueprintDuplicationScopeFlags DuplicationScopeFlags(FBlueprintDuplicationScopeFlags::NoExtraCompilation);
			TransientBP = Cast<UEditorUtilityWidgetBlueprint>(
				StaticDuplicateObject(InBlueprint, GetTransientPackage(), TransientBPName, RF_Transient, UEditorUtilityWidgetBlueprint::StaticClass())
			);
			TransientBPGuard = TStrongObjectPtr<UEditorUtilityWidgetBlueprint>(TransientBP);
			CreatedBlueprints.Contains(InWidgetClass) ? CreatedBlueprints[InWidgetClass] = TransientBP : CreatedBlueprints.Add(InWidgetClass, TransientBP);
			if (TransientBP) ModifyBlueprintInternalReference(TransientBP, InWidgetClass);

		}
		else
		{
			TransientBP = NewObject< UEditorUtilityWidgetBlueprint>(
				GetTransientPackage(),
				MakeUniqueObjectName(GetTransientPackage(), UEditorUtilityWidgetBlueprint::StaticClass(), TransientBPName),
				RF_Transient
			);
			TransientBPGuard = TStrongObjectPtr<UEditorUtilityWidgetBlueprint>(TransientBP);
			CreatedBlueprints.Contains(InWidgetClass) ? CreatedBlueprints[InWidgetClass] = TransientBP : CreatedBlueprints.Add(InWidgetClass, TransientBP);

			if (!FEditorWidgetTreeSerialization::LoadSerializationToWidgetBlueprint(SerializedWidgetTreePath, TransientBP, 
				RegisteredWidgetBPVariablesCache.Contains(InWidgetClass) ? RegisteredWidgetBPVariablesCache[InWidgetClass] : false,
				[&](FString ReferencedSerializedPath) -> UClass*
				{
					// FindRef (not operator[]) so an unregistered widget class yields an empty map instead of
					// asserting: the predicate now also runs when the original entry asset could be resolved.
					TMap<FString, TSubclassOf<UEditorUtilityWidget>> ReferencedWidgetClassMap = RegisteredReferencedWidgets.FindRef(InWidgetClass);
					FString RegisteredRefWidgetBPPath = ReferencedWidgetClassMap.Contains(ReferencedSerializedPath) ? ReferencedSerializedPath : NormalizeToPackageName(ReferencedSerializedPath);
					if (ReferencedWidgetClassMap.Contains(RegisteredRefWidgetBPPath))
					{
						TSubclassOf<UEditorUtilityWidget> RegisteredRefWidgetClass = ReferencedWidgetClassMap[RegisteredRefWidgetBPPath];
						UEditorUtilityWidgetBlueprint* TransientEntryWidgetBP = CreatedBlueprints.Contains(RegisteredRefWidgetClass) ? 
							CreatedBlueprints[RegisteredRefWidgetClass].Get() : CreateTransientBlueprint(RegisteredRefWidgetClass, nullptr, ReferencedSerializedPath);
						if (TransientEntryWidgetBP)
						{
							return TransientEntryWidgetBP->GeneratedClass;
						}
					}
					return nullptr;
				},
				&PendingWidgetClassProperties,
				&PendingVariableDefaultValues))
			{
				UE_LOG(LogTemp, Error, TEXT("Loading Serialized Widget Tree to Transient Blueprint Failed: Path %s"), *SerializedWidgetTreePath);
				return nullptr;
			}
		}

		if (TransientBP)
		{
			if (InBlueprint)
			{
				StripBlueprintAuthoredMembers(TransientBP, RegisteredWidgetBPVariablesCache.Contains(InWidgetClass) ? RegisteredWidgetBPVariablesCache[InWidgetClass] : false);
			}
			TransientBP->ParentClass = InWidgetClass;
			FBlueprintEditorUtils::MarkBlueprintAsStructurallyModified(TransientBP);
			FKismetEditorUtilities::CompileBlueprint(TransientBP, EBlueprintCompileOptions::SkipGarbageCollection);

			if (!IsValid(TransientBP) || TransientBP->Status == BS_Error) {
				CreatedBlueprints.Remove(InWidgetClass);
				return nullptr;
			}

			// The widget class's own properties, now that the compiled Blueprint has a CDO to hold them.
			if (PendingWidgetClassProperties.IsValid())
			{
				FEditorWidgetTreeSerialization::LoadWidgetClassPropertiesToCDO(PendingWidgetClassProperties, TransientBP);
			}
			// The member variables' default values, now that the compiled Blueprint has a CDO to hold them
			// (see ApplyVariableDefaultValuesToCDO).
			if (!PendingVariableDefaultValues.IsEmpty())
			{
				FEditorWidgetTreeSerialization::ApplyVariableDefaultValuesToCDO(PendingVariableDefaultValues, TransientBP);
			}
			return TransientBP;
		}
	}

	return nullptr;
}

inline void UEditorUtilityWidgetSubsystem::ModifyBlueprintInternalReference(UEditorUtilityWidgetBlueprint* InBlueprint, TSubclassOf<UEditorUtilityWidget> InWidgetClass)
{
	if (!InBlueprint) return;

	if (RegisteredReferencedWidgets.Contains(InWidgetClass))
	{
		TMap<FString, TSubclassOf<UEditorUtilityWidget>> ReferencedWidgetClassMap = RegisteredReferencedWidgets[InWidgetClass];
		UWidgetTree* BPWidgetTree = InBlueprint->WidgetTree;
		if (!BPWidgetTree) return;
		BPWidgetTree->ForEachWidgetAndDescendants([this, InBlueprint, &ReferencedWidgetClassMap](UWidget* Content)
		{
			UClass* ContentClass = Content->GetClass();
			if (ContentClass->GetPathName().EndsWith("_C") && ReferencedWidgetClassMap.Contains(ContentClass->GetPackage()->GetPathName()))
			{
				ModifyNamedSlotReference(Content, ReferencedWidgetClassMap);
			}
			else if (UListView* ListViewContent = Cast<UListView>(Content))
			{
				ModifyEntryWidgetReference(ListViewContent, ReferencedWidgetClassMap);
			}
		});
	}
}

inline void UEditorUtilityWidgetSubsystem::ModifyEntryWidgetReference(UListView* ListView, const TMap<FString, TSubclassOf<UEditorUtilityWidget>>& ReferencedWidgetClassMap)
{
	if (TSubclassOf<UUserWidget> OriginalEntryWidgetClass = ListView->GetEntryWidgetClass())
	{
		UEditorUtilityWidgetBlueprint* OriginalEntryWidgetBP = Cast<UEditorUtilityWidgetBlueprint>(OriginalEntryWidgetClass->ClassGeneratedBy);
		FString OriginalEntryWidgetBPPath = OriginalEntryWidgetBP ? OriginalEntryWidgetBP->GetPackage()->GetPathName() : TEXT("");
		TSubclassOf<UEditorUtilityWidget> RegisteredEntryWidgetClass = ReferencedWidgetClassMap.Contains(OriginalEntryWidgetBPPath) ? ReferencedWidgetClassMap[OriginalEntryWidgetBPPath] : nullptr;
		if (RegisteredEntryWidgetClass)
		{
			UEditorUtilityWidgetBlueprint* TransientEntryWidgetBP = CreatedBlueprints.Contains(RegisteredEntryWidgetClass) ?
				CreatedBlueprints[RegisteredEntryWidgetClass].Get() : CreateTransientBlueprint(RegisteredEntryWidgetClass, OriginalEntryWidgetBP);
			if (IsValid(TransientEntryWidgetBP) && TransientEntryWidgetBP->Status != BS_Error)
			{
				FProperty* Property = ListView->GetClass()->FindPropertyByName(TEXT("EntryWidgetClass"));
				if (FClassProperty* ClassProperty = CastField<FClassProperty>(Property))
				{
					void* VariablePtr = ClassProperty->ContainerPtrToValuePtr<void>(ListView);
					if (TSubclassOf<UUserWidget> TransientEntryWidgetBGClass = Cast<UClass>(TransientEntryWidgetBP->GeneratedClass))
					{
						ClassProperty->SetPropertyValue(VariablePtr, TransientEntryWidgetBGClass);
					}
				}
			}
		}
	}
}

inline void UEditorUtilityWidgetSubsystem::ModifyNamedSlotReference(UWidget* Content, const TMap<FString, TSubclassOf<UEditorUtilityWidget>>& ReferencedWidgetClassMap)
{
	UClass* ContentClass = Content->GetClass();
	if (UEditorUtilityWidgetBlueprint* OriginalWidgetBP = Cast<UEditorUtilityWidgetBlueprint>(ContentClass->ClassGeneratedBy))
	{
		TSubclassOf<UEditorUtilityWidget> RegisteredWidgetClass = ReferencedWidgetClassMap[ContentClass->GetPackage()->GetPathName()];
		UEditorUtilityWidgetBlueprint* TransientWidgetBP = CreatedBlueprints.Contains(RegisteredWidgetClass) ?
			CreatedBlueprints[RegisteredWidgetClass].Get() : CreateTransientBlueprint(RegisteredWidgetClass, OriginalWidgetBP);
		if (IsValid(TransientWidgetBP) && TransientWidgetBP->Status != BS_Error)
		{
			// The transient Blueprint's own tree is the canonical Outer for the replacement: the node
			// must exist as a subobject under the tree so the compile-time archetype copy carries it into
			// the generated class and the runtime instancing graph creates and initializes it.
			UWidgetTree* ContentWidgetTree = Cast<UWidgetTree>(Content->GetOuter());
			UPanelSlot* ContentSlot = Content->Slot;

			UWidget* NamedSlotHostWidget = nullptr;
			FName NamedSlotName = NAME_None;
			if (ContentWidgetTree && !ContentSlot && ContentWidgetTree->RootWidget != Content)
			{
				TArray<UWidget*> AllWidgets;
				ContentWidgetTree->GetAllWidgets(AllWidgets);
				for (UWidget* HostWidget : AllWidgets)
				{
					if (INamedSlotInterface* NamedSlotHost = Cast<INamedSlotInterface>(HostWidget))
					{
						TArray<FName> SlotNames;
						NamedSlotHost->GetSlotNames(SlotNames);
						for (FName SlotName : SlotNames)
						{
							if (NamedSlotHost->GetContentForSlot(SlotName) == Content)
							{
								NamedSlotHostWidget = HostWidget;
								NamedSlotName = SlotName;
								break;
							}
						}
					}
					if (NamedSlotHostWidget)
					{
						break;
					}
				}
			}

			if (!ContentWidgetTree || (!ContentSlot && ContentWidgetTree->RootWidget != Content && !NamedSlotHostWidget))
			{
				UE_LOG(LogTemp, Warning, TEXT("Skipped replacing the widget node '%s': its location in the widget tree is unknown."), *Content->GetName());
			}
			else
			{
				const FString ContentName = Content->GetName();

				TMap<FName, FString> ExportedProperties;
				FWidgetBlueprintEditorUtils::ExportPropertiesToText(Content, ExportedProperties);

				Content->Rename(nullptr, GetTransientPackage(), REN_DontCreateRedirectors);

				UWidget* ReplacementWidget = NewObject<UWidget>(ContentWidgetTree, TransientWidgetBP->GeneratedClass, FName(*ContentName), RF_Transactional);
#if WITH_EDITOR
				ReplacementWidget->CreatedFromPalette();
#endif
				FWidgetBlueprintEditorUtils::ImportPropertiesFromText(ReplacementWidget, ExportedProperties);

				if (ContentSlot)
				{
					ReplacementWidget->Slot = ContentSlot;
					ContentSlot->Content = ReplacementWidget;
					Content->Slot = nullptr;
				}
				else if (ContentWidgetTree->RootWidget == Content)
				{
					ContentWidgetTree->RootWidget = ReplacementWidget;
				}
				else if (INamedSlotInterface* NamedSlotHost = Cast<INamedSlotInterface>(NamedSlotHostWidget))
				{
					NamedSlotHost->SetContentForSlot(NamedSlotName, ReplacementWidget);
				}
			}
		}
	}
}
