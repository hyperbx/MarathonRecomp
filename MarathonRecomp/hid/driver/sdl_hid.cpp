#include <hid/hid.h>
#include <kernel/xdm.h>
#include <os/logger.h>
#include <ui/game_window.h>
#include <user/config.h>
#include <app.h>
#include <SDL.h>

#define TRANSLATE_INPUT(S, X) SDL_GameControllerGetButton(pController, S) << FirstBitLow(X)
#define VIBRATION_TIMEOUT_MS 5000

class Controller
{
public:
    SDL_GameController* pController{};
    SDL_Joystick* pJoystick{};
    SDL_JoystickID ID{ -1 };
    SDL_GameControllerType CurrentType{ SDL_CONTROLLER_TYPE_UNKNOWN };
    XAMINPUT_GAMEPAD State{};
    XAMINPUT_VIBRATION Vibration{ 0, 0 };
    hid::InputProhibitor ProhibitedInputs{};
    int Index{ -1 };

    Controller() = default;

    explicit Controller(int index) : Controller(SDL_GameControllerOpen(index), index) {}

    Controller(SDL_GameController* controller, int index = 0) : pController(controller)
    {
        if (!controller)
            return;

        Index = index;

        pJoystick = SDL_GameControllerGetJoystick(controller);
        ID = SDL_JoystickInstanceID(pJoystick);
        CurrentType = GetControllerType();

        LOGFN("Detected controller (P{}): {}", Index + 1, GetControllerName());
    }

    SDL_GameControllerType GetControllerType() const
    {
        return SDL_GameControllerGetType(pController);
    }

    hid::EControllerCategory GetControllerCategory() const
    {
        switch (GetControllerType())
        {
            case SDL_CONTROLLER_TYPE_PS3:
            case SDL_CONTROLLER_TYPE_PS4:
            case SDL_CONTROLLER_TYPE_PS5:
                return hid::EControllerCategory::PlayStation;

            case SDL_CONTROLLER_TYPE_XBOX360:
            case SDL_CONTROLLER_TYPE_XBOXONE:
                return hid::EControllerCategory::Xbox;

            default:
                return hid::EControllerCategory::Unknown;
        }
    }

    const char* GetControllerName() const
    {
        auto result = SDL_GameControllerName(pController);

        if (!result)
            return "Unknown Device";

        return result;
    }

    void Close()
    {
        if (!pController)
            return;

        SDL_GameControllerClose(pController);

        pController = nullptr;
        pJoystick = nullptr;
        ID = -1;
        CurrentType = SDL_CONTROLLER_TYPE_UNKNOWN;
        Index = -1;
    }

    bool CanPoll()
    {
        return pController;
    }

    void PollAxis()
    {
        if (!CanPoll())
            return;

        auto& pad = State;

        pad.sThumbLX = SDL_GameControllerGetAxis(pController, SDL_CONTROLLER_AXIS_LEFTX);
        pad.sThumbLY = ~SDL_GameControllerGetAxis(pController, SDL_CONTROLLER_AXIS_LEFTY);

        pad.sThumbRX = SDL_GameControllerGetAxis(pController, SDL_CONTROLLER_AXIS_RIGHTX);
        pad.sThumbRY = ~SDL_GameControllerGetAxis(pController, SDL_CONTROLLER_AXIS_RIGHTY);

        pad.bLeftTrigger = SDL_GameControllerGetAxis(pController, SDL_CONTROLLER_AXIS_TRIGGERLEFT) >> 7;
        pad.bRightTrigger = SDL_GameControllerGetAxis(pController, SDL_CONTROLLER_AXIS_TRIGGERRIGHT) >> 7;
    }

