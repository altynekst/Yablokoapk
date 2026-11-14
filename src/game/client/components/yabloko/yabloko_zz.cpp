/* Yabloko client — reconstructed from the surviving build.
   ZZetClient-style gameplay assists implementation (basic reconstruction). */

#include "yabloko_zz.h"

#include <engine/shared/config.h>

#include <generated/protocol.h>

#include <game/client/gameclient.h>

#include <cmath>

void CYablokoZz::OnReset()
{
	m_AimbotTarget = -1;
	m_FakeAimAngle = 0.0f;
	m_FakeLagFlip = false;
	m_FakeLagTimer = 0.0f;
	m_CompanionEnabled = false;
}

void CYablokoZz::OnConsoleInit()
{
	Console()->Register("zz_toggle_aimbot", "", CFGFLAG_CLIENT, ConToggleAimbot, this, "Toggle the ZZetClient aimbot");
	Console()->Register("zz_force_zoom", "", CFGFLAG_CLIENT, ConForceZoomNotice, this, "Force zoom even if the server disallows it");
}

void CYablokoZz::ConToggleAimbot(IConsole::IResult *pResult, void *pUserData)
{
	(void)pResult;
	g_Config.m_ClZzAimbotEnabled ^= 1;
	static_cast<CYablokoZz *>(pUserData)->GameClient()->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "zz", g_Config.m_ClZzAimbotEnabled ? "aimbot enabled" : "aimbot disabled");
}

void CYablokoZz::ConForceZoomNotice(IConsole::IResult *pResult, void *pUserData)
{
	(void)pResult;
	(void)pUserData;
	// zoom is applied client-side; the force flag simply ignores the server forbid tune
	g_Config.m_ClZzForceZoom = 1;
	static_cast<CYablokoZz *>(pUserData)->GameClient()->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "zz", "Zoom is forced (ignores server forbid)");
}

int CYablokoZz::FindTarget(float FovDegrees, bool ClosestToPlayer) const
{
	if(!GameClient()->m_Snap.m_pLocalCharacter)
		return -1;

	vec2 Pos = GameClient()->m_LocalCharacterPos;
	vec2 AimDir = GameClient()->m_Controls.m_aMousePos[g_Config.m_ClDummy];
	if(length(AimDir) < 0.001f)
		AimDir = vec2(1, 0);
	AimDir = normalize(AimDir);

	float BestDist = 1e9f;
	int Best = -1;
	for(int i = 0; i < MAX_CLIENTS; i++)
	{
		if(!GameClient()->m_Snap.m_aCharacters[i].m_Active || i == GameClient()->m_Snap.m_LocalClientId)
			continue;
		if(g_Config.m_ClZzAimbotIgnoreFriends && GameClient()->m_aClients[i].m_Friend)
			continue;
		vec2 TargetPos = GameClient()->m_aClients[i].m_RenderPos;
		vec2 Dir = normalize(TargetPos - Pos);
		float Angle = std::acos(std::clamp(dot(Dir, AimDir), -1.0f, 1.0f)) * 180.0f / pi;
		if(Angle > FovDegrees)
			continue;
		float Dist = ClosestToPlayer ? distance(TargetPos, Pos) : Angle;
		if(Dist < BestDist)
		{
			BestDist = Dist;
			Best = i;
		}
	}
	return Best;
}

void CYablokoZz::OnUpdate()
{
	// aimbot target selection
	m_AimbotTarget = -1;
	if(g_Config.m_ClZzAimbotEnabled)
		m_AimbotTarget = FindTarget(g_Config.m_ClZzAimbotFov, g_Config.m_ClZzAimbotTargetMode == 1);

	// fake lag: alternate A/D while moving
	if(g_Config.m_ClZzFakeLag)
	{
		m_FakeLagTimer += 1.0f / 60.0f;
		if(m_FakeLagTimer > 0.5f)
		{
			m_FakeLagTimer = 0.0f;
			m_FakeLagFlip = !m_FakeLagFlip;
		}
	}

	// fake aim: spin the visual cursor
	if(g_Config.m_ClZzFakeAimEnabled)
	{
		float Speed = g_Config.m_ClZzFakeAimSpinSpeed / 100.0f;
		m_FakeAimAngle += Speed * 6.0f / 60.0f;
		if(g_Config.m_ClZzFakeAimMode == 3)
			m_FakeAimAngle += random_float() * 2.0f;
	}
}

