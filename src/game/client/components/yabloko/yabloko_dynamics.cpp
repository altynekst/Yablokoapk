/* Yabloko client — reconstructed from the surviving build.
   Dynamic world effects implementation. */

#include "yabloko_dynamics.h"

#include <engine/graphics.h>
#include <engine/shared/config.h>

#include <game/collision.h>
#include <game/mapitems.h>
#include <game/client/gameclient.h>

#include <algorithm>

void CYablokoDynamics::OnReset()
{
	m_vShards.clear();
	m_vFlames.clear();
	m_aGridNodes.clear();
	m_GridCols = 0;
	m_GridRows = 0;
	m_LastBoostSpeed = 0.0f;
	m_LastRocketState = 0;
}

void CYablokoDynamics::OnRender()
{
	if(Client()->State() != IClient::STATE_ONLINE && Client()->State() != IClient::STATE_DEMOPLAYBACK)
		return;

	RenderFreezeTiles();
	RenderGrid();
	RenderPlasma();
	RenderTesla();
	RenderShatter();
	RenderAfterburner();
}

void CYablokoDynamics::RenderGrid()
{
	if(!g_Config.m_ClYablokoGrid)
		return;

	vec2 ScreenMin, ScreenMax;
	Graphics()->GetScreen(&ScreenMin.x, &ScreenMin.y, &ScreenMax.x, &ScreenMax.y);
	float Spacing = std::clamp(g_Config.m_ClYablokoGridSpacing, 8, 128);
	int Cols = (int)((ScreenMax.x - ScreenMin.x) / Spacing) + 2;
	int Rows = (int)((ScreenMax.y - ScreenMin.y) / Spacing) + 2;
	Cols = std::clamp(Cols, 1, 128);
	Rows = std::clamp(Rows, 1, 128);
	(void)ScreenMin;
	(void)ScreenMax;

	ColorRGBA Color = Yabloko::Col(g_Config.m_ClYablokoGridColor, g_Config.m_ClYablokoGridAlpha / 100.0f);
	float LineWidth = g_Config.m_ClYablokoGridWidth / 10.0f;
	LineWidth = std::clamp(LineWidth, 0.5f, 4.0f);

	// players push the grid
	auto Push = [&](vec2 Pos, float Strength) {
		for(int y = 0; y < Rows; y++)
		{
			for(int x = 0; x < Cols; x++)
			{
				vec2 Node = vec2(ScreenMin.x + x * Spacing, ScreenMin.y + y * Spacing);
				vec2 Delta = Node - Pos;
				float Dist = length(Delta);
				if(Dist < g_Config.m_ClYablokoGridRadius && Dist > 1.0f)
				{
					float Force = Strength * (1.0f - Dist / g_Config.m_ClYablokoGridRadius);
					m_aGridNodes[y * Cols + x].m_Vel += normalize(Delta) * Force * 4.0f;
				}
			}
		}
	};

	if(g_Config.m_ClYablokoGridPlayers && (int)m_aGridNodes.size() == Cols * Rows)
	{
		for(int i = 0; i < MAX_CLIENTS; i++)
		{
			if(GameClient()->m_Snap.m_aCharacters[i].m_Active)
				Push(GameClient()->m_aClients[i].m_RenderPos, 1.0f);
		}
	}

	// resize / init nodes
	if((int)m_aGridNodes.size() != Cols * Rows)
	{
		m_aGridNodes.clear();
		m_aGridNodes.resize(Cols * Rows);
		m_GridCols = Cols;
		m_GridRows = Rows;
	}

	float dt = 1.0f / 60.0f;
	float Stiffness = std::clamp(g_Config.m_ClYablokoGridStiffness / 100.0f, 0.01f, 1.0f);
	float Damping = std::clamp(g_Config.m_ClYablokoGridDamping / 100.0f, 0.01f, 1.0f);

	Graphics()->SetColor(Color.r, Color.g, Color.b, Color.a);
		Graphics()->QuadsBegin();
	for(int y = 0; y < Rows; y++)
	{
		for(int x = 0; x < Cols; x++)
		{
			auto &Node = m_aGridNodes[y * Cols + x];
			Node.m_Vel += -Node.m_Offset * Stiffness * 10.0f * dt;
			Node.m_Vel *= (1.0f - Damping * 0.1f);
			Node.m_Offset += Node.m_Vel * dt * 60.0f;
			vec2 NodePos = vec2(ScreenMin.x + x * Spacing, ScreenMin.y + y * Spacing) + Node.m_Offset;
			IGraphics::CQuadItem Dot(NodePos.x - LineWidth / 2, NodePos.y - LineWidth / 2, LineWidth, LineWidth);
			Graphics()->QuadsDraw(&Dot, 1);
		}
	}
	Graphics()->QuadsEnd();
}

