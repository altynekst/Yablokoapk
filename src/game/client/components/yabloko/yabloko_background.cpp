/* Yabloko client — reconstructed from the surviving build.
   Animated background implementation. */

#include "yabloko_background.h"

#include <engine/graphics.h>
#include <engine/shared/config.h>

#include <game/client/gameclient.h>

#include <algorithm>

void CYablokoBackground::OnReset()
{
	m_vParticles.clear();
	m_LastTheme = -1;
	m_LastCycle = 0.0f;
	m_ThemeRotation = 0;
	m_TexturePhase = 0.0f;
	m_aLoadedTheme[0] = '\0';
}

const char *const ms_apThemeNames[] = {
	"abyssal_ocean", "amber_gold", "amethyst_nebula", "aurora", "black_hole",
	"blue_clouds", "blue_flow", "blue_nebula", "clouds", "crimson_silk",
	"crystal_cathedral", "cyberpunk", "dark", "deep_space", "emerald_fog",
	"glacier_veil", "gravity_grid", "jade_lagoon", "liquid_currents", "monochrome",
	"moon_reflection", "plasma_torus", "plum_ink", "polar_aurora", "purple_cosmos",
	"red_clouds", "red_flow", "red_void", "silent_dunes", "silver_clouds",
	"solar_winds"};
constexpr int NUM_THEMES = std::size(ms_apThemeNames);

void CYablokoBackground::EnsureThemeTexture(int Theme)
{
	if(Theme < 0 || Theme >= NUM_THEMES)
	{
		m_ThemeTexture.Invalidate();
		m_aLoadedTheme[0] = '\0';
		return;
	}
	if(str_comp(m_aLoadedTheme, ms_apThemeNames[Theme]) == 0 && m_ThemeTexture.IsValid())
		return;
	char aBuf[IO_MAX_PATH_LENGTH];
	str_format(aBuf, sizeof(aBuf), "yabloko/backgrounds/%s.png", ms_apThemeNames[Theme]);
	m_ThemeTexture = Graphics()->LoadTexture(aBuf, IStorage::TYPE_ALL);
	str_copy(m_aLoadedTheme, ms_apThemeNames[Theme], sizeof(m_aLoadedTheme));
}

void CYablokoBackground::EnsureParticles()
{
	vec2 ScreenMin, ScreenMax;
	Graphics()->GetScreen(&ScreenMin.x, &ScreenMin.y, &ScreenMax.x, &ScreenMax.y);
	while((int)m_vParticles.size() < 60)
	{
		SBgParticle P;
		P.m_Pos = vec2(ScreenMin.x + random_float() * (ScreenMax.x - ScreenMin.x),
			ScreenMin.y + random_float() * (ScreenMax.y - ScreenMin.y));
		P.m_Vel = vec2(random_float() * 30.0f - 15.0f, random_float() * 30.0f - 15.0f);
		P.m_Size = 1.0f + random_float() * 4.0f;
		P.m_Depth = 0.2f + random_float() * 0.8f;
		m_vParticles.push_back(P);
	}
}

void CYablokoBackground::RenderGradient(const ColorRGBA &Top, const ColorRGBA &Bottom, float Alpha)
{
	vec2 ScreenMin, ScreenMax;
	Graphics()->GetScreen(&ScreenMin.x, &ScreenMin.y, &ScreenMax.x, &ScreenMax.y);
	float W = ScreenMax.x - ScreenMin.x;
	float H = ScreenMax.y - ScreenMin.y;

	Graphics()->SetColor4(Top.WithAlpha(Alpha), Top.WithAlpha(Alpha), Bottom.WithAlpha(Alpha), Bottom.WithAlpha(Alpha));
	IGraphics::CFreeformItem Quad(ScreenMin.x, ScreenMin.y, ScreenMax.x, ScreenMin.y,
		ScreenMin.x, ScreenMax.y, ScreenMax.x, ScreenMax.y);
	Graphics()->QuadsBegin();
	Graphics()->QuadsDrawFreeform(&Quad, 1);
	Graphics()->QuadsEnd();
}

