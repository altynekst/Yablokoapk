/* Yabloko client — reconstructed from the surviving build.
   HUD subsystem implementation. */

#include "yabloko_toolkit.h"

#include <engine/graphics.h>
#include <engine/serverbrowser.h>
#include <engine/shared/config.h>
#include <engine/textrender.h>

#include <generated/protocol.h>

#include <game/client/gameclient.h>
#include <game/client/ui.h>

#include <algorithm>

const char *const CYablokoToolkit::ms_apPanelNames[CYablokoToolkit::NUM_PANELS] = {
	"Server", "Top", "Players", "Results", "Music", "Vote", "Chat", "Radar", "Active", "Inputs"};

void CYablokoToolkit::OnReset()
{
	m_vNotifications.clear();
	m_HudEditActive = false;
	m_HudEditPanel = -1;
	m_BackButtonPress = 0.0f;
	m_BackButtonPos = vec2(0.85f, 0.85f);
}

void CYablokoToolkit::OnStateChange(int NewState, int OldState)
{
	(void)NewState;
	(void)OldState;
}

void CYablokoToolkit::Notify(const char *pText)
{
	m_vNotifications.push_back({std::string(pText), Client()->GlobalTime()});
	if(m_vNotifications.size() > 8)
		m_vNotifications.erase(m_vNotifications.begin());
}

void CYablokoToolkit::NotifyJoin(const char *pName)
{
	if(g_Config.m_ClNotify && g_Config.m_ClNotifyJoin)
	{
		char aBuf[256];
		str_format(aBuf, sizeof(aBuf), "→ %s joined", pName);
		Notify(aBuf);
	}
}

void CYablokoToolkit::NotifyLeave(const char *pName)
{
	if(g_Config.m_ClNotify && g_Config.m_ClNotifyLeave)
	{
		char aBuf[256];
		str_format(aBuf, sizeof(aBuf), "← %s left", pName);
		Notify(aBuf);
	}
}

void CYablokoToolkit::NotifyMention(const char *pName, const char *pText)
{
	(void)pText;
	if(g_Config.m_ClNotify && g_Config.m_ClNotifyMention)
	{
		char aBuf[256];
		str_format(aBuf, sizeof(aBuf), "@ %s mentioned you", pName);
		Notify(aBuf);
	}
}

void CYablokoToolkit::NotifyWhisper(const char *pName, const char *pText)
{
	(void)pText;
	if(g_Config.m_ClNotify && g_Config.m_ClNotifyWhisper)
	{
		char aBuf[256];
		str_format(aBuf, sizeof(aBuf), "✉ %s", pName);
		Notify(aBuf);
	}
}

bool CYablokoToolkit::OnInput(const IInput::CEvent &Event)
{
	// HUD editor: move panels with mouse handled in OnRender, escape stops editing
	if(m_HudEditActive && Event.m_Flags & IInput::FLAG_PRESS && Event.m_Key == KEY_ESCAPE)
	{
		StopHudEdit();
		return true;
	}
	return false;
}

void CYablokoToolkit::StartHudEdit()
{
	m_HudEditActive = true;
	Notify("HUD edit: drag panels, ESC to finish");
}

void CYablokoToolkit::StopHudEdit()
{
	m_HudEditActive = false;
	m_HudEditPanel = -1;
}

void CYablokoToolkit::GetPanelRect(int PanelIndex, vec4 &Rect) const
{
	const CConfig &C = g_Config;
	switch(PanelIndex)
	{
	case 0: Rect = vec4(C.m_ClYablokoHudServerX, C.m_ClYablokoHudServerY, C.m_ClYablokoHudServerWidth, C.m_ClYablokoHudServerHeight); break;
	case 1: Rect = vec4(C.m_ClYablokoHudTopX, C.m_ClYablokoHudTopY, C.m_ClYablokoHudTopWidth, C.m_ClYablokoHudTopHeight); break;
	case 2: Rect = vec4(C.m_ClYablokoHudPlayersX, C.m_ClYablokoHudPlayersY, C.m_ClYablokoHudPlayersWidth, C.m_ClYablokoHudPlayersHeight); break;
	case 3: Rect = vec4(C.m_ClYablokoHudResultsX, C.m_ClYablokoHudResultsY, C.m_ClYablokoHudResultsWidth, C.m_ClYablokoHudResultsHeight); break;
	case 4: Rect = vec4(C.m_ClYablokoHudMusicX, C.m_ClYablokoHudMusicY, C.m_ClYablokoHudMusicWidth, C.m_ClYablokoHudMusicHeight); break;
	case 5: Rect = vec4(C.m_ClYablokoHudVoteX, C.m_ClYablokoHudVoteY, C.m_ClYablokoHudVoteWidth, C.m_ClYablokoHudVoteHeight); break;
	case 6: Rect = vec4(C.m_ClYablokoHudChatX, C.m_ClYablokoHudChatY, C.m_ClYablokoHudChatWidth, C.m_ClYablokoHudChatHeight); break;
	case 7: Rect = vec4(C.m_ClYablokoHudRadarX, C.m_ClYablokoHudRadarY, C.m_ClYablokoHudRadarWidth, C.m_ClYablokoHudRadarHeight); break;
	case 8: Rect = vec4(C.m_ClYablokoHudActiveX, C.m_ClYablokoHudActiveY, C.m_ClYablokoHudActiveWidth, C.m_ClYablokoHudActiveHeight); break;
	default: Rect = vec4(C.m_ClYablokoHudInputsX, C.m_ClYablokoHudInputsY, C.m_ClYablokoHudInputsWidth, C.m_ClYablokoHudInputsHeight); break;
	}
	Rect.x = std::clamp(Rect.x, 0.0f, 100.0f);
	Rect.y = std::clamp(Rect.y, 0.0f, 100.0f);
	Rect.z = std::clamp(Rect.z, 1.0f, 100.0f);
	Rect.w = std::clamp(Rect.w, 1.0f, 100.0f);
}

