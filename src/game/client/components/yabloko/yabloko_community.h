/* Yabloko client — reconstructed from the surviving build.
   Community: Yabloko user detection, highlighting, ads, auto whisper, library. */

#ifndef GAME_CLIENT_COMPONENTS_YABLOKO_YABLOKO_COMMUNITY_H
#define GAME_CLIENT_COMPONENTS_YABLOKO_YABLOKO_COMMUNITY_H

#include <engine/console.h>
#include <game/client/component.h>

#include <game/client/components/yabloko/yabloko.h>

#include <set>
#include <string>
#include <vector>

class CYablokoCommunity : public CComponent
{
public:
	int Sizeof() const override { return sizeof(*this); }

	void OnReset() override;
	void OnUpdate() override;
	void OnConsoleInit() override;

	// Is the given client a publicly marked Yabloko user?
	bool IsYablokoUser(int ClientId) const;
	// Have they been seen before (library)?
	bool IsKnownUser(int ClientId) const;
	// Number of Yabloko users on the current server
	int YablokoCount() const;

	// Clan replacement for StartInfo: publish the marker, keep the saved clan
	const char *PublishClan() const;

	// Ads
	void SendAds();

private:
	static void ConSendAds(IConsole::IResult *pResult, void *pUserData);
	static void ConUsersSettings(IConsole::IResult *pResult, void *pUserData);
	void ConAutoWhisperTick();

	struct SKnownUser
	{
		char m_aName[64];
		char m_aClan[32];
		int m_LastSeen;
	};

	std::set<int> m_Users;
	std::vector<SKnownUser> m_Known;
	float m_NextWhisperTime;
	int m_LocalPrevTeam;

	void LoadLibrary();
	void SaveLibrary();
};

#endif
