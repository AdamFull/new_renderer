#include <engine/runtime/InputEvents.h>

namespace engine
{
    InputEvent::InputEvent(EventSource source) :
        source{ source }
    {
    }

    EventSource InputEvent::GetSource() const
    {
        return source;
    }

    KeyInputEvent::KeyInputEvent(KeyCode code, KeyAction action) :
        InputEvent{ EventSource::Keyboard },
        code{ code },
        action{ action }
    {
    }

    KeyCode KeyInputEvent::GetCode() const
    {
        return code;
    }

    KeyAction KeyInputEvent::GetAction() const
    {
        return action;
    }

    MouseButtonInputEvent::MouseButtonInputEvent(MouseButton button, MouseAction action, float pos_x, float pos_y) :
        InputEvent{ EventSource::Mouse },
        button{ button },
        action{ action },
        pos_x{ pos_x },
        pos_y{ pos_y }
    {
    }

    MouseButton MouseButtonInputEvent::GetButton() const
    {
        return button;
    }

    MouseAction MouseButtonInputEvent::GetAction() const
    {
        return action;
    }

    float MouseButtonInputEvent::GetPosX() const
    {
        return pos_x;
    }

    float MouseButtonInputEvent::GetPosY() const
    {
        return pos_y;
    }

    TouchInputEvent::TouchInputEvent(int32_t pointer_id, std::size_t touch_points, TouchAction action, float pos_x, float pos_y) :
        InputEvent{ EventSource::Touchscreen },
        action{ action },
        pointer_id{ pointer_id },
        touch_points{ touch_points },
        pos_x{ pos_x },
        pos_y{ pos_y }
    {
    }

    TouchAction TouchInputEvent::GetAction() const
    {
        return action;
    }

    int32_t TouchInputEvent::GetPointerId() const
    {
        return pointer_id;
    }

    std::size_t TouchInputEvent::GetTouchPoints() const
    {
        return touch_points;
    }

    float TouchInputEvent::GetPosX() const
    {
        return pos_x;
    }

    float TouchInputEvent::GetPosY() const
    {
        return pos_y;
    }
}