#pragma once

#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"
#include "UObject/Class.h"
#include "Input/Events.h"
#include "Input/Reply.h"
#include "InputCoreTypes.h"
#include "PySlateWrapperTypes.generated.h"

#define LOCTEXT_NAMESPACE "PySlateWrapperTypes"

USTRUCT()
struct FPyInputDeviceId
{
	GENERATED_USTRUCT_BODY()

public:
	FPyInputDeviceId()
		: InternalId(INDEX_NONE)
	{
	}
	FPyInputDeviceId(const FInputDeviceId& InInputDeviceId)
		: InternalId(InInputDeviceId.GetId())
	{
	}

	int32 GetInternalId() const
	{
		return InternalId;
	}

private:
	UPROPERTY()
	int32 InternalId;
};


USTRUCT()
struct FPySlateModifierKeysState
{
	GENERATED_USTRUCT_BODY()

	enum class EModifierKey : uint16
	{
		LeftShift = 1 << 0,
		RightShift = 1 << 1,
		LeftControl = 1 << 2,
		RightControl = 1 << 3,
		LeftAlt = 1 << 4,
		RightAlt = 1 << 5,
		LeftCommand = 1 << 6,
		RightCommand = 1 << 7,
		CapsLock = 1 << 8
	};

	static constexpr EModifierKey LeftShiftMask = EModifierKey::LeftShift;
	static constexpr EModifierKey RightShiftMask = EModifierKey::RightShift;
	static constexpr EModifierKey LeftControlMask = EModifierKey::LeftControl;
	static constexpr EModifierKey RightControlMask = EModifierKey::RightControl;
	static constexpr EModifierKey LeftAltMask = EModifierKey::LeftAlt;
	static constexpr EModifierKey RightAltMask = EModifierKey::RightAlt;
	static constexpr EModifierKey LeftCommandMask = EModifierKey::LeftCommand;
	static constexpr EModifierKey RightCommandMask = EModifierKey::RightCommand;
	static constexpr EModifierKey CapsLockMask = EModifierKey::CapsLock;

	UPROPERTY()
	uint16 ModifierKeysStateMask = 0;

	FPySlateModifierKeysState() {}
	FPySlateModifierKeysState(const FModifierKeysState& InModifierKeysState)
	{
		ModifierKeysStateMask = 0;
		if (InModifierKeysState.IsLeftShiftDown()) ModifierKeysStateMask |= static_cast<uint16>(EModifierKey::LeftShift);
		if (InModifierKeysState.IsRightShiftDown()) ModifierKeysStateMask |= static_cast<uint16>(EModifierKey::RightShift);
		if (InModifierKeysState.IsLeftControlDown()) ModifierKeysStateMask |= static_cast<uint16>(EModifierKey::LeftControl);
		if (InModifierKeysState.IsRightControlDown()) ModifierKeysStateMask |= static_cast<uint16>(EModifierKey::RightControl);
		if (InModifierKeysState.IsLeftAltDown()) ModifierKeysStateMask |= static_cast<uint16>(EModifierKey::LeftAlt);
		if (InModifierKeysState.IsRightAltDown()) ModifierKeysStateMask |= static_cast<uint16>(EModifierKey::RightAlt);
		if (InModifierKeysState.IsLeftCommandDown()) ModifierKeysStateMask |= static_cast<uint16>(EModifierKey::LeftCommand);
		if (InModifierKeysState.IsRightCommandDown()) ModifierKeysStateMask |= static_cast<uint16>(EModifierKey::RightCommand);
		if (InModifierKeysState.AreCapsLocked()) ModifierKeysStateMask |= static_cast<uint16>(EModifierKey::CapsLock);
	}
};


USTRUCT(BlueprintType)
struct FPyInputEvent
{
	GENERATED_USTRUCT_BODY()

public:

