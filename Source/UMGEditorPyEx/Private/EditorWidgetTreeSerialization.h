// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

class UBaseWidgetBlueprint;
class FClassProperty;
class FJsonObject;

/** The CDO-bound values LoadSerializationToWidgetBlueprint parsed but could not yet write into the generated
 *  class CDO — a freshly created Blueprint has no CDO until the caller compiles it. Both the widget class's own
 *  properties and the member variables' default values are destined for the same generated class CDO, so they
 *  are handed back together through one out parameter and written after the compile with ApplyPendingCDOValues. */
struct FPendingCDOValues
{
	/** The parsed 'widget_properties' object: the widget class's own defaults, taken by widget instances. */
	TSharedPtr<FJsonObject> WidgetClassProperties;
	/** The member variables' default values, keyed by variable name, exported as text. */
	TMap<FName, FString> VariableDefaultValues;
};

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
	 *  The widget class's own properties (serialized in 'widget_properties') and the member variables' default
	 *  values both belong in the generated class CDO. When it already exists (e.g. loading into an
	 *  already-compiled Blueprint) they are applied right away; when it does not (a freshly created Blueprint
	 *  the caller compiles afterwards) they are handed back through OutPendingCDOValues for the caller to write
	 *  after its compile with ApplyPendingCDOValues. */
	static bool LoadSerializationToWidgetBlueprint(const FString& FilePath, UBaseWidgetBlueprint* WidgetBlueprint, bool bLoadVariables, TFunctionRef<UClass*(FString)> WidgetClassPredicate=[](FString){return nullptr;}, FPendingCDOValues* OutPendingCDOValues = nullptr);

	/** Writes the CDO-bound values handed back by LoadSerializationToWidgetBlueprint — the widget class's own
	 *  properties and the member variables' default values — into the Blueprint's generated class CDO, once the
	 *  Blueprint has been compiled (the CDO does not exist before that). */
	static void ApplyPendingCDOValues(const FPendingCDOValues& PendingCDOValues, UBaseWidgetBlueprint* WidgetBlueprint);

private:
	static bool SerializeWidgetBlueprintInternal(UBaseWidgetBlueprint* WidgetBlueprint, const FString& DestinationDir, bool bAskAboutReferences, TSet<UBaseWidgetBlueprint*>& SerializedBlueprints);
};
