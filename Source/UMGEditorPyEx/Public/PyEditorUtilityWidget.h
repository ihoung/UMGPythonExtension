// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Layout/Geometry.h"
#include "Input/Events.h"
#include "Input/Reply.h"
#include "EditorUtilityWidget.h"
#include "PySlateWrapperTypes.h"
#include "PyEditorUtilityWidget.generated.h"

/**
 * 
 */
UCLASS(Abstract, MinimalAPI)
class UPyEditorUtilityWidget : public UEditorUtilityWidget
{
	GENERATED_BODY()
	
protected:

	UMGEDITORPYEX_API virtual FReply NativeOnKeyChar(const FGeometry& InGeometry, const FCharacterEvent& InCharEvent) override;
	UMGEDITORPYEX_API virtual FReply NativeOnPreviewKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent) override;
	UMGEDITORPYEX_API virtual FReply NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent) override;
	UMGEDITORPYEX_API virtual FReply NativeOnKeyUp(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent) override;
	UMGEDITORPYEX_API virtual FReply NativeOnAnalogValueChanged(const FGeometry& InGeometry, const FAnalogInputEvent& InAnalogEvent) override;
	UMGEDITORPYEX_API virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	UMGEDITORPYEX_API virtual FReply NativeOnPreviewMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	UMGEDITORPYEX_API virtual FReply NativeOnMouseButtonUp(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	UMGEDITORPYEX_API virtual FReply NativeOnMouseMove(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	UMGEDITORPYEX_API virtual void NativeOnMouseEnter(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	UMGEDITORPYEX_API virtual void NativeOnMouseLeave(const FPointerEvent& InMouseEvent) override;
	UMGEDITORPYEX_API virtual FReply NativeOnMouseWheel(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	UMGEDITORPYEX_API virtual FReply NativeOnMouseButtonDoubleClick(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	UMGEDITORPYEX_API virtual void NativeOnDragDetected(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent, UDragDropOperation*& OutOperation) override;
	UMGEDITORPYEX_API virtual void NativeOnDragEnter(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation) override;
	UMGEDITORPYEX_API virtual void NativeOnDragLeave(const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation) override;
	UMGEDITORPYEX_API virtual bool NativeOnDragOver(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation) override;
	UMGEDITORPYEX_API virtual bool NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation) override;
	UMGEDITORPYEX_API virtual void NativeOnDragCancelled(const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation) override;
	UMGEDITORPYEX_API virtual FReply NativeOnTouchGesture(const FGeometry& InGeometry, const FPointerEvent& InGestureEvent) override;
	UMGEDITORPYEX_API virtual FReply NativeOnTouchStarted(const FGeometry& InGeometry, const FPointerEvent& InGestureEvent) override;
	UMGEDITORPYEX_API virtual FReply NativeOnTouchMoved(const FGeometry& InGeometry, const FPointerEvent& InGestureEvent) override;
	UMGEDITORPYEX_API virtual FReply NativeOnTouchEnded(const FGeometry& InGeometry, const FPointerEvent& InGestureEvent) override;
	UMGEDITORPYEX_API virtual FReply NativeOnMotionDetected(const FGeometry& InGeometry, const FMotionEvent& InMotionEvent) override;
	UMGEDITORPYEX_API virtual FReply NativeOnTouchForceChanged(const FGeometry& MyGeometry, const FPointerEvent& TouchEvent) override;

	UFUNCTION(BlueprintImplementableEvent, BlueprintCosmetic, Category = "Input")
	FPyEventReply PyOnKeyChar(const FGeometry& MyGeometry, const FPyCharacterEvent& InCharacterEvent);

	UFUNCTION(BlueprintImplementableEvent, Category = "Input")
	FPyEventReply PyOnPreviewKeyDown(FGeometry MyGeometry, FPyKeyEvent InKeyEvent);

	UFUNCTION(BlueprintImplementableEvent, BlueprintCosmetic, Category = "Input")
	FPyEventReply PyOnKeyDown(FGeometry MyGeometry, FPyKeyEvent InKeyEvent);

	UFUNCTION(BlueprintImplementableEvent, BlueprintCosmetic, Category = "Input")
	FPyEventReply PyOnKeyUp(FGeometry MyGeometry, FPyKeyEvent InKeyEvent);

	UFUNCTION(BlueprintImplementableEvent, Category = "Input")
	FPyEventReply PyOnAnalogValueChanged(FGeometry MyGeometry, FPyAnalogInputEvent InAnalogInputEvent);

	UFUNCTION(BlueprintImplementableEvent, BlueprintCosmetic, Category = "Mouse")
	FPyEventReply PyOnMouseButtonDown(FGeometry MyGeometry, FPyPointerEvent MouseEvent);

	UFUNCTION(BlueprintImplementableEvent, BlueprintCosmetic, Category = "Mouse")
	FPyEventReply PyOnPreviewMouseButtonDown(FGeometry MyGeometry, const FPyPointerEvent& MouseEvent);

	UFUNCTION(BlueprintImplementableEvent, BlueprintCosmetic, Category = "Mouse")
	FPyEventReply PyOnMouseButtonUp(FGeometry MyGeometry, const FPyPointerEvent& MouseEvent);

	UFUNCTION(BlueprintImplementableEvent, BlueprintCosmetic, Category = "Mouse")
	FPyEventReply PyOnMouseMove(FGeometry MyGeometry, const FPyPointerEvent& MouseEvent);

	UFUNCTION(BlueprintImplementableEvent, BlueprintCosmetic, Category = "Mouse")
	void PyOnMouseEnter(FGeometry MyGeometry, const FPyPointerEvent& MouseEvent);

	UFUNCTION(BlueprintImplementableEvent, BlueprintCosmetic, Category = "Mouse")
	void PyOnMouseLeave(const FPyPointerEvent& MouseEvent);

	UFUNCTION(BlueprintImplementableEvent, BlueprintCosmetic, Category = "Mouse")
	FPyEventReply PyOnMouseWheel(FGeometry MyGeometry, const FPyPointerEvent& MouseEvent);

	UFUNCTION(BlueprintImplementableEvent, BlueprintCosmetic, Category = "Mouse")
	FPyEventReply PyOnMouseButtonDoubleClick(FGeometry InMyGeometry, const FPyPointerEvent& InMouseEvent);

	UFUNCTION(BlueprintImplementableEvent, BlueprintCosmetic, Category = "Drag and Drop")
	void PyOnDragDetected(FGeometry MyGeometry, const FPyPointerEvent& PointerEvent, UDragDropOperation*& Operation);

	UFUNCTION(BlueprintImplementableEvent, BlueprintCosmetic, Category = "Drag and Drop")
	void PyOnDragCancelled(const FPyPointerEvent& PointerEvent, UDragDropOperation* Operation);

	UFUNCTION(BlueprintImplementableEvent, BlueprintCosmetic, Category = "Drag and Drop")
	void PyOnDragEnter(FGeometry MyGeometry, FPyPointerEvent PointerEvent, UDragDropOperation* Operation);

	UFUNCTION(BlueprintImplementableEvent, BlueprintCosmetic, Category = "Drag and Drop")
	void PyOnDragLeave(FPyPointerEvent PointerEvent, UDragDropOperation* Operation);

	UFUNCTION(BlueprintImplementableEvent, BlueprintCosmetic, Category = "Drag and Drop")
	bool PyOnDragOver(FGeometry MyGeometry, FPyPointerEvent PointerEvent, UDragDropOperation* Operation);

	UFUNCTION(BlueprintImplementableEvent, BlueprintCosmetic, Category = "Drag and Drop")
	bool PyOnDrop(FGeometry MyGeometry, FPyPointerEvent PointerEvent, UDragDropOperation* Operation);

	UFUNCTION(BlueprintImplementableEvent, BlueprintCosmetic, Category = "Touch Input")
	FPyEventReply PyOnTouchGesture(FGeometry MyGeometry, const FPyPointerEvent& GestureEvent);

	UFUNCTION(BlueprintImplementableEvent, BlueprintCosmetic, Category = "Touch Input")
	FPyEventReply PyOnTouchStarted(FGeometry MyGeometry, const FPyPointerEvent& InTouchEvent);

	UFUNCTION(BlueprintImplementableEvent, BlueprintCosmetic, Category = "Touch Input")
	FPyEventReply PyOnTouchMoved(FGeometry MyGeometry, const FPyPointerEvent& InTouchEvent);

	UFUNCTION(BlueprintImplementableEvent, BlueprintCosmetic, Category = "Touch Input")
	FPyEventReply PyOnTouchEnded(FGeometry MyGeometry, const FPyPointerEvent& InTouchEvent);

	UFUNCTION(BlueprintImplementableEvent, BlueprintCosmetic, Category = "Touch Input")
	FPyEventReply PyOnMotionDetected(FGeometry MyGeometry, FPyMotionEvent InMotionEvent);

	UFUNCTION(BlueprintImplementableEvent, BlueprintCosmetic, Category = "Touch Input")
	FPyEventReply PyOnTouchForceChanged(FGeometry MyGeometry, const FPyPointerEvent& InTouchEvent);


};
