/* Yabloko client — reconstructed from the surviving build.
   Cosmetic world effects implementation. */

#include "yabloko_cosmetics.h"

#include <engine/shared/config.h>

#include <game/client/animstate.h>
#include <game/client/components/players.h>
#include <game/client/gameclient.h>

#include <algorithm>

void CYablokoCosmetics::OnReset()
{
	m_vSnow.clear();
	m_vRisingSnow.clear();
	m_vTrail.clear();
	m_vGrenadeTrail.clear();
	m_JellySquash = 1.0f;
	m_JellyVel = 0.0f;
	m_DeathEffectEnd = -1.0f;
	m_SpawnEffectEnd = -1.0f;
	m_LastLocalAlive = 0;
	m_SpawnEffectStyle = 0;
	m_EyeLaserActive = false;
	m_EyeLaserWarm = 0.0f;
	m_EyeLaserExtendT = 0.0f;
	m_EyeLaserPrevKey = false;
	m_HammerWaveTime = -1.0f;
	m_LastFireState = 0;
	mem_zero(m_aLastGrenadeCount, sizeof(m_aLastGrenadeCount));
}

void CYablokoCosmetics::OnInit()
{
	// the tentacle hook texture from the build data (optional file)
	m_TentacleHookTexture = Graphics()->LoadTexture("tentacle_hook.png", IStorage::TYPE_ALL);
}

void CYablokoCosmetics::OnRender()
{
	if(Client()->State() != IClient::STATE_ONLINE && Client()->State() != IClient::STATE_DEMOPLAYBACK)
		return;

	UpdateJelly();

	// death / spawn detection for screen effects
	bool LocalAlive = GameClient()->m_Snap.m_pLocalCharacter != nullptr;
	if(!LocalAlive && m_LastLocalAlive && g_Config.m_ClDeathEffect)
	{
		m_DeathEffectEnd = Client()->GlobalTime() + g_Config.m_ClDeathEffectDuration / 1000.0f;
	}
	if(LocalAlive && !m_LastLocalAlive && g_Config.m_ClSpawnEffect)
	{
		m_SpawnEffectEnd = Client()->GlobalTime() + 0.8f;
		m_SpawnEffectStyle = g_Config.m_ClSpawnEffect;
	}
	m_LastLocalAlive = LocalAlive ? 1 : 0;

	// track local movement trail
	if(LocalAlive && g_Config.m_ClYablokoGhostTrail)
	{
		vec2 Pos = GameClient()->m_LocalCharacterPos;
		if(m_vTrail.empty() || distance(m_vTrail.back().m_Pos, Pos) > 8.0f)
		{
			m_vTrail.push_back({Pos, Client()->GlobalTime()});
			if(m_vTrail.size() > 256)
				m_vTrail.erase(m_vTrail.begin());
		}
	}
	else if(m_vTrail.size() > 256)
		m_vTrail.erase(m_vTrail.begin(), m_vTrail.begin() + (m_vTrail.size() - 256));

	// eye laser activation follows the configured bind key
	if(g_Config.m_ClYablokoEyeLaser)
	{
		bool Key = GameClient()->m_YablokoExtras.m_EyeLaserBindActive || Input()->KeyIsPressed(g_Config.m_ClYablokoEyeLaserKey);
		if(Key && !m_EyeLaserPrevKey)
			m_EyeLaserActive = true;
		else if(!Key)
			m_EyeLaserActive = false;
		m_EyeLaserPrevKey = Key;
	}
	else
		m_EyeLaserActive = false;
}

void CYablokoCosmetics::UpdateJelly()
{
	if(!g_Config.m_ClJellyEnabled)
	{
		m_JellySquash = 1.0f;
		return;
	}
	bool Hooked = false;
	if(GameClient()->m_Snap.m_pLocalCharacter && GameClient()->m_Snap.m_pLocalCharacter->m_HookState > 0)
		Hooked = true;
	// squash when hooked or landing
	float Target = 1.0f;
	if(Hooked)
		Target = 1.0f - g_Config.m_ClJellyStrength / 400.0f;
	m_JellyVel += (Target - m_JellySquash) * 0.35f;
	m_JellyVel *= 0.75f;
	m_JellySquash += m_JellyVel;
}

