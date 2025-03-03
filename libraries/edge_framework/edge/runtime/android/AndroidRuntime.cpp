#include "AndroidRuntime.h"

#include "AndroidRuntimeContext.h"

#include "AndroidCommon.h"
#include "AndroidWindow.h"

#include "../../util/Logger.h"

#include <game-activity/GameActivity.h>

namespace edge
{
	namespace android
	{
		inline KeyCode TranslateKeyCode(int key)
		{
			static const std::unordered_map<int, KeyCode> key_lookup =
			{
				{AKEYCODE_SPACE, KeyCode::Space},
				{AKEYCODE_APOSTROPHE, KeyCode::Apostrophe},
				{AKEYCODE_COMMA, KeyCode::Comma},
				{AKEYCODE_MINUS, KeyCode::Minus},
				{AKEYCODE_PERIOD, KeyCode::Period},
				{AKEYCODE_SLASH, KeyCode::Slash},
				{AKEYCODE_0, KeyCode::_0},
				{AKEYCODE_1, KeyCode::_1},
				{AKEYCODE_2, KeyCode::_2},
				{AKEYCODE_3, KeyCode::_3},
				{AKEYCODE_4, KeyCode::_4},
				{AKEYCODE_5, KeyCode::_5},
				{AKEYCODE_6, KeyCode::_6},
				{AKEYCODE_7, KeyCode::_7},
				{AKEYCODE_8, KeyCode::_8},
				{AKEYCODE_9, KeyCode::_9},
				{AKEYCODE_SEMICOLON, KeyCode::Semicolon},
				{AKEYCODE_EQUALS, KeyCode::Equal},
				{AKEYCODE_A, KeyCode::A},
				{AKEYCODE_B, KeyCode::B},
				{AKEYCODE_C, KeyCode::C},
				{AKEYCODE_D, KeyCode::D},
				{AKEYCODE_E, KeyCode::E},
				{AKEYCODE_F, KeyCode::F},
				{AKEYCODE_G, KeyCode::G},
				{AKEYCODE_H, KeyCode::H},
				{AKEYCODE_I, KeyCode::I},
				{AKEYCODE_J, KeyCode::J},
				{AKEYCODE_K, KeyCode::K},
				{AKEYCODE_L, KeyCode::L},
				{AKEYCODE_M, KeyCode::M},
				{AKEYCODE_N, KeyCode::N},
				{AKEYCODE_O, KeyCode::O},
				{AKEYCODE_P, KeyCode::P},
				{AKEYCODE_Q, KeyCode::Q},
				{AKEYCODE_R, KeyCode::R},
				{AKEYCODE_S, KeyCode::S},
				{AKEYCODE_T, KeyCode::T},
				{AKEYCODE_U, KeyCode::U},
				{AKEYCODE_V, KeyCode::V},
				{AKEYCODE_W, KeyCode::W},
				{AKEYCODE_X, KeyCode::X},
				{AKEYCODE_Y, KeyCode::Y},
				{AKEYCODE_Z, KeyCode::Z},
				{AKEYCODE_LEFT_BRACKET, KeyCode::LeftBracket},
				{AKEYCODE_BACKSLASH, KeyCode::Backslash},
				{AKEYCODE_RIGHT_BRACKET, KeyCode::RightBracket},
				{AKEYCODE_ESCAPE, KeyCode::Escape},
				{AKEYCODE_BACK, KeyCode::Back},
				{AKEYCODE_ENTER, KeyCode::Enter},
				{AKEYCODE_TAB, KeyCode::Tab},
				{AKEYCODE_DEL, KeyCode::Backspace},
				{AKEYCODE_INSERT, KeyCode::Insert},
				{AKEYCODE_DEL, KeyCode::DelKey},
				{AKEYCODE_SYSTEM_NAVIGATION_RIGHT, KeyCode::Right},
				{AKEYCODE_SYSTEM_NAVIGATION_LEFT, KeyCode::Left},
				{AKEYCODE_SYSTEM_NAVIGATION_DOWN, KeyCode::Down},
				{AKEYCODE_SYSTEM_NAVIGATION_UP, KeyCode::Up},
				{AKEYCODE_PAGE_UP, KeyCode::PageUp},
				{AKEYCODE_PAGE_DOWN, KeyCode::PageDown},
				{AKEYCODE_HOME, KeyCode::Home},
				{AKEYCODE_CAPS_LOCK, KeyCode::CapsLock},
				{AKEYCODE_SCROLL_LOCK, KeyCode::ScrollLock},
				{AKEYCODE_NUM_LOCK, KeyCode::NumLock},
				{AKEYCODE_BREAK, KeyCode::Pause},
				{AKEYCODE_F1, KeyCode::F1},
				{AKEYCODE_F2, KeyCode::F2},
				{AKEYCODE_F3, KeyCode::F3},
				{AKEYCODE_F4, KeyCode::F4},
				{AKEYCODE_F5, KeyCode::F5},
				{AKEYCODE_F6, KeyCode::F6},
				{AKEYCODE_F7, KeyCode::F7},
				{AKEYCODE_F8, KeyCode::F8},
				{AKEYCODE_F9, KeyCode::F9},
				{AKEYCODE_F10, KeyCode::F10},
				{AKEYCODE_F11, KeyCode::F11},
				{AKEYCODE_F12, KeyCode::F12},
				{AKEYCODE_NUMPAD_0, KeyCode::KP_0},
				{AKEYCODE_NUMPAD_1, KeyCode::KP_1},
				{AKEYCODE_NUMPAD_2, KeyCode::KP_2},
				{AKEYCODE_NUMPAD_3, KeyCode::KP_3},
				{AKEYCODE_NUMPAD_4, KeyCode::KP_4},
				{AKEYCODE_NUMPAD_5, KeyCode::KP_5},
				{AKEYCODE_NUMPAD_6, KeyCode::KP_6},
				{AKEYCODE_NUMPAD_7, KeyCode::KP_7},
				{AKEYCODE_NUMPAD_8, KeyCode::KP_8},
				{AKEYCODE_NUMPAD_9, KeyCode::KP_9},
				{AKEYCODE_NUMPAD_DOT, KeyCode::KP_Decimal},
				{AKEYCODE_NUMPAD_DIVIDE, KeyCode::KP_Divide},
				{AKEYCODE_NUMPAD_MULTIPLY, KeyCode::KP_Multiply},
				{AKEYCODE_NUMPAD_SUBTRACT, KeyCode::KP_Subtract},
				{AKEYCODE_NUMPAD_ADD, KeyCode::KP_Add},
				{AKEYCODE_NUMPAD_ENTER, KeyCode::KP_Enter},
				{AKEYCODE_NUMPAD_EQUALS, KeyCode::KP_Equal},
				{AKEYCODE_SHIFT_LEFT, KeyCode::LeftShift},
				{AKEYCODE_CTRL_LEFT, KeyCode::LeftControl},
				{AKEYCODE_ALT_LEFT, KeyCode::LeftAlt},
				{AKEYCODE_SHIFT_RIGHT, KeyCode::RightShift},
				{AKEYCODE_CTRL_RIGHT, KeyCode::RightControl},
				{AKEYCODE_ALT_RIGHT, KeyCode::RightAlt} };

			auto key_it = key_lookup.find(key);

			if (key_it == key_lookup.end())
			{
				return KeyCode::Unknown;
			}

			return key_it->second;
		}

