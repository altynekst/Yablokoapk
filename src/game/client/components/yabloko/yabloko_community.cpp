/* Yabloko client — reconstructed from the surviving build.
   Community implementation. */

#include "yabloko_community.h"

#include <engine/console.h>
#include <engine/shared/config.h>
#include <engine/storage.h>

#include <game/client/gameclient.h>

#include <algorithm>
#include <cstdio>

void CYablokoCommunity::OnReset()
{
	m_Users.clear();
	m_NextWhisperTime = 0.0f;
	m_LocalPrevTeam = -1;
}

void CYablokoCommunity::OnConsoleInit()
{
	Console()->Register("yabloko_send_ads", "", CFGFLAG_CLIENT, ConSendAds, this, "Send the advertising text to the current chat");
	Console()->Register("yabloko_users_settings", "", CFGFLAG_CLIENT, ConUsersSettings, this, "Open Yabloko user highlighting and whisper settings");

	LoadLibrary();
}

void CYablokoCommunity::ConUsersSettings(IConsole::IResult *pResult, void *pUserData)
{
	(void)pResult;
	(void)pUserData;
	g_Config.m_UiSettingsPage = 9; // SETTINGS_YABLOKO
	g_Config.m_UiPage = CMenus::PAGE_SETTINGS;
}

void CYablokoCommunity::ConSendAds(IConsole::IResult *pResult, void *pUserData)
{
	(void)pResult;
	((CYablokoCommunity *)pUserData)->SendAds();
}

void CYablokoCommunity::LoadLibrary()
{
	char aBuf[IO_MAX_PATH_LENGTH];
	void *pData = nullptr;
	unsigned Size = 0;
	if(!Storage()->ReadFile("yabloko/library.json", IStorage::TYPE_SAVE, &pData, &Size))
		return;
	const char *pCursor = (const char *)pData;
	const char *pEnd = pCursor + Size;
	while(pCursor < pEnd)
	{
		const char *pLineEnd = (const char *)memchr(pCursor, '\n', pEnd - pCursor);
		if(!pLineEnd)
			pLineEnd = pEnd;
		char aLine[512];
		size_t Len = std::min<size_t>(pLineEnd - pCursor, sizeof(aLine) - 1);
		str_copy(aLine, pCursor, Len + 1);
		pCursor = pLineEnd + 1;

		SKnownUser User;
		mem_zero(&User, sizeof(User));
		char aName[128];
		char aClan[128];
		if(sscanf(aLine, "{\"name\": \"%127[^\"]\", \"clan\": \"%127[^\"]\", \"last_seen\": %d}", aName, aClan, &User.m_LastSeen) == 3)
		{
			str_copy(User.m_aName, aName, sizeof(User.m_aName));
			str_copy(User.m_aClan, aClan, sizeof(User.m_aClan));
			m_Known.push_back(User);
		}
	}
	free(pData);
}

void CYablokoCommunity::SaveLibrary()
{
	Storage()->CreateFolder("yabloko", IStorage::TYPE_SAVE);
	char aBuf[IO_MAX_PATH_LENGTH];
	IOHANDLE File = Storage()->OpenFile("yabloko/library.json.tmp", IOFLAG_WRITE, IStorage::TYPE_SAVE, aBuf, sizeof(aBuf));
	if(!File)
		return;
	for(const auto &User : m_Known)
	{
		char aLine[512];
		str_format(aLine, sizeof(aLine), "{\"name\": \"%s\", \"clan\": \"%s\", \"last_seen\": %d}\n", User.m_aName, User.m_aClan, User.m_LastSeen);
		io_write(File, aLine, str_length(aLine));
	}
	io_close(File);
	Storage()->RenameFile("yabloko/library.json.tmp", "yabloko/library.json", IStorage::TYPE_SAVE);
}

bool CYablokoCommunity::IsYablokoUser(int ClientId) const
{
	if(ClientId < 0 || ClientId >= MAX_CLIENTS || !GameClient()->m_Snap.m_aCharacters[ClientId].m_Active)
	{
		// check player info even when not in game (spectators leave)
		if(ClientId >= 0 && ClientId < MAX_CLIENTS && GameClient()->m_aClients[ClientId].m_Active)
			return Yabloko::IsYablokoName(GameClient()->m_aClients[ClientId].m_aClan) ||
			       Yabloko::IsYablokoName(GameClient()->m_aClients[ClientId].m_aName);
		return false;
	}
	return Yabloko::IsYablokoName(GameClient()->m_aClients[ClientId].m_aClan) ||
	       Yabloko::IsYablokoName(GameClient()->m_aClients[ClientId].m_aName);
}