    void Poll()
    {
        if (!CanPoll())
            return;

        auto& pad = State;

        pad.wButtons = 0;

        pad.wButtons |= TRANSLATE_INPUT(SDL_CONTROLLER_BUTTON_DPAD_UP, XAMINPUT_GAMEPAD_DPAD_UP);
        pad.wButtons |= TRANSLATE_INPUT(SDL_CONTROLLER_BUTTON_DPAD_DOWN, XAMINPUT_GAMEPAD_DPAD_DOWN);
        pad.wButtons |= TRANSLATE_INPUT(SDL_CONTROLLER_BUTTON_DPAD_LEFT, XAMINPUT_GAMEPAD_DPAD_LEFT);
        pad.wButtons |= TRANSLATE_INPUT(SDL_CONTROLLER_BUTTON_DPAD_RIGHT, XAMINPUT_GAMEPAD_DPAD_RIGHT);

        pad.wButtons |= TRANSLATE_INPUT(SDL_CONTROLLER_BUTTON_START, XAMINPUT_GAMEPAD_START);
        pad.wButtons |= TRANSLATE_INPUT(SDL_CONTROLLER_BUTTON_BACK, XAMINPUT_GAMEPAD_BACK);
        pad.wButtons |= TRANSLATE_INPUT(SDL_CONTROLLER_BUTTON_TOUCHPAD, XAMINPUT_GAMEPAD_BACK);

        pad.wButtons |= TRANSLATE_INPUT(SDL_CONTROLLER_BUTTON_LEFTSTICK, XAMINPUT_GAMEPAD_LEFT_THUMB);
        pad.wButtons |= TRANSLATE_INPUT(SDL_CONTROLLER_BUTTON_RIGHTSTICK, XAMINPUT_GAMEPAD_RIGHT_THUMB);

        pad.wButtons |= TRANSLATE_INPUT(SDL_CONTROLLER_BUTTON_LEFTSHOULDER, XAMINPUT_GAMEPAD_LEFT_SHOULDER);
        pad.wButtons |= TRANSLATE_INPUT(SDL_CONTROLLER_BUTTON_RIGHTSHOULDER, XAMINPUT_GAMEPAD_RIGHT_SHOULDER);

        pad.wButtons |= TRANSLATE_INPUT(SDL_CONTROLLER_BUTTON_A, XAMINPUT_GAMEPAD_A);
        pad.wButtons |= TRANSLATE_INPUT(SDL_CONTROLLER_BUTTON_B, XAMINPUT_GAMEPAD_B);
        pad.wButtons |= TRANSLATE_INPUT(SDL_CONTROLLER_BUTTON_X, XAMINPUT_GAMEPAD_X);
        pad.wButtons |= TRANSLATE_INPUT(SDL_CONTROLLER_BUTTON_Y, XAMINPUT_GAMEPAD_Y);
    }

    void SetVibration(const XAMINPUT_VIBRATION& vibration)
    {
        if (!CanPoll())
            return;

        Vibration = vibration;

        SDL_GameControllerRumble(pController, vibration.wLeftMotorSpeed, vibration.wRightMotorSpeed, VIBRATION_TIMEOUT_MS);
    }

    void SetProhibitedInputs(uint16_t wButtons, bool leftStick, bool rightStick)
    {
        ProhibitedInputs = hid::InputProhibitor{ wButtons, leftStick, rightStick };
    }

    void SetLED(const uint8_t r, const uint8_t g, const uint8_t b) const
    {
        SDL_GameControllerSetLED(pController, r, g, b);
    }
};

std::array<Controller, 4> g_controllers;
hid::EControllerCategory g_controllerCategory;

inline Controller* EnsureController(uint32_t dwUserIndex)
{
    if (!g_controllers[dwUserIndex].pController)
        return nullptr;

    return &g_controllers[dwUserIndex];
}

inline size_t FindFreeController()
{
    for (size_t i = 0; i < g_controllers.size(); i++)
    {
        if (!g_controllers[i].pController)
            return i;
    }

    return -1;
}

inline Controller* FindController(uint32_t dwUserIndex)
{
    for (auto& controller : g_controllers)
    {
        if (controller.ID == dwUserIndex)
            return &controller;
    }

    return nullptr;
}