void CYablokoToolkit::SetPanelRect(int PanelIndex, const vec4 &Rect)
{
	CConfig *pC = &g_Config;
	int X = (int)std::clamp(Rect.x, 0.0f, 100.0f);
	int Y = (int)std::clamp(Rect.y, 0.0f, 100.0f);
	int W = (int)std::clamp(Rect.z, 1.0f, 100.0f);
	int H = (int)std::clamp(Rect.w, 1.0f, 100.0f);
	switch(PanelIndex)
	{
	case 0: pC->m_ClYablokoHudServerX = X; pC->m_ClYablokoHudServerY = Y; pC->m_ClYablokoHudServerWidth = W; pC->m_ClYablokoHudServerHeight = H; break;
	case 1: pC->m_ClYablokoHudTopX = X; pC->m_ClYablokoHudTopY = Y; pC->m_ClYablokoHudTopWidth = W; pC->m_ClYablokoHudTopHeight = H; break;
	case 2: pC->m_ClYablokoHudPlayersX = X; pC->m_ClYablokoHudPlayersY = Y; pC->m_ClYablokoHudPlayersWidth = W; pC->m_ClYablokoHudPlayersHeight = H; break;
	case 3: pC->m_ClYablokoHudResultsX = X; pC->m_ClYablokoHudResultsY = Y; pC->m_ClYablokoHudResultsWidth = W; pC->m_ClYablokoHudResultsHeight = H; break;
	case 4: pC->m_ClYablokoHudMusicX = X; pC->m_ClYablokoHudMusicY = Y; pC->m_ClYablokoHudMusicWidth = W; pC->m_ClYablokoHudMusicHeight = H; break;
	case 5: pC->m_ClYablokoHudVoteX = X; pC->m_ClYablokoHudVoteY = Y; pC->m_ClYablokoHudVoteWidth = W; pC->m_ClYablokoHudVoteHeight = H; break;
	case 6: pC->m_ClYablokoHudChatX = X; pC->m_ClYablokoHudChatY = Y; pC->m_ClYablokoHudChatWidth = W; pC->m_ClYablokoHudChatHeight = H; break;
	case 7: pC->m_ClYablokoHudRadarX = X; pC->m_ClYablokoHudRadarY = Y; pC->m_ClYablokoHudRadarWidth = W; pC->m_ClYablokoHudRadarHeight = H; break;
	case 8: pC->m_ClYablokoHudActiveX = X; pC->m_ClYablokoHudActiveY = Y; pC->m_ClYablokoHudActiveWidth = W; pC->m_ClYablokoHudActiveHeight = H; break;
	default: pC->m_ClYablokoHudInputsX = X; pC->m_ClYablokoHudInputsY = Y; pC->m_ClYablokoHudInputsWidth = W; pC->m_ClYablokoHudInputsHeight = H; break;
	}
}

void CYablokoToolkit::OnRender()
{
	bool InGame = Client()->State() == IClient::STATE_ONLINE || Client()->State() == IClient::STATE_DEMOPLAYBACK;
	if(!InGame)
		return;

	RenderAll();
}

