/* Yabloko client — reconstructed from the surviving build.
   Media: private-message sounds, in-game media interaction state. */

#ifndef GAME_CLIENT_COMPONENTS_YABLOKO_YABLOKO_MEDIA_H
#define GAME_CLIENT_COMPONENTS_YABLOKO_YABLOKO_MEDIA_H

#include <game/client/component.h>

#include <game/client/components/yabloko/yabloko.h>

class CYablokoMedia : public CComponent
{
public:
	int Sizeof() const override { return sizeof(*this); }

	void OnReset() override;
	void OnRender() override;

	// Play the private message notification sound (called from chat)
	void OnPrivateMessage();

private:
	bool m_SoundQueued;
	float m_NextSoundTime;
};

#endif
