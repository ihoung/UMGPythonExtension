// Fill out your copyright notice in the Description page of Project Settings.


#include "EditorWidgetTreeSerialization.h"
#include "BaseWidgetBlueprint.h"
#include "Blueprint/UserWidgetBlueprint.h"
#include "Blueprint/WidgetTree.h"
#include "WidgetBlueprint.h"
#include "Components/Widget.h"
#include "Components/PanelWidget.h"
#include "Components/PanelSlot.h"
#include "Components/NamedSlotInterface.h"
#include "Dom/JsonObject.h"
#include "Dom/JsonValue.h"
#include "Serialization/JsonSerializer.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonWriter.h"
#include "Policies/CondensedJsonPrintPolicy.h"
#include "JsonObjectConverter.h"
#include "UObject/UnrealType.h"
#include "UObject/UObjectHash.h"
#include "UObject/UObjectGlobals.h"
#include "Misc/MessageDialog.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "Misc/PackageName.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "AssetRegistry/IAssetRegistry.h"

#define LOCTEXT_NAMESPACE "FEditorWidgetTreeSerialization"

namespace
{
	TSharedRef<FJsonObject> ConvertPropertiesToJsonObject(const UObject* Object)
	{
		TSharedRef<FJsonObject> PropertiesObject = MakeShared<FJsonObject>();
		FJsonObjectConverter::UStructToJsonObject(Object->GetClass(), Object, PropertiesObject, CPF_Edit, CPF_Transient);

		// Delegate properties are not recorded in the json: their bindings live in the Blueprint graph and
		// cannot be restored with the widget tree, and unbound dynamic delegates serialize as "(null).None",
		// which would fail to import on load (see the deserialization side for the fallback filtering).
		TArray<FString> DelegateKeys;
		for (const TPair<FString, TSharedPtr<FJsonValue>>& PropertyPair : PropertiesObject->Values)
		{
			const FProperty* Property = FindFProperty<FProperty>(Object->GetClass(), *PropertyPair.Key);
			if (Property && (CastField<FDelegateProperty>(Property) || CastField<FMulticastDelegateProperty>(Property)))
			{
				DelegateKeys.Add(PropertyPair.Key);
			}
		}
		for (const FString& DelegateKey : DelegateKeys)
		{
			PropertiesObject->RemoveField(DelegateKey);
		}

		return PropertiesObject;
	}

	/** Returns the 'EntryWidgetClass' property of the widget class, if it has one (ListView-style widgets). */
	FClassProperty* FindEntryWidgetClassProperty(const UClass* WidgetClass)
	{
		FProperty* WidgetProperty = WidgetClass->PropertyLink;
		while (WidgetProperty)
		{
			if (WidgetProperty->GetFName() == TEXT("EntryWidgetClass"))
			{
				break;
			}
			WidgetProperty = WidgetProperty->PropertyLinkNext;
		}
		return CastField<FClassProperty>(WidgetProperty);
	}

	/** Collects the Widget Blueprint of the entry widget (ListView 'entryWidgetClass') referenced by the widget, if any. */
	void CollectEntryWidgetBlueprint(UWidget* Widget, TArray<UBaseWidgetBlueprint*>& OutEntryWidgetBlueprints)
	{
		// ListView-style widgets (ListView, TileView, TreeView, ColumnView, ...) expose an editable
		// TSubclassOf<UUserWidget> 'EntryWidgetClass' property that is usually created from a Widget Blueprint.
		FClassProperty* EntryWidgetClassProperty = FindEntryWidgetClassProperty(Widget->GetClass());
		if (!EntryWidgetClassProperty)
		{
			return;
		}

		UClass* EntryWidgetClass = Cast<UClass>(EntryWidgetClassProperty->GetObjectPropertyValue_InContainer(Widget));
		if (!EntryWidgetClass)
		{
			return;
		}

		if (UBaseWidgetBlueprint* EntryWidgetBlueprint = Cast<UBaseWidgetBlueprint>(EntryWidgetClass->ClassGeneratedBy))
		{
			OutEntryWidgetBlueprints.AddUnique(EntryWidgetBlueprint);
		}
	}

