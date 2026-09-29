// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "UMGEditorPyPluginSettings.generated.h"

/**
 * 
 */
UCLASS(config=EditorPerProjectUserSettings)
class UUMGEditorPyPluginSettings : public UDeveloperSettings
{
	GENERATED_BODY()
	
public:
	UUMGEditorPyPluginSettings();

#if WITH_EDITOR
	//~ Begin UDeveloperSettings Interface
	virtual FText GetSectionText() const override;
	//~ End UDeveloperSettings Interface
#endif

	/**
	 * Should Developer Mode be enabled?
	 *
	 * This will enable the entry on Content Browser's context menu to serialize the EditorUtilityWidget Blueprint Design.
	 */
	UPROPERTY(config, EditAnywhere, Category="Serialization (For Python EditorUtilityWidget)", meta =(ConfigRestartRequired=true, DisplayName="Developer Mode"))
	bool bDeveloperMode;
};