	/**
	 * UStruct Constructor.  Not meant for normal usage.
	 */
	FPyInputEvent()
		: ModifierKeys(FModifierKeysState())
		, bIsRepeat(false)
		, UserIndex(0)
		, InputDeviceId(INPUTDEVICEID_NONE)
		, EventPath(nullptr)
	{
	}

	FPyInputEvent(const FInputEvent& InInputEvent)
		: ModifierKeys(InInputEvent.GetModifierKeys())
		, bIsRepeat(InInputEvent.IsRepeat())
		, UserIndex(InInputEvent.GetUserIndex())
		, InputDeviceId(InInputEvent.GetInputDeviceId())
		, EventPath(nullptr)
	{
	}
	/**
	 * Virtual destructor.
	 */
	virtual ~FPyInputEvent() {}

public:

	/**
	 * Returns whether or not this character is an auto-repeated keystroke
	 *
	 * @return  True if this character is a repeat
	 */
	bool IsRepeat() const
	{
		return bIsRepeat;
	}

	/**
	 * Returns true if either shift key was down when this event occurred
	 *
	 * @return  True if shift is pressed
	 */
	bool IsShiftDown() const
	{
		return GetModifierKeys().IsShiftDown();
	}

	/**
	 * Returns true if left shift key was down when this event occurred
	 */
	bool IsLeftShiftDown() const
	{
		return GetModifierKeys().IsLeftShiftDown();
	}

	/**
	 * Returns true if right shift key was down when this event occurred
	 */
	bool IsRightShiftDown() const
	{
		return GetModifierKeys().IsRightShiftDown();
	}

	/**
	 * Returns true if either control key was down when this event occurred
	 */
	bool IsControlDown() const
	{
		return GetModifierKeys().IsControlDown();
	}

	/**
	 * Returns true if left control key was down when this event occurred
	 */
	bool IsLeftControlDown() const
	{
		return GetModifierKeys().IsLeftControlDown();
	}

	/**
	 * Returns true if right control key was down when this event occurred
	 */
	bool IsRightControlDown() const
	{
		return GetModifierKeys().IsRightControlDown();
	}

	/**
	 * Returns true if either alt key was down when this event occurred
	 */
	bool IsAltDown() const
	{
		return GetModifierKeys().IsAltDown();
	}

	/**
	 * Returns true if left alt key was down when this event occurred
	 */
	bool IsLeftAltDown() const
	{
		return GetModifierKeys().IsLeftAltDown();
	}

	/**
	 * Returns true if right alt key was down when this event occurred
	 */
	bool IsRightAltDown() const
	{
		return GetModifierKeys().IsRightAltDown();
	}

	/**
	 * Returns true if either command key was down when this event occurred
	 */
	bool IsCommandDown() const
	{
		return GetModifierKeys().IsCommandDown();
	}

	/**
	 * Returns true if left command key was down when this event occurred
	 */
	bool IsLeftCommandDown() const
	{
		return GetModifierKeys().IsLeftCommandDown();
	}

	/**
	 * Returns true if right command key was down when this event occurred
	 */
	bool IsRightCommandDown() const
	{
		return GetModifierKeys().IsRightCommandDown();
	}

	/**
	 * Returns true if caps lock was on when this event occurred
	 */
	bool AreCapsLocked() const
	{
		return GetModifierKeys().AreCapsLocked();
	}