void CYablokoToolkit::RenderAll()
{
	float Time = Client()->GlobalTime();
	(void)Time;

	vec2 ScreenMin, ScreenMax;
	Graphics()->GetScreen(&ScreenMin.x, &ScreenMin.y, &ScreenMax.x, &ScreenMax.y);
	float SW = ScreenMax.x - ScreenMin.x;
	float SH = ScreenMax.y - ScreenMin.y;

	// HUD editor drag logic (bind state or settings checkbox)
	m_HudEditActive = m_HudEditActive || g_Config.m_ClYablokoHudEdit != 0;
	if(m_HudEditActive)
	{
		vec2 Cursor = Ui()->MousePos();
		if(m_HudEditPanel < 0)
		{
			if(Ui()->MouseButtonClicked(0))
			{
				for(int p = 0; p < NUM_PANELS; p++)
				{
					vec4 Rect;
					GetPanelRect(p, Rect);
					CUIRect PanelRect;
					PanelRect.x = ScreenMin.x + SW * Rect.x / 100.0f;
					PanelRect.y = ScreenMin.y + SH * Rect.y / 100.0f;
					PanelRect.w = SW * Rect.z / 100.0f;
					PanelRect.h = SH * Rect.w / 100.0f;
					if(PanelRect.Inside(Cursor))
					{
						m_HudEditPanel = p;
						m_HudEditDragOffset = Cursor - vec2(PanelRect.x, PanelRect.y);
						break;
					}
				}
			}
		}
		else
		{
			if(!Ui()->MouseButton(0))
			{
				m_HudEditPanel = -1;
			}
			else
			{
				vec4 Rect;
				GetPanelRect(m_HudEditPanel, Rect);
				vec2 NewPos = Cursor - m_HudEditDragOffset;
				Rect.x = (NewPos.x - ScreenMin.x) / SW * 100.0f;
				Rect.y = (NewPos.y - ScreenMin.y) / SH * 100.0f;
				SetPanelRect(m_HudEditPanel, Rect);
			}
		}
	}

	// glass destra panels
	if(g_Config.m_ClYablokoDestraHud)
	{
		RenderDestraPanels();
	}

	// server info widget
	if(g_Config.m_ClYablokoServerWidget && !g_Config.m_ClYablokoDestraHud)
	{
		vec4 Rect;
		GetPanelRect(0, Rect);
		CUIRect PanelRect;
		PanelRect.x = ScreenMin.x + SW * Rect.x / 100.0f;
		PanelRect.y = ScreenMin.y + SH * Rect.y / 100.0f;
		PanelRect.w = SW * Rect.z / 100.0f;
		PanelRect.h = SH * Rect.w / 100.0f;
		RenderDestraPanel(PanelRect, 0.6f);
		RenderTopPanel(PanelRect);
	}

	// hook duration bar
	RenderHookDurationBar();

	// radar
	if(g_Config.m_ClYablokoRadarHud && !g_Config.m_ClYablokoDestraHud)
		RenderRadar();

	// active features list
	if(g_Config.m_ClYablokoActiveFeatures && !g_Config.m_ClYablokoDestraHud)
		RenderActiveFeatures();

	// path prediction
	if(g_Config.m_ClYablokoPathPrediction)
		RenderPathPrediction();

	// partner input overlay
	if(g_Config.m_ClYablokoInputOverlay)
		RenderInputOverlay();

	// back button
	RenderBackButton();

	// notifications
	RenderNotifications();
}

void CYablokoToolkit::RenderNotifications()
{
	float Time = Client()->GlobalTime();
	const float Duration = 4.0f;
	const float SlideIn = 0.2f;

	vec2 ScreenMin, ScreenMax;
	Graphics()->GetScreen(&ScreenMin.x, &ScreenMin.y, &ScreenMax.x, &ScreenMax.y);

	float y = ScreenMin.y + 20.0f;
	float x = ScreenMax.x - 240.0f;

	// remove expired
	m_vNotifications.erase(std::remove_if(m_vNotifications.begin(), m_vNotifications.end(),
				   [&](const SNotification &N) { return Time - N.m_Time > Duration; }),
		m_vNotifications.end());

	for(const auto &N : m_vNotifications)
	{
		float Age = Time - N.m_Time;
		float Alpha = 1.0f;
		if(Age < SlideIn)
			Alpha = Age / SlideIn;
		if(Age > Duration - 0.5f)
			Alpha = std::clamp((Duration - Age) / 0.5f, 0.0f, 1.0f);

		float Slide = (1.0f - std::clamp(Age / SlideIn, 0.0f, 1.0f)) * 60.0f;

		ColorRGBA Accent = Yabloko::ThemeAccent(g_Config.m_ClYablokoTheme, g_Config.m_ClYablokoThemeAccent);
		CUIRect Rect;
		Rect.x = x + Slide;
		Rect.y = y;
		Rect.w = 220.0f;
		Rect.h = 20.0f;

		Rect.Draw(ColorRGBA(0.05f, 0.07f, 0.05f, 0.85f * Alpha), IGraphics::CORNER_ALL, 6.0f);
		Rect.x += 2; Rect.y += 2; Rect.w -= 4; Rect.h -= 4;
		Rect.Draw(ColorRGBA(Accent.r, Accent.g, Accent.b, 0.25f * Alpha), IGraphics::CORNER_ALL, 5.0f);

		float tw = TextRender()->TextWidth(14.0f, N.m_Text.c_str(), -1, -1);
		CUIRect Label = Rect;
		Label.x += std::clamp((Label.w - tw) / 2, 4.0f, Label.w - 4.0f);
		TextRender()->TextColor(1.0f, 1.0f, 1.0f, Alpha);
		Ui()->DoLabel(&Label, N.m_Text.c_str(), 14.0f, TEXTALIGN_ML);
		TextRender()->TextColor(1.0f, 1.0f, 1.0f, 1.0f);
		y += Rect.h + 4.0f;
	}
}

