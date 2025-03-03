#pragma once

#include <cstddef>
#include <cstdint>
#include <bitset>

namespace edge
{
	enum class EventSource
	{
		Keyboard,
		Character,
		Mouse,
		Touchscreen
	};

	class InputEvent
	{
	public:
		InputEvent(EventSource source) :
			source{ source }
		{

		}

		[[nodiscard]] auto GetSource() const noexcept -> EventSource
		{
			return source;
		}

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
		KeyInputEvent(KeyCode code, KeyAction action) :
			InputEvent{ EventSource::Keyboard },
			code{ code },
			action{ action }
		{
		}

		[[nodiscard]] auto GetCode() const noexcept -> KeyCode
		{
			return code;
		}

		[[nodiscard]] auto GetAction() const noexcept -> KeyAction
		{
			return action;
		}

	private:
		KeyCode code;
		KeyAction action;
	};

	class CharacterInputEvent final : public InputEvent
	{
	public:
		CharacterInputEvent(uint32_t charcode) :
			InputEvent{ EventSource::Character },
			character_code{ charcode }
		{
		}

		[[nodiscard]] auto GetCharacterCode() const noexcept -> uint32_t
		{
			return character_code;
		}
	private:
		uint32_t character_code;
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
		MouseButtonInputEvent(KeyCode button, MouseAction action, float value_x, float value_y, float max_x, float max_y) :
			InputEvent{ EventSource::Mouse },
			button{ button },
			action{ action },
			value_x{ value_x },
			value_y{ value_y },
			max_x{ max_x },
			max_y{ max_y }
		{
		}

		[[nodiscard]] auto GetButton() const noexcept -> KeyCode
		{
			return button;
		}

		[[nodiscard]] auto GetAction() const noexcept -> MouseAction
		{
			return action;
		}

		[[nodiscard]] auto GetValueX() const noexcept -> float
		{
			return value_x;
		}

		[[nodiscard]] auto GetValueY() const noexcept -> float
		{
			return value_y;
		}

		[[nodiscard]] auto GetMaxX() const noexcept -> float
		{
			return max_x;
		}

		[[nodiscard]] auto GetMaxY() const noexcept -> float
		{
			return max_y;
		}

	private:
		KeyCode button;

		MouseAction action;

		float value_x, value_y;
		float max_x, max_y;
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
		TouchInputEvent(int32_t pointer_id, size_t pointer_count, TouchAction action, float pos_x, float pos_y) :
			InputEvent{ EventSource::Touchscreen },
			action{ action },
			pointer_id{ pointer_id },
			touch_points{ touch_points },
			pos_x{ pos_x },
			pos_y{ pos_y }
		{
		}

		[[nodiscard]] auto GetAction() const noexcept -> TouchAction
		{
			return action;
		}

		[[nodiscard]] auto GetPointerId() const noexcept -> int32_t
		{
			return pointer_id;
		}

		[[nodiscard]] auto GetTouchPoints() const noexcept -> std::size_t
		{
			return touch_points;
		}

		[[nodiscard]] auto GetPosX() const noexcept -> float
		{
			return pos_x;
		}

		[[nodiscard]] auto GetPosY() const noexcept -> float
		{
			return pos_y;
		}

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