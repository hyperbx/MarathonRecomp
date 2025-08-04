#pragma once

#include <Marathon.inl>
#include <Sonicteam/TextEntity.h>

namespace Sonicteam
{
    class PauseTask : public SoX::Engine::Task
    {
    public:
        MARATHON_INSERT_PADDING(0x230);
        xpointer<TextEntity> m_pMissionText;
    };
}
