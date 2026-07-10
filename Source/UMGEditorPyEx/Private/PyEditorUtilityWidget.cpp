// Fill out your copyright notice in the Description page of Project Settings.


#include "PyEditorUtilityWidget.h"

FReply UPyEditorUtilityWidget::NativeOnKeyChar(const FGeometry& InGeometry, const FCharacterEvent& InCharEvent)
{
	return PyOnKeyChar(InGeometry, InCharEvent).ToNativeReply();
}

FReply UPyEditorUtilityWidget::NativeOnPreviewKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent)
{
	return PyOnPreviewKeyDown(InGeometry, InKeyEvent).ToNativeReply();
}

FReply UPyEditorUtilityWidget::NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent)
{
	return PyOnKeyDown(InGeometry, InKeyEvent).ToNativeReply();
}

FReply UPyEditorUtilityWidget::NativeOnKeyUp(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent)
{
	return PyOnKeyUp(InGeometry, InKeyEvent).ToNativeReply();
}

FReply UPyEditorUtilityWidget::NativeOnAnalogValueChanged(const FGeometry& InGeometry, const FAnalogInputEvent& InAnalogEvent)
{
	return PyOnAnalogValueChanged(InGeometry, InAnalogEvent).ToNativeReply();
}

FReply UPyEditorUtilityWidget::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	return PyOnMouseButtonDown(InGeometry, InMouseEvent).ToNativeReply();
}

FReply UPyEditorUtilityWidget::NativeOnPreviewMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	return PyOnPreviewMouseButtonDown(InGeometry, InMouseEvent).ToNativeReply();
}

FReply UPyEditorUtilityWidget::NativeOnMouseButtonUp(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	return PyOnMouseButtonUp(InGeometry, InMouseEvent).ToNativeReply();
}

FReply UPyEditorUtilityWidget::NativeOnMouseMove(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	return PyOnMouseMove(InGeometry, InMouseEvent).ToNativeReply();
}

void UPyEditorUtilityWidget::NativeOnMouseEnter(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	PyOnMouseEnter(InGeometry, InMouseEvent);
}

void UPyEditorUtilityWidget::NativeOnMouseLeave(const FPointerEvent& InMouseEvent)
{
	PyOnMouseLeave(InMouseEvent);
}

FReply UPyEditorUtilityWidget::NativeOnMouseWheel(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	return PyOnMouseWheel(InGeometry, InMouseEvent).ToNativeReply();
}

FReply UPyEditorUtilityWidget::NativeOnMouseButtonDoubleClick(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	return PyOnMouseButtonDoubleClick(InGeometry, InMouseEvent).ToNativeReply();
}

void UPyEditorUtilityWidget::NativeOnDragDetected(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent, UDragDropOperation*& OutOperation)
{
	PyOnDragDetected(InGeometry, InMouseEvent, OutOperation);
}

void UPyEditorUtilityWidget::NativeOnDragEnter(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation)
{
	PyOnDragEnter(InGeometry, InDragDropEvent, InOperation);
}

void UPyEditorUtilityWidget::NativeOnDragLeave(const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation)
{
	PyOnDragLeave(InDragDropEvent, InOperation);
}

bool UPyEditorUtilityWidget::NativeOnDragOver(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation)
{
	return PyOnDragOver(InGeometry, InDragDropEvent, InOperation);
}

bool UPyEditorUtilityWidget::NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation)
{
	return PyOnDrop(InGeometry, InDragDropEvent, InOperation);
}

void UPyEditorUtilityWidget::NativeOnDragCancelled(const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation)
{
	PyOnDragCancelled(InDragDropEvent, InOperation);
}

FReply UPyEditorUtilityWidget::NativeOnTouchGesture(const FGeometry& InGeometry, const FPointerEvent& InGestureEvent)
{
	return PyOnTouchGesture(InGeometry, InGestureEvent).ToNativeReply();
}

FReply UPyEditorUtilityWidget::NativeOnTouchStarted(const FGeometry& InGeometry, const FPointerEvent& InGestureEvent)
{
	return PyOnTouchStarted(InGeometry, InGestureEvent).ToNativeReply();
}

FReply UPyEditorUtilityWidget::NativeOnTouchMoved(const FGeometry& InGeometry, const FPointerEvent& InGestureEvent)
{
	return PyOnTouchMoved(InGeometry, InGestureEvent).ToNativeReply();
}

FReply UPyEditorUtilityWidget::NativeOnTouchEnded(const FGeometry& InGeometry, const FPointerEvent& InGestureEvent)
{
	return PyOnTouchEnded(InGeometry, InGestureEvent).ToNativeReply();
}

FReply UPyEditorUtilityWidget::NativeOnMotionDetected(const FGeometry& InGeometry, const FMotionEvent& InMotionEvent)
{
	return PyOnMotionDetected(InGeometry, InMotionEvent).ToNativeReply();
}

FReply UPyEditorUtilityWidget::NativeOnTouchForceChanged(const FGeometry& MyGeometry, const FPointerEvent& TouchEvent)
{
	return PyOnTouchForceChanged(MyGeometry, TouchEvent).ToNativeReply();
}
