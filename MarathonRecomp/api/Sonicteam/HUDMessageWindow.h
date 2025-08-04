#pragma once

#include <Marathon.inl>
#include <Sonicteam/SoX/Engine/Message.h>

namespace Sonicteam
{
    class HUDMessageWindow
    {
    public:
        class MsgChangeState : public SoX::Engine::Message
        {
        public:
            be<uint32_t> m_State{};
            be<uint32_t> m_Field08{};

            MsgChangeState(const uint32_t state) : m_State(state)
            {
                m_ID = 0x1B051;
            }
        };
    };
}