float CYablokoCosmetics::GetJellyScale(int ClientId, bool Local) const
{
	(void)ClientId;
	(void)Local;
	if(!g_Config.m_ClJellyEnabled)
		return 1.0f;
	return m_JellySquash;
}

bool CYablokoCosmetics::ShouldHidePlayer(int ClientId) const
{
	(void)ClientId;
	return g_Config.m_ClYablokoHidePlayers != 0;
}

float CYablokoCosmetics::GetPlayerAlpha(int ClientId) const
{
	(void)ClientId;
	return g_Config.m_ClYablokoPlayersAlpha / 100.0f;
}

void CYablokoCosmetics::RenderSnow()
{
	if(!g_Config.m_ClSnow || Client()->State() != IClient::STATE_ONLINE)
		return;

	float Time = Client()->GlobalTime();
	float dt = 1.0f / 60.0f;
	float Density = g_Config.m_ClSnowDensity;
	int Wanted = (int)(Density * 1.5f);
	vec2 ScreenMin, ScreenMax;
	Graphics()->GetScreen(&ScreenMin.x, &ScreenMin.y, &ScreenMax.x, &ScreenMax.y);
	(void)ScreenMin;
	(void)ScreenMax;

	while((int)m_vSnow.size() < Wanted)
	{
		SSnowParticle P;
		P.m_Pos = vec2(random_float() * (ScreenMax.x - ScreenMin.x) + ScreenMin.x,
			ScreenMin.y - 20.0f);
		P.m_Vel = vec2(random_float() * 40.0f - 20.0f, 80.0f + random_float() * 60.0f);
		P.m_Size = 1.5f + random_float() * 3.0f;
		P.m_Life = random_float() * 10.0f;
		m_vSnow.push_back(P);
	}
	while((int)m_vSnow.size() > Wanted + 32)
		m_vSnow.erase(m_vSnow.begin());

	Graphics()->BlendNormal();
	for(auto &P : m_vSnow)
	{
		P.m_Pos += P.m_Vel * dt;
		P.m_Pos.x += std::sin(Time * 2.0f + P.m_Life) * 12.0f * dt;
		if(P.m_Pos.y > ScreenMax.y + 20.0f)
		{
			P.m_Pos.y = ScreenMin.y - 20.0f;
			P.m_Pos.x = random_float() * (ScreenMax.x - ScreenMin.x) + ScreenMin.x;
		}
		if(P.m_Pos.x < ScreenMin.x - 20.0f)
			P.m_Pos.x = ScreenMax.x + 20.0f;
		if(P.m_Pos.x > ScreenMax.x + 20.0f)
			P.m_Pos.x = ScreenMin.x - 20.0f;

		float Alpha = std::clamp(1.0f - P.m_Life / 12.0f, 0.1f, 1.0f) * (g_Config.m_ClSnowIntensity / 100.0f);
		IGraphics::CQuadItem Quad(P.m_Pos.x, P.m_Pos.y, P.m_Size, P.m_Size);
		Graphics()->SetColor(1.0f, 1.0f, 1.0f, Alpha * 0.9f);
		Graphics()->QuadsBegin();
		Graphics()->QuadsDraw(&Quad, 1);
		Graphics()->QuadsEnd();
	}
}

void CYablokoCosmetics::RenderTeeEffects(int ClientId, vec2 Pos, bool Local)
{
	bool LocalAlive = GameClient()->m_Snap.m_pLocalCharacter != nullptr;
	bool Self = Local;
	bool Teammate = !Local && GameClient()->m_Snap.m_LocalClientId >= 0 && GameClient()->m_Teams.SameTeam(GameClient()->m_Snap.m_LocalClientId, ClientId);
	bool SpecTeammate = GameClient()->IsOtherTeam(ClientId);

	float Alpha = SpecTeammate ? g_Config.m_ClShowOthersAlpha / 100.0f : 1.0f;
	if(Alpha <= 0.01f)
		return;

	RenderOrbs(ClientId, Pos, Local);
	RenderRisingSnow(ClientId, Pos, Local);
	RenderBodyGlow(ClientId, Pos, Local);
	RenderFreezeGlow(ClientId, Pos, Local);
	RenderHat(ClientId, Pos, Local);
	RenderArrival(ClientId, Pos, Local);

	(void)LocalAlive;
	(void)Self;
	(void)Teammate;
	(void)Alpha;
}

