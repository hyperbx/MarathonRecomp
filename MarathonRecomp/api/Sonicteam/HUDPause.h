#pragma once

#include <Marathon.inl>

namespace Sonicteam
{
    class HUDPause : public SoX::Engine::Task
    {
    public:
        class MsgChangeState : public SoX::Engine::Message
        {
        public:
            be<uint32_t> m_State{};
            be<uint32_t> m_Field08{};

            MsgChangeState(const uint32_t state) : m_State(state)
            {
                m_ID = 0x1B04E;
            }
        };

        MARATHON_INSERT_PADDING(0xAC);
        be<float> m_TextPriority;
        MARATHON_INSERT_PADDING(0x44);
        bool m_ShowMissionWindow;
    };

    MARATHON_ASSERT_OFFSETOF(HUDPause, m_TextPriority, 0xF8);
    MARATHON_ASSERT_OFFSETOF(HUDPause, m_ShowMissionWindow, 0x140);
}
