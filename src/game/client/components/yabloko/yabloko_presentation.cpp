/* Yabloko client — reconstructed from the surviving build.
   Presentation implementation. */

#include "yabloko_presentation.h"

#include <engine/graphics.h>
#include <engine/shared/config.h>
#include <engine/textrender.h>

#include <game/client/gameclient.h>
#include <game/client/ui.h>

#include <algorithm>

void CYablokoPresentation::OnReset()
{
	m_vParticles.clear();
	m_LastMenuTime = 0.0f;
	m_LoadingStart = 0.0f;
}

void CYablokoPresentation::OnRender()
{
	float Time = Client()->GlobalTime();

	bool Offline = Client()->State() == IClient::STATE_OFFLINE;
	bool Loading = Client()->State() == IClient::STATE_LOADING;

	// menu particles with mouse parallax
	if((Offline || GameClient()->m_Menus.IsActive()) && (g_Config.m_ClYablokoMenuParticles || g_Config.m_ClYablokoMenuWeather != 0))
	{
		if(Time - m_LastMenuTime > 0.5f)
		{
			m_LastMenuTime = Time;
			if(m_vParticles.size() < 120)
			{
				vec2 ScreenMin, ScreenMax;
				Graphics()->GetScreen(&ScreenMin.x, &ScreenMin.y, &ScreenMax.x, &ScreenMax.y);
				SMenuParticle P;
				P.m_Pos = vec2(ScreenMin.x + random_float() * (ScreenMax.x - ScreenMin.x), ScreenMax.y + 20.0f);
				P.m_Vel = vec2(random_float() * 20.0f - 10.0f, -20.0f - random_float() * 40.0f);
				P.m_Size = 1.0f + random_float() * 3.5f;
				P.m_Depth = 0.3f + random_float() * 0.7f;
				m_vParticles.push_back(P);
			}
		}
	}

	if(Offline || GameClient()->m_Menus.IsActive())
	{
		if(g_Config.m_ClYablokoMenuParticles)
			RenderMenuParticles();
		if(g_Config.m_ClYablokoMenuWeather != 0)
			RenderMenuWeather();
		if(g_Config.m_ClYablokoCursor)
			RenderNeonCursor();
		if(GameClient()->m_Menus.IsActive() && g_Config.m_ClYablokoPauseSidebar)
			RenderPauseSidebar();
	}
	else
	{
		m_vParticles.clear();
	}

	if(Loading || Client()->State() == IClient::STATE_CONNECTING)
	{
		if(m_LoadingStart <= 0)
			m_LoadingStart = Time;
		if(g_Config.m_ClYablokoLoadingOverlay)
			RenderLoadingOverlay();
	}
	else
		m_LoadingStart = 0.0f;
}

void CYablokoPresentation::RenderMenuParticles()
{
	float Time = Client()->GlobalTime();
	float dt = 1.0f / 60.0f;

	vec2 ScreenMin, ScreenMax;
	Graphics()->GetScreen(&ScreenMin.x, &ScreenMin.y, &ScreenMax.x, &ScreenMax.y);

	ColorRGBA Accent = Yabloko::ThemeAccent(g_Config.m_ClYablokoTheme, g_Config.m_ClYablokoThemeAccent);

	// mouse parallax offset
	vec2 Mouse = Ui()->MousePos();
	vec2 Center = vec2((ScreenMin.x + ScreenMax.x) / 2, (ScreenMin.y + ScreenMax.y) / 2);
	vec2 Parallax = (Mouse - Center) * 0.02f;

	for(auto &P : m_vParticles)
	{
		P.m_Pos += P.m_Vel * dt * (g_Config.m_ClYablokoMenuAnimationTime > 0 ? 60.0f / g_Config.m_ClYablokoMenuAnimationTime : 1.0f);
		if(P.m_Pos.y < ScreenMin.y - 30.0f)
		{
			P.m_Pos.y = ScreenMax.y + 20.0f;
			P.m_Pos.x = ScreenMin.x + random_float() * (ScreenMax.x - ScreenMin.x);
		}
		vec2 RenderPos = P.m_Pos + Parallax * P.m_Depth;
		float Alpha = 0.35f * P.m_Depth;
		Graphics()->SetColor(Accent.r, Accent.g, Accent.b, Alpha);
		Graphics()->DrawCircle(RenderPos.x, RenderPos.y, P.m_Size * P.m_Depth, 8);
	}
	(void)Time;
}

