/* Yabloko client — reconstructed from the surviving build.
   Extras implementation. */

#include "yabloko_extras.h"

#include <engine/shared/config.h>
#include <engine/sound.h>

#include <generated/protocol.h>

#include <game/client/gameclient.h>

void CYablokoExtras::OnReset()
{
	m_EmoteSpamActive = false;
	m_NextEmoteTime = 0.0f;
	m_LastEmote = -1;
	m_EmoteSpamPrev = false;
}

void CYablokoExtras::OnConsoleInit()
{
	Console()->Register("yabloko_chat_search", "s[text]", CFGFLAG_CLIENT, ConChatSearch, this, "Search the chat (empty text resets)");
	Console()->Register("yabloko_random_copy", "", CFGFLAG_CLIENT, ConRandomCopy, this, "Copy a random player's skin settings");
	Console()->Register("yabloko_reset_hud", "", CFGFLAG_CLIENT, ConResetHud, this, "Reset the HUD editor panel layout");
	Console()->Register("yabloko_effects_settings", "", CFGFLAG_CLIENT, ConEffectsSettings, this, "Open Yabloko world effect settings");
	Console()->Register("krx_identity_save", "", CFGFLAG_CLIENT, ConKrxSaveIdentity, this, "Save your current identity");
	Console()->Register("krx_identity_load", "", CFGFLAG_CLIENT, ConKrxLoadIdentity, this, "Load your saved identity");
	
	Console()->Register("+yabloko_emote_spam", "i[stroke]", CFGFLAG_CLIENT, ConStateEmoteSpam, this, "Cycle selected emotes while enabled by a bind");
	Console()->Register("-yabloko_emote_spam", "i[stroke]", CFGFLAG_CLIENT, ConStateEmoteSpam, this, "");
	Console()->Register("+yabloko_eye_laser", "i[stroke]", CFGFLAG_CLIENT, ConStateEyeLaser, this, "Cosmetic eye beam bind");
	Console()->Register("-yabloko_eye_laser", "i[stroke]", CFGFLAG_CLIENT, ConStateEyeLaser, this, "");
	Console()->Register("+yabloko_edit_hud", "i[stroke]", CFGFLAG_CLIENT, ConStateEditHud, this, "Edit HUD layout directly in game");
	Console()->Register("-yabloko_edit_hud", "i[stroke]", CFGFLAG_CLIENT, ConStateEditHud, this, "");
}

void CYablokoExtras::ConChatSearch(IConsole::IResult *pResult, void *pUserData)
{
	(void)pUserData;
	const char *pText = pResult->GetString(0);
	str_copy(((CYablokoExtras *)pUserData)->m_aChatSearch, pText, sizeof(((CYablokoExtras *)pUserData)->m_aChatSearch));
}

void CYablokoExtras::ConRandomCopy(IConsole::IResult *pResult, void *pUserData)
{
	(void)pResult;
	CYablokoExtras *pSelf = (CYablokoExtras *)pUserData;
	// copy the skin of a random active player
	int NumPlayers = 0;
	int Pick = (int)(random_float() * MAX_CLIENTS);
	for(int i = 0; i < MAX_CLIENTS; i++)
	{
		if(i == Pick)
		{
			str_copy(g_Config.m_ClPlayerSkin, pSelf->GameClient()->m_aClients[i].m_aSkinName, sizeof(g_Config.m_ClPlayerSkin));
			char aBuf[128];
			str_format(aBuf, sizeof(aBuf), "Copied skin from %s", pSelf->GameClient()->m_aClients[i].m_aName);
			pSelf->GameClient()->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "yabloko", aBuf);
			return;
		}
		if(pSelf->GameClient()->m_aClients[i].m_Active)
			NumPlayers++;
	}
	(void)NumPlayers;
}

