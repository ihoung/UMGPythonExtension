// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

class UBaseWidgetBlueprint;
class FClassProperty;
class FJsonObject;
/**
 *
 */
class FEditorWidgetTreeSerialization
{
public:
	FEditorWidgetTreeSerialization();
	~FEditorWidgetTreeSerialization();

	/** Serializes the widget tree of a single Widget Blueprint to '<DestinationDir>/<AssetName>.json'.
	 *  Asks the user whether to also serialize the Widget Blueprints referenced by the widget tree — widget
	 *  instances of a Blueprint class placed in panels/named slots and ListView 'entryWidgetClass' properties —
	 *  to the same directory. */
	static bool SerializeWidgetBlueprint(UBaseWidgetBlueprint* WidgetBlueprint, const FString& DestinationDir);

	/** Create a Widget Blueprint and load the serialized Json file as its widget tree.
	 *  bLoadVariables controls whether the Blueprint member variables serialized in the json are also
	 *  loaded into the Blueprint's NewVariables; the WidgetTree's own variables are restored from the
	 *  widget tree's 'bIsVariable' flags instead.
	 *  WidgetClassPredicate is asked for the class of each referenced Widget Blueprint (a widget instance of a
	 *  Blueprint class in the tree, or a ListView 'entryWidgetClass') when its serialized widget tree exists next
	 *  to the loaded file; returning nullptr keeps the original Widget Blueprint. A Widget Blueprint referenced
	 *  from several places resolves to one single generated class: the predicate is asked only once per load.
	 *  The widget class's own properties (serialized in 'widget_properties') are applied to the generated class
	 *  CDO when it already exists; when it does not (a freshly created Blueprint the caller compiles
	 *  afterwards), the parsed properties are handed back through OutWidgetClassProperties so the caller can
	 *  load them after its compile with LoadWidgetClassPropertiesToCDO.
	 *  The member variables' default values follow the same rule: the compiler only applies them to the CDO
	 *  from its reinstancing stage, which a freshly created Blueprint never reaches, so when the CDO does not
	 *  exist yet they are handed back through OutVariableDefaultValues for the caller to apply after its
	 *  compile with ApplyVariableDefaultValuesToCDO. */
	static bool LoadSerializationToWidgetBlueprint(const FString& FilePath, UBaseWidgetBlueprint* WidgetBlueprint, bool bLoadVariables, TFunctionRef<UClass*(FString)> WidgetClassPredicate=[](FString){return nullptr;}, TSharedPtr<FJsonObject>* OutWidgetClassProperties = nullptr, TMap<FName, FString>* OutVariableDefaultValues = nullptr);

	/** Applies the 'widget_properties' parsed by LoadSerializationToWidgetBlueprint to the Blueprint's
	 *  generated class CDO, once the Blueprint has been compiled (the CDO does not exist before that). */
	static bool LoadWidgetClassPropertiesToCDO(const TSharedPtr<FJsonObject>& WidgetClassProperties, UBaseWidgetBlueprint* WidgetBlueprint);

	/** Applies the variable default values handed back by LoadSerializationToWidgetBlueprint to the
	 *  Blueprint's generated class CDO, once the Blueprint has been compiled (the CDO does not exist before
	 *  that). */
	static void ApplyVariableDefaultValuesToCDO(const TMap<FName, FString>& VariableDefaultValues, UBaseWidgetBlueprint* WidgetBlueprint);

private:
	static bool SerializeWidgetBlueprintInternal(UBaseWidgetBlueprint* WidgetBlueprint, const FString& DestinationDir, bool bAskAboutReferences, TSet<UBaseWidgetBlueprint*>& SerializedBlueprints);
};
