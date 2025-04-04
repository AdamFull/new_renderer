#include "rt_android_runtime.h"

#include "rt_android_runtime_context.h"

#include "rt_android_common.h"
#include "rt_android_window.h"

#include "../../util/Logger.h"

#include "../../fs/zip/fs_zip_filesystem.h"

#include <game-activity/GameActivity.h>

namespace edge::android
{
	inline KeyCode translate_key_code(int key) {
		static const std::unordered_map<int, KeyCode> key_lookup = {
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

		if (key_it == key_lookup.end()) {
			return KeyCode::Unknown;
		}

		return key_it->second;
	}

	inline KeyAction translate_key_action(int action) {
		if (action == AKEY_STATE_DOWN) {
			return KeyAction::Down;
		}
		else if (action == AKEY_STATE_UP) {
			return KeyAction::Up;
		}

		return KeyAction::Unknown;
	}

	inline KeyCode translate_mouse_button(int button) {
		if (button < 3) {
			return static_cast<KeyCode>(button + static_cast<int32_t>(KeyCode::MouseLeft));
		}

		return KeyCode::Unknown;
	}

	inline MouseAction translate_mouse_action(int action)
	{
		if (action == AMOTION_EVENT_ACTION_DOWN) {
			return MouseAction::Down;
		}
		else if (action == AMOTION_EVENT_ACTION_UP) {
			return MouseAction::Up;
		}
		else if (action == AMOTION_EVENT_ACTION_MOVE) {
			return MouseAction::Move;
		}

		return MouseAction::Unknown;
	}

	inline TouchAction translate_touch_action(int action) {
		action &= AMOTION_EVENT_ACTION_MASK;
		if (action == AMOTION_EVENT_ACTION_DOWN || action == AMOTION_EVENT_ACTION_POINTER_DOWN) {
			return TouchAction::Down;
		}
		else if (action == AMOTION_EVENT_ACTION_UP || action == AMOTION_EVENT_ACTION_POINTER_UP) {
			return TouchAction::Up;
		}
		else if (action == AMOTION_EVENT_ACTION_CANCEL) {
			return TouchAction::Cancel;
		}
		else if (action == AMOTION_EVENT_ACTION_MOVE) {
			return TouchAction::Move;
		}

		return TouchAction::Unknown;
	}

	void on_app_cmd(android_app* app, int32_t cmd) {
		auto runtime = reinterpret_cast<AndroidRuntime*>(app->userData);
		assert(runtime && "Runtime is not valid");

		switch (cmd) {
		case APP_CMD_INIT_WINDOW: {
			runtime->on_resize(ANativeWindow_getWidth(app->window), ANativeWindow_getHeight(app->window));
			runtime->set_surface_ready();
			break;
		}
		case APP_CMD_CONTENT_RECT_CHANGED: {
			// Get the new size
			auto width = app->contentRect.right - app->contentRect.left;
			auto height = app->contentRect.bottom - app->contentRect.top;
			runtime->on_resize(width, height);
			break;
		}
		case APP_CMD_GAINED_FOCUS: {
			runtime->set_focus(true);
			break;
		}
		case APP_CMD_LOST_FOCUS: {
			runtime->set_focus(false);
			break;
		}
		}
	}

	bool key_event_filter(const GameActivityKeyEvent* event) {
		if (event->source == AINPUT_SOURCE_KEYBOARD) {
			return true;
		}
		return false;
	}

	bool motion_event_filter(const GameActivityMotionEvent* event) {
		if ((event->source == AINPUT_SOURCE_MOUSE) ||
			(event->source == AINPUT_SOURCE_TOUCHSCREEN)) {
			return true;
		}
		return false;
	}

	AndroidRuntime::AndroidRuntime(IRuntimeContext& context) :
            Runtime(context) {
		ctx = static_cast<AndroidRuntimeContext*>(&context);
	}

    auto get_jni_env(android_app* app) -> JNIEnv* {
        JNIEnv* env{ nullptr };
        jint get_env_result = app->activity->vm->GetEnv((void**)&env, JNI_VERSION_1_6);
        if (get_env_result == JNI_EDETACHED) {
            if (app->activity->vm->AttachCurrentThread(&env, nullptr) != JNI_OK) {
                return nullptr;
            }
        }

        return env;
    }

    auto get_package_code_path(android_app* app) -> std::string {
        auto* env = get_jni_env(app);

        jclass clazz = env->GetObjectClass(app->activity->javaGameActivity);
        jmethodID methodID = env->GetMethodID(clazz, "getPackageCodePath", "()Ljava/lang/String;");
        jstring apk_path_java = (jstring)env->CallObjectMethod(app->activity->javaGameActivity, methodID);

        const char* apk_path_chars = env->GetStringUTFChars(apk_path_java, nullptr);
        return std::string(apk_path_chars ? apk_path_chars : "");
    }

	int32_t AndroidRuntime::initialize() {
		auto* app = ctx->get_app();

        internal_resources_path = get_package_code_path(app);
        external_resources_path = app->activity->obbPath;
        internal_data_path = app->activity->internalDataPath;
        external_data_path = app->activity->externalDataPath;

		android_app_set_key_event_filter(app, key_event_filter);
		android_app_set_motion_event_filter(app, motion_event_filter);

		app->onAppCmd = on_app_cmd;
		app->userData = this;

		auto code = Runtime::initialize();
		if (code != ExitCode::Success) {
			return code;
		}

		do {
			if (!process_android_events(app)) {
				// Android requested for the app to close
				EDGE_LOGI("Android app has been destroyed by the OS");
				return ExitCode::Close;
			}
		} while (!surface_ready);

		return ExitCode::Success;
	}

	void AndroidRuntime::terminate(int32_t code) {
		switch (static_cast<ExitCode>(code)) {
		case ExitCode::Success:
		case ExitCode::Close:
			break;
		case ExitCode::FatalError: {
			// TODO: crash dump!
		} break;
		default:
			break;
		}

		auto* app = ctx->get_app();
		while (process_android_events(app));

		Runtime::terminate(code);
	}

	void AndroidRuntime::process_input_events() {
		auto* app = ctx->get_app();
		auto input_buf = android_app_swap_input_buffers(app);
		if (!input_buf) {
			return;
		}

		if (input_buf->motionEventsCount) {
			for (int idx = 0; idx < input_buf->motionEventsCount; idx++) {
				auto event = &input_buf->motionEvents[idx];
				assert((event->source == AINPUT_SOURCE_MOUSE ||
					event->source == AINPUT_SOURCE_TOUCHSCREEN) &&
					"Invalid motion event source");

				std::int32_t action = event->action;

				float x = GameActivityPointerAxes_getX(&event->pointers[0]);
				float y = GameActivityPointerAxes_getY(&event->pointers[0]);

				if (event->source == AINPUT_SOURCE_MOUSE) {
					// TODO: Add screen size here
					on_input_event(MouseButtonInputEvent{
						translate_mouse_button(0),
						translate_mouse_action(action),
						x, y, 0.f, 0.f });
				}
				else if (event->source == AINPUT_SOURCE_TOUCHSCREEN) {
					// Multiple pointers are not supported.
					size_t pointer_count = event->pointerCount;
					std::int32_t pointer_id = event->pointers[0].id;

					on_input_event(TouchInputEvent{
						pointer_id,
						pointer_count,
						translate_touch_action(action),
						x, y });
				}
			}
			android_app_clear_motion_events(input_buf);
		}

		if (input_buf->keyEventsCount) {
			for (int idx = 0; idx < input_buf->keyEventsCount; idx++) {
				auto event = &input_buf->keyEvents[idx];
				assert((event->source == AINPUT_SOURCE_KEYBOARD) &&
					"Invalid key event source");
				on_input_event(KeyInputEvent{
					translate_key_code(event->keyCode),
					translate_key_action(event->action) });
			}
			android_app_clear_key_events(input_buf);
		}
	}

	void AndroidRuntime::set_surface_ready() {
		surface_ready = true;
	}

	android_app* AndroidRuntime::get_android_app() {
		return ctx->get_app();
	}

	android_app const* AndroidRuntime::get_android_app() const {
		return ctx->get_app();
	}

	void AndroidRuntime::create_window(const Window::Properties& properties) {
		auto app = ctx->get_app();
		window = std::make_unique<AndroidWindow>(*this, app->window, properties);
	}
}