void CYablokoCosmetics::RenderHookEffects(int ClientId, vec2 Pos, vec2 HookPos, float Alpha)
{
	if(g_Config.m_ClTentacle)
		RenderTentacleHook(ClientId, Pos, HookPos, Alpha);
	else if(g_Config.m_KrxCustomcolorhook && !g_Config.m_KrxCustomcolorhookrainbow)
		RenderHookGlow(Pos, HookPos, Alpha);
}

void CYablokoCosmetics::RenderTentacleHook(int ClientId, vec2 Pos, vec2 HookPos, float Alpha)
{
	(void)ClientId;
	if(!m_TentacleHookTexture.IsValid())
		return;

	float Time = Client()->GlobalTime();
	vec2 Dir = normalize(HookPos - Pos + vec2(0.001f, 0.0f));
	float Length = distance(Pos, HookPos);
	int Quality = g_Config.m_ClTentacleQuality == 0 ? 6 : (g_Config.m_ClTentacleQuality == 1 ? 12 : 24);
	float Size = 24.0f * g_Config.m_ClTentacleSize / 100.0f;
	float Waves = g_Config.m_ClTentacleWaves / 100.0f * 6.0f;
	ColorRGBA Color = GameClient()->m_aClients[std::clamp(ClientId, 0, MAX_CLIENTS - 1)].m_RenderInfo.m_ColorBody;
	float Glow = g_Config.m_ClTentacleGlow / 100.0f;

	vec2 Perp = vec2(-Dir.y, Dir.x);
	Graphics()->TextureSet(m_TentacleHookTexture);
	Graphics()->BlendNormal();
	for(int i = 0; i < Quality; i++)
	{
		float t = (i + 0.5f) / Quality;
		vec2 P = HookPos + Dir * (Length * t);
		float WaveOffset = std::sin(t * Waves * 2.0f * pi + Time * 4.0f) * Waves * 3.0f * t;
		P += Perp * WaveOffset;
		float SegSize = Size * (1.0f - t * 0.35f);
		float a = Alpha * (0.4f + 0.6f * (1.0f - t * 0.3f));
		Graphics()->SetColor(Color.r, Color.g, Color.b, a);
		Graphics()->QuadsSetRotation(std::atan2(Dir.y, Dir.x));
		IGraphics::CQuadItem Quad(P.x, P.y, SegSize, SegSize);
		Graphics()->QuadsBegin();
		Graphics()->QuadsDraw(&Quad, 1);
		Graphics()->QuadsEnd();
	}
	Graphics()->QuadsSetRotation(0);

	// kagune glow
	if(Glow > 0)
	{
		Graphics()->BlendAdditive();
		for(int i = 0; i < Quality; i += 2)
		{
			float t = (i + 0.5f) / Quality;
			vec2 P = HookPos + Dir * (Length * t);
			float WaveOffset = std::sin(t * Waves * 2.0f * pi + Time * 4.0f) * Waves * 3.0f * t;
			P += Perp * WaveOffset;
			Graphics()->SetColor(Color.r, Color.g, Color.b, Alpha * Glow * 0.2f);
			Graphics()->DrawCircle(P.x, P.y, 14.0f * (1.0f - t * 0.3f), 8);
		}
		Graphics()->BlendNone();
	}
}

void CYablokoCosmetics::RenderHookGlow(vec2 Pos, vec2 HookPos, float Alpha)
{
	// KRX custom hook color: additive glow along the chain
	ColorRGBA Color = GameClient()->m_YablokoExtras.HookColor();
	vec2 Dir = normalize(HookPos - Pos + vec2(0.001f, 0.0f));
	float Length = distance(Pos, HookPos);
	int Steps = std::clamp((int)(Length / 24.0f), 2, 48);
	Graphics()->BlendAdditive();
	for(int i = 0; i <= Steps; i++)
	{
		float t = i / (float)Steps;
		vec2 P = Pos + Dir * (Length * t);
		Graphics()->SetColor(Color.r, Color.g, Color.b, Alpha * 0.25f);
		Graphics()->DrawCircle(P.x, P.y, 7.0f, 8);
	}
	Graphics()->BlendNone();
}

