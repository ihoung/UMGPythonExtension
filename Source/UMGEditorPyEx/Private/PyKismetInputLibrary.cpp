// Fill out your copyright notice in the Description page of Project Settings.


#include "PyKismetInputLibrary.h"


UPyKismetInputLibrary::UPyKismetInputLibrary(const FObjectInitializer& ObjectInitializer)
	:Super(ObjectInitializer)
{
}

void UPyKismetInputLibrary::CalibrateTilt() {
	UKismetInputLibrary::CalibrateTilt();
}

bool UPyKismetInputLibrary::EqualEqual_KeyKey(FKey A, FKey B) {
    return UKismetInputLibrary::EqualEqual_KeyKey(A, B);
}

bool UPyKismetInputLibrary::Key_IsModifierKey(const FKey& Key) {
    return UKismetInputLibrary::Key_IsModifierKey(Key);
}

bool UPyKismetInputLibrary::Key_IsGamepadKey(const FKey& Key) {
    return UKismetInputLibrary::Key_IsGamepadKey(Key);
}

bool UPyKismetInputLibrary::Key_IsMouseButton(const FKey& Key) {
    return UKismetInputLibrary::Key_IsMouseButton(Key);
}

bool UPyKismetInputLibrary::Key_IsKeyboardKey(const FKey& Key) {
    return UKismetInputLibrary::Key_IsKeyboardKey(Key);
}

bool UPyKismetInputLibrary::Key_IsVectorAxis(const FKey& Key) {
    return Key.IsAxis2D() || Key.IsAxis3D();
}

bool UPyKismetInputLibrary::Key_IsAxis1D(const FKey& Key) {
    return UKismetInputLibrary::Key_IsAxis1D(Key);
}

bool UPyKismetInputLibrary::Key_IsAxis2D(const FKey& Key) {
    return UKismetInputLibrary::Key_IsAxis2D(Key);
}

bool UPyKismetInputLibrary::Key_IsAxis3D(const FKey& Key) {
    return UKismetInputLibrary::Key_IsAxis3D(Key);
}

bool UPyKismetInputLibrary::Key_IsButtonAxis(const FKey& Key) {
    return UKismetInputLibrary::Key_IsButtonAxis(Key);
}

bool UPyKismetInputLibrary::Key_IsAnalog(const FKey& Key) {
    return UKismetInputLibrary::Key_IsAnalog(Key);
}

bool UPyKismetInputLibrary::Key_IsDigital(const FKey& Key) {
    return UKismetInputLibrary::Key_IsDigital(Key);
}

bool UPyKismetInputLibrary::Key_IsValid(const FKey& Key) {
    return UKismetInputLibrary::Key_IsValid(Key);
}

EUINavigationAction UPyKismetInputLibrary::Key_GetNavigationAction(const FKey& Key)
{
    return UKismetInputLibrary::Key_GetNavigationAction(Key);
}

EUINavigationAction UPyKismetInputLibrary::Key_GetNavigationActionFromKey(const FPyKeyEvent& InKeyEvent)
{
    return UKismetInputLibrary::Key_GetNavigationActionFromKey(InKeyEvent.ToNativeEvent());
}

EUINavigation UPyKismetInputLibrary::Key_GetNavigationDirectionFromKey(const FPyKeyEvent& InKeyEvent)
{
    return UKismetInputLibrary::Key_GetNavigationDirectionFromKey(InKeyEvent.ToNativeEvent());
}

EUINavigation UPyKismetInputLibrary::Key_GetNavigationDirectionFromAnalog(const FPyAnalogInputEvent& InAnalogEvent)
{
    return UKismetInputLibrary::Key_GetNavigationDirectionFromAnalog(InAnalogEvent.ToNativeEvent());
}

FText UPyKismetInputLibrary::Key_GetDisplayName(const FKey& Key, bool bLongDisplayName)
{
	return UKismetInputLibrary::Key_GetDisplayName(Key, bLongDisplayName);
}

