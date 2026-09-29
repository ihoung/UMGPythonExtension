// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "PySlateWrapperTypes.h"
#include "Kismet/KismetInputLibrary.h"
#include "PyKismetInputLibrary.generated.h"

/**
 * 
 */
UCLASS(NotBlueprintable, meta = (ScriptName = "PyInputLibrary"), MinimalAPI)
class UPyKismetInputLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_UCLASS_BODY()

	/** Calibrate the tilt for the input device */
	UFUNCTION(BlueprintCallable, Category = "Python|Input")
	static UMGEDITORPYEX_API void CalibrateTilt();

	/**
	 * Test if the input key are equal (A == B)
	 * @param A - The key to compare against
	 * @param B - The key to compare
	 * Returns true if the key are equal, false otherwise
	 */
	UFUNCTION(BlueprintPure, Category = "Python|Input")
	static UMGEDITORPYEX_API bool EqualEqual_KeyKey(FKey A, FKey B);

	/**
	 * Returns true if the key is a modifier key: Ctrl, Command, Alt, Shift
	 */
	UFUNCTION(BlueprintPure, Category = "Python|Input")
	static UMGEDITORPYEX_API bool Key_IsModifierKey(const FKey& Key);

	/**
	 * Returns true if the key is a gamepad button
	 */
	UFUNCTION(BlueprintPure, Category = "Python|Input")
	static UMGEDITORPYEX_API bool Key_IsGamepadKey(const FKey& Key);

	/**
	 * Returns true if the key is a mouse button
	 */
	UFUNCTION(BlueprintPure, Category = "Python|Input")
	static UMGEDITORPYEX_API bool Key_IsMouseButton(const FKey& Key);

	/**
	 * Returns true if the key is a keyboard button
	 */
	UFUNCTION(BlueprintPure, Category = "Python|Input")
	static UMGEDITORPYEX_API bool Key_IsKeyboardKey(const FKey& Key);

	/**
	 * Returns true if the key is a vector axis
	 * @note Deprecated. Use Is Axis 2D/3D instead.
	 */
	UFUNCTION(BlueprintPure, Category = "Python|Input", meta = (DeprecatedFunction, DeprecationMessage = "Use Is Axis 2D/3D instead."))
	static UMGEDITORPYEX_API bool Key_IsVectorAxis(const FKey& Key);

	/**
	 * Returns true if the key is a 1D (float) axis
	 */
	UFUNCTION(BlueprintPure, Category = "Python|Input")
	static UMGEDITORPYEX_API bool Key_IsAxis1D(const FKey& Key);

	/**
	 * Returns true if the key is a 2D (vector) axis
	 */
	UFUNCTION(BlueprintPure, Category = "Python|Input")
	static UMGEDITORPYEX_API bool Key_IsAxis2D(const FKey& Key);

	/**
	 * Returns true if the key is a 3D (vector) axis
	 */
	UFUNCTION(BlueprintPure, Category = "Python|Input")
	static UMGEDITORPYEX_API bool Key_IsAxis3D(const FKey& Key);

	/**
	 * Returns true if the key is a 1D axis emulating a digital button press.
	 */
	UFUNCTION(BlueprintPure, Category = "Python|Input")
	static UMGEDITORPYEX_API bool Key_IsButtonAxis(const FKey& Key);

	/**
	 * Returns true if the key is an analog axis
	 */
	UFUNCTION(BlueprintPure, Category = "Python|Input")
	static UMGEDITORPYEX_API bool Key_IsAnalog(const FKey& Key);

	/**
	 * Returns true if the key is a digital button press
	 */
	UFUNCTION(BlueprintPure, Category = "Python|Input")
	static UMGEDITORPYEX_API bool Key_IsDigital(const FKey& Key);

	/**
	 * Returns true if this is a valid key.
	 */
	UFUNCTION(BlueprintPure, Category = "Python|Input")
	static UMGEDITORPYEX_API bool Key_IsValid(const FKey& Key);

	UFUNCTION(BlueprintPure, meta = (DeprecatedFunction, DeprecationMessage = "Use Get Key Event Navigation Action instead"))
	static UMGEDITORPYEX_API EUINavigationAction Key_GetNavigationAction(const FKey& InKey);

	/** Returns the navigation action corresponding to this key, or Invalid if not found */
	UFUNCTION(BlueprintPure, Category = "Python|Input")
	static UMGEDITORPYEX_API EUINavigationAction Key_GetNavigationActionFromKey(const FPyKeyEvent& InKeyEvent);

	/** Returns the navigation action corresponding to this key, or Invalid if not found */
	UFUNCTION(BlueprintPure, Category = "Python|Input")
	static UMGEDITORPYEX_API EUINavigation Key_GetNavigationDirectionFromKey(const FPyKeyEvent& InKeyEvent);

	/** Returns the navigation action corresponding to this key, or Invalid if not found */
	UFUNCTION(BlueprintPure, Category = "Python|Input")
	static UMGEDITORPYEX_API EUINavigation Key_GetNavigationDirectionFromAnalog(const FPyAnalogInputEvent& InAnalogEvent);

	/**
	 * Returns the display name of the key.
	 */
	UFUNCTION(BlueprintPure, Category = "Python|Input")
	static UMGEDITORPYEX_API FText Key_GetDisplayName(const FKey& Key, bool bLongDisplayName = true);

	/**
	 * Returns whether or not this character is an auto-repeated keystroke
	 */
	UFUNCTION(BlueprintPure, Category = "Python|Input")
	static UMGEDITORPYEX_API bool InputEvent_IsRepeat(const FPyInputEvent& Input);

	/**
	 * Returns true if either shift key was down when this event occurred
	 */
	UFUNCTION(BlueprintPure, Category = "Python|Input")
	static UMGEDITORPYEX_API bool InputEvent_IsShiftDown(const FPyInputEvent& Input);

	/**
	 * Returns true if left shift key was down when this event occurred
	 */
	UFUNCTION(BlueprintPure, Category = "Python|Input")
	static UMGEDITORPYEX_API bool InputEvent_IsLeftShiftDown(const FPyInputEvent& Input);

	/**
	 * Returns true if right shift key was down when this event occurred
	 */
	UFUNCTION(BlueprintPure, Category = "Python|Input")
	static UMGEDITORPYEX_API bool InputEvent_IsRightShiftDown(const FPyInputEvent& Input);

	/**
	 * Returns true if either control key was down when this event occurred
	 */
	UFUNCTION(BlueprintPure, Category = "Python|Input")
	static UMGEDITORPYEX_API bool InputEvent_IsControlDown(const FPyInputEvent& Input);

	/**
	 * Returns true if left control key was down when this event occurred
	 */
	UFUNCTION(BlueprintPure, Category = "Python|Input")
	static UMGEDITORPYEX_API bool InputEvent_IsLeftControlDown(const FPyInputEvent& Input);

	/**
	 * Returns true if left control key was down when this event occurred
	 */
	UFUNCTION(BlueprintPure, Category = "Python|Input")
	static UMGEDITORPYEX_API bool InputEvent_IsRightControlDown(const FPyInputEvent& Input);

	/**
	 * Returns true if either alt key was down when this event occurred
	 */
	UFUNCTION(BlueprintPure, Category = "Python|Input")
	static UMGEDITORPYEX_API bool InputEvent_IsAltDown(const FPyInputEvent& Input);

	/**
	 * Returns true if left alt key was down when this event occurred
	 */
	UFUNCTION(BlueprintPure, Category = "Python|Input")
	static UMGEDITORPYEX_API bool InputEvent_IsLeftAltDown(const FPyInputEvent& Input);

	/**
	 * Returns true if right alt key was down when this event occurred
	 */
	UFUNCTION(BlueprintPure, Category = "Python|Input")
	static UMGEDITORPYEX_API bool InputEvent_IsRightAltDown(const FPyInputEvent& Input);

	/**
	 * Returns true if either command key was down when this event occurred
	 */
	UFUNCTION(BlueprintPure, Category = "Python|Input")
	static UMGEDITORPYEX_API bool InputEvent_IsCommandDown(const FPyInputEvent& Input);

	/**
	 * Returns true if left command key was down when this event occurred
	 */
	UFUNCTION(BlueprintPure, Category = "Python|Input")
	static UMGEDITORPYEX_API bool InputEvent_IsLeftCommandDown(const FPyInputEvent& Input);

	/**
	 * Returns true if right command key was down when this event occurred
	 */
	UFUNCTION(BlueprintPure, Category = "Python|Input")
	static UMGEDITORPYEX_API bool InputEvent_IsRightCommandDown(const FPyInputEvent& Input);

	/**
	 * Returns true if either shift key was down when the key state was captured
	 */
	UFUNCTION(BlueprintPure, Category = "Python|Input")
	static UMGEDITORPYEX_API bool ModifierKeysState_IsShiftDown(const FSlateModifierKeysState& KeysState);

	/**
	 * Returns true if either control key was down when the key state was captured
	 */
	UFUNCTION(BlueprintPure, Category = "Python|Input")
	static UMGEDITORPYEX_API bool ModifierKeysState_IsControlDown(const FSlateModifierKeysState& KeysState);

	/**
	 * Returns true if either alt key was down when the key state was captured
	 */
	UFUNCTION(BlueprintPure, Category = "Python|Input")
	static UMGEDITORPYEX_API bool ModifierKeysState_IsAltDown(const FSlateModifierKeysState& KeysState);

	/**
	 * Returns true if either command key was down when the key state was captured
	 */
	UFUNCTION(BlueprintPure, Category = "Python|Input")
	static UMGEDITORPYEX_API bool ModifierKeysState_IsCommandDown(const FSlateModifierKeysState& KeysState);

	/** Returns a snapshot of the cached modifier-keys state for the application. */
	UFUNCTION(BlueprintPure, Category = "Python|Input")
	static UMGEDITORPYEX_API FSlateModifierKeysState GetModifierKeysState();

	/**
	 * Returns the key for this event.
	 *
	 * @return  Key name
	 */
	UFUNCTION(BlueprintPure, Category = "Python|Input")
	static UMGEDITORPYEX_API FKey GetKey(const FPyKeyEvent& Input);

	UFUNCTION(BlueprintPure, Category = "Python|Input")
	static UMGEDITORPYEX_API int32 GetUserIndex(const FPyKeyEvent& Input);

	UFUNCTION(BlueprintPure, Category = "Python|Input")
	static UMGEDITORPYEX_API float GetAnalogValue(const FPyAnalogInputEvent& Input);

	/** Returns The position of the cursor in screen space */
	UFUNCTION(BlueprintPure, Category = "Python|Input")
	static UMGEDITORPYEX_API FVector2D PointerEvent_GetScreenSpacePosition(const FPyPointerEvent& Input);

	/** Returns the position of the cursor in screen space last time we handled an input event */
	UFUNCTION(BlueprintPure, Category = "Python|Input")
	static UMGEDITORPYEX_API FVector2D PointerEvent_GetLastScreenSpacePosition(const FPyPointerEvent& Input);

	/** Returns the distance the mouse traveled since the last event was handled. */
	UFUNCTION(BlueprintPure, Category = "Python|Input")
	static UMGEDITORPYEX_API FVector2D PointerEvent_GetCursorDelta(const FPyPointerEvent& Input);

	/** Mouse buttons that are currently pressed */
	UFUNCTION(BlueprintPure, Category = "Python|Input")
	static UMGEDITORPYEX_API bool PointerEvent_IsMouseButtonDown(const FPyPointerEvent& Input, FKey MouseButton);

	/** Mouse button that caused this event to be raised (possibly FKey::Invalid) */
	UFUNCTION(BlueprintPure, Category = "Python|Input")
	static UMGEDITORPYEX_API FKey PointerEvent_GetEffectingButton(const FPyPointerEvent& Input);

	/** How much did the mouse wheel turn since the last mouse event */
	UFUNCTION(BlueprintPure, Category = "Python|Input")
	static UMGEDITORPYEX_API float PointerEvent_GetWheelDelta(const FPyPointerEvent& Input);

	/** Returns the index of the user that caused the event */
	UFUNCTION(BlueprintPure, Category = "Python|Input")
	static UMGEDITORPYEX_API int32 PointerEvent_GetUserIndex(const FPyPointerEvent& Input);

	/** Returns the unique identifier of the pointer (e.g., finger index) */
	UFUNCTION(BlueprintPure, Category = "Python|Input")
	static UMGEDITORPYEX_API int32 PointerEvent_GetPointerIndex(const FPyPointerEvent& Input);

	/** Returns the index of the touch pad that generated this event (for platforms with multiple touch pads per user) */
	UFUNCTION(BlueprintPure, Category = "Python|Input")
	static UMGEDITORPYEX_API int32 PointerEvent_GetTouchpadIndex(const FPyPointerEvent& Input);

	/** Returns true if this event a result from a touch (as opposed to a mouse) */
	UFUNCTION(BlueprintPure, Category = "Python|Input")
	static UMGEDITORPYEX_API bool PointerEvent_IsTouchEvent(const FPyPointerEvent& Input);

	/** Returns the type of touch gesture */
	UFUNCTION(BlueprintPure, Category = "Python|Input")
	static UMGEDITORPYEX_API ESlateGesture PointerEvent_GetGestureType(const FPyPointerEvent& Input);

	/** Returns the change in gesture value since the last gesture event of the same type. */
	UFUNCTION(BlueprintPure, Category = "Python|Input")
	static UMGEDITORPYEX_API FVector2D PointerEvent_GetGestureDelta(const FPyPointerEvent& Input);

	UFUNCTION(BlueprintPure, Category = "Python|Input")
	static UMGEDITORPYEX_API FPyEventReply Handled();

	UFUNCTION(BlueprintPure, Category = "Python|Input")
	static UMGEDITORPYEX_API FPyEventReply Unhandled();
};