	/**
	 * Returns the complete set of modifier keys
	 */
	FModifierKeysState GetModifierKeys() const
	{
		return FModifierKeysState(
			(ModifierKeys.ModifierKeysStateMask & static_cast<uint16>(FPySlateModifierKeysState::LeftShiftMask)) != 0,
			(ModifierKeys.ModifierKeysStateMask & static_cast<uint16>(FPySlateModifierKeysState::RightShiftMask)) != 0,
			(ModifierKeys.ModifierKeysStateMask & static_cast<uint16>(FPySlateModifierKeysState::LeftControlMask)) != 0,
			(ModifierKeys.ModifierKeysStateMask & static_cast<uint16>(FPySlateModifierKeysState::RightControlMask)) != 0,
			(ModifierKeys.ModifierKeysStateMask & static_cast<uint16>(FPySlateModifierKeysState::LeftAltMask)) != 0,
			(ModifierKeys.ModifierKeysStateMask & static_cast<uint16>(FPySlateModifierKeysState::RightAltMask)) != 0,
			(ModifierKeys.ModifierKeysStateMask & static_cast<uint16>(FPySlateModifierKeysState::LeftCommandMask)) != 0,
			(ModifierKeys.ModifierKeysStateMask & static_cast<uint16>(FPySlateModifierKeysState::RightCommandMask)) != 0,
			(ModifierKeys.ModifierKeysStateMask & static_cast<uint16>(FPySlateModifierKeysState::CapsLockMask)) != 0
		);
	}

	/**
	* Returns the index of the user that generated this event.
	*/
	uint32 GetUserIndex() const
	{
		return UserIndex;
	}

	/**
	 * Returns the input device that caused this event.
	 */
	FInputDeviceId GetInputDeviceId() const
	{
		return FInputDeviceId::CreateFromInternalId(InputDeviceId.GetInternalId());
	}

	/**
	 * Returns the associated platform user that caused this event
	 */
	FPlatformUserId GetPlatformUserId() const
	{
		return IPlatformInputDeviceMapper::Get().GetUserForInputDevice(GetInputDeviceId());
	}

	/** The event path provides additional context for handling */
	UMGEDITORPYEX_API FGeometry FindGeometry(const TSharedRef<SWidget>& WidgetToFind) const
	{
		return EventPath->FindArrangedWidget(WidgetToFind).Get(FArrangedWidget::GetNullWidget()).Geometry;
	}

	UMGEDITORPYEX_API TSharedRef<SWindow> GetWindow() const
	{
		return EventPath->GetWindow();
	}

	/** Set the widget path along which this event will be routed */
	void SetEventPath(const FWidgetPath& InEventPath)
	{
		EventPath = &InEventPath;
	}

	const FWidgetPath* GetEventPath() const
	{
		return EventPath;
	}

	UMGEDITORPYEX_API virtual FText ToText() const
	{
		return NSLOCTEXT("PySlateWrapperTypes", "Unimplemented", "Unimplemented");
	}

	/** Is this event a pointer event (touch or cursor). */
	UMGEDITORPYEX_API virtual bool IsPointerEvent() const { return false; }

	/** Is this event a key event. */
	UMGEDITORPYEX_API virtual bool IsKeyEvent() const { return false; }

	/* Convert to native InputEvent */
	UMGEDITORPYEX_API FInputEvent ToNativeEvent() const
	{
		return FInputEvent(GetModifierKeys(), UserIndex, bIsRepeat);
	}

protected:

	// State of modifier keys when this event happened.
	UPROPERTY()
	FPySlateModifierKeysState ModifierKeys;

	// True if this key was auto-repeated.
	UPROPERTY()
	bool bIsRepeat;

	// The index of the user that caused the event.
	UPROPERTY()
	uint32 UserIndex;

	// The ID of the input device that caused this event.
	UPROPERTY()
	FPyInputDeviceId InputDeviceId;

	// Events are sent along paths. See (GetEventPath).
	const FWidgetPath* EventPath;
};


USTRUCT(BlueprintType)
struct FPyKeyEvent : public FPyInputEvent
{
	GENERATED_USTRUCT_BODY()

public:
	/**
	 * UStruct Constructor.  Not meant for normal usage.
	 */
	FPyKeyEvent()
		: FPyInputEvent()
		, Key()
		, CharacterCode(0)
		, KeyCode(0)
	{
	}

