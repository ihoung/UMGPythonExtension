// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Styling/SlateStyle.h"
#include "Styling/SlateStyleRegistry.h"

/**
 * 
 */
class FEditorWidgetTreeExporterStyle
	: public FSlateStyleSet
{
public:
	static FName StyleName;

	/** Access the singleton instance for this style set */
	static FEditorWidgetTreeExporterStyle& Get();

private:
	FEditorWidgetTreeExporterStyle();
	~FEditorWidgetTreeExporterStyle();
};