void CYablokoDynamics::RenderFreezeTiles()
{
	if(!g_Config.m_ClYablokoFreezeTileGlow)
		return;

	vec2 ScreenMin, ScreenMax;
	Graphics()->GetScreen(&ScreenMin.x, &ScreenMin.y, &ScreenMax.x, &ScreenMax.y);
	float Radius = g_Config.m_ClYablokoFreezeTileRadius;
	ColorRGBA Color = Yabloko::Col(g_Config.m_ClYablokoFreezeTileColor, g_Config.m_ClYablokoFreezeTileAlpha / 100.0f);
	if(g_Config.m_ClYablokoFreezeTileColor <= 0)
		Color = ColorRGBA(0.35f, 0.65f, 1.0f, g_Config.m_ClYablokoFreezeTileAlpha / 100.0f);

	float Time = Client()->GlobalTime();
	float Pulse = 1.0f;
	if(g_Config.m_ClYablokoFreezeTilePulse)
		Pulse = 0.8f + 0.2f * std::sin(Time * (g_Config.m_ClYablokoFreezeTilePulseSpeed / 10.0f));

	Graphics()->BlendAdditive();
	int TileSize = 32;
	int StartX = (int)(ScreenMin.x / TileSize) - 1;
	int StartY = (int)(ScreenMin.y / TileSize) - 1;
	int EndX = (int)(ScreenMax.x / TileSize) + 1;
	int EndY = (int)(ScreenMax.y / TileSize) + 1;
	for(int y = StartY; y < EndY; y++)
	{
		for(int x = StartX; x < EndX; x++)
		{
			vec2 TilePos = vec2(x * TileSize + TileSize / 2.0f, y * TileSize + TileSize / 2.0f);
			// distance to local player limits glow range
			float Dist = distance(TilePos, GameClient()->m_LocalCharacterPos);
			if(Dist > Radius)
				continue;
			if(GameClient()->Collision()->GetTileIndex(GameClient()->Collision()->GetPureMapIndex(TilePos)) == TILE_FREEZE)
			{
				float Fade = 1.0f - Dist / Radius;
				Graphics()->SetColor(Color.r, Color.g, Color.b, Color.a * Fade * Pulse);
				Graphics()->DrawCircle(TilePos.x, TilePos.y, TileSize * 0.55f * Pulse, 10);
			}
		}
	}
	Graphics()->BlendNone();
}

void CYablokoDynamics::RenderPlasma()
{
	if(!g_Config.m_ClYablokoPlasma)
		return;
	if(!GameClient()->m_Snap.m_pLocalCharacter)
		return;

	vec2 Pos = GameClient()->m_LocalCharacterPos;
	float Speed = length(vec2(GameClient()->m_Snap.m_pLocalCharacter->m_VelX / 256.0f, GameClient()->m_Snap.m_pLocalCharacter->m_VelY / 256.0f));
	float Activation = g_Config.m_ClYablokoPlasmaSpeed / 100.0f * 10.0f;
	if(Speed < Activation)
		return;

	float Alpha = g_Config.m_ClYablokoPlasmaAlpha / 100.0f;
	ColorRGBA Color = Yabloko::Col(g_Config.m_ClYablokoPlasmaColor, Alpha);
	if(g_Config.m_ClYablokoPlasmaColor <= 0)
		Color = ColorRGBA(0.3f, 0.9f, 1.0f, Alpha);
	if(g_Config.m_ClYablokoPlasmaSpeedColor)
	{
		float T = std::clamp((Speed - Activation) / 5.0f, 0.0f, 1.0f);
		Color = ColorRGBA(Color.r, Color.g + (1.0f - Color.g) * T, Color.b, Alpha);
	}

	float Size = g_Config.m_ClYablokoPlasmaSize / 2.0f;
	float Time = Client()->GlobalTime();

	Graphics()->BlendAdditive();
	for(int i = 0; i < 6; i++)
	{
		float Angle = Time * 2.5f + i * 2.0f * pi / 6;
		vec2 P = Pos + direction(Angle) * Size;
		Graphics()->SetColor(Color.r, Color.g, Color.b, Alpha * 0.5f);
		Graphics()->DrawCircle(P.x, P.y, 4.0f, 8);
		// arcs toward next orb
		vec2 Next = Pos + direction(Angle + 2.0f * pi / 6) * Size;
		int Segments = g_Config.m_ClYablokoPlasmaWidth;
		for(int s = 1; s < Segments; s++)
		{
			float t = s / (float)Segments;
			vec2 LineP = P + (Next - P) * t;
			Graphics()->SetColor(Color.r, Color.g, Color.b, Alpha * 0.35f);
			Graphics()->DrawCircle(LineP.x, LineP.y, 1.2f, 6);
		}
	}
	Graphics()->BlendNone();
}