	FPyKeyEvent(const FKeyEvent& InKeyEvent)
		: FPyInputEvent(static_cast<const FInputEvent&>(InKeyEvent))
		, Key(InKeyEvent.GetKey())
		, CharacterCode(InKeyEvent.GetCharacter())
		, KeyCode(InKeyEvent.GetKeyCode())
	{
	}

	/**
	 * Returns the name of the key for this event
	 *
	 * @return  Key name
	 */
	FKey GetKey() const
	{
		return Key;
	}

	/**
	 * Returns the character code for this event.
	 *
	 * @return  Character code or 0 if this event was not a character key press
	 */
	uint32 GetCharacter() const
	{
		return CharacterCode;
	}

	/**
	 * Returns the key code received from hardware before any conversion/mapping.
	 *
	 * @return  Key code received from hardware
	 */
	uint32 GetKeyCode() const
	{
		return KeyCode;
	}

	UMGEDITORPYEX_API virtual FText ToText() const override { return FText::Format(NSLOCTEXT("PySlateWrapperTypes", "Key", "Key({0})"), Key.GetDisplayName()); }

	UMGEDITORPYEX_API virtual bool IsKeyEvent() const override { return true; }

	UMGEDITORPYEX_API FKeyEvent ToNativeEvent() const
	{
		return FKeyEvent(Key, GetModifierKeys(), UserIndex, bIsRepeat, CharacterCode, KeyCode);
	}

private:
	// Name of the key that was pressed.
	UPROPERTY()
	FKey Key;

	// The character code of the key that was pressed.  Only applicable to typed character keys, 0 otherwise.
	UPROPERTY()
	uint32 CharacterCode;

	// Original key code received from hardware before any conversion/mapping
	UPROPERTY()
	uint32 KeyCode;
};


USTRUCT(BlueprintType)
struct FPyAnalogInputEvent
	: public FPyKeyEvent
{
	GENERATED_USTRUCT_BODY()

public:
	/**
	* UStruct Constructor.  Not meant for normal usage.
	*/
	FPyAnalogInputEvent()
		: FPyKeyEvent(FKeyEvent())
		, AnalogValue(0.0f)
	{
	}

	FPyAnalogInputEvent(const FAnalogInputEvent& InAnalogInputEvent)
		: FPyKeyEvent(static_cast<const FKeyEvent&>(InAnalogInputEvent))
		, AnalogValue(InAnalogInputEvent.GetAnalogValue())
	{
	}

	/**
	 * Returns the analog value between 0 and 1.
	 * 0 being not pressed at all, 1 being fully pressed.
	 * Non analog keys will only be 0 or 1.
	 *
	 * @return Analog value between 0 and 1.  1 being fully pressed, 0 being not pressed at all
	 */
	float GetAnalogValue() const { return AnalogValue; }

	UMGEDITORPYEX_API virtual FText ToText() const override {
		return FText::Format(NSLOCTEXT("PySlateWrapperTypes", "AnalogInput", "AnalogInput(key:{0}, value:{1}"), GetKey().GetDisplayName(), AnalogValue); 
	}

	UMGEDITORPYEX_API FAnalogInputEvent ToNativeEvent() const
	{
		return FAnalogInputEvent(GetKey(), GetModifierKeys(), UserIndex, bIsRepeat, GetCharacter(), GetKeyCode(), AnalogValue);
	}

private:
	//  Analog value between 0 and 1 (0 being not pressed at all, 1 being fully pressed).
	UPROPERTY()
	float AnalogValue;
};