inline void SetControllerLED(Controller& controller, EPlayerCharacter player)
{
    uint8_t r, g, b;

    // TODO
    switch (player)
    {
        case EPlayerCharacter::Sonic:
        case EPlayerCharacter::Shadow:
        case EPlayerCharacter::Silver:
        case EPlayerCharacter::Blaze:
        case EPlayerCharacter::Amy:
        case EPlayerCharacter::Tails:
        case EPlayerCharacter::Rouge:
        case EPlayerCharacter::Knuckles:
            break;
    }

    r = 0;
    g = 37;
    b = 184;

    controller.SetLED(r, g, b);
}

int HID_OnSDLEvent(void*, SDL_Event* event)
{
    switch (event->type)
    {
        case SDL_CONTROLLERDEVICEADDED:
        {
            const auto freeIndex = FindFreeController();

            if (freeIndex != -1)
            {
                auto controller = Controller(event->cdevice.which);

                g_controllers[freeIndex] = controller;

                SetControllerLED(controller, App::s_playerCharacter);
            }

            break;
        }

        case SDL_CONTROLLERDEVICEREMOVED:
        {
            auto* controller = FindController(event->cdevice.which);

            if (controller)
                controller->Close();

            break;
        }

        case SDL_CONTROLLERBUTTONDOWN:
        case SDL_CONTROLLERBUTTONUP:
        case SDL_CONTROLLERAXISMOTION:
        case SDL_CONTROLLERTOUCHPADDOWN:
        {
            auto* controller = FindController(event->cdevice.which);

            if (!controller)
                break;

            if (event->type == SDL_CONTROLLERAXISMOTION)
            {
                if (abs(event->caxis.value) > 8000)
                {
                    SDL_ShowCursor(SDL_DISABLE);

                    g_controllerCategory = controller->GetControllerCategory();
                }

                controller->PollAxis();
            }
            else
            {
                SDL_ShowCursor(SDL_DISABLE);

                g_controllerCategory = controller->GetControllerCategory();

                controller->Poll();
            }

            break;
        }

        case SDL_KEYDOWN:
        case SDL_KEYUP:
            g_controllerCategory = hid::EControllerCategory::Keyboard;
            break;

        case SDL_MOUSEMOTION:
        case SDL_MOUSEBUTTONDOWN:
        case SDL_MOUSEBUTTONUP:
        {
            if (!GameWindow::IsFullscreen() || GameWindow::s_isFullscreenCursorVisible)
                SDL_ShowCursor(SDL_ENABLE);

            g_controllerCategory = hid::EControllerCategory::Mouse;

            break;
        }

        case SDL_WINDOWEVENT:
        {
            if (event->window.event == SDL_WINDOWEVENT_FOCUS_LOST)
            {
                // Stop vibrating controllers on focus lost.
                for (auto& controller : g_controllers)
                    controller.SetVibration({ 0, 0 });
            }

            break;
        }

        case SDL_USER_PLAYER_CHAR:
        {
            for (auto& controller : g_controllers)
                SetControllerLED(controller, static_cast<EPlayerCharacter>(event->user.code));

            break;
        }
    }

    return 0;
}

