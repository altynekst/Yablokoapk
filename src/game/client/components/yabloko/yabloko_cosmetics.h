/* Yabloko client — reconstructed from the surviving build.
   Cosmetic world effects: glow, snow, orbs, jelly, tentacle hook render data. */

#ifndef GAME_CLIENT_COMPONENTS_YABLOKO_YABLOKO_COSMETICS_H
#define GAME_CLIENT_COMPONENTS_YABLOKO_YABLOKO_COSMETICS_H

#include <engine/graphics.h>

#include <game/client/component.h>

#include <game/client/components/yabloko/yabloko.h>

class CYablokoCosmetics : public CComponent
{
public:
	int Sizeof() const override { return sizeof(*this); }

	void OnReset() override;
	void OnInit() override;
	void OnRender() override;

	// Falling snow particles
	struct SSnowParticle
	{
		vec2 m_Pos;
		vec2 m_Vel;
		float m_Size;
		float m_Life;
	};
	std::vector<SSnowParticle> m_vSnow;

	// Rising snow (cosmetic snowflakes rising around the local tee)
	struct SRisingSnow
	{
		vec2 m_Pos;
		float m_Seed;
		float m_Life;
	};
	std::vector<SRisingSnow> m_vRisingSnow;

	// Movement trail history for the local tee
	struct STrailPoint
	{
		vec2 m_Pos;
		float m_Time;
	};
	std::vector<STrailPoint> m_vTrail;

	// Jelly squash state
	float m_JellySquash;
	float m_JellyVel;

	// Death / spawn screen effect state
	float m_DeathEffectEnd;
	float m_SpawnEffectEnd;
	int m_LastLocalAlive;
	int m_SpawnEffectStyle;

	// Grenade tracer history: projectile id -> last positions
	struct SGrenadeTrail
	{
		vec2 m_Pos;
		float m_Time;
	};
	std::vector<SGrenadeTrail> m_vGrenadeTrail;
	int m_aLastGrenadeCount[2];

	// Eye laser state
	bool m_EyeLaserActive;
	float m_EyeLaserWarm;
	float m_EyeLaserExtendT;
	bool m_EyeLaserPrevKey;

	// Hammer wave state
	float m_HammerWaveTime;
	vec2 m_HammerWavePos;
	int m_LastFireState;

	// Tentacle hook texture
	IGraphics::CTextureHandle m_TentacleHookTexture;

	void RenderWorldEffects();
	void RenderTeeEffects(int ClientId, vec2 Pos, bool Local);
	void RenderHookEffects(int ClientId, vec2 Pos, vec2 HookPos, float Alpha);
	void RenderTentacleHook(int ClientId, vec2 Pos, vec2 HookPos, float Alpha);
	void RenderHookGlow(vec2 Pos, vec2 HookPos, float Alpha);
	void RenderScreenEffects();

	// Jelly scale factor for tee rendering (1.0 = none)
	float GetJellyScale(int ClientId, bool Local) const;

	// Hide/alpha helpers used by CPlayers
	bool ShouldHidePlayer(int ClientId) const;
	float GetPlayerAlpha(int ClientId) const;

private:
	void RenderSnow();
	void RenderRisingSnow(int ClientId, vec2 Pos, bool Local);
	void RenderOrbs(int ClientId, vec2 Pos, bool Local);
	void RenderBodyGlow(int ClientId, vec2 Pos, bool Local);
	void RenderFreezeGlow(int ClientId, vec2 Pos, bool Local);
	void RenderHat(int ClientId, vec2 Pos, bool Local);
	void RenderEyeLaser();
	void RenderTrail();
	void RenderGhostDuplicate();
	void RenderGrenadeTrails();
	void RenderHammerWave();
	void RenderArrival(int ClientId, vec2 Pos, bool Local);
	void UpdateJelly();
};

#endif
