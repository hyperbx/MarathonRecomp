#pragma once

#include <Marathon.inl>

namespace Sonicteam
{
    class MessageWindowTask : public SoX::Engine::Task
    {
    public:
        MARATHON_INSERT_PADDING(0x14);
        be<uint32_t> m_State;
        MARATHON_INSERT_PADDING(8);
        xpointer<HUDMessageWindow> m_pHUDMessageWindow;
    };
}
