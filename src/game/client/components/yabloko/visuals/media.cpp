/* Yabloko client — reconstructed from the surviving build.
   visuals/media.cpp — mirrors the original source path from the leaked build. */

#include <game/client/components/yabloko/yabloko_media.h>

#include <engine/shared/config.h>
#include <engine/sound.h>

#include <game/client/components/sounds.h>

#include <generated/client_data.h>

#include <game/client/gameclient.h>

void CYablokoMedia::OnReset()
{
	m_SoundQueued = false;
	m_NextSoundTime = 0.0f;
}

void CYablokoMedia::OnRender()
{
	if(m_SoundQueued && Client()->GlobalTime() >= m_NextSoundTime)
	{
		m_SoundQueued = false;
		if(g_Config.m_ClYablokoPrivateSoundVolume > 0)
		{
			// play the chat sound for private message notification
			float Volume = g_Config.m_ClYablokoPrivateSoundVolume / 100.0f;
			GameClient()->m_Sounds.Play(CSounds::CHN_GLOBAL, SOUND_CHAT_SERVER, Volume);
		}
	}
}

void CYablokoMedia::OnPrivateMessage()
{
	if(g_Config.m_ClYablokoPrivateSound <= 0)
		return;
	// quiet notification sound while the game window is unfocused
	m_SoundQueued = true;
	m_NextSoundTime = Client()->GlobalTime();
}
