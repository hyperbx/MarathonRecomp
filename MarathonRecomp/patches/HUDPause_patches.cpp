#include <api/Marathon.h>
#include <user/config.h>
#include <app.h>

static std::vector<xpointer<Sonicteam::HUDMessageWindow>> g_messageWindows;

// Sonicteam::HUDPause::ProcessMessage
PPC_FUNC_IMPL(__imp__sub_824EF788);
PPC_FUNC(sub_824EF788)
{
    if (!Config::RestorePauseMissionText)
    {
        __imp__sub_824EF788(ctx, base);
        return;
    }

    auto pHUDPause = (Sonicteam::HUDPause*)(base + ctx.r3.u32);
    auto pMessage = (Sonicteam::HUDPause::MsgChangeState*)(base + ctx.r4.u32);

    pHUDPause->m_ShowMissionWindow = true;

    if (pMessage->m_State == 0)
    {
        guest_stack_var<Sonicteam::HUDMessageWindow::MsgChangeState> msgChangeState(2);

        for (auto& pMessageWindow : g_messageWindows)
            GuestToHostFunction<int>(sub_824E9FB8, pMessageWindow.get(), msgChangeState.get());
    }

    __imp__sub_824EF788(ctx, base);
}

// Sonicteam::HUDMessageWindow::HUDMessageWindow
PPC_FUNC_IMPL(__imp__sub_824E9E30);
PPC_FUNC(sub_824E9E30)
{
    __imp__sub_824E9E30(ctx, base);

    if (!Config::RestorePauseMissionText)
        return;
    
    g_messageWindows.push_back(xpointer<Sonicteam::HUDMessageWindow>((Sonicteam::HUDMessageWindow*)(base + ctx.r3.u32)));
}

// Sonicteam::HUDMessageWindow::~HUDMessageWindow
PPC_FUNC_IMPL(__imp__sub_824E9AE0);
PPC_FUNC(sub_824E9AE0)
{
    if (Config::RestorePauseMissionText)
    {
        for (int i = 0; i < g_messageWindows.size(); i++)
        {
            if (g_messageWindows[i].ptr == ctx.r3.u32)
                g_messageWindows.erase(g_messageWindows.begin() + i);
        }
    }

    __imp__sub_824E9AE0(ctx, base);
}

// The mission text is drawn at a lower priority
// than the mission box by default. 1001.0f is the
// priority value used by the rest of the pause menu.
void SetMissionTextPriority(PPCRegister& priority)
{
    priority.f64 = 1001.0f;
}
