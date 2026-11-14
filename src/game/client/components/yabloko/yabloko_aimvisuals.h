/* Yabloko client — reconstructed from the surviving build.
   Aim visuals: ZZetClient ghost aura rings around the aimbot target. */

#ifndef GAME_CLIENT_COMPONENTS_YABLOKO_YABLOKO_AIMVISUALS_H
#define GAME_CLIENT_COMPONENTS_YABLOKO_YABLOKO_AIMVISUALS_H

#include <game/client/component.h>

#include <game/client/components/yabloko/yabloko.h>

class CYablokoAimVisuals : public CComponent
{
public:
	int Sizeof() const override { return sizeof(*this); }

	void OnRender() override;
};

#endif