	/** Recursively serializes a widget and its children into a JSON object. */
	TSharedRef<FJsonObject> ConvertWidgetToJsonObject(UWidget* Widget, TArray<UBaseWidgetBlueprint*>& OutEntryWidgetBlueprints)
	{
		TSharedRef<FJsonObject> WidgetObject = MakeShared<FJsonObject>();
		WidgetObject->SetStringField(TEXT("name"), Widget->GetName());
		WidgetObject->SetStringField(TEXT("class"), Widget->GetClass()->GetName());

		// 'bIsVariable' is declared as a bare UPROPERTY() (no CPF_Edit flag) in UWidget, so it is filtered
		// out by FJsonObjectConverter's flag checks; serialize it explicitly instead.
		WidgetObject->SetBoolField(TEXT("bIsVariable"), Widget->bIsVariable);

		// Collect the entry Widget Blueprint referenced by a ListView-style 'entryWidgetClass' property, if present.
		// The property itself is still serialized below like any other widget property.
		CollectEntryWidgetBlueprint(Widget, OutEntryWidgetBlueprints);

		// Slot (layout information within the parent panel)
		if (UPanelSlot* Slot = Widget->Slot)
		{
			TSharedRef<FJsonObject> SlotObject = MakeShared<FJsonObject>();
			SlotObject->SetStringField(TEXT("class"), Slot->GetClass()->GetName());
			SlotObject->SetObjectField(TEXT("properties"), ConvertPropertiesToJsonObject(Slot));
			WidgetObject->SetObjectField(TEXT("slot"), SlotObject);
		}

		// Widget design properties
		WidgetObject->SetObjectField(TEXT("properties"), ConvertPropertiesToJsonObject(Widget));

		// Children
		TArray<TSharedPtr<FJsonValue>> Children;
		if (UPanelWidget* PanelWidget = Cast<UPanelWidget>(Widget))
		{
			const int32 NumChildren = PanelWidget->GetChildrenCount();
			for (int32 ChildIndex = 0; ChildIndex < NumChildren; ++ChildIndex)
			{
				if (UWidget* ChildWidget = PanelWidget->GetChildAt(ChildIndex))
				{
					Children.Add(MakeShared<FJsonValueObject>(ConvertWidgetToJsonObject(ChildWidget, OutEntryWidgetBlueprints)));
				}
			}
		}
		WidgetObject->SetArrayField(TEXT("children"), MoveTemp(Children));

		// Named slot content (e.g. an ExpandableArea's 'Header'/'Body' sections). Slot content widgets are not
		// panel children and their backing properties (e.g. 'HeaderContent'/'BodyContent') are bare UPROPERTY()s,
		// so they are invisible to both the children loop above and FJsonObjectConverter's CPF_Edit filter.
		// Serialize them through INamedSlotInterface, which is how the UMG designer manages them.
		if (INamedSlotInterface* NamedSlotHost = Cast<INamedSlotInterface>(Widget))
		{
			TArray<FName> SlotNames;
			NamedSlotHost->GetSlotNames(SlotNames);

			TArray<TSharedPtr<FJsonValue>> NamedSlots;
			for (FName SlotName : SlotNames)
			{
				if (UWidget* SlotContent = NamedSlotHost->GetContentForSlot(SlotName))
				{
					TSharedRef<FJsonObject> NamedSlotObject = MakeShared<FJsonObject>();
					NamedSlotObject->SetStringField(TEXT("name"), SlotName.ToString());
					NamedSlotObject->SetObjectField(TEXT("content"), ConvertWidgetToJsonObject(SlotContent, OutEntryWidgetBlueprints));
					NamedSlots.Add(MakeShared<FJsonValueObject>(NamedSlotObject));
				}
			}
			WidgetObject->SetArrayField(TEXT("named_slots"), MoveTemp(NamedSlots));
		}

		return WidgetObject;
	}

