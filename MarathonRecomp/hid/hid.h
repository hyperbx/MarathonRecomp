#pragma once

namespace hid
{
    struct InputProhibitor
    {
        uint16_t Buttons{};
        bool LeftStick{};
        bool RightStick{};
    };

    enum class EControllerCategory
    {
        Unknown,
        Keyboard,
        Mouse,
        Xbox,
        PlayStation
    };

    void Init();

    uint32_t GetState(uint32_t dwUserIndex, XAMINPUT_STATE* pState);
    uint32_t SetState(uint32_t dwUserIndex, XAMINPUT_VIBRATION* pVibration);
    uint32_t GetCapabilities(uint32_t dwUserIndex, XAMINPUT_CAPABILITIES* pCapabilities);
    EControllerCategory GetControllerCategory();
    EControllerCategory GetControllerCategory(uint32_t dwUserIndex);
    void SetProhibitedInputs(uint32_t dwUserIndex = 0, uint16_t wButtons = 0, bool leftStick = false, bool rightStick = false);
    InputProhibitor GetProhibitedInputs(uint32_t dwUserIndex);

    bool IsInputAllowed();
    bool IsGamepad(uint32_t dwUserIndex = 0);
    bool IsPlayStation(uint32_t dwUserIndex = 0);
}
