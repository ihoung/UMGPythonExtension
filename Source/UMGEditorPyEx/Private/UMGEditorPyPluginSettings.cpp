// Fill out your copyright notice in the Description page of Project Settings.


#include "UMGEditorPyPluginSettings.h"

#define LOCTEXT_NAMESPACE "UMGEditorPyEx"

UUMGEditorPyPluginSettings::UUMGEditorPyPluginSettings()
{
	CategoryName = TEXT("Plugins");
	SectionName  = TEXT("PythonUMG");
}

#if WITH_EDITOR
FText UUMGEditorPyPluginSettings::GetSectionText() const
{
	return LOCTEXT("UserSettingsDisplayName", "Python UMG");
}
#endif

#undef LOCTEXT_NAMESPACE