	FString SerializeJsonObjectToString(const TSharedRef<FJsonObject>& JsonObject)
	{
		// Condensed policy: no newlines or indentation, so complex widget trees stay compact on disk.
		FString JsonString;
		TSharedRef<TJsonWriter<TCHAR, TCondensedJsonPrintPolicy<TCHAR>>> Writer = TJsonWriterFactory<TCHAR, TCondensedJsonPrintPolicy<TCHAR>>::Create(&JsonString);
		FJsonSerializer::Serialize(JsonObject, Writer);
		return JsonString;
	}

	/** Resolves a widget class from its short (serialized) name, e.g. 'TextBlock' or 'WBP_MyWidget_C'. */
	UClass* ResolveWidgetClass(const FString& ClassName)
	{
		// Any already loaded UWidget-derived class (native classes of any module and loaded Blueprint classes)
		TArray<UClass*> WidgetClasses;
		GetDerivedClasses(UWidget::StaticClass(), WidgetClasses, /*bRecursive*/true);
		for (UClass* WidgetClass : WidgetClasses)
		{
			if (WidgetClass->GetFName() == ClassName)
			{
				return WidgetClass;
			}
		}

		// A Blueprint widget class that is not loaded yet: search the asset registry by Blueprint asset name ('WBP_MyWidget_C' -> 'WBP_MyWidget')
		if (ClassName.EndsWith(TEXT("_C")))
		{
			const FString BlueprintName = ClassName.LeftChop(2);
			IAssetRegistry& AssetRegistry = FModuleManager::LoadModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry")).Get();
			TArray<FAssetData> WidgetBlueprintAssets;
			AssetRegistry.GetAssetsByClass(UWidgetBlueprint::StaticClass()->GetClassPathName(), WidgetBlueprintAssets, /*bSearchSubClasses*/true);
			for (const FAssetData& AssetData : WidgetBlueprintAssets)
			{
				if (AssetData.AssetName.ToString() == BlueprintName)
				{
					if (UWidgetBlueprint* WidgetBlueprint = Cast<UWidgetBlueprint>(AssetData.GetAsset()))
					{
						return WidgetBlueprint->GeneratedClass;
					}
				}
			}
		}

		return nullptr;
	}

	/** Derives the serialized json file of an entry widget class ('/Game/UI/WBP_Entry.WBP_Entry_C' -> '<JsonDir>/WBP_Entry.json'), if present. */
	FString FindSerializedFileForEntryClass(const FString& JsonDir, const FString& EntryClassPath)
	{
		FString ClassName;
		FString ObjectPath;
		if (FPackageName::ParseExportTextPath(EntryClassPath, &ClassName, &ObjectPath))
		{
			FString EntryClassName = ObjectPath;
			int32 DotIndex = INDEX_NONE;
			if (EntryClassName.FindLastChar(TEXT('.'), DotIndex))
			{
				EntryClassName = EntryClassName.RightChop(DotIndex + 1);
			}
			if (EntryClassName.EndsWith(TEXT("_C")))
			{
				EntryClassName = EntryClassName.LeftChop(2);
			}

			const FString SerializedFile = FPaths::Combine(JsonDir, EntryClassName + TEXT(".json"));
			return FPaths::FileExists(SerializedFile) ? SerializedFile : FString();
		}
		return FString();
	}

