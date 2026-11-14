/* Yabloko client — reconstructed from the surviving build.
   Extras: emote spam, chat search, KRX features, custom font. */

#ifndef GAME_CLIENT_COMPONENTS_YABLOKO_YABLOKO_EXTRAS_H
#define GAME_CLIENT_COMPONENTS_YABLOKO_YABLOKO_EXTRAS_H

#include <engine/console.h>

#include <game/client/component.h>

#include <game/client/components/yabloko/yabloko.h>

class CYablokoExtras : public CComponent
{
public:
	int Sizeof() const override { return sizeof(*this); }

	void OnReset() override;
	void OnConsoleInit() override;
	void OnUpdate() override;

	// KRX custom hook color (with optional rainbow)
	ColorRGBA HookColor() const;

	char m_aChatSearch[128];

	// Emote spam state
	bool m_EmoteSpamActive;
	bool m_EyeLaserBindActive;
	float m_NextEmoteTime;
	int m_LastEmote;
	bool m_EmoteSpamPrev;

private:
	static void ConChatSearch(IConsole::IResult *pResult, void *pUserData);
	static void ConRandomCopy(IConsole::IResult *pResult, void *pUserData);
	static void ConResetHud(IConsole::IResult *pResult, void *pUserData);
	static void ConEffectsSettings(IConsole::IResult *pResult, void *pUserData);
	static void ConKrxSaveIdentity(IConsole::IResult *pResult, void *pUserData);
	static void ConKrxLoadIdentity(IConsole::IResult *pResult, void *pUserData);
	static void ConStateEmoteSpam(IConsole::IResult *pResult, void *pUserData);
	static void ConStateEyeLaser(IConsole::IResult *pResult, void *pUserData);
	static void ConStateEditHud(IConsole::IResult *pResult, void *pUserData);
};

#endif