bool CYablokoCommunity::IsKnownUser(int ClientId) const
{
	if(!IsYablokoUser(ClientId))
		return false;
	for(const auto &User : m_Known)
	{
		if(str_comp(User.m_aName, GameClient()->m_aClients[ClientId].m_aName) == 0)
			return true;
	}
	return false;
}

int CYablokoCommunity::YablokoCount() const
{
	int Count = 0;
	for(int i = 0; i < MAX_CLIENTS; i++)
	{
		if(GameClient()->m_aClients[i].m_Active && IsYablokoUser(i) && i != GameClient()->m_Snap.m_LocalClientId)
			Count++;
	}
	return Count;
}

const char *CYablokoCommunity::PublishClan() const
{
	if(!g_Config.m_ClYablokoPublishIdentity)
		return g_Config.m_PlayerClan;
	static char aBuf[32];
	if(Yabloko::IsYablokoName(g_Config.m_PlayerClan))
		return g_Config.m_PlayerClan;
	str_copy(aBuf, "[Yabloko]", sizeof(aBuf));
	return aBuf;
}

void CYablokoCommunity::SendAds()
{
	CGameClient *pGC = GameClient();
	if(Client()->State() != IClient::STATE_ONLINE || !pGC->m_Snap.m_pGameInfoObj)
		return;

	if(g_Config.m_ClYablokoAdsTeam)
	{
		pGC->Console()->ExecuteLine(g_Config.m_ClYablokoAdsText, IConsole::CLIENT_ID_UNSPECIFIED);
	}
	else
	{
		pGC->Console()->ExecuteLine(g_Config.m_ClYablokoAdsText, IConsole::CLIENT_ID_UNSPECIFIED);
	}
}

void CYablokoCommunity::OnUpdate()
{
	if(Client()->State() != IClient::STATE_ONLINE)
	{
		m_Users.clear();
		return;
	}

	// remember users
	for(int i = 0; i < MAX_CLIENTS; i++)
	{
		if(!GameClient()->m_aClients[i].m_Active || i == GameClient()->m_Snap.m_LocalClientId)
			continue;
		if(IsYablokoUser(i) && !m_Users.count(i))
		{
			m_Users.insert(i);
			if(!IsKnownUser(i))
			{
				SKnownUser User;
				str_copy(User.m_aName, GameClient()->m_aClients[i].m_aName, sizeof(User.m_aName));
				str_copy(User.m_aClan, GameClient()->m_aClients[i].m_aClan, sizeof(User.m_aClan));
				User.m_LastSeen = (int)(Client()->GlobalTime() / 100.0f);
				m_Known.push_back(User);
				if(m_Known.size() > 512)
					m_Known.erase(m_Known.begin());
				SaveLibrary();
			}
		}
	}

	// timed promotional whispers
	if(g_Config.m_ClYablokoAutoWhisper)
	{
		float Time = Client()->GlobalTime();
		if(m_NextWhisperTime <= 0)
			m_NextWhisperTime = Time + g_Config.m_ClYablokoAutoWhisperInterval;
		if(Time >= m_NextWhisperTime)
		{
			m_NextWhisperTime = Time + g_Config.m_ClYablokoAutoWhisperInterval;
			// pick a recipient
			for(int i = 0; i < MAX_CLIENTS; i++)
			{
				if(!GameClient()->m_aClients[i].m_Active || i == GameClient()->m_Snap.m_LocalClientId)
					continue;
				if(IsYablokoUser(i))
					continue;
								char aCmd[512];
				str_format(aCmd, sizeof(aCmd), "/w \"%s\" %s", GameClient()->m_aClients[i].m_aName, g_Config.m_ClYablokoAdsText);
				GameClient()->Console()->ExecuteLine(aCmd, IConsole::CLIENT_ID_UNSPECIFIED);
				break;
			}
		}
	}
}