	/**
	 * Handles the 'EntryWidgetClass' property after the widget properties have been imported. The
	 * property itself is excluded from the bulk import (see LoadWidgetFromJsonObject); it is resolved
	 * here with the following priority:
	 * 1. The serialized json file of the entry widget found next to the loaded file, passed to the
	 *    predicate, which may create a transient Widget Blueprint (e.g. derived from a registered
	 *    native entry class) so that the entry widget is rebuilt from its serialized tree.
	 * 2. The generated class of the original entry Widget Blueprint asset, when no serialized file
	 *    exists or the predicate could not provide a class.
	 */
	bool ResolveEntryWidgetClass(UWidget* Widget, const TSharedPtr<FJsonObject>& PropertiesObject, const FString& JsonDir, TFunctionRef<UClass*(FString)> EntryWidgetClassPredicate)
	{
		FClassProperty* EntryWidgetClassProperty = FindEntryWidgetClassProperty(Widget->GetClass());
		if (!EntryWidgetClassProperty || !PropertiesObject.IsValid())
		{
			return true;
		}

		FString EntryClassPath;
		// The JSON key is the property's authored name, whose casing differs between engine versions
		// ('EntryWidgetClass' vs the standardized 'entryWidgetClass'); look it up case-insensitively.
		const TSharedPtr<FJsonValue>* EntryClassValue = nullptr;
		for (const TPair<FString, TSharedPtr<FJsonValue>>& PropertyPair : PropertiesObject->Values)
		{
			if (PropertyPair.Key.Equals(TEXT("EntryWidgetClass"), ESearchCase::IgnoreCase))
			{
				EntryClassValue = &PropertyPair.Value;
				break;
			}
		}
		if (!EntryClassValue || !(*EntryClassValue)->TryGetString(EntryClassPath) || EntryClassPath.IsEmpty())
		{
			return true;
		}

		UClass* EntryWidgetClass = nullptr;

		// Prefer rebuilding the entry widget from its serialized json file: the predicate may create a
		// transient Widget Blueprint derived from a registered entry class (e.g. a Python
		// UPyEditorUtilityEntryWidget), which must take precedence over the original Widget Blueprint
		// asset's generated class whenever both are available.
		const FString SerializedFile = FindSerializedFileForEntryClass(JsonDir, EntryClassPath);
		if (!SerializedFile.IsEmpty())
		{
			EntryWidgetClass = EntryWidgetClassPredicate(SerializedFile);
		}

		// Fall back to the generated class of the original entry Widget Blueprint asset, e.g. when no
		// serialized file exists next to the loaded file or the entry was not registered with the
		// predicate's owner.
		if (!EntryWidgetClass)
		{
			EntryWidgetClass = StaticLoadClass(EntryWidgetClassProperty->PropertyClass, nullptr, *EntryClassPath, nullptr, LOAD_NoWarn | LOAD_Quiet);
		}

		if (EntryWidgetClass)
		{
			EntryWidgetClassProperty->SetObjectPropertyValue_InContainer(Widget, EntryWidgetClass);
			return true;
		}

		UE_LOG(LogTemp, Warning, TEXT("Entry widget class '%s' could be resolved neither from a serialized file in '%s' nor from its original asset."), *EntryClassPath, *JsonDir);
		return false;
	}

	/** Removes all widgets of the tree so that a serialized tree can fully replace it (frees the widget names). */
	void ClearWidgetTree(UWidgetTree* WidgetTree)
	{
		if (!WidgetTree->RootWidget)
		{
			return;
		}

		TArray<UWidget*> ExistingWidgets;
		WidgetTree->GetAllWidgets(ExistingWidgets);

		WidgetTree->RemoveWidget(WidgetTree->RootWidget);
		WidgetTree->NamedSlotBindings.Empty();

		for (UWidget* ExistingWidget : ExistingWidgets)
		{
			// Move the old template widgets out of the tree's package so their names can be reused
			ExistingWidget->Rename(nullptr, GetTransientPackage(), REN_DontCreateRedirectors);
		}
	}