void CYablokoZz::ApplyInput(CNetObj_PlayerInput &InputData, bool Local)
{
	if(!Local)
		return;

	// aimbot: aim at the selected target
	if(g_Config.m_ClZzAimbotEnabled && m_AimbotTarget >= 0 &&
		m_AimbotTarget < MAX_CLIENTS && GameClient()->m_Snap.m_aCharacters[m_AimbotTarget].m_Active)
	{
		vec2 TargetPos = GameClient()->m_aClients[m_AimbotTarget].m_RenderPos;
		vec2 Pos = GameClient()->m_LocalCharacterPos;
		vec2 Dir = TargetPos - Pos;
		if(g_Config.m_ClZzAimbotSilent)
		{
			// aim without moving the visible cursor: keep input target, hook handled separately
		}
		else
		{
			// store aim as absolute input direction
			float Len = length(Dir);
			if(Len > 1.0f)
			{
				// use the input flag scheme: direction encoded through mouse pos via aim flag
				GameClient()->m_Controls.m_aShowHookColl[g_Config.m_ClDummy] = false;
			}
			GameClient()->m_Controls.m_aMousePos[g_Config.m_ClDummy] = Dir;
			GameClient()->m_Controls.m_aMouseInputType[g_Config.m_ClDummy] = CControls::EMouseInputType::AUTOMATED;
		}
	}

	// autohook players: hook the aimbot target while the key is held
	if(g_Config.m_ClZzAimbotAutoHookPlayersKey && Input()->KeyIsPressed(g_Config.m_ClZzAimbotAutoHookPlayersKey))
	{
		if(m_AimbotTarget >= 0)
			InputData.m_Hook = 1;
	}

	// aimbot fire while key held
	if(g_Config.m_ClZzAimbotKey && Input()->KeyIsPressed(g_Config.m_ClZzAimbotKey))
	{
		if(m_AimbotTarget >= 0)
			InputData.m_Fire = (InputData.m_Fire + 1) & ~1; // keep even parity, alternating below
	}

	// fake lag: alternate directions while moving
	if(g_Config.m_ClZzFakeLag && InputData.m_Direction != 0)
	{
		InputData.m_Direction = m_FakeLagFlip ? 1 : -1;
		if(!g_Config.m_ClZzFakeLagDrift && InputData.m_Jump)
			InputData.m_Direction = InputData.m_Direction;
	}

	// balance bot: walk towards the nearest player
	if(g_Config.m_ClZzBalanceBot)
	{
		int Best = -1;
		float BestDist = 1e9f;
		for(int i = 0; i < MAX_CLIENTS; i++)
		{
			if(!GameClient()->m_Snap.m_aCharacters[i].m_Active || i == GameClient()->m_Snap.m_LocalClientId)
				continue;
			float Dist = distance(GameClient()->m_aClients[i].m_RenderPos, GameClient()->m_LocalCharacterPos);
			if(Dist < BestDist)
			{
				BestDist = Dist;
				Best = i;
			}
		}
		if(Best >= 0)
		{
			vec2 Delta = GameClient()->m_aClients[Best].m_RenderPos - GameClient()->m_LocalCharacterPos;
			if(std::abs(Delta.x) > 32.0f)
				InputData.m_Direction = Delta.x > 0 ? 1 : -1;
		}
	}

	// hammer-gun: replace gun fire with hammer strike
	if(g_Config.m_ClZzHammerGun && GameClient()->m_Snap.m_pLocalCharacter &&
		GameClient()->m_Snap.m_pLocalCharacter->m_Weapon == WEAPON_GUN)
	{
		if((InputData.m_Fire % 2) != 0)
		{
			InputData.m_WantedWeapon = WEAPON_HAMMER + 1;
		}
	}

	// fast hammer / fast gun spam at the server tune rate
	if((g_Config.m_ClZzFastHammer || g_Config.m_ClZzFastGun) && GameClient()->m_Snap.m_pLocalCharacter)
	{
		int Weapon = GameClient()->m_Snap.m_pLocalCharacter->m_Weapon;
		if((g_Config.m_ClZzFastHammer && Weapon == WEAPON_HAMMER) || (g_Config.m_ClZzFastGun && Weapon == WEAPON_GUN))
		{
			// alternate fire parity every tick to respect cooldown
			static int s_LastTick = 0;
			int Tick = Client()->GameTick(g_Config.m_ClDummy);
			if(Tick != s_LastTick)
			{
				InputData.m_Fire = (InputData.m_Fire + 1) & ~1;
				s_LastTick = Tick;
			}
		}
	}
}