bool UPyKismetInputLibrary::InputEvent_IsRepeat(const FPyInputEvent& Input)
{
	return UKismetInputLibrary::InputEvent_IsRepeat(Input.ToNativeEvent());
}

bool UPyKismetInputLibrary::InputEvent_IsShiftDown(const FPyInputEvent& Input)
{
    return UKismetInputLibrary::InputEvent_IsShiftDown(Input.ToNativeEvent());
}

bool UPyKismetInputLibrary::InputEvent_IsLeftShiftDown(const FPyInputEvent& Input)
{
    return UKismetInputLibrary::InputEvent_IsLeftShiftDown(Input.ToNativeEvent());
}

bool UPyKismetInputLibrary::InputEvent_IsRightShiftDown(const FPyInputEvent& Input)
{
    return UKismetInputLibrary::InputEvent_IsRightShiftDown(Input.ToNativeEvent());
}

bool UPyKismetInputLibrary::InputEvent_IsControlDown(const FPyInputEvent& Input)
{
    return UKismetInputLibrary::InputEvent_IsControlDown(Input.ToNativeEvent());
}

bool UPyKismetInputLibrary::InputEvent_IsLeftControlDown(const FPyInputEvent& Input)
{
    return UKismetInputLibrary::InputEvent_IsLeftControlDown(Input.ToNativeEvent());
}

bool UPyKismetInputLibrary::InputEvent_IsRightControlDown(const FPyInputEvent& Input)
{
    return UKismetInputLibrary::InputEvent_IsRightControlDown(Input.ToNativeEvent());
}

bool UPyKismetInputLibrary::InputEvent_IsAltDown(const FPyInputEvent& Input)
{
    return UKismetInputLibrary::InputEvent_IsAltDown(Input.ToNativeEvent());
}

bool UPyKismetInputLibrary::InputEvent_IsLeftAltDown(const FPyInputEvent& Input)
{
    return UKismetInputLibrary::InputEvent_IsLeftAltDown(Input.ToNativeEvent());
}

bool UPyKismetInputLibrary::InputEvent_IsRightAltDown(const FPyInputEvent& Input)
{
    return UKismetInputLibrary::InputEvent_IsRightAltDown(Input.ToNativeEvent());
}

bool UPyKismetInputLibrary::InputEvent_IsCommandDown(const FPyInputEvent& Input)
{
    return UKismetInputLibrary::InputEvent_IsCommandDown(Input.ToNativeEvent());
}

bool UPyKismetInputLibrary::InputEvent_IsLeftCommandDown(const FPyInputEvent& Input)
{
    return UKismetInputLibrary::InputEvent_IsLeftCommandDown(Input.ToNativeEvent());
}

bool UPyKismetInputLibrary::InputEvent_IsRightCommandDown(const FPyInputEvent& Input)
{
    return UKismetInputLibrary::InputEvent_IsRightCommandDown(Input.ToNativeEvent());
}

bool UPyKismetInputLibrary::ModifierKeysState_IsShiftDown(const FSlateModifierKeysState& KeysState) 
{
    return UKismetInputLibrary::ModifierKeysState_IsShiftDown(KeysState);
}

bool UPyKismetInputLibrary::ModifierKeysState_IsControlDown(const FSlateModifierKeysState& KeysState) 
{
    return UKismetInputLibrary::ModifierKeysState_IsControlDown(KeysState);
}

bool UPyKismetInputLibrary::ModifierKeysState_IsAltDown(const FSlateModifierKeysState& KeysState) 
{
    return UKismetInputLibrary::ModifierKeysState_IsAltDown(KeysState);
}

bool UPyKismetInputLibrary::ModifierKeysState_IsCommandDown(const FSlateModifierKeysState& KeysState) 
{
    return UKismetInputLibrary::ModifierKeysState_IsCommandDown(KeysState);
}

FSlateModifierKeysState UPyKismetInputLibrary::GetModifierKeysState()
{
	return UKismetInputLibrary::GetModifierKeysState();
}

FKey UPyKismetInputLibrary::GetKey(const FPyKeyEvent& Input)
{
    return Input.GetKey();
}