void CYablokoToolkit::RenderHookDurationBar()
{
	if(!g_Config.m_ClHookDurationBar)
		return;
	if(Client()->State() != IClient::STATE_ONLINE)
		return;

	for(int i = 0; i < MAX_CLIENTS; i++)
	{
		if(!GameClient()->m_Snap.m_aCharacters[i].m_Active || i == GameClient()->m_Snap.m_LocalClientId)
			continue;
		const CNetObj_Character *pChar = &GameClient()->m_Snap.m_aCharacters[i].m_Cur;
		if(pChar->m_HookState <= 0)
			continue;

		const float HookDurationTicks = 1.25f * Client()->GameTickSpeed(); // server tune HookDuration (1.25 s)
		float Progress = std::clamp((Client()->GameTick(g_Config.m_ClDummy) - pChar->m_HookTick) / HookDurationTicks, 0.0f, 1.0f);

		vec2 Pos = GameClient()->m_aClients[i].m_RenderPos;
		float Width = 40.0f;
		vec2 ScreenMin, ScreenMax;
		Graphics()->GetScreen(&ScreenMin.x, &ScreenMin.y, &ScreenMax.x, &ScreenMax.y);
		(void)ScreenMin;
		(void)ScreenMax;

		ColorRGBA Accent = Yabloko::ThemeAccent(g_Config.m_ClYablokoTheme, g_Config.m_ClYablokoThemeAccent);
		CUIRect Bar;
		Bar.x = Pos.x - Width / 2;
		Bar.y = Pos.y + 24.0f;
		Bar.w = Width;
		Bar.h = 4.0f;
		Bar.Draw(ColorRGBA(0.0f, 0.0f, 0.0f, 0.5f), IGraphics::CORNER_ALL, 2.0f);
		CUIRect Fill = Bar;
		Fill.w *= (1.0f - Progress);
		Fill.Draw(ColorRGBA(Accent.r, Accent.g, Accent.b, 0.9f), IGraphics::CORNER_ALL, 2.0f);
	}
}

void CYablokoToolkit::RenderBackButton()
{
	if(!g_Config.m_ClBackButton)
		return;

	vec2 ScreenMin, ScreenMax;
	Graphics()->GetScreen(&ScreenMin.x, &ScreenMin.y, &ScreenMax.x, &ScreenMax.y);
	float SW = ScreenMax.x - ScreenMin.x;
	float SH = ScreenMax.y - ScreenMin.y;

	vec2 Pos = vec2(ScreenMin.x + SW * g_Config.m_ClBackButtonX / 100.0f,
		ScreenMin.y + SH * g_Config.m_ClBackButtonY / 100.0f);
	float Size = 28.0f;

	vec2 Cursor = Ui()->MousePos();
	bool Hover = distance(Cursor, Pos) < Size;
	bool Pressed = Hover && Ui()->MouseButtonClicked(0);
	if(Pressed)
	{
		m_BackButtonPress = Client()->GlobalTime() + 0.15f;
		// act as ESC/back (works in menus, chat, game)
		Ui()->SetHotkey(CUi::HOTKEY_ESCAPE);
	}

	ColorRGBA Accent = Yabloko::ThemeAccent(g_Config.m_ClYablokoTheme, g_Config.m_ClYablokoThemeAccent);
	float Alpha = Hover ? 0.95f : 0.6f;
	Graphics()->SetColor(Accent.r, Accent.g, Accent.b, Alpha * 0.25f);
	Graphics()->DrawCircle(Pos.x, Pos.y, Size, 16);
	Graphics()->SetColor(Accent.r, Accent.g, Accent.b, Alpha * 0.5f);
	Graphics()->DrawCircle(Pos.x, Pos.y, Size - 6.0f, 16);

	// arrow
	Graphics()->SetColor(1.0f, 1.0f, 1.0f, Alpha);
	Graphics()->QuadsBegin();
	IGraphics::CFreeformItem Arrow(Pos.x - 8.0f, Pos.y, Pos.x + 2.0f, Pos.y, Pos.x - 2.0f, Pos.y - 7.0f, Pos.x - 2.0f, Pos.y + 7.0f);
	Graphics()->QuadsDrawFreeform(&Arrow, 1);
	Graphics()->QuadsEnd();
}