USTRUCT(BlueprintType)
struct FPyCharacterEvent
	: public FPyInputEvent
{
	GENERATED_USTRUCT_BODY()

public:
	/**
	 * UStruct Constructor.  Not meant for normal usage.
	 */
	FPyCharacterEvent()
		: FPyInputEvent()
		, Character(FString())
	{
	}

	FPyCharacterEvent(const FCharacterEvent& InCharacterEvent)
		: FPyInputEvent(static_cast<const FInputEvent&>(InCharacterEvent))
		, Character(FString::Chr(InCharacterEvent.GetCharacter()))
	{
	}

	/**
	 * Returns the character for this event
	 *
	 * @return  Character
	 */
	TCHAR GetCharacter() const
	{
		return Character.IsEmpty() ? '\0' : Character[0];
	}

	UMGEDITORPYEX_API virtual FText ToText() const override {
		return FText::Format(NSLOCTEXT("PySlateWrapperTypes", "Char", "Char({0})"), FText::FromString(Character));
	}

	UMGEDITORPYEX_API FCharacterEvent ToNativeEvent() const
	{
		return FCharacterEvent(GetCharacter(), GetModifierKeys(), UserIndex, bIsRepeat);
	}

private:

	// The character that was pressed.
	UPROPERTY()
	FString Character;
};


UENUM(BlueprintType)
enum class EPyGestureEvent : uint8
{
	None = static_cast<uint8>(EGestureEvent::None),
	Scroll = static_cast<uint8>(EGestureEvent::Scroll),
	Magnify = static_cast<uint8>(EGestureEvent::Magnify),
	Swipe = static_cast<uint8>(EGestureEvent::Swipe),
	Rotate = static_cast<uint8>(EGestureEvent::Rotate),
	LongPress = static_cast<uint8>(EGestureEvent::LongPress),
	Count = static_cast<uint8>(EGestureEvent::Count)
};


