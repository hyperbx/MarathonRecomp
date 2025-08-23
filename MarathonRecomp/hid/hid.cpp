#include "hid.h"
#include <ui/game_window.h>
#include <user/config.h>

bool hid::IsInputAllowed()
{
    return GameWindow::s_isFocused || Config::AllowBackgroundInput;
}

bool hid::IsGamepad(uint32_t dwUserIndex)
{
    auto category = hid::GetControllerCategory(dwUserIndex);

    return category != hid::EControllerCategory::Keyboard &&
           category != hid::EControllerCategory::Mouse;
}

bool hid::IsPlayStation(uint32_t dwUserIndex)
{
    auto isPlayStation = Config::ControllerIcons == EControllerIcons::PlayStation;

    if (Config::ControllerIcons == EControllerIcons::Auto)
        isPlayStation = hid::GetControllerCategory(dwUserIndex) == hid::EControllerCategory::PlayStation;

    return isPlayStation;
}
