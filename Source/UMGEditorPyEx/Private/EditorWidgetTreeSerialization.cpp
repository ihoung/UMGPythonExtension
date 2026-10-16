// Fill out your copyright notice in the Description page of Project Settings.


#include "EditorWidgetTreeSerialization.h"
#include "BaseWidgetBlueprint.h"
#include "Blueprint/UserWidgetBlueprint.h"
#include "Blueprint/WidgetTree.h"
#include "Engine/Blueprint.h"
#include "Engine/BlueprintGeneratedClass.h"
#include "EdGraphSchema_K2.h"
#include "Kismet2/BlueprintEditorUtils.h"
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

	/** Collects the Widget Blueprint referenced by the widget, if any. This covers both reference styles:
	 *  - The widget itself may be an instance of a Widget Blueprint class placed in a panel or a named slot
	 *    (e.g. 'WBP_MyButton_C'); such a reference is the node's 'class', not a property, so it is invisible
	 *    to the property-based checks.
	 *  - ListView-style widgets (ListView, TileView, TreeView, ColumnView, ...) expose an editable
	 *    TSubclassOf<UUserWidget> 'EntryWidgetClass' property that is usually created from a Widget Blueprint. */
	void CollectReferencedWidgetBlueprint(UWidget* Widget, TArray<UBaseWidgetBlueprint*>& OutReferencedWidgetBlueprints)
	{
		if (UBaseWidgetBlueprint* WidgetBlueprint = Cast<UBaseWidgetBlueprint>(Widget->GetClass()->ClassGeneratedBy))
		{
			OutReferencedWidgetBlueprints.AddUnique(WidgetBlueprint);
		}

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
			OutReferencedWidgetBlueprints.AddUnique(EntryWidgetBlueprint);
		}
	}

	/** Recursively serializes a widget and its children into a JSON object. */
	TSharedRef<FJsonObject> ConvertWidgetToJsonObject(UWidget* Widget, TArray<UBaseWidgetBlueprint*>& OutReferencedWidgetBlueprints)
	{
		TSharedRef<FJsonObject> WidgetObject = MakeShared<FJsonObject>();
		WidgetObject->SetStringField(TEXT("name"), Widget->GetName());

		// A Blueprint-generated class is a reference to another Widget Blueprint and is recorded in the same
		// format as an 'EntryWidgetClass' property value — the generated class's export path (e.g.
		// '/Script/UMG.WidgetBlueprintGeneratedClass'/Game/UI/WBP_MyButton.WBP_MyButton_C'') — so that
		// deserialization can locate the referenced Widget Blueprint's serialized widget tree from the class
		// path (see ResolveWidgetClassForNode). Native classes keep their short name.
		UClass* WidgetClass = Widget->GetClass();
		FString WidgetClassName = WidgetClass->GetName();
		if (WidgetClass->ClassGeneratedBy)
		{
			const UObject* WidgetClassObject = WidgetClass;
			WidgetClassName = FObjectPropertyBase::GetExportPath(TObjectPtr<const UObject>(WidgetClassObject), nullptr, nullptr, PPF_None);
		}
		WidgetObject->SetStringField(TEXT("class"), WidgetClassName);

		// 'bIsVariable' is declared as a bare UPROPERTY() (no CPF_Edit flag) in UWidget, so it is filtered
		// out by FJsonObjectConverter's flag checks; serialize it explicitly instead.
		WidgetObject->SetBoolField(TEXT("bIsVariable"), Widget->bIsVariable);

		// Collect the Widget Blueprint referenced by the widget itself (a Blueprint class instance placed in a
		// panel or named slot) or by a ListView-style 'entryWidgetClass' property, if present.
		// The properties themselves are still serialized below like any other widget property.
		CollectReferencedWidgetBlueprint(Widget, OutReferencedWidgetBlueprints);

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
					Children.Add(MakeShared<FJsonValueObject>(ConvertWidgetToJsonObject(ChildWidget, OutReferencedWidgetBlueprints)));
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
					NamedSlotObject->SetObjectField(TEXT("content"), ConvertWidgetToJsonObject(SlotContent, OutReferencedWidgetBlueprints));
					NamedSlots.Add(MakeShared<FJsonValueObject>(NamedSlotObject));
				}
			}
			WidgetObject->SetArrayField(TEXT("named_slots"), MoveTemp(NamedSlots));
		}

		return WidgetObject;
	}

	/**
	 * Whether the variable describes an Event Dispatcher: the K2 delegate pin categories are set up so a
	 * delegate's 'SubCategoryObject' is its own signature UFunction, so a native/Python parent class'
	 * delegate has the '/Script/...' package while a Blueprint-declared one points at its generated class.
	 * Both are excluded from the json: their signature function lives in a DelegateSignatureGraph that is
	 * not serialized here, so a restored delegate property would compile without a SignatureFunction and
	 * log "[Compiler] No SignatureFunction in MulticastDelegateProperty" (see LoadVariablesFromJsonArray).
	 */
	bool IsEventDispatcherVariable(const FBPVariableDescription& Variable)
	{
		const FName PinCategory = Variable.VarType.PinCategory;
		if (PinCategory != UEdGraphSchema_K2::PC_MCDelegate && PinCategory != UEdGraphSchema_K2::PC_Delegate)
		{
			return false;
		}

		UObject* Signature = Variable.VarType.PinSubCategoryObject.Get();
		return Signature == nullptr || !Signature->GetOutermost()->GetName().StartsWith(TEXT("/Script/"));
	}

	/**
	 * Converts the Blueprint's member variables (UBlueprint::NewVariables) into a json array. The WidgetTree's
	 * own variables (widgets promoted to variables via
	 * 'bIsVariable') are not part of NewVariables — they are rebuilt from the widget tree's own
	 * 'bIsVariable' flags when the Blueprint is compiled — so the two kinds are kept apart. Each variable
	 * is converted on its own so an unconvertible one only drops itself instead of the whole array.
	 *
	 * Event Dispatcher variables are skipped entirely (IsEventDispatcherVariable): their signature lives in a
	 * DelegateSignatureGraph that is not serialized here, so restoring just the variable on load would compile
	 * into a delegate property without a SignatureFunction.
	 *
	 * The variable's default value is not taken from the description's 'DefaultValue' string: the compiler
	 * clears that string after applying it to the generated class's CDO (FKismetCompilerContext), so for a
	 * compiled Blueprint it is usually empty. The authoritative value lives in the CDO and is exported from
	 * it the same way the editor does when duplicating a variable
	 * (FBlueprintEditorUtils::DuplicateVariableDescription); the description's string is only kept as a
	 * fallback when the CDO/property is not available (e.g. a Blueprint that has not been compiled yet).
	 */
	TArray<TSharedPtr<FJsonValue>> ConvertVariablesToJsonArray(const UBlueprint* Blueprint)
	{
		UClass* GeneratedClass = Blueprint ? Blueprint->GeneratedClass : nullptr;
		UObject* GeneratedCDO = GeneratedClass ? GeneratedClass->GetDefaultObject(/*bCreateIfNeeded=*/false) : nullptr;

		TArray<TSharedPtr<FJsonValue>> Variables;
		for (const FBPVariableDescription& Variable : Blueprint->NewVariables)
		{
			if (IsEventDispatcherVariable(Variable))
			{
				continue;
			}

			// CheckFlags=0: VarGuid is declared as a bare UPROPERTY() and must be kept as well.
			TSharedPtr<FJsonObject> VariableObject = FJsonObjectConverter::UStructToJsonObject(Variable, 0, CPF_Transient | CPF_Deprecated);
			if (!VariableObject.IsValid())
			{
				UE_LOG(LogTemp, Warning, TEXT("Variable '%s' of '%s' could not be serialized; it was skipped."), *Variable.VarName.ToString(), *Blueprint->GetName());
				continue;
			}

			// The real default value, exported from the generated class's CDO
			const FProperty* VariableProperty = GeneratedCDO ? FindFProperty<FProperty>(GeneratedClass, Variable.VarName) : nullptr;
			const void* PropertyValue = VariableProperty ? VariableProperty->ContainerPtrToValuePtr<void>(GeneratedCDO) : nullptr;
			if (VariableProperty && PropertyValue)
			{
				FString DefaultValue;
				VariableProperty->ExportTextItem_Direct(DefaultValue, PropertyValue, /*DefaultValue=*/nullptr, /*Parent=*/nullptr, PPF_SerializedAsImportText);
				// Overwrites the (usually empty) 'defaultValue' key written by the conversion above; the
				// json object's key lookup is case-insensitive, so the case-standardized key matches.
				VariableObject->SetStringField(TEXT("defaultValue"), DefaultValue);
			}

			Variables.Add(MakeShared<FJsonValueObject>(VariableObject));
		}
		return Variables;
	}

	/**
	 * Deserializes the 'variables' json array into the Blueprint's member variables (UBlueprint::NewVariables),
	 * replacing whatever is already there. The WidgetTree's own variables are not handled here: they are
	 * regenerated at compile time from each widget's 'bIsVariable' flag (see ConvertVariablesToJsonArray).
	 * Each variable is imported on its own so an unconvertible one only drops itself instead of the rest.
	 *
	 * Event Dispatcher variables are dropped, mirroring the serialization side (IsEventDispatcherVariable):
	 * their DelegateSignatureGraphs are not part of the json, so restoring the variable alone would produce a
	 * delegate property without a SignatureFunction and trigger a compiler warning.
	 *
	 * The variable default values are also handed back through OutVariableDefaultValues: the compiler only
	 * stages FBPVariableDescription::DefaultValue and flushes it to the generated class CDO from the
	 * reinstancing stage, which a freshly created Blueprint (no old generated class to reinstance) never
	 * reaches — and the Full compile even clears the stored string (FKismetCompilerContext). The caller must
	 * apply the collected values to the CDO itself after the compile (see ApplyVariableDefaultValuesToCDO).
	 */
	void LoadVariablesFromJsonArray(const TArray<TSharedPtr<FJsonValue>>& VariableValues, UBlueprint* Blueprint, TMap<FName, FString>* OutVariableDefaultValues)
	{
		TArray<FBPVariableDescription> LoadedVariables;
		for (const TSharedPtr<FJsonValue>& VariableValue : VariableValues)
		{
			const TSharedPtr<FJsonObject> VariableObject = VariableValue.IsValid() ? VariableValue->AsObject() : nullptr;
			if (!VariableObject.IsValid())
			{
				continue;
			}

			// CheckFlags=0 mirrors the serialization side: VarGuid is a bare UPROPERTY() and must be kept.
			FBPVariableDescription Variable;
			FText ImportFailReason;
			if (FJsonObjectConverter::JsonObjectToUStruct(VariableObject.ToSharedRef(), FBPVariableDescription::StaticStruct(), &Variable, 0, CPF_Transient | CPF_Deprecated, /*bStrictMode=*/false, &ImportFailReason))
			{
				if (IsEventDispatcherVariable(Variable))
				{
					continue;
				}

				if (OutVariableDefaultValues && !Variable.DefaultValue.IsEmpty())
				{
					OutVariableDefaultValues->Add(Variable.VarName, Variable.DefaultValue);
				}
				LoadedVariables.Add(MoveTemp(Variable));
			}
			else
			{
				UE_LOG(LogTemp, Warning, TEXT("A variable of '%s' could not be imported and was skipped: %s"), *Blueprint->GetName(), *ImportFailReason.ToString());
			}
		}

		Blueprint->NewVariables = MoveTemp(LoadedVariables);
	}

	/**
	 * Serializes the widget class's own properties — the class defaults of the Blueprint's widget object
	 * itself (e.g. a UEditorUtilityWidget's interaction/appearance settings, edited in the Class Defaults
	 * panel) — into a json object. They are kept apart from the Blueprint's member variables (see
	 * ConvertVariablesToJsonArray): the properties generated by the Blueprint's own compile (its
	 * NewVariables and the WidgetTree variables derived from 'bIsVariable' widgets) are declared by the
	 * Blueprint's generated class and are excluded here, so only the widget's own properties remain.
	 */
	TSharedRef<FJsonObject> ConvertWidgetClassPropertiesToJsonObject(const UBlueprint* Blueprint)
	{
		UClass* GeneratedClass = Blueprint ? Blueprint->GeneratedClass : nullptr;
		UObject* GeneratedCDO = GeneratedClass ? GeneratedClass->GetDefaultObject(/*bCreateIfNeeded=*/false) : nullptr;
		if (!GeneratedCDO)
		{
			UE_LOG(LogTemp, Warning, TEXT("'%s' has no generated class; its widget class properties were not serialized."), *Blueprint->GetName());
			return MakeShared<FJsonObject>();
		}

		TSharedRef<FJsonObject> PropertiesObject = ConvertPropertiesToJsonObject(GeneratedCDO);

		TArray<FString> GeneratedPropertyKeys;
		for (const TPair<FString, TSharedPtr<FJsonValue>>& PropertyPair : PropertiesObject->Values)
		{
			const FProperty* Property = FindFProperty<FProperty>(GeneratedClass, *PropertyPair.Key);
			const UStruct* OwnerStruct = Property ? Property->GetOwnerStruct() : nullptr;
			if (OwnerStruct && (OwnerStruct == GeneratedClass || OwnerStruct == Blueprint->SkeletonGeneratedClass))
			{
				GeneratedPropertyKeys.Add(PropertyPair.Key);
			}
		}
		for (const FString& GeneratedPropertyKey : GeneratedPropertyKeys)
		{
			PropertiesObject->RemoveField(GeneratedPropertyKey);
		}

		return PropertiesObject;
	}

	/**
	 * Applies the widget class's own properties (see ConvertWidgetClassPropertiesToJsonObject) to the
	 * Blueprint's generated class CDO, which is where widget instances take their class defaults from.
	 * The CDO only exists once the Blueprint has been compiled: when it is not available yet (e.g. a fresh
	 * transient Blueprint that the caller compiles afterwards) the caller must load them after its compile
	 * through LoadWidgetClassPropertiesToCDO instead.
	 *
	 * Each property is imported on its own: the CDO's editable property set contains values that cannot be
	 * imported back (e.g. instanced subobjects like UWidget's 'Navigation'), and
	 * FJsonObjectConverter::JsonObjectToUStruct aborts on the first failing property, which would silently
	 * drop every property iterated after it (e.g. UWidget's 'Visibility').
	 */
	bool ApplyWidgetClassPropertiesObject(const TSharedPtr<FJsonObject>& PropertiesObject, UBlueprint* Blueprint)
	{
		if (!PropertiesObject.IsValid())
		{
			return true;
		}

		UClass* GeneratedClass = Blueprint ? Blueprint->GeneratedClass : nullptr;
		UObject* GeneratedCDO = GeneratedClass ? GeneratedClass->GetDefaultObject(/*bCreateIfNeeded=*/false) : nullptr;
		if (!GeneratedCDO)
		{
			UE_LOG(LogTemp, Warning, TEXT("'%s' has no generated class CDO; its widget class properties were not loaded."), *Blueprint->GetName());
			return false;
		}

		for (const TPair<FJsonObject::FStringType, TSharedPtr<FJsonValue>>& PropertyPair : PropertiesObject->Values)
		{
			FProperty* Property = FindFProperty<FProperty>(GeneratedClass, *PropertyPair.Key);
			if (!Property)
			{
				continue;
			}

			// - Delegate properties are filtered like on the serialization side (see ConvertPropertiesToJsonObject):
			//   unbound dynamic delegates serialize as "(null).None" and would fail to import.
			// - Non-editable/transient properties mirror the flags used at serialization time.
			if (CastField<FDelegateProperty>(Property) || CastField<FMulticastDelegateProperty>(Property)
				|| !Property->HasAnyPropertyFlags(CPF_Edit) || Property->HasAnyPropertyFlags(CPF_Transient))
			{
				continue;
			}

			// Import through a single-attribute map: it isolates the failure to this property while keeping
			// the same conversion path as a batch JsonObjectToUStruct call (including the CDO as the outer
			// for instanced subobjects such as UWidget's 'Navigation').
			TMap<FJsonObject::FStringType, TSharedPtr<FJsonValue>> SinglePropertyAttribute;
			// The json key is the authored name the property was found by, and is already the map's key type.
			SinglePropertyAttribute.Add(PropertyPair.Key, PropertyPair.Value);

			FText ImportFailReason;
			if (!FJsonObjectConverter::JsonAttributesToUStruct(SinglePropertyAttribute, GeneratedClass, GeneratedCDO, CPF_Edit, CPF_Transient, /*bStrictMode=*/false, &ImportFailReason))
			{
				UE_LOG(LogTemp, Warning, TEXT("Widget class property '%s' of '%s' could not be imported and was skipped: %s"), *Property->GetName(), *Blueprint->GetName(), *ImportFailReason.ToString());
			}
		}

		// Native inherited properties (e.g. UWidget's 'Visibility') are not in the class's PostConstructLink:
		// new instances take them from CustomPropertyListForPostConstruction, a snapshot of the native properties
		// whose CDO value differs from the native CDO, rebuilt only on compile. Writing the CDO afterwards leaves
		// that snapshot stale (the value still matched the native default at compile time), so instances would
		// keep the native constructor's value. Rebuild it like the editor does after a class-defaults edit.
		if (UBlueprintGeneratedClass* GeneratedBPClass = Cast<UBlueprintGeneratedClass>(GeneratedClass))
		{
			GeneratedBPClass->UpdateCustomPropertyListForPostConstruction();
		}
		return true;
	}

	FString SerializeJsonObjectToString(const TSharedRef<FJsonObject>& JsonObject)
	{
		// Condensed policy: no newlines or indentation, so complex widget trees stay compact on disk.
		FString JsonString;
		TSharedRef<TJsonWriter<TCHAR, TCondensedJsonPrintPolicy<TCHAR>>> Writer = TJsonWriterFactory<TCHAR, TCondensedJsonPrintPolicy<TCHAR>>::Create(&JsonString);
		FJsonSerializer::Serialize(JsonObject, Writer);
		return JsonString;
	}

	/**
	 * Returns the directory the given Widget Blueprint's own serialization file belongs in — the directory of
	 * its content package — so referenced Blueprints are serialized next to their own asset rather than next to
	 * the Blueprint that references them. Falls back to FallbackDir (the referencing file's directory) when the
	 * package cannot be mapped to a file path, e.g. transient Blueprints.
	 */
	FString GetOwningDirectoryForWidgetBlueprint(const UBaseWidgetBlueprint* WidgetBlueprint, const FString& FallbackDir)
	{
		FString PackageFilename;
		if (FPackageName::TryConvertLongPackageNameToFilename(WidgetBlueprint->GetOutermost()->GetName(), PackageFilename))
		{
			const FString PackageDir = FPaths::GetPath(PackageFilename);
			if (!PackageDir.IsEmpty())
			{
				return PackageDir;
			}
		}
		return FallbackDir;
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

	/**
	 * Asks the predicate for the class of a referenced Widget Blueprint's serialized widget tree, reusing the
	 * result of an earlier lookup during the same load. The same Widget Blueprint can be referenced from several
	 * places in one tree (widget nodes of its class in panels/named slots, and ListView-style 'EntryWidgetClass'
	 * properties), and without the cache each reference would trigger the predicate again, which may return a
	 * different transient generated class per call. Successful resolutions are cached by the serialized file,
	 * which uniquely identifies the referenced Widget Blueprint.
	 */
	UClass* ResolveSerializedWidgetClass(const FString& SerializedFile, TFunctionRef<UClass*(FString)> WidgetClassPredicate, TMap<FString, UClass*>& ResolvedSerializedClasses)
	{
		if (UClass* const* ResolvedClass = ResolvedSerializedClasses.Find(SerializedFile))
		{
			return *ResolvedClass;
		}

		UClass* PredicateWidgetClass = WidgetClassPredicate(SerializedFile);
		if (PredicateWidgetClass)
		{
			ResolvedSerializedClasses.Add(SerializedFile, PredicateWidgetClass);
		}
		return PredicateWidgetClass;
	}

	/**
	 * Parses a referenced Widget Blueprint's generated class path — the format recorded for a Blueprint class
	 * widget node's 'class' field and the 'EntryWidgetClass' property (e.g.
	 * '/Script/UMG.WidgetBlueprintGeneratedClass'/Game/UI/WBP_MyButton.WBP_MyButton_C'') — into the referenced
	 * Widget Blueprint's package path and generated class name ('/Game/UI/WBP_MyButton', 'WBP_MyButton_C').
	 */
	bool ParseReferencedWidgetClassPath(const FString& ClassPath, FString& OutPackagePath, FString& OutClassName)
	{
		// The export text form Class'ObjectPath' used by the 'EntryWidgetClass' property value. A bare class
		// path is accepted too and parsed as the object path itself.
		FString ObjectPath = ClassPath;
		FString ExportedClassName;
		FPackageName::ParseExportTextPath(ClassPath, &ExportedClassName, &ObjectPath);

		int32 DotIndex = INDEX_NONE;
		if (!ObjectPath.FindLastChar(TEXT('.'), DotIndex))
		{
			return false;
		}
		OutPackagePath = ObjectPath.Left(DotIndex);
		OutClassName = ObjectPath.RightChop(DotIndex + 1);
		return OutClassName.EndsWith(TEXT("_C"));
	}

	/**
	 * Derives the serialized json file of a referenced Widget Blueprint from its generated class path
	 * ('/Game/UI/WBP_Entry.WBP_Entry_C' -> '<content dir>/UI/WBP_Entry.json'), if the file exists. The file is
	 * searched in the directory of the class path — where the referenced Widget Blueprint's own serialized
	 * widget tree is kept — and not next to the loaded json file: the referenced Widget Blueprint can live in
	 * a completely different directory.
	 */
	FString FindSerializedFileForClassPath(const FString& ClassPath)
	{
		FString PackagePath;
		FString ClassName;
		if (!ParseReferencedWidgetClassPath(ClassPath, PackagePath, ClassName))
		{
			return FString();
		}

		FString SerializedFile;
		if (!FPackageName::TryConvertLongPackageNameToFilename(PackagePath, SerializedFile, TEXT(".json")))
		{
			return FString();
		}
		return FPaths::FileExists(SerializedFile) ? SerializedFile : FString();
	}

	/**
	 * Resolves the class of a serialized widget node. Native classes are stored by short name (e.g.
	 * 'TextBlock'). A Blueprint-generated class is a reference to another Widget Blueprint — e.g. a widget
	 * instance of a Blueprint class placed in a panel or a named slot — and is stored by its full class path
	 * like an 'EntryWidgetClass' (see ResolveEntryWidgetClass): the referenced Widget Blueprint's serialized
	 * widget tree is located from the class path and the predicate is asked first for the class to use (e.g. a
	 * transient Widget Blueprint rebuilt from that file), so that it takes priority over the original Widget
	 * Blueprint asset. Old files store such nodes by short name ('WBP_MyButton_C') without a class path; for
	 * those the serialized tree is searched next to the loaded file. The class is finally resolved by name
	 * when no serialized file exists or the predicate provides no class.
	 */
	UClass* ResolveWidgetClassForNode(const FString& ClassName, const FString& JsonDir, TFunctionRef<UClass*(FString)> WidgetClassPredicate, TMap<FString, UClass*>& ResolvedSerializedClasses)
	{
		FString PackagePath;
		FString GeneratedClassName;
		if (ParseReferencedWidgetClassPath(ClassName, PackagePath, GeneratedClassName))
		{
			const FString SerializedFile = FindSerializedFileForClassPath(ClassName);
			if (!SerializedFile.IsEmpty())
			{
				if (UClass* PredicateWidgetClass = ResolveSerializedWidgetClass(SerializedFile, WidgetClassPredicate, ResolvedSerializedClasses))
				{
					return PredicateWidgetClass;
				}
			}
			return ResolveWidgetClass(GeneratedClassName);
		}

		if (ClassName.EndsWith(TEXT("_C")))
		{
			const FString SerializedFile = FPaths::Combine(JsonDir, ClassName.LeftChop(2) + TEXT(".json"));
			if (FPaths::FileExists(SerializedFile))
			{
				if (UClass* PredicateWidgetClass = ResolveSerializedWidgetClass(SerializedFile, WidgetClassPredicate, ResolvedSerializedClasses))
				{
					return PredicateWidgetClass;
				}
			}
		}

		return ResolveWidgetClass(ClassName);
	}

	/**
	 * Handles the 'EntryWidgetClass' property after the widget properties have been imported. The
	 * property itself is excluded from the bulk import (see LoadWidgetFromJsonObject); it is resolved
	 * here with the following priority:
	 * 1. The serialized json file of the entry widget, located from the recorded class path (see
	 *    FindSerializedFileForClassPath) and passed to the predicate, which may create a transient
	 *    Widget Blueprint (e.g. derived from a registered native entry class) so that the entry widget
	 *    is rebuilt from its serialized tree.
	 * 2. The generated class of the original entry Widget Blueprint asset, when no serialized file
	 *    exists or the predicate could not provide a class.
	 */
	bool ResolveEntryWidgetClass(UWidget* Widget, const TSharedPtr<FJsonObject>& PropertiesObject, TFunctionRef<UClass*(FString)> EntryWidgetClassPredicate, TMap<FString, UClass*>& ResolvedSerializedClasses)
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
		const FString SerializedFile = FindSerializedFileForClassPath(EntryClassPath);
		if (!SerializedFile.IsEmpty())
		{
			// Reuse the class resolved earlier in this load when the same entry Widget Blueprint is shared by
			// several widgets (see ResolveSerializedWidgetClass).
			EntryWidgetClass = ResolveSerializedWidgetClass(SerializedFile, EntryWidgetClassPredicate, ResolvedSerializedClasses);
		}

		// Fall back to the generated class of the original entry Widget Blueprint asset, e.g. when no
		// serialized file exists in the class path's directory or the entry was not registered with the
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

		UE_LOG(LogTemp, Warning, TEXT("Entry widget class '%s' could be resolved neither from its serialized json file nor from its original asset."), *EntryClassPath);
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

	/** Recursively builds a widget (and its children) from a serialized json node into the given tree.
	 *  ResolvedSerializedClasses caches the classes the predicate returned for referenced Widget Blueprints so
	 *  that a Blueprint shared by several nodes/entry properties resolves to one single generated class. */
	UWidget* LoadWidgetFromJsonObject(const TSharedRef<FJsonObject>& WidgetObject, UWidgetTree* WidgetTree, UPanelWidget* ParentPanel, const FString& JsonDir, TFunctionRef<UClass*(FString)> WidgetClassPredicate, TMap<FString, UClass*>& ResolvedSerializedClasses, UWidget* ParentNamedSlotHost = nullptr, FName ParentNamedSlotName = NAME_None)
	{
		FString WidgetClassName;
		if (!WidgetObject->TryGetStringField(TEXT("class"), WidgetClassName) || WidgetClassName.IsEmpty())
		{
			UE_LOG(LogTemp, Warning, TEXT("Skipped a serialized widget node without a 'class' field."));
			return nullptr;
		}

		// A node whose class is a Blueprint-generated class references another Widget Blueprint (e.g. a widget
		// instance placed in a panel or a named slot) and is resolved through the predicate first, like a
		// ListView 'EntryWidgetClass' (see ResolveWidgetClassForNode).
		UClass* WidgetClass = ResolveWidgetClassForNode(WidgetClassName, JsonDir, WidgetClassPredicate, ResolvedSerializedClasses);
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
			if (!ResolveEntryWidgetClass(NewWidget, *PropertiesObjectPtr, WidgetClassPredicate, ResolvedSerializedClasses)) return nullptr;
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
					if (!LoadWidgetFromJsonObject(ChildObjectPtr.ToSharedRef(), WidgetTree, PanelWidget, JsonDir, WidgetClassPredicate, ResolvedSerializedClasses))
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
					if (!LoadWidgetFromJsonObject(SlotContentObjectPtr->ToSharedRef(), WidgetTree, /*ParentPanel=*/nullptr, JsonDir, WidgetClassPredicate, ResolvedSerializedClasses, /*ParentNamedSlotHost=*/NewWidget, FName(*SlotNameString)))
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
	return SerializeWidgetBlueprintInternal(WidgetBlueprint, DestinationDir, /*bAskAboutReferences=*/true, SerializedBlueprints);
}

bool FEditorWidgetTreeSerialization::SerializeWidgetBlueprintInternal(UBaseWidgetBlueprint* WidgetBlueprint, const FString& DestinationDir, bool bAskAboutReferences, TSet<UBaseWidgetBlueprint*>& SerializedBlueprints)
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

	// References to other Widget Blueprints — widget instances of a Blueprint class placed in panels or named
	// slots, and ListView-style 'EntryWidgetClass' properties — are serialized like everything else, while the
	// Widget Blueprints they reference are collected for the user prompt below.
	TArray<UBaseWidgetBlueprint*> ReferencedWidgetBlueprints;
	TSharedRef<FJsonObject> TreeObject = MakeShared<FJsonObject>();
	if (UWidget* RootWidget = WidgetTree->RootWidget)
	{
		TreeObject = ConvertWidgetToJsonObject(RootWidget, ReferencedWidgetBlueprints);
	}
	RootObject->SetObjectField(TEXT("widget_tree"), TreeObject);

	// The Blueprint's member variables, in their own 'variables' array (see ConvertVariablesToJsonArray).
	RootObject->SetArrayField(TEXT("variables"), ConvertVariablesToJsonArray(WidgetBlueprint));

	// The widget class's own properties (class defaults of the Blueprint's widget object itself), in their
	// own 'widget_properties' object (see ConvertWidgetClassPropertiesToJsonObject).
	RootObject->SetObjectField(TEXT("widget_properties"), ConvertWidgetClassPropertiesToJsonObject(WidgetBlueprint));

	const FString JsonString = SerializeJsonObjectToString(RootObject);
	const FString DestinationFile = FPaths::Combine(DestinationDir, WidgetBlueprint->GetName() + TEXT(".json"));
	if (!FFileHelper::SaveStringToFile(JsonString, *DestinationFile))
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to save the widget tree of '%s' to '%s'."), *WidgetBlueprint->GetName(), *DestinationFile);
		return false;
	}
	UE_LOG(LogTemp, Log, TEXT("Serialized the widget tree of '%s' to '%s'."), *WidgetBlueprint->GetName(), *DestinationFile);

	// Ask whether to also serialize the referenced Widget Blueprints (widget instances of a Blueprint class
	// placed in panels/named slots and 'entryWidgetClass' properties)
	ReferencedWidgetBlueprints.RemoveAll([&SerializedBlueprints](UBaseWidgetBlueprint* ReferencedWidgetBlueprint)
	{
		return SerializedBlueprints.Contains(ReferencedWidgetBlueprint);
	});
	if (!ReferencedWidgetBlueprints.IsEmpty())
	{
		bool bSerializeReferences = true;
		if (bAskAboutReferences)
		{
			TArray<FText> ReferencedNameTexts;
			for (const UBaseWidgetBlueprint* ReferencedWidgetBlueprint : ReferencedWidgetBlueprints)
			{
				ReferencedNameTexts.Add(FText::FromString(ReferencedWidgetBlueprint->GetName()));
			}
			const FText Prompt = FText::Format(
				LOCTEXT("SerializeReferencedBlueprintsPrompt", "The widget tree references the following Widget Blueprint(s):\n\n{0}\n\nDo you also want to serialize them to their own directories (next to each Blueprint asset)?"),
				FText::Join(FText::FromString(TEXT("\n")), ReferencedNameTexts));
			bSerializeReferences = FMessageDialog::Open(EAppMsgType::YesNo, Prompt, LOCTEXT("SerializeReferencedBlueprintsTitle", "Serialize Referenced Widget Blueprints")) == EAppReturnType::Yes;
		}

		if (bSerializeReferences)
		{
			for (UBaseWidgetBlueprint* ReferencedWidgetBlueprint : ReferencedWidgetBlueprints)
			{
				// Each referenced Blueprint is serialized next to its own asset, not in the referencing
				// Blueprint's directory, so the class-path-based lookup during deserialization finds it.
				const FString ReferencedDir = GetOwningDirectoryForWidgetBlueprint(ReferencedWidgetBlueprint, DestinationDir);
				SerializeWidgetBlueprintInternal(ReferencedWidgetBlueprint, ReferencedDir, /*bAskAboutReferences=*/false, SerializedBlueprints);
			}
		}
	}

	return true;
}