USTRUCT(BlueprintType)
struct FPyPointerEvent
	: public FPyInputEvent
{
	GENERATED_USTRUCT_BODY()
public:

	/**
	 * UStruct Constructor.  Not meant for normal usage.
	 */
	FPyPointerEvent()
		: ScreenSpacePosition(FVector2f(0.f, 0.f))
		, LastScreenSpacePosition(FVector2f(0.f, 0.f))
		, CursorDelta(FVector2f(0.f, 0.f))
		, PressedButtons(FTouchKeySet::EmptySet)
		, EffectingButton()
		, PointerIndex(0)
		, TouchpadIndex(0)
		, Force(1.0f)
		, bIsTouchEvent(false)
		, GestureType(EPyGestureEvent::None)
		, WheelOrGestureDelta(0.0f, 0)
		, bIsDirectionInvertedFromDevice(false)
		, bIsTouchForceChanged(false)
		, bIsTouchFirstMove(false)
	{
	}


	FPyPointerEvent(const FPointerEvent& InPointerEvent)
		: ScreenSpacePosition(InPointerEvent.GetScreenSpacePosition())
		, LastScreenSpacePosition(InPointerEvent.GetLastScreenSpacePosition())
		, CursorDelta(InPointerEvent.GetCursorDelta())
		, PressedButtons(InPointerEvent.GetPressedButtons())
		, EffectingButton(InPointerEvent.GetEffectingButton())
		, PointerIndex(InPointerEvent.GetPointerIndex())
		, TouchpadIndex(InPointerEvent.GetTouchpadIndex())
		, Force(InPointerEvent.GetTouchForce())
		, bIsTouchEvent(InPointerEvent.IsTouchEvent())
		, GestureType(static_cast<EPyGestureEvent>(InPointerEvent.GetGestureType()))
		, WheelOrGestureDelta(InPointerEvent.GetGestureDelta())
		, bIsDirectionInvertedFromDevice(InPointerEvent.IsDirectionInvertedFromDevice())
		, bIsTouchForceChanged(InPointerEvent.IsTouchForceChangedEvent())
		, bIsTouchFirstMove(InPointerEvent.IsTouchFirstMoveEvent())
	{
	}

public:

	/** Returns The position of the cursor in screen space */
	const FDeprecateSlateVector2D& GetScreenSpacePosition() const { return ScreenSpacePosition; }

	/** Returns the position of the cursor in screen space last time we handled an input event */
	const FDeprecateSlateVector2D& GetLastScreenSpacePosition() const { return LastScreenSpacePosition; }

	/** Returns the distance the mouse traveled since the last event was handled. */
	const FDeprecateSlateVector2D& GetCursorDelta() const { return CursorDelta; }

	/** Mouse buttons that are currently pressed */
	bool IsMouseButtonDown(FKey MouseButton) const { return PressedButtons.Contains(MouseButton); }

	/** Mouse button that caused this event to be raised (possibly FKey::Invalid) */
	FKey GetEffectingButton() const { return EffectingButton; }

	/** How much did the mouse wheel turn since the last mouse event */
	float GetWheelDelta() const { return WheelOrGestureDelta.Y; }

	/** Returns the index of the user that caused the event */
	int32 GetUserIndex() const { return UserIndex; }

	/** Returns the unique identifier of the pointer (e.g., finger index) */
	uint32 GetPointerIndex() const { return PointerIndex; }

	/** Returns the index of the touch pad that generated this event (for platforms with multiple touch pads per user) */
	uint32 GetTouchpadIndex() const { return TouchpadIndex; }

	/** Returns the force of a touch (1.0f is mapped to an general touch force, < 1 is "light", > 1 is "heavy", and 10 is the max force possible) */
	float GetTouchForce() const { return Force; }

	/** Is this event a result from a touch (as opposed to a mouse) */
	bool IsTouchEvent() const { return bIsTouchEvent; }

	/** Is this event a special force-change touch event */
	bool IsTouchForceChangedEvent() const { return bIsTouchForceChanged; }

	/** Is this event a special first-move touch event */
	bool IsTouchFirstMoveEvent() const { return bIsTouchFirstMove; }

	/** Returns the type of touch gesture */
	EGestureEvent GetGestureType() const { return static_cast<EGestureEvent>(GestureType); }

	/** Returns the change in gesture value since the last gesture event of the same type. */
	const FDeprecateSlateVector2D& GetGestureDelta() const { return WheelOrGestureDelta; }

	/** Is the gesture delta inverted */
	bool IsDirectionInvertedFromDevice() const { return bIsDirectionInvertedFromDevice; }

	/** Returns the full set of pressed buttons */
	TSet<FKey> GetPressedButtons() const { return PressedButtons; }

	UMGEDITORPYEX_API virtual FText ToText() const override { 
		return FText::Format(NSLOCTEXT("PySlateWrapperTypes", "Pointer", "Pointer(key:{0}, pos:{1}x{2}, delta:{3}x{4})"), EffectingButton.GetDisplayName(), ScreenSpacePosition.X, ScreenSpacePosition.Y, CursorDelta.X, CursorDelta.Y); 
	}

	UMGEDITORPYEX_API virtual bool IsPointerEvent() const override { return true; }

	template<typename PointerEventType>
	static PointerEventType MakeTranslatedEvent(const PointerEventType& InPointerEvent, const FVirtualPointerPosition& VirtualPosition)
	{
		PointerEventType NewEvent = InPointerEvent;
		NewEvent.ScreenSpacePosition = VirtualPosition.CurrentCursorPosition;
		NewEvent.LastScreenSpacePosition = VirtualPosition.LastCursorPosition;
		//NewEvent.CursorDelta = VirtualPosition.GetDelta();
		return NewEvent;
	}

	UMGEDITORPYEX_API FPointerEvent ToNativeEvent() const
	{
		if (GestureType != EPyGestureEvent::None)
		{
			return FPointerEvent(
				ScreenSpacePosition,
				LastScreenSpacePosition,
				PressedButtons,
				GetModifierKeys(),
				GetGestureType(),
				GetGestureDelta(),
				bIsDirectionInvertedFromDevice
			);
		}
		else if (bIsTouchEvent)
		{
			return FPointerEvent(
				UserIndex,
				PointerIndex,
				ScreenSpacePosition,
				LastScreenSpacePosition,
				Force,
				PressedButtons.Difference(FTouchKeySet::StandardSet).IsEmpty(),
				bIsTouchForceChanged,
				bIsTouchFirstMove,
				GetModifierKeys(),
				TouchpadIndex
			);
		}
		return FPointerEvent(
			UserIndex,
			PointerIndex,
			ScreenSpacePosition,
			LastScreenSpacePosition,
			PressedButtons,
			EffectingButton,
			WheelOrGestureDelta.Y,
			GetModifierKeys()
		);
	}

private:

	UPROPERTY()
	FDeprecateSlateVector2D ScreenSpacePosition;

	UPROPERTY()
	FDeprecateSlateVector2D LastScreenSpacePosition;

	UPROPERTY()
	FDeprecateSlateVector2D CursorDelta;

	UPROPERTY()
	TSet<FKey> PressedButtons;

	UPROPERTY()
	FKey EffectingButton;

	UPROPERTY()
	uint32 PointerIndex;

	UPROPERTY()
	uint32 TouchpadIndex;

	UPROPERTY()
	float Force;

	UPROPERTY()
	bool bIsTouchEvent;

	UPROPERTY()
	EPyGestureEvent GestureType;

	UPROPERTY()
	FDeprecateSlateVector2D WheelOrGestureDelta;

	UPROPERTY()
	bool bIsDirectionInvertedFromDevice;

	UPROPERTY()
	bool bIsTouchForceChanged;

	UPROPERTY()
	bool bIsTouchFirstMove;

};