	/** Recursively builds a widget (and its children) from a serialized json node into the given tree. */
	UWidget* LoadWidgetFromJsonObject(const TSharedRef<FJsonObject>& WidgetObject, UWidgetTree* WidgetTree, UPanelWidget* ParentPanel, const FString& JsonDir, TFunctionRef<UClass*(FString)> EntryWidgetClassPredicate, UWidget* ParentNamedSlotHost = nullptr, FName ParentNamedSlotName = NAME_None)
	{
		FString WidgetClassName;
		if (!WidgetObject->TryGetStringField(TEXT("class"), WidgetClassName) || WidgetClassName.IsEmpty())
		{
			UE_LOG(LogTemp, Warning, TEXT("Skipped a serialized widget node without a 'class' field."));
			return nullptr;
		}

		UClass* WidgetClass = ResolveWidgetClass(WidgetClassName);
		if (!WidgetClass)
		{
			UE_LOG(LogTemp, Warning, TEXT("Widget class '%s' could not be resolved; the widget node was skipped."), *WidgetClassName);
			return nullptr;
		}

		// Create the widget in the tree the same way the UMG designer palette does
		FName WidgetName = NAME_None;
		FString WidgetNameString;
		if (WidgetObject->TryGetStringField(TEXT("name"), WidgetNameString) && !WidgetNameString.IsEmpty())
		{
			WidgetName = FName(*WidgetNameString);
		}
		UWidget* NewWidget = NewObject<UWidget>(WidgetTree, WidgetClass, WidgetName, RF_Transactional);
#if WITH_EDITOR
		NewWidget->CreatedFromPalette();
#endif

		// Restore 'bIsVariable' explicitly (it is not covered by FJsonObjectConverter, see the serialization
		// side); must come after CreatedFromPalette() which may reset it to false.
		bool bIsVariable = true;
		WidgetObject->TryGetBoolField(TEXT("bIsVariable"), bIsVariable);
		NewWidget->bIsVariable = bIsVariable;

		// Attach the widget to its parent panel, to a named slot of its parent host, or make it the tree root
		if (ParentPanel)
		{
			ParentPanel->AddChild(NewWidget);

			// Slot (layout information within the parent panel)
			const TSharedPtr<FJsonObject>* SlotObjectPtr = nullptr;
			if (NewWidget->Slot && WidgetObject->TryGetObjectField(TEXT("slot"), SlotObjectPtr))
			{
				const TSharedPtr<FJsonObject>* SlotPropertiesPtr = nullptr;
				if ((*SlotObjectPtr)->TryGetObjectField(TEXT("properties"), SlotPropertiesPtr))
				{
					FJsonObjectConverter::JsonObjectToUStruct(SlotPropertiesPtr->ToSharedRef(), NewWidget->Slot->GetClass(), NewWidget->Slot, CPF_Edit, CPF_Transient);
				}
			}
		}
		else if (ParentNamedSlotHost)
		{
			// Named slot content (e.g. an ExpandableArea's 'Header'/'Body' section) is not a panel child and has
			// no slot; attach it through INamedSlotInterface the same way the UMG designer drop handler does.
			if (INamedSlotInterface* NamedSlotHost = Cast<INamedSlotInterface>(ParentNamedSlotHost))
			{
				NamedSlotHost->SetContentForSlot(ParentNamedSlotName, NewWidget);
			}
		}
		else
		{
			WidgetTree->RootWidget = NewWidget;
		}

		// Widget design properties
		const TSharedPtr<FJsonObject>* PropertiesObjectPtr = nullptr;
		if (WidgetObject->TryGetObjectField(TEXT("properties"), PropertiesObjectPtr))
		{
			// Filter out properties that FJsonObjectConverter cannot import:
			// - Delegate properties: unbound dynamic delegates serialize as "(null).None", which fails to
			//   import through FProperty::ImportText; since FJsonObjectConverter::JsonObjectToUStruct aborts
			//   on the first failing property, all properties iterated after the delegate would silently keep
			//   their defaults (e.g. a TreeView's 'verticalEntrySpacing' is never restored because its
			//   'bP_OnGetItemChildren' bindable event is iterated first). Delegate bindings live in the
			//   Blueprint graph and cannot be restored with the widget tree anyway.
			// - UWidget's instanced 'Slot' property: the slot is already restored from the dedicated 'slot'
			//   node above; importing the nested subobject here would replace the slot created by AddChild
			//   with a duplicate that has no parent panel.
			// - The ListView-style 'EntryWidgetClass' property: importing it resolves (and loads) the
			//   original entry Widget Blueprint asset as a side effect of FClassProperty::ImportText, after
			//   which the entry could never be rebuilt from its own serialized json anymore; when the asset
			//   does not exist, the import would fail and abort the remaining properties (same as the
			//   delegate case above). The property is resolved explicitly by ResolveEntryWidgetClass below
			//   instead, with the serialized file taking priority over the original asset.
			TSharedRef<FJsonObject> ImportProperties = MakeShared<FJsonObject>();
			for (const TPair<FString, TSharedPtr<FJsonValue>>& PropertyPair : (*PropertiesObjectPtr)->Values)
			{
				const FProperty* Property = FindFProperty<FProperty>(NewWidget->GetClass(), *PropertyPair.Key);
				if (Property
					&& (CastField<FDelegateProperty>(Property)
						|| CastField<FMulticastDelegateProperty>(Property)
						|| (Property->GetFName() == FName(TEXT("Slot")) && Property->GetOwnerStruct() == UWidget::StaticClass())
						|| (CastField<FClassProperty>(Property) && Property->GetFName() == FName(TEXT("EntryWidgetClass")))))
				{
					continue;
				}
				ImportProperties->SetField(PropertyPair.Key, PropertyPair.Value);
			}

			FText ImportFailReason;
			if (!FJsonObjectConverter::JsonObjectToUStruct(ImportProperties, NewWidget->GetClass(), NewWidget, CPF_Edit, CPF_Transient, /*bStrictMode=*/false, &ImportFailReason))
			{
				UE_LOG(LogTemp, Warning, TEXT("Some properties of widget '%s' could not be imported: %s"), *NewWidget->GetName(), *ImportFailReason.ToString());
			}
			if (!ResolveEntryWidgetClass(NewWidget, *PropertiesObjectPtr, JsonDir, EntryWidgetClassPredicate)) return nullptr;
		}

		// Children
		UPanelWidget* PanelWidget = Cast<UPanelWidget>(NewWidget);
		const TArray<TSharedPtr<FJsonValue>>* ChildrenPtr = nullptr;
		if (PanelWidget && WidgetObject->TryGetArrayField(TEXT("children"), ChildrenPtr))
		{
			for (const TSharedPtr<FJsonValue>& ChildValue : *ChildrenPtr)
			{
				const TSharedPtr<FJsonObject>& ChildObjectPtr = ChildValue.IsValid() ? ChildValue->AsObject() : nullptr;
				if (ChildObjectPtr && ChildObjectPtr.IsValid())
				{
					if (!LoadWidgetFromJsonObject(ChildObjectPtr.ToSharedRef(), WidgetTree, PanelWidget, JsonDir, EntryWidgetClassPredicate))
					{
						return nullptr;
					}
				}
			}
		}

		// Named slot content (e.g. an ExpandableArea's 'Header'/'Body' sections) — see the serialization side
		const TArray<TSharedPtr<FJsonValue>>* NamedSlotsPtr = nullptr;
		if (WidgetObject->TryGetArrayField(TEXT("named_slots"), NamedSlotsPtr))
		{
			for (const TSharedPtr<FJsonValue>& NamedSlotValue : *NamedSlotsPtr)
			{
				const TSharedPtr<FJsonObject>& NamedSlotObject = NamedSlotValue.IsValid() ? NamedSlotValue->AsObject() : nullptr;
				FString SlotNameString;
				const TSharedPtr<FJsonObject>* SlotContentObjectPtr = nullptr;
				if (NamedSlotObject.IsValid()
					&& NamedSlotObject->TryGetStringField(TEXT("name"), SlotNameString)
					&& NamedSlotObject->TryGetObjectField(TEXT("content"), SlotContentObjectPtr)
					&& SlotContentObjectPtr->IsValid())
				{
					if (!LoadWidgetFromJsonObject(SlotContentObjectPtr->ToSharedRef(), WidgetTree, /*ParentPanel=*/nullptr, JsonDir, EntryWidgetClassPredicate, /*ParentNamedSlotHost=*/NewWidget, FName(*SlotNameString)))
					{
						return nullptr;
					}
				}
			}
		}

		return NewWidget;
	}
}