void CYablokoExtras::ConResetHud(IConsole::IResult *pResult, void *pUserData)
{
	(void)pResult;
	(void)pUserData;
	static const int Defaults[][4] = {
		{50, 3, 40, 6}, // server
		{50, 3, 40, 6}, // top
		{2, 10, 22, 40}, // players
		{2, 52, 22, 12}, // results
		{76, 52, 22, 12}, // music
		{76, 66, 22, 20}, // vote
		{2, 66, 40, 30}, // chat
		{76, 12, 22, 36}, // radar
		{2, 92, 30, 6}, // active
		{44, 92, 12, 6}, // inputs
	};
	g_Config.m_ClYablokoHudServerX = Defaults[0][0]; g_Config.m_ClYablokoHudServerY = Defaults[0][1]; g_Config.m_ClYablokoHudServerWidth = Defaults[0][2]; g_Config.m_ClYablokoHudServerHeight = Defaults[0][3];
	g_Config.m_ClYablokoHudTopX = Defaults[1][0]; g_Config.m_ClYablokoHudTopY = Defaults[1][1]; g_Config.m_ClYablokoHudTopWidth = Defaults[1][2]; g_Config.m_ClYablokoHudTopHeight = Defaults[1][3];
	g_Config.m_ClYablokoHudPlayersX = Defaults[2][0]; g_Config.m_ClYablokoHudPlayersY = Defaults[2][1]; g_Config.m_ClYablokoHudPlayersWidth = Defaults[2][2]; g_Config.m_ClYablokoHudPlayersHeight = Defaults[2][3];
	g_Config.m_ClYablokoHudResultsX = Defaults[3][0]; g_Config.m_ClYablokoHudResultsY = Defaults[3][1]; g_Config.m_ClYablokoHudResultsWidth = Defaults[3][2]; g_Config.m_ClYablokoHudResultsHeight = Defaults[3][3];
	g_Config.m_ClYablokoHudMusicX = Defaults[4][0]; g_Config.m_ClYablokoHudMusicY = Defaults[4][1]; g_Config.m_ClYablokoHudMusicWidth = Defaults[4][2]; g_Config.m_ClYablokoHudMusicHeight = Defaults[4][3];
	g_Config.m_ClYablokoHudVoteX = Defaults[5][0]; g_Config.m_ClYablokoHudVoteY = Defaults[5][1]; g_Config.m_ClYablokoHudVoteWidth = Defaults[5][2]; g_Config.m_ClYablokoHudVoteHeight = Defaults[5][3];
	g_Config.m_ClYablokoHudChatX = Defaults[6][0]; g_Config.m_ClYablokoHudChatY = Defaults[6][1]; g_Config.m_ClYablokoHudChatWidth = Defaults[6][2]; g_Config.m_ClYablokoHudChatHeight = Defaults[6][3];
	g_Config.m_ClYablokoHudRadarX = Defaults[7][0]; g_Config.m_ClYablokoHudRadarY = Defaults[7][1]; g_Config.m_ClYablokoHudRadarWidth = Defaults[7][2]; g_Config.m_ClYablokoHudRadarHeight = Defaults[7][3];
	g_Config.m_ClYablokoHudActiveX = Defaults[8][0]; g_Config.m_ClYablokoHudActiveY = Defaults[8][1]; g_Config.m_ClYablokoHudActiveWidth = Defaults[8][2]; g_Config.m_ClYablokoHudActiveHeight = Defaults[8][3];
	g_Config.m_ClYablokoHudInputsX = Defaults[9][0]; g_Config.m_ClYablokoHudInputsY = Defaults[9][1]; g_Config.m_ClYablokoHudInputsWidth = Defaults[9][2]; g_Config.m_ClYablokoHudInputsHeight = Defaults[9][3];
}

void CYablokoExtras::ConEffectsSettings(IConsole::IResult *pResult, void *pUserData)
{
	(void)pResult;
	(void)pUserData;
	g_Config.m_UiSettingsPage = 9; // SETTINGS_YABLOKO
}

void CYablokoExtras::ConKrxSaveIdentity(IConsole::IResult *pResult, void *pUserData)
{
	(void)pResult;
	(void)pUserData;
	g_Config.m_KrxSavedIdentity = 1;
	str_copy(g_Config.m_KrxSavedNam, g_Config.m_PlayerName, sizeof(g_Config.m_KrxSavedNam));
	str_copy(g_Config.m_KrxSavedClan, g_Config.m_PlayerClan, sizeof(g_Config.m_KrxSavedClan));
	str_copy(g_Config.m_KrxSavedSkin, g_Config.m_ClPlayerSkin, sizeof(g_Config.m_KrxSavedSkin));
	g_Config.m_KrxSavedUseCustomColor = g_Config.m_ClPlayerUseCustomColor;
	g_Config.m_KrxSavedColorBody = g_Config.m_ClPlayerColorBody;
	g_Config.m_KrxSavedColorFeet = g_Config.m_ClPlayerColorFeet;
	static_cast<CYablokoExtras *>(pUserData)->GameClient()->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "yabloko", "Identity saved");
}