void CYablokoCosmetics::RenderOrbs(int ClientId, vec2 Pos, bool Local)
{
	(void)ClientId;
	if(!g_Config.m_ClOrbs || !Local)
		return;
	if(!g_Config.m_ClYablokoOrbCount && !g_Config.m_ClOrbs)
		return;

	float Time = Client()->GlobalTime();
	ColorRGBA Color = Yabloko::Col(g_Config.m_ClYablokoOrbColor, 1.0f);
	float Light = g_Config.m_ClYablokoOrbLight / 100.0f;
	if(g_Config.m_ClYablokoOrbLight <= 0)
		Color = ColorRGBA(0.9f, 1.0f, 0.9f, 1.0f);
	(void)Light;

	Graphics()->BlendNormal();
	for(int i = 0; i < g_Config.m_ClYablokoOrbCount; i++)
	{
		float Angle = Time * (g_Config.m_ClYablokoOrbSpeed / 100.0f) + i * 2.0f * pi / g_Config.m_ClYablokoOrbCount;
		vec2 OrbPos = Pos + vec2(std::sin(Angle), -std::cos(Angle)) * (g_Config.m_ClYablokoOrbRadius / 32.0f);
		float Size = g_Config.m_ClYablokoOrbSize / 16.0f;

		// soft glow layers
		for(int Layer = 3; Layer >= 1; Layer--)
		{
			float LayerSize = Size * (1.0f + Layer * 0.6f);
			float LayerAlpha = 0.10f * (g_Config.m_ClYablokoOrbDensity / 100.0f) / Layer;
			Graphics()->SetColor(Color.r, Color.g, Color.b, LayerAlpha);
			Graphics()->DrawCircle(OrbPos.x, OrbPos.y, LayerSize, 12);
		}
		Graphics()->SetColor(Color.r, Color.g, Color.b, 0.8f);
		Graphics()->DrawCircle(OrbPos.x, OrbPos.y, Size * 0.5f, 10);
	}
}

void CYablokoCosmetics::RenderRisingSnow(int ClientId, vec2 Pos, bool Local)
{
	(void)ClientId;
	if(!g_Config.m_ClYablokoRisingSnow || !Local)
		return;

	float Time = Client()->GlobalTime();
	ColorRGBA Color = Yabloko::Col(g_Config.m_ClYablokoRisingSnowColor);
	float Alpha = g_Config.m_ClYablokoRisingSnowDensity / 100.0f;

	if((int)m_vRisingSnow.size() != g_Config.m_ClYablokoRisingSnowCount)
	{
		m_vRisingSnow.clear();
		for(int i = 0; i < g_Config.m_ClYablokoRisingSnowCount; i++)
		{
			SRisingSnow S;
			S.m_Pos = Pos + vec2(random_float() * 300.0f - 150.0f, random_float() * 300.0f - 150.0f);
			S.m_Seed = random_float();
			S.m_Life = random_float();
			m_vRisingSnow.push_back(S);
		}
	}

	Graphics()->BlendNormal();
	for(auto &S : m_vRisingSnow)
	{
		float Life = 4.0f + S.m_Seed * 3.0f;
		float Phase = std::fmod(Time / Life + S.m_Life, 1.0f);
		vec2 P = S.m_Pos + vec2(std::sin(Time * 1.5f + S.m_Seed * 10.0f) * 20.0f, -Phase * 200.0f * (g_Config.m_ClYablokoRisingSnowSpeed / 100.0f));
		float Size = g_Config.m_ClYablokoRisingSnowSize / 8.0f;
		float Pulse = 1.0f + std::sin(Time * g_Config.m_ClYablokoRisingSnowPulses + S.m_Seed * 6.28f) * 0.2f;
		float Fade = std::sin(Phase * pi);
		Graphics()->SetColor(Color.r, Color.g, Color.b, Alpha * Fade * (g_Config.m_ClYablokoRisingSnowBrightness / 100.0f));
		Graphics()->DrawCircle(P.x, P.y, Size * Pulse, 8);
	}
}