void CYablokoDynamics::RenderTesla()
{
	if(!g_Config.m_ClYablokoTeslaHook)
		return;
	if(!GameClient()->m_Snap.m_pLocalCharacter)
		return;

	const CNetObj_Character *pChar = GameClient()->m_Snap.m_pLocalCharacter;
	if(pChar->m_HookState <= 0)
		return;

	vec2 Pos = GameClient()->m_LocalCharacterPos;
	vec2 HookPos = vec2(pChar->m_HookX, pChar->m_HookY);
	float Time = Client()->GlobalTime();
	float Alpha = g_Config.m_ClYablokoTeslaAlpha / 100.0f;
	ColorRGBA Color = Yabloko::Col(g_Config.m_ClYablokoTeslaColor, Alpha);
	if(g_Config.m_ClYablokoTeslaColor <= 0)
		Color = ColorRGBA(0.4f, 0.8f, 1.0f, Alpha);

	int Segments = std::clamp(g_Config.m_ClYablokoTeslaSegments, 2, 64);
	float Amplitude = g_Config.m_ClYablokoTeslaAmplitude / 10.0f;
	float Thickness = g_Config.m_ClYablokoTeslaWidth / 10.0f;

	Graphics()->BlendAdditive();
	vec2 Prev = Pos;
	for(int i = 1; i <= Segments; i++)
	{
		float t = i / (float)Segments;
		vec2 LinePos = Pos + (HookPos - Pos) * t;
		vec2 Perp = normalize(HookPos - Pos + vec2(0.001f, 0.0f));
		Perp = vec2(-Perp.y, Perp.x);
		float Zigzag = ((i % 2) * 2 - 1) * Amplitude * (1.0f - t) * std::sin(Time * g_Config.m_ClYablokoTeslaSpeed + i * 1.7f);
		vec2 Point = LinePos + Perp * Zigzag;
		Graphics()->SetColor(Color.r, Color.g, Color.b, Alpha * (1.0f - t * 0.4f));
		Graphics()->DrawCircle(Point.x, Point.y, Thickness, 6);
		Prev = Point;
	}

	// sparks at contact
	if(g_Config.m_ClYablokoTeslaSparks > 0)
	{
		for(int i = 0; i < g_Config.m_ClYablokoTeslaSparks / 4; i++)
		{
			float Angle = Time * 9.0f + i * 2.4f;
			vec2 P = HookPos + direction(Angle) * (8.0f + 6.0f * std::sin(Time * 13.0f + i));
			Graphics()->SetColor(1.0f, 1.0f, 1.0f, Alpha * 0.6f);
			Graphics()->DrawCircle(P.x, P.y, 1.5f, 6);
		}
	}
	Graphics()->BlendNone();
}

void CYablokoDynamics::SpawnShatter(vec2 Pos, const ColorRGBA &Color)
{
	int Count = std::clamp(g_Config.m_ClYablokoShatterCount, 2, 64);
	for(int i = 0; i < Count; i++)
	{
		SShard Shard;
		Shard.m_Pos = Pos + vec2(random_float() * 16.0f - 8.0f, random_float() * 16.0f - 8.0f);
		float Angle = random_float() * 2.0f * pi;
		float Speed = random_float() * g_Config.m_ClYablokoShatterSpeed / 100.0f * 400.0f;
		Shard.m_Vel = direction(Angle) * Speed;
		Shard.m_Time = 0.0f;
		Shard.m_Lifetime = g_Config.m_ClYablokoShatterLifetime / 1000.0f;
		Shard.m_Size = g_Config.m_ClYablokoShatterSize / 4.0f + random_float() * 2.0f;
		m_vShards.push_back(Shard);
		(void)Color;
	}
}