void CYablokoToolkit::RenderDestraPanel(CUIRect Rect, float Alpha)
{
	ColorRGBA Accent = Yabloko::ThemeAccent(g_Config.m_ClYablokoTheme, g_Config.m_ClYablokoThemeAccent);
	ColorRGBA Glass = Yabloko::Col(g_Config.m_ClYablokoGlassColor, 0.35f * Alpha * (g_Config.m_ClYablokoDestraOpacity / 100.0f));
	if(g_Config.m_ClYablokoGlassColor <= 0)
		Glass = ColorRGBA(0.08f, 0.10f, 0.08f, 0.55f * Alpha * (g_Config.m_ClYablokoDestraOpacity / 100.0f));

	Rect.Draw(Glass, IGraphics::CORNER_ALL, 8.0f);
	Rect.Margin(1.0f, &Rect);
	Rect.Draw(ColorRGBA(Accent.r, Accent.g, Accent.b, 0.10f * Alpha), IGraphics::CORNER_ALL, 7.0f);
}

void CYablokoToolkit::RenderDestraPanels()
{
	vec2 ScreenMin, ScreenMax;
	Graphics()->GetScreen(&ScreenMin.x, &ScreenMin.y, &ScreenMax.x, &ScreenMax.y);
	float SW = ScreenMax.x - ScreenMin.x;
	float SH = ScreenMax.y - ScreenMin.y;
	float Scale = g_Config.m_ClYablokoDestraScale / 100.0f;

	auto PanelRect = [&](int Panel) {
		vec4 Rect;
		GetPanelRect(Panel, Rect);
		CUIRect R;
		R.x = ScreenMin.x + SW * Rect.x / 100.0f;
		R.y = ScreenMin.y + SH * Rect.y / 100.0f;
		R.w = SW * Rect.z / 100.0f;
		R.h = SH * Rect.w / 100.0f;
		return R;
	};

	if(g_Config.m_ClYablokoDestraTop)
	{
		CUIRect R = PanelRect(0);
		RenderDestraPanel(R, 0.8f);
		RenderTopPanel(R);
	}
	if(g_Config.m_ClYablokoDestraPlayers)
	{
		CUIRect R = PanelRect(2);
		RenderDestraPanel(R, 0.8f);
		RenderPlayersPanel(R);
	}
	if(g_Config.m_ClYablokoDestraResults)
	{
		CUIRect R = PanelRect(3);
		RenderDestraPanel(R, 0.8f);
		RenderResultsPanel(R);
	}
	if(g_Config.m_ClYablokoDestraVote)
	{
		CUIRect R = PanelRect(5);
		RenderDestraPanel(R, 0.8f);
		RenderVotePanel(R);
	}
	if(g_Config.m_ClYablokoDestraChat)
	{
		CUIRect R = PanelRect(6);
		RenderDestraPanel(R, 0.5f);
		RenderChatPanel(R);
	}
	if(g_Config.m_ClYablokoDestraMusic)
	{
		CUIRect R = PanelRect(4);
		RenderDestraPanel(R, 0.8f);
		RenderMusicPanel(R);
	}
	(void)Scale;
}

void CYablokoToolkit::RenderTopPanel(CUIRect Rect)
{
	CServerInfo Info;
	Client()->GetServerInfo(&Info);
	char aBuf[256];
	if(Info.m_aMap[0])
		str_format(aBuf, sizeof(aBuf), "%s | %d ms | %d fps", Info.m_aMap, Info.m_Latency, (int)std::round(1.0f / Client()->FrameTimeAverage()));
	else
		str_format(aBuf, sizeof(aBuf), "%d fps", (int)std::round(1.0f / Client()->FrameTimeAverage()));

	float FontSize = 12.0f * (g_Config.m_ClYablokoDestraFont > 0 ? g_Config.m_ClYablokoDestraFont / 100.0f : 1.0f);
	float tw = TextRender()->TextWidth(FontSize, aBuf, -1, -1);
	CUIRect Label = Rect;
	Label.x += std::clamp((Label.w - tw) / 2, 4.0f, Label.w - 4.0f);
	Ui()->DoLabel(&Label, aBuf, FontSize, TEXTALIGN_ML);
}