		inline KeyAction TranslateKeyAction(int action)
		{
			if (action == AKEY_STATE_DOWN)
			{
				return KeyAction::Down;
			}
			else if (action == AKEY_STATE_UP)
			{
				return KeyAction::Up;
			}

			return KeyAction::Unknown;
		}

		inline KeyCode TranslateMouseButton(int button)
		{
			if (button < 3)
			{
				return static_cast<KeyCode>(button + static_cast<int32_t>(KeyCode::MouseLeft));
			}

			return KeyCode::Unknown;
		}

		inline MouseAction TranslateMouseAction(int action)
		{
			if (action == AMOTION_EVENT_ACTION_DOWN)
			{
				return MouseAction::Down;
			}
			else if (action == AMOTION_EVENT_ACTION_UP)
			{
				return MouseAction::Up;
			}
			else if (action == AMOTION_EVENT_ACTION_MOVE)
			{
				return MouseAction::Move;
			}

			return MouseAction::Unknown;
		}

		inline TouchAction TranslateTouchAction(int action)
		{
			action &= AMOTION_EVENT_ACTION_MASK;
			if (action == AMOTION_EVENT_ACTION_DOWN || action == AMOTION_EVENT_ACTION_POINTER_DOWN)
			{
				return TouchAction::Down;
			}
			else if (action == AMOTION_EVENT_ACTION_UP || action == AMOTION_EVENT_ACTION_POINTER_UP)
			{
				return TouchAction::Up;
			}
			else if (action == AMOTION_EVENT_ACTION_CANCEL)
			{
				return TouchAction::Cancel;
			}
			else if (action == AMOTION_EVENT_ACTION_MOVE)
			{
				return TouchAction::Move;
			}

			return TouchAction::Unknown;
		}

		void OnContentRectChanged(GameActivity* activity, const ARect* rect)
		{
			EDGE_LOGI("ContentRectChanged: {:p}\n", static_cast<void*>(activity));
			struct android_app* app = reinterpret_cast<struct android_app*>(activity->instance);
			auto cmd = APP_CMD_CONTENT_RECT_CHANGED;

			app->contentRect = *rect;

			if (write(app->msgwrite, &cmd, sizeof(cmd)) != sizeof(cmd))
			{
				EDGE_LOGE("Failure writing android_app cmd: {}\n", strerror(errno));
			}
		}

