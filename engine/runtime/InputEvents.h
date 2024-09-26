#pragma once

#include <cstddef>
#include <cstdint>
#include <bitset>

namespace engine
{
	enum class EventSource
	{
		Keyboard,
		Mouse,
		Touchscreen
	};

	class InputEvent
	{
	public:
		InputEvent(EventSource source);

		EventSource GetSource() const;

	private:
		EventSource source;
	};

	enum class KeyCode
	{
		Unknown,
		Space,
		Apostrophe, /* ' */
		Comma,      /* , */
		Minus,      /* - */
		Period,     /* . */
		Slash,      /* / */
		_0,
		_1,
		_2,
		_3,
		_4,
		_5,
		_6,
		_7,
		_8,
		_9,
		Semicolon, /* ; */
		Equal,     /* = */
		A,
		B,
		C,
		D,
		E,
		F,
		G,
		H,
		I,
		J,
		K,
		L,
		M,
		N,
		O,
		P,
		Q,
		R,
		S,
		T,
		U,
		V,
		W,
		X,
		Y,
		Z,
		LeftBracket,  /* [ */
		Backslash,    /* \ */
		RightBracket, /* ] */
		GraveAccent,  /* ` */
		Escape,
		Enter,
		Tab,
		Backspace,
		Insert,
		DelKey,
		Right,
		Left,
		Down,
		Up,
		PageUp,
		PageDown,
		Home,
		End,
		Back,
		CapsLock,
		ScrollLock,
		NumLock,
		PrintScreen,
		Pause,
		F1,
		F2,
		F3,
		F4,
		F5,
		F6,
		F7,
		F8,
		F9,
		F10,
		F11,
		F12,
		KP_0,
		KP_1,
		KP_2,
		KP_3,
		KP_4,
		KP_5,
		KP_6,
		KP_7,
		KP_8,
		KP_9,
		KP_Decimal,
		KP_Divide,
		KP_Multiply,
		KP_Subtract,
		KP_Add,
		KP_Enter,
		KP_Equal,
		LeftShift,
		LeftControl,
		LeftAlt,
		RightShift,
		RightControl,
		RightAlt,

		MouseLeft,
		MouseRight,
		MouseMiddle,

		Count
	};

	enum class KeyAction
	{
		Down,
		Up,
		Repeat,
		Unknown
	};

	class KeyInputEvent final : public InputEvent
	{
	public:
		KeyInputEvent(KeyCode code, KeyAction action);

		KeyCode GetCode() const;

		KeyAction GetAction() const;

	private:
		KeyCode code;

		KeyAction action;
	};

	enum class MouseAction
	{
		Down,
		Up,
		Move,
		Scroll,
		Unknown
	};

	class MouseButtonInputEvent final : public InputEvent
	{
	public:
		MouseButtonInputEvent(KeyCode button, MouseAction action, float value_x, float value_y);

		KeyCode GetButton() const;

		MouseAction GetAction() const;

		float GetValueX() const;
		float GetValueY() const;

	private:
		KeyCode button;

		MouseAction action;

		float value_x, value_y;
	};

	enum class TouchAction
	{
		Down,
		Up,
		Move,
		Cancel,
		PointerDown,
		PointerUp,
		Unknown
	};

	class TouchInputEvent final : public InputEvent
	{
	public:
		TouchInputEvent(int32_t pointer_id, size_t pointer_count, TouchAction action, float pos_x, float pos_y);

		TouchAction GetAction() const;

		int32_t GetPointerId() const;

		size_t GetTouchPoints() const;

		float GetPosX() const;

		float GetPosY() const;

	private:
		TouchAction action;

		int32_t pointer_id;

		size_t touch_points;

		float pos_x;

		float pos_y;
	};

	using keystate = std::bitset<static_cast<uint64_t>(KeyCode::Count)>;

	class CKeyDecoder
	{
	public:
		bool Update(KeyCode eCode, bool bPressed)
		{
			auto code = static_cast<uint64_t>(eCode);

			if (bPressed != states.test(code))
			{
				if (bPressed)
					states.set(static_cast<uint64_t>(eCode));
				else
					states.reset(static_cast<uint64_t>(eCode));

				return true;
			}

			return false;
		}

		bool Test(KeyCode eCode)
		{
			return states.test(static_cast<uint64_t>(eCode));
		}
	private:
		keystate states;
	};
}