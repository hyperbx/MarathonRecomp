#pragma once

#include <Marathon.inl>

namespace Sonicteam
{
    class HintWindowTask : public SoX::Engine::Task
    {
    public:
        MARATHON_INSERT_PADDING(0x24);
        xpointer<HUDMessageWindow> m_pHUDMessageWindow;
    };
}