void CYablokoExtras::ConKrxLoadIdentity(IConsole::IResult *pResult, void *pUserData)
{
	(void)pResult;
	(void)pUserData;
	if(!g_Config.m_KrxSavedIdentity)
		return;
	str_copy(g_Config.m_PlayerName, g_Config.m_KrxSavedNam, sizeof(g_Config.m_PlayerName));
	str_copy(g_Config.m_PlayerClan, g_Config.m_KrxSavedClan, sizeof(g_Config.m_PlayerClan));
	str_copy(g_Config.m_ClPlayerSkin, g_Config.m_KrxSavedSkin, sizeof(g_Config.m_ClPlayerSkin));
	g_Config.m_ClPlayerUseCustomColor = g_Config.m_KrxSavedUseCustomColor;
	g_Config.m_ClPlayerColorBody = g_Config.m_KrxSavedColorBody;
	g_Config.m_ClPlayerColorFeet = g_Config.m_KrxSavedColorFeet;
	static_cast<CYablokoExtras *>(pUserData)->GameClient()->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "yabloko", "Identity loaded");
}

void CYablokoExtras::ConStateEmoteSpam(IConsole::IResult *pResult, void *pUserData)
{
	((CYablokoExtras *)pUserData)->m_EmoteSpamActive = pResult->GetInteger(0) != 0;
}

void CYablokoExtras::ConStateEyeLaser(IConsole::IResult *pResult, void *pUserData)
{
	((CYablokoExtras *)pUserData)->m_EyeLaserBindActive = pResult->GetInteger(0) != 0;
}

void CYablokoExtras::ConStateEditHud(IConsole::IResult *pResult, void *pUserData)
{
	CYablokoExtras *pSelf = (CYablokoExtras *)pUserData;
	if(pResult->GetInteger(0))
		pSelf->GameClient()->m_YablokoToolkit.StartHudEdit();
	else
		pSelf->GameClient()->m_YablokoToolkit.StopHudEdit();
}

ColorRGBA CYablokoExtras::HookColor() const
{
	if(g_Config.m_KrxCustomcolorhookrainbow)
	{
		float Speed = g_Config.m_KrxCustomcolorrainbowspeed / 100.0f;
		float Phase = std::fmod(Client()->GlobalTime() * Speed, 1.0f);
		return Yabloko::Rainbow(Phase, 1.0f);
	}
	return Yabloko::Col(g_Config.m_KrxCustomcolorhookcol);
}

void CYablokoExtras::OnUpdate()
{
	// emote spam bind
	bool Active = false;
	// the bind state is tracked through chat-level state: emote spam toggled via +yabloko_emote_spam
	if(g_Config.m_ClYablokoEmoteSpam && m_EmoteSpamActive)
		Active = true;

	float Time = Client()->GlobalTime();
	if(Active && Time >= m_NextEmoteTime && GameClient()->m_Snap.m_pLocalCharacter)
	{
		m_NextEmoteTime = Time + g_Config.m_ClYablokoEmoteDelay / 1000.0f;
		// pick next enabled emote from the mask
		int Mask = g_Config.m_ClYablokoEmoteMask;
		if(Mask == 0)
			Mask = 0x1F;
		int Emote = -1;
		for(int Attempt = 0; Attempt < NUM_EMOTICONS; Attempt++)
		{
			int Candidate = (m_LastEmote + 1 + Attempt) % NUM_EMOTICONS;
			if(Mask & (1 << Candidate))
			{
				Emote = Candidate;
				break;
			}
		}
		if(Emote >= 0)
		{
			m_LastEmote = Emote;
			CNetMsg_Cl_Emoticon Msg;
			Msg.m_Emoticon = Emote;
			Client()->SendPackMsgActive(&Msg, MSGFLAG_VITAL);
		}
	}
}