void CYablokoToolkit::RenderPlayersPanel(CUIRect Rect)
{
	if(Client()->State() != IClient::STATE_ONLINE)
		return;
	float FontSize = 10.0f * (g_Config.m_ClYablokoDestraFont > 0 ? g_Config.m_ClYablokoDestraFont / 100.0f : 1.0f);
	float RowH = 14.0f;
	int Rows = 0;
	int Limit = g_Config.m_ClYablokoDestraPlayerLimit > 0 ? g_Config.m_ClYablokoDestraPlayerLimit : 8;
	ColorRGBA Accent = Yabloko::ThemeAccent(g_Config.m_ClYablokoTheme, g_Config.m_ClYablokoThemeAccent);

	for(int i = 0; i < MAX_CLIENTS && Rows < Limit; i++)
	{
		const CNetObj_PlayerInfo *pInfo = GameClient()->m_Snap.m_apInfoByDDTeamScore[i];
		if(!pInfo || !GameClient()->m_Snap.m_aCharacters[i].m_Active)
			continue;
		CUIRect Row = Rect;
		Row.y += Rows * RowH + 2.0f;
		Row.h = RowH;
		Row.x += 4.0f;
		Row.w -= 8.0f;
		bool Local = pInfo->m_ClientId == GameClient()->m_Snap.m_LocalClientId;
		ColorRGBA Color = Local ? Accent : ColorRGBA(1.0f, 1.0f, 1.0f, 0.85f);
		TextRender()->TextColor(Color);
		Ui()->DoLabel(&Row, GameClient()->m_aClients[i].m_aName, FontSize, TEXTALIGN_ML);
		TextRender()->TextColor(1.0f, 1.0f, 1.0f, 1.0f);
		Rows++;
	}
}

void CYablokoToolkit::RenderResultsPanel(CUIRect Rect)
{
	if(!GameClient()->m_Snap.m_pGameInfoObj)
		return;
	float FontSize = 10.0f * (g_Config.m_ClYablokoDestraFont > 0 ? g_Config.m_ClYablokoDestraFont / 100.0f : 1.0f);
	char aBuf[128];
	int Score = GameClient()->m_Snap.m_LocalClientId >= 0 && GameClient()->m_Snap.m_apInfoByDDTeamScore[GameClient()->m_Snap.m_LocalClientId] ? GameClient()->m_Snap.m_apInfoByDDTeamScore[GameClient()->m_Snap.m_LocalClientId]->m_Score : 0;
	str_format(aBuf, sizeof(aBuf), "Score: %d", Score);
	CUIRect Label = Rect;
	Label.x += 6.0f;
	Ui()->DoLabel(&Label, aBuf, FontSize, TEXTALIGN_ML);
}

void CYablokoToolkit::RenderVotePanel(CUIRect Rect)
{
	if(!GameClient()->m_Voting.IsVoting())
		return;
	float FontSize = 10.0f * (g_Config.m_ClYablokoDestraFont > 0 ? g_Config.m_ClYablokoDestraFont / 100.0f : 1.0f);
	CUIRect Label = Rect;
	Label.x += 6.0f;
	Label.y += 2.0f;
	Label.w -= 12.0f;
	const char *pDesc = GameClient()->m_Voting.VoteDescription();
	if(pDesc && pDesc[0])
		Ui()->DoLabel(&Label, pDesc, FontSize, TEXTALIGN_ML);
}

void CYablokoToolkit::RenderChatPanel(CUIRect Rect)
{
	// dark styled chat backdrop (chat itself renders below normally)
	(void)Rect;
}

void CYablokoToolkit::RenderMusicPanel(CUIRect Rect)
{
	float FontSize = 10.0f;
	CUIRect Label = Rect;
	Label.x += 6.0f;
	Label.y += 2.0f;
	Ui()->DoLabel(&Label, "♪ media", FontSize, TEXTALIGN_ML);
}