void hid::Init()
{
    SDL_SetHint(SDL_HINT_JOYSTICK_ALLOW_BACKGROUND_EVENTS, "1");
    SDL_SetHint(SDL_HINT_JOYSTICK_HIDAPI_GAMECUBE, "1");
    SDL_SetHint(SDL_HINT_JOYSTICK_HIDAPI_PS3, "1");
    SDL_SetHint(SDL_HINT_JOYSTICK_HIDAPI_PS4, "1");
    SDL_SetHint(SDL_HINT_JOYSTICK_HIDAPI_PS4_RUMBLE, "1");
    SDL_SetHint(SDL_HINT_JOYSTICK_HIDAPI_PS5, "1");
    SDL_SetHint(SDL_HINT_JOYSTICK_HIDAPI_PS5_PLAYER_LED, "1");
    SDL_SetHint(SDL_HINT_JOYSTICK_HIDAPI_PS5_RUMBLE, "1");
    SDL_SetHint(SDL_HINT_JOYSTICK_HIDAPI_WII, "1");
    SDL_SetHint(SDL_HINT_JOYSTICK_HIDAPI_STEAM, "1");
    SDL_SetHint(SDL_HINT_JOYSTICK_HIDAPI_STEAMDECK, "1");
    SDL_SetHint(SDL_HINT_XINPUT_ENABLED, "1");
    
    // This hint is disabled for Nintendo controllers.
    SDL_SetHint(SDL_HINT_GAMECONTROLLER_USE_BUTTON_LABELS, "0");

    SDL_InitSubSystem(SDL_INIT_EVENTS);
    SDL_AddEventWatch(HID_OnSDLEvent, nullptr);
    SDL_InitSubSystem(SDL_INIT_GAMECONTROLLER);

    // Load controller mappings from database.
    if (int mappings = SDL_GameControllerAddMappingsFromFile("gamecontrollerdb.txt"); mappings > 0)
        LOGFN("Loaded {} controller mappings: {}", mappings, "gamecontrollerdb.txt");
}

uint32_t hid::GetState(uint32_t dwUserIndex, XAMINPUT_STATE* pState)
{
    static uint32_t packet;

    if (!pState)
        return ERROR_BAD_ARGUMENTS;

    memset(pState, 0, sizeof(*pState));

    pState->dwPacketNumber = packet++;

    if (auto pController = EnsureController(dwUserIndex))
    {
        pState->Gamepad = pController->State;
    }
    else
    {
        return ERROR_DEVICE_NOT_CONNECTED;
    }

    return ERROR_SUCCESS;
}

uint32_t hid::SetState(uint32_t dwUserIndex, XAMINPUT_VIBRATION* pVibration)
{
    if (!pVibration)
        return ERROR_BAD_ARGUMENTS;

    if (auto pController = EnsureController(dwUserIndex))
    {
        pController->SetVibration(*pVibration);
    }
    else
    {
        return ERROR_DEVICE_NOT_CONNECTED;
    }

    return ERROR_SUCCESS;
}

uint32_t hid::GetCapabilities(uint32_t dwUserIndex, XAMINPUT_CAPABILITIES* pCapabilities)
{
    if (!pCapabilities)
        return ERROR_BAD_ARGUMENTS;

    if (auto pController = EnsureController(dwUserIndex))
    {
        memset(pCapabilities, 0, sizeof(*pCapabilities));

        pCapabilities->Type = XAMINPUT_DEVTYPE_GAMEPAD;
        pCapabilities->SubType = XAMINPUT_DEVSUBTYPE_GAMEPAD;
        pCapabilities->Flags = 0;
        pCapabilities->Gamepad = pController->State;
        pCapabilities->Vibration = pController->Vibration;
    }
    else
    {
        return ERROR_DEVICE_NOT_CONNECTED;
    }

    return ERROR_SUCCESS;
}

hid::EControllerCategory hid::GetControllerCategory()
{
    return g_controllerCategory;
}

hid::EControllerCategory hid::GetControllerCategory(uint32_t dwUserIndex)
{
    auto pController = EnsureController(dwUserIndex);

    if (!pController)
        return g_controllerCategory;

    return pController->GetControllerCategory();
}

void hid::SetProhibitedInputs(uint32_t dwUserIndex, uint16_t wButtons, bool leftStick, bool rightStick)
{
    auto pController = EnsureController(dwUserIndex);

    if (!pController)
        return;

    pController->SetProhibitedInputs(wButtons, leftStick, rightStick);
}

hid::InputProhibitor hid::GetProhibitedInputs(uint32_t dwUserIndex)
{
    auto pController = EnsureController(dwUserIndex);

    if (!pController)
        return {};

    return pController->ProhibitedInputs;
}