void CYablokoCosmetics::RenderBodyGlow(int ClientId, vec2 Pos, bool Local)
{
	(void)ClientId;
	if(g_Config.m_ClYablokoBodyGlow <= 0 || !Local)
		return;

	float Strength = g_Config.m_ClYablokoBodyGlow / 100.0f;
	float Radius = 48.0f * g_Config.m_ClYablokoBodyGlowSize / 100.0f;
	ColorRGBA Color = Yabloko::Col(g_Config.m_ClYablokoBodyGlowColor);
	float Density = g_Config.m_ClYablokoBodyGlowDensity / 100.0f;

	Graphics()->BlendAdditive();
	for(int i = 4; i >= 1; i--)
	{
		float R = Radius * i / 4.0f;
		float A = Strength * (0.16f / i) * (0.5f + Density * 0.5f);
		Graphics()->SetColor(Color.r, Color.g, Color.b, A);
		Graphics()->DrawCircle(Pos.x, Pos.y + 4.0f, R, 16);
	}
	Graphics()->BlendNone();
}

void CYablokoCosmetics::RenderFreezeGlow(int ClientId, vec2 Pos, bool Local)
{
	(void)Local;
	if(g_Config.m_ClYablokoFreezeGlow <= 0)
		return;

	auto &ClientData = GameClient()->m_aClients[ClientId];
	if(ClientData.m_Predicted.m_FreezeEnd <= 0 || !GameClient()->m_Snap.m_aCharacters[ClientId].m_HasExtendedDisplayInfo)
		return;

	float Strength = g_Config.m_ClYablokoFreezeGlow / 100.0f;
	ColorRGBA Color = Yabloko::Col(g_Config.m_ClYablokoFreezeGlowColor);
	float MixFactor = g_Config.m_ClYablokoFreezeGlowColorMix / 100.0f;
	ColorRGBA FreezeBlue = ColorRGBA(0.35f, 0.65f, 1.0f, 1.0f);
	ColorRGBA Final = ColorRGBA(FreezeBlue.r + (Color.r - FreezeBlue.r) * MixFactor,
		FreezeBlue.g + (Color.g - FreezeBlue.g) * MixFactor,
		FreezeBlue.b + (Color.b - FreezeBlue.b) * MixFactor, 1.0f);

	Graphics()->BlendAdditive();
	for(int i = 3; i >= 1; i--)
	{
		float R = 40.0f * i;
		float A = Strength * (0.12f / i) * (g_Config.m_ClYablokoFreezeGlowStrength / 100.0f);
		Graphics()->SetColor(Final.r, Final.g, Final.b, A);
		Graphics()->DrawCircle(Pos.x, Pos.y + 4.0f, R, 16);
	}
	Graphics()->BlendNone();
}

void CYablokoCosmetics::RenderHat(int ClientId, vec2 Pos, bool Local)
{
	(void)ClientId;
	(void)Local;
	if(!g_Config.m_ClYablokoHat)
		return;

	float Alpha = g_Config.m_ClYablokoHatAlpha / 100.0f;
	if(Alpha <= 0.01f)
		return;

	ColorRGBA Color = Yabloko::Col(g_Config.m_ClYablokoHatColor, Alpha);
	float Width = g_Config.m_ClYablokoHatWidth / 8.0f;
	float Height = g_Config.m_ClYablokoHatHeight / 8.0f;
	float Tilt = g_Config.m_ClYablokoHatTilt / 100.0f;
	float Offset = g_Config.m_ClYablokoHatOffset / 16.0f;
	float Time = Client()->GlobalTime();

	Graphics()->BlendNormal();
	Graphics()->SetColor(Color.r, Color.g, Color.b, Color.a);
	// draw hat as rounded shape above the tee with tilt wobble
	float Wobble = g_Config.m_ClYablokoHatJelly ? std::sin(Time * 6.0f) * 2.0f : 0.0f;
	vec2 HatPos = Pos + vec2(Tilt * 10.0f, -28.0f - Offset + Wobble);
	IGraphics::CQuadItem Quad(HatPos.x - Width / 2, HatPos.y - Height / 2, Width, Height);
	Graphics()->QuadsBegin();
	Graphics()->QuadsDraw(&Quad, 1);
	Graphics()->QuadsEnd();
	// brim
	IGraphics::CQuadItem Brim(HatPos.x - Width / 2 - 4.0f, HatPos.y + Height / 2 - 2.0f, Width + 8.0f, 3.0f);
	Graphics()->SetColor(Color.r * 0.8f, Color.g * 0.8f, Color.b * 0.8f, Color.a);
	Graphics()->QuadsBegin();
	Graphics()->QuadsDraw(&Brim, 1);
	Graphics()->QuadsEnd();
}

