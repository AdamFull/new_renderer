#pragma once

#include <memory>
#include <string>
#include <bitset>

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
		uint32_t flags{ 0u };

		int32_t position_x{ 0 };
		int32_t position_y{ 0 };
		int32_t width{ 0 };
		int32_t height{ 0 };
	};

    enum class EKeyState
    {
        ePress = 0,
        eRelease,
        ePressed,
        eReleased
    };

    enum class EKeyCode : size_t
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

    using keystate = std::bitset<static_cast<size_t>(EKeyCode::eCount)>;

    class CKeyDecoder
    {
    public:
        bool update(EKeyCode eCode, bool bPressed)
        {
            auto code = static_cast<size_t>(eCode);

            if (bPressed != mStates.test(code))
            {
                if (bPressed)
                    mStates.set(static_cast<size_t>(eCode));
                else
                    mStates.reset(static_cast<size_t>(eCode));

                return true;
            }

            return false;
        }

        bool test(EKeyCode eCode)
        {
            return mStates.test(static_cast<size_t>(eCode));
        }
    private:
        keystate mStates;
    };

	class IWindowInterface
	{
	public:
		virtual ~IWindowInterface() = default;

		virtual void create(const FWindowCreateInfo& window_ci) = 0;
		virtual void destroy() = 0;

		virtual bool process() = 0;

		virtual void getWindowSize(int32_t& width, int32_t& height) = 0;
		virtual bool isResized() = 0;
		virtual bool isMinimized() = 0;
	};
}