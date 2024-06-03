#pragma once

#include <standart_library.h>

namespace engine
{
	enum EWindowFlagBits
	{
		WINDOW_SHOWN_FLAG_BIT = 1 << 0,
		WINDOW_HIDDEN_FLAG_BIT = 1 << 1,
		WINDOW_RESIZABLE_FLAG_BIT = 1 << 2,
		WINDOW_VISIBLE_FLAG_BIT = 1 << 3,
		WINDOW_DISABLED_FLAG_BIT = 1 << 4,

		WINDOW_MINIMIZED_FLAG_BIT = 1 << 5,
		WINDOW_MAXIMIZED_FLAG_BIT = 1 << 6,
		WINDOW_BORDERLESS_FLAG_BIT = 1 << 7,
		WINDOW_FULLSCREEN_FLAG_BIT = 1 << 8,
	};

	struct FWindowCreateInfo
	{
		std::string name{};
		u32 flags{ 0u };

		i32 position_x{ 0 };
		i32 position_y{ 0 };
		i32 width{ 0 };
		i32 height{ 0 };
	};

    enum class EKeyState
    {
        ePress = 0,
        eRelease,
        ePressed,
        eReleased
    };

    enum class EKeyCode : u64
    {
        eUnknown,

        //Keyboard input
        eKeyA,
        eKeyB,
        eKeyC,
        eKeyD,
        eKeyE,
        eKeyF,
        eKeyG,
        eKeyH,
        eKeyI,
        eKeyJ,
        eKeyK,
        eKeyL,
        eKeyM,
        eKeyN,
        eKeyO,
        eKeyP,
        eKeyQ,
        eKeyR,
        eKeyS,
        eKeyT,
        eKeyU,
        eKeyV,
        eKeyW,
        eKeyX,
        eKeyY,
        eKeyZ,

        eKey1,
        eKey2,
        eKey3,
        eKey4,
        eKey5,
        eKey6,
        eKey7,
        eKey8,
        eKey9,
        eKey0,

        eKyeReturn,
        eKyeEscape,
        eKyeBackspace,
        eKyeTab,
        eKeySpace,
        eKeyLCtrl,
        eKeyRCtrl,
        eKeyLShift,
        eKeyRShift,

        eKeyMinus,
        eKeyEquals,
        eKeyLeftBracket,
        eKeyRightBracket,
        eKeyBackslash,
        eKeySemicolon,
        eKeyApostrophe,
        eKeyComma,
        eKeyPeriod,
        eKeySlash,
        eKeyCapslock,

        eKeyF1,
        eKeyF2,
        eKeyF3,
        eKeyF4,
        eKeyF5,
        eKeyF6,
        eKeyF7,
        eKeyF8,
        eKeyF9,
        eKeyF10,
        eKeyF11,
        eKeyF12,

        eKeyPrintscreen,
        eKeyScrollLock,
        eKeyPause,
        eKeyInsert,

        eKeyHome,
        eKeyPageUp,
        eKeyDelete,
        eKeyEnd,
        eKeyPageDown,
        eKeyRight,
        eKeyLeft,
        eKeyDown,
        eKeyUp,

        //Mouse/touch input
        eMouseLeft,
        eMouseMiddle,
        eMouseRight,
        eMouseX1,
        eMouseX2,

        //Game controller
        eButtonA,
        eButtonB,
        eButtonX,
        eButtonY,
        eButtonBack,
        eButtonGuide,
        eButtonStart,
        eButtonLeftStick,
        eButtonRightStick,
        eButtonLeftShoulder,
        eButtonRightShoulder,
        eButtonDPADUp,
        eButtonDPADDown,
        eButtonDPADLeft,
        eButtonDPADRight,

        eCount
    };

    using keystate = std::bitset<static_cast<u64>(EKeyCode::eCount)>;

    class CKeyDecoder
    {
    public:
        bool Update(EKeyCode eCode, bool bPressed)
        {
            auto code = static_cast<u64>(eCode);

            if (bPressed != mStates.test(code))
            {
                if (bPressed)
                    mStates.set(static_cast<u64>(eCode));
                else
                    mStates.reset(static_cast<u64>(eCode));

                return true;
            }

            return false;
        }

        bool Test(EKeyCode eCode)
        {
            return mStates.test(static_cast<u64>(eCode));
        }
    private:
        keystate mStates;
    };

	class IWindowInterface
	{
	public:
		virtual ~IWindowInterface() = default;

		virtual void Create(const FWindowCreateInfo& window_ci) = 0;
		virtual void Destroy() = 0;

		virtual bool Process() = 0;

        virtual void SetTitle(const std::string& title) = 0;

		virtual void GetWindowSize(i32& width, i32& height) = 0;
		virtual bool IsResized() = 0;
		virtual bool IsMinimized() = 0;
	};
}