void CYablokoToolkit::RenderActiveFeatures()
{
	vec4 Rect;
	GetPanelRect(8, Rect);
	vec2 ScreenMin, ScreenMax;
	Graphics()->GetScreen(&ScreenMin.x, &ScreenMin.y, &ScreenMax.x, &ScreenMax.y);
	float SW = ScreenMax.x - ScreenMin.x;
	float SH = ScreenMax.y - ScreenMin.y;
	CUIRect PanelRect;
	PanelRect.x = ScreenMin.x + SW * Rect.x / 100.0f;
	PanelRect.y = ScreenMin.y + SH * Rect.y / 100.0f;
	PanelRect.w = SW * Rect.z / 100.0f;
	PanelRect.h = SH * Rect.w / 100.0f;

	float FontSize = g_Config.m_ClYablokoActiveFont > 0 ? g_Config.m_ClYablokoActiveFont : 10.0f;
	float RowH = FontSize + 2.0f;
	int Rows = 0;
	int MaxRows = g_Config.m_ClYablokoActiveRows > 0 ? g_Config.m_ClYablokoActiveRows : 6;

	ColorRGBA Accent = Yabloko::ThemeAccent(g_Config.m_ClYablokoTheme, g_Config.m_ClYablokoThemeAccent);
	TextRender()->TextColor(Accent.r, Accent.g, Accent.b, 0.9f);

	const char *apFeatures[32];
	int NumFeatures = 0;
	if(g_Config.m_ClSnow) apFeatures[NumFeatures++] = "snow";
	if(g_Config.m_ClTentacle) apFeatures[NumFeatures++] = "tentacle";
	if(g_Config.m_ClOrbs) apFeatures[NumFeatures++] = "orbs";
	if(g_Config.m_ClJellyEnabled) apFeatures[NumFeatures++] = "jelly";
	if(g_Config.m_ClYablokoTeeGlow) apFeatures[NumFeatures++] = "tee-glow";
	if(g_Config.m_ClYablokoBodyGlow) apFeatures[NumFeatures++] = "body-glow";
	if(g_Config.m_ClYablokoHat) apFeatures[NumFeatures++] = "hat";
	if(g_Config.m_ClYablokoEyeLaser) apFeatures[NumFeatures++] = "eye-laser";
	if(g_Config.m_ClYablokoRisingSnow) apFeatures[NumFeatures++] = "rising-snow";
	if(g_Config.m_ClYablokoGhostTrail) apFeatures[NumFeatures++] = "ghost-trail";
	if(g_Config.m_ClYablokoGrenadeTracer) apFeatures[NumFeatures++] = "grenade-trail";
	if(g_Config.m_ClYablokoTeslaHook) apFeatures[NumFeatures++] = "tesla";
	if(g_Config.m_ClYablokoPlasma) apFeatures[NumFeatures++] = "plasma";
	if(g_Config.m_ClYablokoAfterburner) apFeatures[NumFeatures++] = "afterburner";
	if(g_Config.m_ClYablokoGrid) apFeatures[NumFeatures++] = "grid";
	if(g_Config.m_ClNotify) apFeatures[NumFeatures++] = "notify";
	if(g_Config.m_ClYablokoAutopilot) apFeatures[NumFeatures++] = "autopilot";
	if(g_Config.m_ClZzAimbotEnabled) apFeatures[NumFeatures++] = "aimbot";
	if(NumFeatures > (int)std::size(apFeatures))
		NumFeatures = std::size(apFeatures);

	for(int i = 0; i < NumFeatures && Rows < MaxRows; i++, Rows++)
	{
		CUIRect Row = PanelRect;
		Row.y += Rows * RowH;
		Row.h = RowH;
		Ui()->DoLabel(&Row, apFeatures[i], FontSize, TEXTALIGN_ML);
	}
	TextRender()->TextColor(1.0f, 1.0f, 1.0f, 1.0f);
}

void CYablokoToolkit::RenderRadar()
{
	vec4 Rect;
	GetPanelRect(7, Rect);
	vec2 ScreenMin, ScreenMax;
	Graphics()->GetScreen(&ScreenMin.x, &ScreenMin.y, &ScreenMax.x, &ScreenMax.y);
	float SW = ScreenMax.x - ScreenMin.x;
	float SH = ScreenMax.y - ScreenMin.y;
	CUIRect PanelRect;
	PanelRect.x = ScreenMin.x + SW * Rect.x / 100.0f;
	PanelRect.y = ScreenMin.y + SH * Rect.y / 100.0f;
	PanelRect.w = SW * Rect.z / 100.0f;
	PanelRect.h = SH * Rect.w / 100.0f;

	RenderDestraPanel(PanelRect, 0.7f);

	float Zoom = g_Config.m_ClYablokoRadarZoom / 100.0f;
	if(Zoom <= 0.01f)
		Zoom = 1.0f;
	float TileSize = PanelRect.w / (20.0f / Zoom); // 20 tiles wide view

	vec2 LocalPos = GameClient()->m_LocalCharacterPos;
	if(g_Config.m_ClYablokoRadarFollow && g_Config.m_ClYablokoRadarSelected >= 0 &&
		GameClient()->m_Snap.m_aCharacters[g_Config.m_ClYablokoRadarSelected].m_Active)
		LocalPos = GameClient()->m_aClients[g_Config.m_ClYablokoRadarSelected].m_RenderPos;

	vec2 Center = vec2(PanelRect.x + PanelRect.w / 2, PanelRect.y + PanelRect.h / 2);
	ColorRGBA Accent = Yabloko::ThemeAccent(g_Config.m_ClYablokoTheme, g_Config.m_ClYablokoThemeAccent);

	for(int i = 0; i < MAX_CLIENTS; i++)
	{
		if(!GameClient()->m_Snap.m_aCharacters[i].m_Active)
			continue;
		vec2 P = GameClient()->m_aClients[i].m_RenderPos;
		vec2 Rel = (P - LocalPos) / 32.0f * TileSize;
		vec2 Dot = Center + Rel;
		if(Dot.x < PanelRect.x || Dot.x > PanelRect.x + PanelRect.w ||
			Dot.y < PanelRect.y || Dot.y > PanelRect.y + PanelRect.h)
			continue;
		bool Local = i == GameClient()->m_Snap.m_LocalClientId;
		Graphics()->SetColor(Local ? Accent.r : 1.0f, Local ? Accent.g : 1.0f, Local ? Accent.b : 1.0f, 0.9f);
		Graphics()->DrawCircle(Dot.x, Dot.y, Local ? 3.0f : 2.5f, 8);
	}
}