bool FEditorWidgetTreeSerialization::LoadSerializationToWidgetBlueprint(const FString& FilePath, UBaseWidgetBlueprint* WidgetBlueprint, bool bLoadVariables, TFunctionRef<UClass*(FString)> WidgetClassPredicate, TSharedPtr<FJsonObject>* OutWidgetClassProperties, TMap<FName, FString>* OutVariableDefaultValues)
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

	// An empty tree object means the Widget Blueprint had no root widget. The cache makes sure the predicate is
	// asked only once per referenced Widget Blueprint even when it is shared by several nodes/entry properties.
	if ((*TreeObjectPtr)->HasField(TEXT("class")))
	{
		TMap<FString, UClass*> ResolvedSerializedClasses;
		if (!LoadWidgetFromJsonObject(TreeObjectPtr->ToSharedRef(), WidgetTree, /*ParentPanel=*/nullptr, FPaths::GetPath(FilePath), WidgetClassPredicate, ResolvedSerializedClasses))
		{
			UE_LOG(LogTemp, Error, TEXT("Failed to build the widget tree from the file '%s'."), *FilePath);
			return false;
		}
	}

	// The Blueprint's member variables, imported on request (see LoadVariablesFromJsonArray). Their default
	// values belong in the generated class CDO (see ApplyVariableDefaultValuesToCDO): when the CDO already
	// exists (e.g. loading into an already-compiled Blueprint) they are applied right away; a freshly created
	// Blueprint has no CDO until the caller compiles it, so the values are handed back instead, for the caller
	// to apply after its compile.
	if (bLoadVariables)
	{
		const TArray<TSharedPtr<FJsonValue>>* VariableValues = nullptr;
		if (RootObject->TryGetArrayField(TEXT("variables"), VariableValues))
		{
			TMap<FName, FString> VariableDefaultValues;
			LoadVariablesFromJsonArray(*VariableValues, WidgetBlueprint, &VariableDefaultValues);

			UClass* GeneratedClass = WidgetBlueprint->GeneratedClass;
			if (GeneratedClass && GeneratedClass->GetDefaultObject(/*bCreateIfNeeded=*/false))
			{
				ApplyVariableDefaultValuesToCDO(VariableDefaultValues, WidgetBlueprint);
			}
			else if (OutVariableDefaultValues)
			{
				*OutVariableDefaultValues = MoveTemp(VariableDefaultValues);
			}
		}
	}

	// The widget class's own properties (see ApplyWidgetClassPropertiesObject). They live in the generated
	// class CDO: if it already exists (e.g. loading into an already-compiled Blueprint) they are applied
	// right away; a freshly created Blueprint has no CDO until the caller compiles it, so the parsed object
	// is handed back instead, for the caller to load after its compile through LoadWidgetClassPropertiesToCDO.
	const TSharedPtr<FJsonObject>* WidgetClassPropertiesPtr = nullptr;
	if (RootObject->TryGetObjectField(TEXT("widget_properties"), WidgetClassPropertiesPtr) && WidgetClassPropertiesPtr->IsValid())
	{
		UClass* GeneratedClass = WidgetBlueprint->GeneratedClass;
		if (GeneratedClass && GeneratedClass->GetDefaultObject(/*bCreateIfNeeded=*/false))
		{
			ApplyWidgetClassPropertiesObject(*WidgetClassPropertiesPtr, WidgetBlueprint);
		}
		else if (OutWidgetClassProperties)
		{
			*OutWidgetClassProperties = *WidgetClassPropertiesPtr;
		}
	}

	WidgetBlueprint->MarkPackageDirty();
	UE_LOG(LogTemp, Log, TEXT("Loaded the widget tree of '%s' from '%s'."), *WidgetBlueprint->GetName(), *FilePath);

	return true;
}