FEditorWidgetTreeSerialization::FEditorWidgetTreeSerialization()
{
}

FEditorWidgetTreeSerialization::~FEditorWidgetTreeSerialization()
{
}


bool FEditorWidgetTreeSerialization::SerializeWidgetBlueprint(UBaseWidgetBlueprint* WidgetBlueprint, const FString& DestinationDir)
{
	TSet<UBaseWidgetBlueprint*> SerializedBlueprints;
	return SerializeWidgetBlueprintInternal(WidgetBlueprint, DestinationDir, /*bAskAboutEntries=*/true, SerializedBlueprints);
}

bool FEditorWidgetTreeSerialization::SerializeWidgetBlueprintInternal(UBaseWidgetBlueprint* WidgetBlueprint, const FString& DestinationDir, bool bAskAboutEntries, TSet<UBaseWidgetBlueprint*>& SerializedBlueprints)
{
	SerializedBlueprints.Add(WidgetBlueprint);

	UWidgetTree* WidgetTree = WidgetBlueprint->WidgetTree;
	if (!WidgetTree)
	{
		UE_LOG(LogTemp, Warning, TEXT("Widget Blueprint '%s' has no Widget Tree."), *WidgetBlueprint->GetName());
		return false;
	}

	TSharedRef<FJsonObject> RootObject = MakeShared<FJsonObject>();
	RootObject->SetStringField(TEXT("asset_name"), WidgetBlueprint->GetName());
	RootObject->SetStringField(TEXT("asset_type"), WidgetBlueprint->GetClass()->GetName());
	RootObject->SetStringField(TEXT("asset_path"), WidgetBlueprint->GetPathName());

	// The 'EntryWidgetClass' property (ListView-style widgets) is serialized like any other property,
	// while the Widget Blueprints it references are collected for the user prompt below.
	TArray<UBaseWidgetBlueprint*> EntryWidgetBlueprints;
	TSharedRef<FJsonObject> TreeObject = MakeShared<FJsonObject>();
	if (UWidget* RootWidget = WidgetTree->RootWidget)
	{
		TreeObject = ConvertWidgetToJsonObject(RootWidget, EntryWidgetBlueprints);
	}
	RootObject->SetObjectField(TEXT("widget_tree"), TreeObject);

	const FString JsonString = SerializeJsonObjectToString(RootObject);
	const FString DestinationFile = FPaths::Combine(DestinationDir, WidgetBlueprint->GetName() + TEXT(".json"));
	if (!FFileHelper::SaveStringToFile(JsonString, *DestinationFile))
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to save the widget tree of '%s' to '%s'."), *WidgetBlueprint->GetName(), *DestinationFile);
		return false;
	}
	UE_LOG(LogTemp, Log, TEXT("Serialized the widget tree of '%s' to '%s'."), *WidgetBlueprint->GetName(), *DestinationFile);

	// Ask whether to also serialize the entry Widget Blueprints (from 'entryWidgetClass' properties)
	EntryWidgetBlueprints.RemoveAll([&SerializedBlueprints](UBaseWidgetBlueprint* EntryWidgetBlueprint)
	{
		return SerializedBlueprints.Contains(EntryWidgetBlueprint);
	});
	if (!EntryWidgetBlueprints.IsEmpty())
	{
		bool bSerializeEntries = true;
		if (bAskAboutEntries)
		{
			TArray<FText> EntryNameTexts;
			for (const UBaseWidgetBlueprint* EntryWidgetBlueprint : EntryWidgetBlueprints)
			{
				EntryNameTexts.Add(FText::FromString(EntryWidgetBlueprint->GetName()));
			}
			const FText Prompt = FText::Format(
				LOCTEXT("SerializeEntryBlueprintsPrompt", "The widget tree references the following List View entry widget(s):\n\n{0}\n\nDo you also want to serialize their Widget Blueprints to the same directory?"),
				FText::Join(FText::FromString(TEXT("\n")), EntryNameTexts));
			bSerializeEntries = FMessageDialog::Open(EAppMsgType::YesNo, Prompt, LOCTEXT("SerializeEntryBlueprintsTitle", "Serialize Entry Widget Blueprints")) == EAppReturnType::Yes;
		}

		if (bSerializeEntries)
		{
			for (UBaseWidgetBlueprint* EntryWidgetBlueprint : EntryWidgetBlueprints)
			{
				SerializeWidgetBlueprintInternal(EntryWidgetBlueprint, DestinationDir, /*bAskAboutEntries=*/false, SerializedBlueprints);
			}
		}
	}

	return true;
}