void CYablokoToolkit::RenderPathPrediction()
{
	if(Client()->State() != IClient::STATE_ONLINE)
		return;
	if(!GameClient()->m_Snap.m_pLocalCharacter)
		return;

	vec2 Pos = GameClient()->m_LocalCharacterPos;
	vec2 Vel = vec2(GameClient()->m_Snap.m_pLocalCharacter->m_VelX / 256.0f, GameClient()->m_Snap.m_pLocalCharacter->m_VelY / 256.0f);
	float Seconds = std::clamp(g_Config.m_ClYablokoPathSeconds / 10.0f, 0.5f, 5.0f);
	float dt = 1.0f / Client()->GameTickSpeed();

	ColorRGBA Color = Yabloko::Col(g_Config.m_ClYablokoPathColor, 0.6f);
	if(g_Config.m_ClYablokoPathColor <= 0)
		Color = ColorRGBA(1.0f, 1.0f, 1.0f, 0.5f);

	Graphics()->SetColor(Color.r, Color.g, Color.b, Color.a);
	float Radius = std::clamp(g_Config.m_ClYablokoPathDensity / 100.0f, 0.3f, 3.0f);
	for(float t = 0; t < Seconds; t += dt)
	{
		// simplified kinematics with DDNet gravity
		Vel.y += 700.0f * dt; // gravity
		Pos += Vel * 32.0f * dt;
		Graphics()->DrawCircle(Pos.x, Pos.y, Radius, 6);
	}
}

void CYablokoToolkit::RenderInputOverlay()
{
	int Target = g_Config.m_ClYablokoInputTarget;
	int Id = GameClient()->m_Snap.m_LocalClientId;
	if(Target == 0 && g_Config.m_ClYablokoPartnerId >= 0 && g_Config.m_ClYablokoPartnerId < MAX_CLIENTS)
		Id = g_Config.m_ClYablokoPartnerId;
	if(Id < 0 || Id >= MAX_CLIENTS || !GameClient()->m_Snap.m_aCharacters[Id].m_Active)
		return;

	const CNetObj_Character *pChar = &GameClient()->m_Snap.m_aCharacters[Id].m_Cur;

	vec2 ScreenMin, ScreenMax;
	Graphics()->GetScreen(&ScreenMin.x, &ScreenMin.y, &ScreenMax.x, &ScreenMax.y);
	float x = ScreenMin.x + 60.0f;
	float y = ScreenMax.y - 50.0f;

	ColorRGBA Accent = Yabloko::ThemeAccent(g_Config.m_ClYablokoTheme, g_Config.m_ClYablokoThemeAccent);
	auto Dot = [&](bool Active, vec2 P) {
		if(Active)
		{
			Graphics()->SetColor(Accent.r, Accent.g, Accent.b, 0.9f);
			Graphics()->DrawCircle(P.x, P.y, 6.0f, 10);
		}
		else
		{
			Graphics()->SetColor(1.0f, 1.0f, 1.0f, 0.2f);
			Graphics()->DrawCircle(P.x, P.y, 5.0f, 10);
		}
	};

	Dot(pChar->m_Direction < 0, vec2(x - 14.0f, y));
	Dot(pChar->m_Direction > 0, vec2(x + 14.0f, y));
	Dot((pChar->m_Jumped & 1) != 0, vec2(x, y - 14.0f));
	Dot(pChar->m_HookState > 0, vec2(x, y + 14.0f));
}
