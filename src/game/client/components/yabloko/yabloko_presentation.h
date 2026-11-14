/* Yabloko client — reconstructed from the surviving build.
   Presentation: menu particles/weather, neon cursor, loading overlay. */

#ifndef GAME_CLIENT_COMPONENTS_YABLOKO_YABLOKO_PRESENTATION_H
#define GAME_CLIENT_COMPONENTS_YABLOKO_YABLOKO_PRESENTATION_H

#include <game/client/component.h>

#include <game/client/components/yabloko/yabloko.h>

#include <vector>

class CYablokoPresentation : public CComponent
{
public:
	int Sizeof() const override { return sizeof(*this); }

	void OnReset() override;
	void OnRender() override;

private:
	struct SMenuParticle
	{
		vec2 m_Pos;
		vec2 m_Vel;
		float m_Size;
		float m_Depth;
	};

	std::vector<SMenuParticle> m_vParticles;
	float m_LastMenuTime;
	float m_LoadingStart;

	void RenderMenuParticles();
	void RenderMenuWeather();
	void RenderNeonCursor();
	void RenderLoadingOverlay();
	void RenderPauseSidebar();
};

#endif