		void OnAppCmd(android_app* app, int32_t cmd)
		{
			auto runtime = reinterpret_cast<AndroidRuntime*>(app->userData);
			assert(runtime && "Runtime is not valid");

			switch (cmd)
			{
			case APP_CMD_INIT_WINDOW:
			{
				runtime->OnResize(ANativeWindow_getWidth(app->window), ANativeWindow_getHeight(app->window));
				runtime->SetSurfaceReady();
				break;
			}
			case APP_CMD_CONTENT_RECT_CHANGED:
			{
				// Get the new size
				auto width = app->contentRect.right - app->contentRect.left;
				auto height = app->contentRect.bottom - app->contentRect.top;
				runtime->OnResize(width, height);
				break;
			}
			case APP_CMD_GAINED_FOCUS:
			{
				runtime->SetFocus(true);
				break;
			}
			case APP_CMD_LOST_FOCUS:
			{
				runtime->SetFocus(false);
				break;
			}
			}
		}

		bool KeyEventFilter(const GameActivityKeyEvent* event)
		{
			if (event->source == AINPUT_SOURCE_KEYBOARD)
			{
				return true;
			}
			return false;
		}

		bool MotionEventFilter(const GameActivityMotionEvent* event)
		{
			if ((event->source == AINPUT_SOURCE_MOUSE) ||
				(event->source == AINPUT_SOURCE_TOUCHSCREEN))
			{
				return true;
			}
			return false;
		}

		AndroidRuntime::AndroidRuntime(const IRuntimeContext& context) :
			IRuntime(context)
		{
			ctx = static_cast<const AndroidRuntimeContext*>(&context);
		}

		int32_t AndroidRuntime::Initialize()
		{
			auto* app = ctx->GetApp();

			android_app_set_key_event_filter(app, KeyEventFilter);
			android_app_set_motion_event_filter(app, MotionEventFilter);

			app->onAppCmd = OnAppCmd;
			//app->activity->callbacks->onContentRectChanged = OnContentRectChanged;
			app->userData = this;

			auto code = Runtime::Initialize();
			if (code != ExitCode::Success)
			{
				return code;
			}

			do
			{
				if (!ProcessAndroidEvents(app))
				{
					// Android requested for the app to close
					EDGE_LOGI("Android app has been destroyed by the OS");
					return ExitCode::Close;
				}
			} while (!surface_ready);

			return ExitCode::Success;
		}

		void AndroidRuntime::Terminate(int32_t code)
		{
			switch (static_cast<ExitCode>(code))
			{
			case ExitCode::Success:
			case ExitCode::Close:
				break;
			case ExitCode::FatalError:
			{
				// TODO: crash dump!
			} break;
			default:
				break;
			}

			auto* app = ctx->GetApp();
			while (ProcessAndroidEvents(app));

			Runtime::Terminate(code);
		}

		void AndroidRuntime::ProcessInputEvents()
		{
			auto* app = ctx->GetApp();
			auto input_buf = android_app_swap_input_buffers(app);
			if (!input_buf)
			{
				return;
			}

			if (input_buf->motionEventsCount)
			{
				for (int idx = 0; idx < input_buf->motionEventsCount; idx++)
				{
					auto event = &input_buf->motionEvents[idx];
					assert((event->source == AINPUT_SOURCE_MOUSE ||
						event->source == AINPUT_SOURCE_TOUCHSCREEN) &&
						"Invalid motion event source");

					std::int32_t action = event->action;

					float x = GameActivityPointerAxes_getX(&event->pointers[0]);
					float y = GameActivityPointerAxes_getY(&event->pointers[0]);

					if (event->source == AINPUT_SOURCE_MOUSE)
					{
						// TODO: Add screen size here
						OnInputEvent(MouseButtonInputEvent{
							TranslateMouseButton(0),
							TranslateMouseAction(action),
							x, y, 0.f, 0.f });
					}
					else if (event->source == AINPUT_SOURCE_TOUCHSCREEN)
					{
						// Multiple pointers are not supported.
						size_t pointer_count = event->pointerCount;
						std::int32_t pointer_id = event->pointers[0].id;

						OnInputEvent(TouchInputEvent{
							pointer_id,
							pointer_count,
							TranslateTouchAction(action),
							x, y });
					}
				}
				android_app_clear_motion_events(input_buf);
			}

			if (input_buf->keyEventsCount)
			{
				for (int idx = 0; idx < input_buf->keyEventsCount; idx++)
				{
					auto event = &input_buf->keyEvents[idx];
					assert((event->source == AINPUT_SOURCE_KEYBOARD) &&
						"Invalid key event source");
					OnInputEvent(KeyInputEvent{
						TranslateKeyCode(event->keyCode),
						TranslateKeyAction(event->action) });
				}
				android_app_clear_key_events(input_buf);
			}
		}

		void AndroidRuntime::SetSurfaceReady()
		{
			surface_ready = true;
		}

		android_app* AndroidRuntime::GetAndroidApp()
		{
			return ctx->GetApp();
		}

		android_app const* AndroidRuntime::GetAndroidApp() const
		{
			return ctx->GetApp();
		}

		void AndroidRuntime::CreateWindow(const Window::Properties& properties)
		{
			auto app = ctx->GetApp();
			window = std::make_unique<AndroidWindow>(*this, app->window, properties);
		}
	}
}