void CYablokoDynamics::RenderShatter()
{
	if(m_vShards.empty())
		return;
	float dt = 1.0f / 60.0f;
	float Alpha = g_Config.m_ClYablokoShatterAlpha / 100.0f;
	float Bounce = g_Config.m_ClYablokoShatterBounce / 100.0f;

	Graphics()->BlendNormal();
	m_vShards.erase(std::remove_if(m_vShards.begin(), m_vShards.end(),
			    [&](SShard &S) {
				    S.m_Time += dt;
				    S.m_Vel.y += 900.0f * dt; // gravity
				    S.m_Pos += S.m_Vel * dt;
				    if(S.m_Vel.y > 0 && GameClient()->Collision()->CheckPoint(S.m_Pos))
				    {
					    S.m_Vel.y = -S.m_Vel.y * Bounce;
					    S.m_Vel.x *= 0.7f;
				    }
				    return S.m_Time >= S.m_Lifetime;
			    }),
		m_vShards.end());

	for(const auto &S : m_vShards)
	{
		float Fade = 1.0f - S.m_Time / S.m_Lifetime;
		Graphics()->SetColor(0.85f, 0.95f, 1.0f, Alpha * Fade);
		IGraphics::CQuadItem Shard(S.m_Pos.x - S.m_Size / 2, S.m_Pos.y - S.m_Size / 2, S.m_Size, S.m_Size);
		Graphics()->QuadsBegin();
		Graphics()->QuadsDraw(&Shard, 1);
		Graphics()->QuadsEnd();
	}
}

void CYablokoDynamics::RenderAfterburner()
{
	if(!g_Config.m_ClYablokoAfterburner)
		return;
	if(!GameClient()->m_Snap.m_pLocalCharacter)
		return;

	// detect rocket jump: sudden speed increase right after an explosion
	vec2 Vel = vec2(GameClient()->m_Snap.m_pLocalCharacter->m_VelX / 256.0f, GameClient()->m_Snap.m_pLocalCharacter->m_VelY / 256.0f);
	float Speed = length(Vel);
	float BoostThreshold = g_Config.m_ClYablokoAfterburnerBoost / 10.0f;
	if(m_LastBoostSpeed > 0 && Speed - m_LastBoostSpeed > BoostThreshold)
	{
		for(int i = 0; i < 8; i++)
		{
			SFlame F;
			F.m_Pos = GameClient()->m_LocalCharacterPos + vec2(random_float() * 10.0f - 5.0f, random_float() * 10.0f - 5.0f);
			F.m_Vel = -Vel * 0.5f + vec2(random_float() * 60.0f - 30.0f, random_float() * 60.0f - 30.0f);
			F.m_Time = 0.0f;
			m_vFlames.push_back(F);
		}
	}
	m_LastBoostSpeed = Speed;

	float dt = 1.0f / 60.0f;
	float Alpha = g_Config.m_ClYablokoAfterburnerAlpha / 100.0f;
	float Lifetime = g_Config.m_ClYablokoAfterburnerLifetime / 1000.0f;
	float Size = g_Config.m_ClYablokoAfterburnerSize / 4.0f;
	ColorRGBA Color = Yabloko::Col(g_Config.m_ClYablokoAfterburnerColor, Alpha);
	if(g_Config.m_ClYablokoAfterburnerColor <= 0)
		Color = ColorRGBA(1.0f, 0.55f, 0.15f, Alpha);

	Graphics()->BlendAdditive();
	m_vFlames.erase(std::remove_if(m_vFlames.begin(), m_vFlames.end(),
			    [&](SFlame &F) {
				    F.m_Time += dt;
				    F.m_Vel *= 0.9f;
				    F.m_Pos += F.m_Vel * dt;
				    return F.m_Time >= Lifetime;
			    }),
		m_vFlames.end());

	for(const auto &F : m_vFlames)
	{
		float Fade = 1.0f - F.m_Time / Lifetime;
		Graphics()->SetColor(Color.r, Color.g, Color.b, Alpha * Fade * 0.7f);
		Graphics()->DrawCircle(F.m_Pos.x, F.m_Pos.y, Size * (0.5f + Fade * 0.5f), 8);
	}
	Graphics()->BlendNone();
}