void CYablokoPresentation::RenderMenuWeather()
{
	// 1 = rain, 2 = snow — reuse the particle field with weather looks
	vec2 ScreenMin, ScreenMax;
	Graphics()->GetScreen(&ScreenMin.x, &ScreenMin.y, &ScreenMax.x, &ScreenMax.y);
	float dt = 1.0f / 60.0f;

	if(g_Config.m_ClYablokoMenuWeather == 1)
	{
		// rain streaks
		Graphics()->SetColor(0.5f, 0.65f, 1.0f, 0.35f);
		for(int i = 0; i < 40; i++)
		{
			float Seed = i * 17.13f;
			float Speed = 400.0f + std::fmod(Seed, 200.0f);
			float x = ScreenMin.x + std::fmod(Seed * 31.7f, ScreenMax.x - ScreenMin.x);
			float y = ScreenMin.y + std::fmod(Client()->GlobalTime() * Speed + Seed * 97.0f, ScreenMax.y - ScreenMin.y + 80.0f);
			IGraphics::CFreeformItem Drop(x, y, x + 2.0f, y + 14.0f, x + 1.0f, y + 14.0f, x + 3.0f, y);
			Graphics()->QuadsBegin();
			Graphics()->QuadsDrawFreeform(&Drop, 1);
			Graphics()->QuadsEnd();
		}
		(void)dt;
	}
	else if(g_Config.m_ClYablokoMenuWeather == 2)
	{
		Graphics()->SetColor(1.0f, 1.0f, 1.0f, 0.5f);
		for(int i = 0; i < 60; i++)
		{
			float Seed = i * 13.37f;
			float x = ScreenMin.x + std::fmod(Seed * 53.1f + std::sin(Client()->GlobalTime() + Seed) * 20.0f, ScreenMax.x - ScreenMin.x);
			float y = ScreenMin.y + std::fmod(Client()->GlobalTime() * (60.0f + std::fmod(Seed, 80.0f)) + Seed * 71.0f, ScreenMax.y - ScreenMin.y + 40.0f);
			Graphics()->DrawCircle(x, y, 1.5f + std::fmod(Seed, 2.0f), 6);
		}
	}
}

void CYablokoPresentation::RenderNeonCursor()
{
	vec2 Mouse = Ui()->MousePos();
	ColorRGBA Color;
	switch(g_Config.m_ClYablokoCursorColor)
	{
	case 1: Color = ColorRGBA(0.55f, 0.30f, 0.90f, 1.0f); break;
	case 2: Color = ColorRGBA(0.95f, 0.25f, 0.30f, 1.0f); break;
	case 3: Color = ColorRGBA(0.20f, 0.85f, 0.45f, 1.0f); break;
	case 4: Color = ColorRGBA(0.05f, 0.05f, 0.05f, 0.9f); break;
	default:
		Color = Yabloko::ThemeAccent(g_Config.m_ClYablokoTheme, g_Config.m_ClYablokoThemeAccent);
		break;
	}

	if(g_Config.m_ClYablokoCursorGlow)
	{
		Graphics()->BlendAdditive();
		Graphics()->SetColor(Color.r, Color.g, Color.b, 0.25f);
		Graphics()->DrawCircle(Mouse.x, Mouse.y, 16.0f, 12);
		Graphics()->SetColor(Color.r, Color.g, Color.b, 0.12f);
		Graphics()->DrawCircle(Mouse.x, Mouse.y, 26.0f, 12);
		Graphics()->BlendNone();
	}
	Graphics()->SetColor(Color.r, Color.g, Color.b, 0.8f);
	Graphics()->DrawCircle(Mouse.x, Mouse.y, 4.0f, 8);
}

void CYablokoPresentation::RenderLoadingOverlay()
{
	vec2 ScreenMin, ScreenMax;
	Graphics()->GetScreen(&ScreenMin.x, &ScreenMin.y, &ScreenMax.x, &ScreenMax.y);
	float W = ScreenMax.x - ScreenMin.x;
	float H = ScreenMax.y - ScreenMin.y;
	float Time = Client()->GlobalTime();

	ColorRGBA Accent = Yabloko::ThemeAccent(g_Config.m_ClYablokoTheme, g_Config.m_ClYablokoThemeAccent);

	// dim backdrop
	Graphics()->SetColor(0.0f, 0.0f, 0.0f, 0.55f);
	IGraphics::CQuadItem Back(ScreenMin.x, ScreenMin.y, W, H);
	Graphics()->QuadsBegin();
	Graphics()->QuadsDraw(&Back, 1);
	Graphics()->QuadsEnd();

	// spinning apple (circle + arc)
	vec2 Center = vec2(ScreenMin.x + W / 2, ScreenMin.y + H / 2);
	for(int i = 0; i < 12; i++)
	{
		float Angle = Time * 3.0f + i * pi / 6;
		float Alpha = i / 12.0f;
		vec2 P = Center + direction(Angle) * 26.0f;
		Graphics()->SetColor(Accent.r, Accent.g, Accent.b, Alpha);
		Graphics()->DrawCircle(P.x, P.y, 4.0f, 6);
	}

	// brand
	CUIRect Label;
	Label.x = ScreenMin.x;
	Label.y = Center.y + 40.0f;
	Label.w = W;
	Label.h = 24.0f;
	TextRender()->TextColor(Accent.r, Accent.g, Accent.b, 1.0f);
	Ui()->DoLabel(&Label, "Yabloko", 20.0f, TEXTALIGN_MC);
	TextRender()->TextColor(1.0f, 1.0f, 1.0f, 1.0f);
}

void CYablokoPresentation::RenderPauseSidebar()
{
	(void)m_vParticles;
}