USTRUCT(BlueprintType)
struct FPyMotionEvent
	: public FPyInputEvent
{
	GENERATED_USTRUCT_BODY()

public:
	/**
	* UStruct Constructor.  Not meant for normal usage.
	*/
	FPyMotionEvent()
		: Tilt(FVector(0, 0, 0))
		, RotationRate(FVector(0, 0, 0))
		, Gravity(FVector(0, 0, 0))
		, Acceleration(FVector(0, 0, 0))
	{
	}

	FPyMotionEvent(const FMotionEvent& InMotionEvent)
		: Tilt(InMotionEvent.GetTilt())
		, RotationRate(InMotionEvent.GetRotationRate())
		, Gravity(InMotionEvent.GetGravity())
		, Acceleration(InMotionEvent.GetAcceleration())
	{
	}

public:

	/** Returns the index of the user that caused the event */
	uint32 GetUserIndex() const { return UserIndex; }

	/** Returns the current tilt of the device/controller */
	const FVector& GetTilt() const { return Tilt; }

	/** Returns otation speed */
	const FVector& GetRotationRate() const { return RotationRate; }

	/** Returns the gravity vector (pointing down into the ground) */
	const FVector& GetGravity() const { return Gravity; }

	/** Returns the 3D acceleration of the device */
	const FVector& GetAcceleration() const { return Acceleration; }

	UMGEDITORPYEX_API FMotionEvent ToNativeEvent() const
	{
		return FMotionEvent(UserIndex, Tilt, RotationRate, Gravity, Acceleration);
	}

private:

	// The current tilt of the device/controller.
	UPROPERTY()
	FVector Tilt;

	// The rotation speed.
	UPROPERTY()
	FVector RotationRate;

	// The gravity vector (pointing down into the ground).
	UPROPERTY()
	FVector Gravity;

	// The 3D acceleration of the device.
	UPROPERTY()
	FVector Acceleration;
};


/**
 * Allows users to handle events and return information to the underlying UI layer.
 */
USTRUCT(BlueprintType)
struct FPyEventReply
{
	GENERATED_USTRUCT_BODY()

public:

	FPyEventReply(bool IsHandled = false)
        : bHandled(IsHandled)
	{
	}

	/** Returns an FReply representing this struct's state. */
	FReply ToNativeReply() const
	{
		return bHandled ? FReply::Handled() : FReply::Unhandled();
	}

private:

	UPROPERTY()
	bool bHandled;

};

#undef LOCTEXT_NAMESPACE