bool FEditorWidgetTreeSerialization::LoadWidgetClassPropertiesToCDO(const TSharedPtr<FJsonObject>& WidgetClassProperties, UBaseWidgetBlueprint* WidgetBlueprint)
{
	return ApplyWidgetClassPropertiesObject(WidgetClassProperties, WidgetBlueprint);
}

void FEditorWidgetTreeSerialization::ApplyVariableDefaultValuesToCDO(const TMap<FName, FString>& VariableDefaultValues, UBaseWidgetBlueprint* WidgetBlueprint)
{
	if (VariableDefaultValues.IsEmpty())
	{
		return;
	}

	UClass* GeneratedClass = WidgetBlueprint ? WidgetBlueprint->GeneratedClass : nullptr;
	UObject* GeneratedCDO = GeneratedClass ? GeneratedClass->GetDefaultObject(/*bCreateIfNeeded=*/false) : nullptr;
	if (!GeneratedCDO)
	{
		UE_LOG(LogTemp, Warning, TEXT("'%s' has no generated class CDO; its variable default values were not applied."), *WidgetBlueprint->GetName());
		return;
	}

	for (const TPair<FName, FString>& DefaultValuePair : VariableDefaultValues)
	{
		FProperty* Property = FindFProperty<FProperty>(GeneratedClass, DefaultValuePair.Key);
		if (!Property)
		{
			continue;
		}

		// The same import path the compiler itself uses to apply a variable's DefaultValue to the CDO
		// (FKismetCompilerContext::CopyTermDefaultsToDefaultObject).
		if (!FBlueprintEditorUtils::PropertyValueFromString(Property, DefaultValuePair.Value, reinterpret_cast<uint8*>(GeneratedCDO), GeneratedCDO))
		{
			UE_LOG(LogTemp, Warning, TEXT("The default value '%s' of variable '%s' of '%s' could not be imported and was skipped."), *DefaultValuePair.Value, *DefaultValuePair.Key.ToString(), *WidgetBlueprint->GetName());
		}
	}
}

#undef LOCTEXT_NAMESPACE