bool FEditorWidgetTreeSerialization::LoadSerializationToWidgetBlueprint(const FString& FilePath, UBaseWidgetBlueprint* WidgetBlueprint, TFunctionRef<UClass*(FString)> EntryWidgetClassPredicate)
{
	UWidgetTree* WidgetTree = WidgetBlueprint ? WidgetBlueprint->WidgetTree : nullptr;
	if (!WidgetTree)
	{
		UE_LOG(LogTemp, Error, TEXT("Cannot load a widget tree into a Widget Blueprint without a Widget Tree."));
		return false;
	}

	// Read and parse the serialized json file
	FString JsonString;
	if (!FFileHelper::LoadFileToString(JsonString, *FilePath))
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to load the widget tree file '%s'."), *FilePath);
		return false;
	}

	TSharedPtr<FJsonObject> RootObject;
	const TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(JsonString);
	if (!FJsonSerializer::Deserialize(Reader, RootObject) || !RootObject.IsValid())
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to parse the widget tree file '%s'."), *FilePath);
		return false;
	}

	const TSharedPtr<FJsonObject>* TreeObjectPtr = nullptr;
	if (!RootObject->TryGetObjectField(TEXT("widget_tree"), TreeObjectPtr) || !TreeObjectPtr->IsValid())
	{
		UE_LOG(LogTemp, Error, TEXT("The file '%s' does not contain a 'widget_tree' field."), *FilePath);
		return false;
	}

	WidgetBlueprint->Modify();

	// Remove the current tree so that the serialized one can fully replace it (keeps the widget names unique)
	ClearWidgetTree(WidgetTree);

	// An empty tree object means the Widget Blueprint had no root widget
	if ((*TreeObjectPtr)->HasField(TEXT("class")))
	{
		if (!LoadWidgetFromJsonObject(TreeObjectPtr->ToSharedRef(), WidgetTree, /*ParentPanel=*/nullptr, FPaths::GetPath(FilePath), EntryWidgetClassPredicate))
		{
			UE_LOG(LogTemp, Error, TEXT("Failed to build the widget tree from the file '%s'."), *FilePath);
			return false;
		}
	}

	WidgetBlueprint->MarkPackageDirty();
	UE_LOG(LogTemp, Log, TEXT("Loaded the widget tree of '%s' from '%s'."), *WidgetBlueprint->GetName(), *FilePath);

	return true;
}

#undef LOCTEXT_NAMESPACE