int32 UPyKismetInputLibrary::GetUserIndex(const FPyKeyEvent& Input)
{
    return Input.GetUserIndex();
}

float UPyKismetInputLibrary::GetAnalogValue(const FPyAnalogInputEvent& Input)
{
    return Input.GetAnalogValue();
}

FVector2D UPyKismetInputLibrary::PointerEvent_GetScreenSpacePosition(const FPyPointerEvent& Input)
{
    return Input.GetScreenSpacePosition();
}

FVector2D UPyKismetInputLibrary::PointerEvent_GetLastScreenSpacePosition(const FPyPointerEvent& Input)
{
    return Input.GetLastScreenSpacePosition();
}

FVector2D UPyKismetInputLibrary::PointerEvent_GetCursorDelta(const FPyPointerEvent& Input)
{
    return Input.GetCursorDelta();
}

bool UPyKismetInputLibrary::PointerEvent_IsMouseButtonDown(const FPyPointerEvent& Input, FKey MouseButton)
{
    return Input.IsMouseButtonDown(MouseButton);
}

FKey UPyKismetInputLibrary::PointerEvent_GetEffectingButton(const FPyPointerEvent& Input)
{
    return Input.GetEffectingButton();
}

float UPyKismetInputLibrary::PointerEvent_GetWheelDelta(const FPyPointerEvent& Input)
{
    return Input.GetWheelDelta();
}

int32 UPyKismetInputLibrary::PointerEvent_GetUserIndex(const FPyPointerEvent& Input)
{
    return Input.GetUserIndex();
}

int32 UPyKismetInputLibrary::PointerEvent_GetPointerIndex(const FPyPointerEvent& Input)
{
    return Input.GetPointerIndex();
}

int32 UPyKismetInputLibrary::PointerEvent_GetTouchpadIndex(const FPyPointerEvent& Input)
{
    return Input.GetTouchpadIndex();
}

bool UPyKismetInputLibrary::PointerEvent_IsTouchEvent(const FPyPointerEvent& Input)
{
    return Input.IsTouchEvent();
}

ESlateGesture UPyKismetInputLibrary::PointerEvent_GetGestureType(const FPyPointerEvent& Input)
{
    static_assert((int32)EGestureEvent::None == (int32)ESlateGesture::None, "EGestureEvent == ESlateGesture");
    static_assert((int32)EGestureEvent::Scroll == (int32)ESlateGesture::Scroll, "EGestureEvent == ESlateGesture");
    static_assert((int32)EGestureEvent::Magnify == (int32)ESlateGesture::Magnify, "EGestureEvent == ESlateGesture");
    static_assert((int32)EGestureEvent::Swipe == (int32)ESlateGesture::Swipe, "EGestureEvent == ESlateGesture");
    static_assert((int32)EGestureEvent::Rotate == (int32)ESlateGesture::Rotate, "EGestureEvent == ESlateGesture");
    static_assert((int32)EGestureEvent::LongPress == (int32)ESlateGesture::LongPress, "EGestureEvent == ESlateGesture");

    switch (Input.GetGestureType())
    {
    case EGestureEvent::Scroll:
        return ESlateGesture::Scroll;
    case EGestureEvent::Magnify:
        return ESlateGesture::Magnify;
    case EGestureEvent::Swipe:
        return ESlateGesture::Swipe;
    case EGestureEvent::Rotate:
        return ESlateGesture::Rotate;
    case EGestureEvent::LongPress:
        return ESlateGesture::LongPress;
    case EGestureEvent::None:
    default:
        return ESlateGesture::None;
    }
}

FVector2D UPyKismetInputLibrary::PointerEvent_GetGestureDelta(const FPyPointerEvent& Input)
{
    return Input.GetGestureDelta();
}

FPyEventReply UPyKismetInputLibrary::Handled()
{
    return FPyEventReply(true);
}

FPyEventReply UPyKismetInputLibrary::Unhandled()
{
    return FPyEventReply(false);
}
