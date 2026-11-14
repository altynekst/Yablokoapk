/* Yabloko client — reconstructed from the surviving build.
   Animated in-game background: themes, particles, parallax, cycle. */

#ifndef GAME_CLIENT_COMPONENTS_YABLOKO_YABLOKO_BACKGROUND_H
#define GAME_CLIENT_COMPONENTS_YABLOKO_YABLOKO_BACKGROUND_H

#include <game/client/component.h>

#include <game/client/components/yabloko/yabloko.h>

#include <engine/graphics.h>

#include <vector>

class CYablokoBackground : public CComponent
{
public:
	int Sizeof() const override { return sizeof(*this); }

	void OnReset() override;
	void OnRender() override;

private:
	struct SBgParticle
	{
		vec2 m_Pos;
		vec2 m_Vel;
		float m_Size;
		float m_Depth;
	};

	std::vector<SBgParticle> m_vParticles;
	int m_LastTheme;
	float m_LastCycle;
	int m_ThemeRotation;
	IGraphics::CTextureHandle m_ThemeTexture;
	char m_aLoadedTheme[64];
	float m_TexturePhase;

	void EnsureThemeTexture(int Theme);

	void EnsureParticles();
	void RenderGradient(const ColorRGBA &Top, const ColorRGBA &Bottom, float Alpha);
	void RenderTheme(int Theme);
};

#endif