void CYablokoBackground::RenderTheme(int Theme)
{
	float Time = Client()->GlobalTime();

	// base gradient from the theme
	ColorRGBA Accent = Yabloko::ThemeAccent(Theme == 4 ? 4 : Theme % 5, g_Config.m_ClYablokoThemeAccent);
	ColorRGBA Back = Yabloko::ThemeBackground(Theme == 4 ? 4 : Theme % 5, g_Config.m_ClYablokoThemeBackground);
	ColorRGBA Top = Back;
	ColorRGBA Bottom = ColorRGBA(Back.r * 0.5f + Accent.r * 0.12f, Back.g * 0.5f + Accent.g * 0.12f, Back.b * 0.5f + Accent.b * 0.12f, 1.0f);

	float Speed = g_Config.m_ClYablokoBackgroundSpeed / 100.0f;
	float Brightness = g_Config.m_ClYablokoBackgroundBrightness / 100.0f;
	float Contrast = g_Config.m_ClYablokoBackgroundContrast / 100.0f;
	float Saturation = g_Config.m_ClYablokoBackgroundSaturation / 100.0f;

	// animated hue drift for classic
	if(g_Config.m_ClYablokoBackgroundAnimation)
	{
		float Wave = (std::sin(Time * 0.3f * Speed) + 1.0f) * 0.5f;
		Top = Top.Multiply(0.7f + 0.3f * Wave);
		Bottom = Bottom.Multiply(0.7f + 0.3f * (1.0f - Wave));
	}

	float Alpha = 1.0f;
	RenderGradient(Top, Bottom, Alpha);

	// themed background image from yabloko/backgrounds
	EnsureThemeTexture(Theme);
	if(m_ThemeTexture.IsValid())
	{
		vec2 ScreenMin, ScreenMax;
		Graphics()->GetScreen(&ScreenMin.x, &ScreenMin.y, &ScreenMax.x, &ScreenMax.y);
		float W = ScreenMax.x - ScreenMin.x;
		float H = ScreenMax.y - ScreenMin.y;
		float ImgW = 1920.0f, ImgH = 1080.0f;
		float Scale = std::max(W / ImgW, H / ImgH) * 1.05f;
		float ImgW2 = ImgW * Scale, ImgH2 = ImgH * Scale;
		Graphics()->BlendNormal();
		Graphics()->TextureSet(m_ThemeTexture);
		Graphics()->SetColor(Brightness, Brightness, Brightness, Alpha);
		IGraphics::CQuadItem Quad(ScreenMin.x + (W - ImgW2) / 2 + std::sin(Time * 0.05f * Speed) * 20.0f,
			ScreenMin.y + (H - ImgH2) / 2 + std::cos(Time * 0.04f * Speed) * 20.0f, ImgW2, ImgH2);
		Graphics()->QuadsBegin();
		Graphics()->QuadsDraw(&Quad, 1);
		Graphics()->QuadsEnd();
	}

	// soft accent glow that wanders across the screen
	vec2 ScreenMin, ScreenMax;
	Graphics()->GetScreen(&ScreenMin.x, &ScreenMin.y, &ScreenMax.x, &ScreenMax.y);
	float W = ScreenMax.x - ScreenMin.x;
	float H = ScreenMax.y - ScreenMin.y;
	vec2 GlowPos = vec2(ScreenMin.x + W * (0.5f + 0.35f * std::sin(Time * 0.4f * Speed)),
		ScreenMin.y + H * (0.5f + 0.35f * std::cos(Time * 0.3f * Speed)));
	Graphics()->BlendAdditive();
	for(int i = 6; i >= 1; i--)
	{
		Graphics()->SetColor(Accent.r, Accent.g, Accent.b, 0.02f * i * (0.5f + Saturation * 0.5f));
		Graphics()->DrawCircle(GlowPos.x, GlowPos.y, 60.0f * i, 24);
	}
	Graphics()->BlendNone();

	// parallax particles
	if(g_Config.m_ClYablokoBackgroundParticles > 0)
	{
		EnsureParticles();
		float dt = 1.0f / 60.0f;
		vec2 CameraOffset = vec2(0, 0);
		if(g_Config.m_ClYablokoBackgroundParallax && GameClient()->m_Snap.m_pLocalCharacter)
		{
			CameraOffset = -GameClient()->m_LocalCharacterPos * 0.04f;
		}
		float ParticleAlpha = g_Config.m_ClYablokoBackgroundParticles / 100.0f;
		for(auto &P : m_vParticles)
		{
			P.m_Pos += P.m_Vel * dt * Speed;
			if(P.m_Pos.x < ScreenMin.x - 20.0f)
				P.m_Pos.x = ScreenMax.x + 20.0f;
			if(P.m_Pos.x > ScreenMax.x + 20.0f)
				P.m_Pos.x = ScreenMin.x - 20.0f;
			if(P.m_Pos.y < ScreenMin.y - 20.0f)
				P.m_Pos.y = ScreenMax.y + 20.0f;
			if(P.m_Pos.y > ScreenMax.y + 20.0f)
				P.m_Pos.y = ScreenMin.y - 20.0f;
			vec2 RenderPos = P.m_Pos + CameraOffset * P.m_Depth;
			Graphics()->SetColor(Accent.r, Accent.g, Accent.b, ParticleAlpha * 0.5f * P.m_Depth);
			Graphics()->DrawCircle(RenderPos.x, RenderPos.y, P.m_Size, 6);
		}
	}

	// dim overlay for HUD readability
	if(g_Config.m_ClYablokoBackgroundDim > 0)
	{
		float Dim = g_Config.m_ClYablokoBackgroundDim / 100.0f;
		Graphics()->SetColor(0.0f, 0.0f, 0.0f, Dim * 0.6f);
		IGraphics::CQuadItem Full(ScreenMin.x, ScreenMin.y, W, H);
		Graphics()->QuadsBegin();
		Graphics()->QuadsDraw(&Full, 1);
		Graphics()->QuadsEnd();
	}

	(void)Brightness;
	(void)Contrast;
}

void CYablokoBackground::OnRender()
{
	if(Client()->State() != IClient::STATE_ONLINE && Client()->State() != IClient::STATE_DEMOPLAYBACK)
		return;
	if(!g_Config.m_ClYablokoBackground)
		return;

	// automatic background cycling
	if(g_Config.m_ClYablokoBackgroundCycle > 0)
	{
		float Time = Client()->GlobalTime();
		if(Time - m_LastCycle > g_Config.m_ClYablokoBackgroundCycle)
		{
			m_LastCycle = Time;
			m_ThemeRotation = (m_ThemeRotation + 1) % 5;
		}
		RenderTheme(m_ThemeRotation);
	}
	else
		RenderTheme(g_Config.m_ClYablokoBackgroundTheme);
}