void CYablokoCosmetics::RenderEyeLaser()
{
	if(!g_Config.m_ClYablokoEyeLaser)
		return;
	(void)m_EyeLaserActive;
	(void)m_EyeLaserWarm;
	(void)m_EyeLaserExtendT;
	(void)m_EyeLaserPrevKey;
}

void CYablokoCosmetics::RenderTrail()
{
	(void)m_vTrail;
}

void CYablokoCosmetics::RenderGhostDuplicate()
{
}

void CYablokoCosmetics::RenderGrenadeTrails()
{
	(void)m_vGrenadeTrail;
	(void)m_aLastGrenadeCount;
}

void CYablokoCosmetics::RenderHammerWave()
{
	(void)m_HammerWaveTime;
	(void)m_HammerWavePos;
	(void)m_LastFireState;
}

void CYablokoCosmetics::RenderArrival(int ClientId, vec2 Pos, bool Local)
{
	(void)ClientId;
	(void)Pos;
	(void)Local;
}

void CYablokoCosmetics::RenderWorldEffects()
{
	RenderSnow();
	RenderEyeLaser();
	RenderTrail();
	RenderGhostDuplicate();
	RenderGrenadeTrails();
	RenderHammerWave();
}

void CYablokoCosmetics::RenderScreenEffects()
{
	// death effect
	if(m_DeathEffectEnd > 0 && Client()->GlobalTime() < m_DeathEffectEnd)
	{
		float Progress = (m_DeathEffectEnd - Client()->GlobalTime()) / (g_Config.m_ClDeathEffectDuration / 1000.0f);
		float Alpha = std::clamp(Progress, 0.0f, 1.0f) * g_Config.m_ClDeathEffectStrength / 100.0f;
		vec2 ScreenMin, ScreenMax;
		Graphics()->GetScreen(&ScreenMin.x, &ScreenMin.y, &ScreenMax.x, &ScreenMax.y);
		float W = ScreenMax.x - ScreenMin.x;
		float H = ScreenMax.y - ScreenMin.y;
		// dark vignette pulse
		Graphics()->SetColor(0.8f, 0.0f, 0.0f, Alpha * 0.35f);
		IGraphics::CQuadItem Quad(ScreenMin.x, ScreenMin.y, W, H);
		Graphics()->BlendNormal();
		Graphics()->QuadsBegin();
		Graphics()->QuadsDraw(&Quad, 1);
		Graphics()->QuadsEnd();
	}
	if(m_SpawnEffectEnd > 0 && Client()->GlobalTime() < m_SpawnEffectEnd)
	{
		float Progress = (m_SpawnEffectEnd - Client()->GlobalTime()) / 0.8f;
		float Alpha = std::clamp(Progress, 0.0f, 1.0f) * 0.5f;
		vec2 Pos = GameClient()->m_LocalCharacterPos;
		Graphics()->BlendAdditive();
		ColorRGBA Color = ColorRGBA(0.9f, 1.0f, 0.9f, Alpha);
		for(int i = 3; i >= 1; i--)
		{
			Graphics()->SetColor(Color.r, Color.g, Color.b, Alpha * 0.2f / i);
			Graphics()->DrawCircle(Pos.x, Pos.y, 30.0f + i * 18.0f * (1.0f - Progress), 14);
		}
		(void)Color;
		Graphics()->BlendNone();
	}
}
