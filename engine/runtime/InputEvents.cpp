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

    CharacterInputEvent::CharacterInputEvent(uint32_t charcode) :
        InputEvent{ EventSource::Character },
        character_code{ charcode }
    {
    }

    uint32_t CharacterInputEvent::GetCharacterCode() const
    {
        return character_code;
    }

    MouseButtonInputEvent::MouseButtonInputEvent(KeyCode button, MouseAction action, float value_x, float value_y, float max_x, float max_y) :
        InputEvent{ EventSource::Mouse },
        button{ button },
        action{ action },
        value_x{ value_x },
        value_y{ value_y },
        max_x{ max_x },
        max_y{ max_y }
    {
    }

    KeyCode MouseButtonInputEvent::GetButton() const
    {
        return button;
    }

    MouseAction MouseButtonInputEvent::GetAction() const
    {
        return action;
    }

    float MouseButtonInputEvent::GetValueX() const
    {
        return value_x;
    }

    float MouseButtonInputEvent::GetValueY() const
    {
        return value_y;
    }

    float MouseButtonInputEvent::GetMaxX() const
    {
        return max_x;
    }

    float MouseButtonInputEvent::GetMaxY() const
    {
        return max_y;
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