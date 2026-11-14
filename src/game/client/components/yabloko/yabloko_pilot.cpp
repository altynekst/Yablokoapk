/* Yabloko client — reconstructed from the surviving build.
   Pilot implementation: conservative reconstruction of the movement AI. */

#include "yabloko_pilot.h"

#include <engine/shared/config.h>

#include <generated/protocol.h>

#include <game/collision.h>
#include <game/client/gameclient.h>

#include <cmath>

// -----------------------------------------------------------------------
// YablokoNativeAvoid::CSimulation
// -----------------------------------------------------------------------

namespace YablokoNativeAvoid
{
	void CSimulation::SetState(vec2 Pos, vec2 Vel, bool Frozen)
	{
		m_Pos = Pos;
		m_Vel = Vel;
		m_Frozen = Frozen;
	}

	bool CSimulation::Step(int Direction, bool Jump, bool Hook, vec2 HookTarget, CCollision *pCollision, int Ticks)
	{
		const float PhysicalSize = 28.0f;
		const float GroundedForce = 50.0f; // tune GroundedSwing? simplified DDNet physics constants
		const float Accel = 32.0f * 100.0f;
		const float MaxSpeed = 700.0f;

		for(int Tick = 0; Tick < Ticks; Tick++)
		{
			if(Direction != 0)
				m_Vel.x += Direction * Accel * (1.0f / 50.0f);
			m_Vel.x = std::clamp(m_Vel.x, -MaxSpeed, MaxSpeed);

			vec2 NextPos = m_Pos + m_Vel * (1.0f / 50.0f) * 32.0f;

			// solid collision check (simplified AABB)
			vec2 Center = NextPos;
			if(pCollision->CheckPoint(Center + vec2(-PhysicalSize / 2, -PhysicalSize / 2)) ||
				pCollision->CheckPoint(Center + vec2(PhysicalSize / 2, -PhysicalSize / 2)) ||
				pCollision->CheckPoint(Center + vec2(-PhysicalSize / 2, PhysicalSize / 2)) ||
				pCollision->CheckPoint(Center + vec2(PhysicalSize / 2, PhysicalSize / 2)))
			{
				// stop horizontal movement on wall hit
				m_Vel.x = 0;
				NextPos = m_Pos;
			}
			m_Pos = NextPos;

			// hazards
			int Tile = pCollision->GetTileIndex(pCollision->GetPureMapIndex(m_Pos));
			if(Tile == TILE_FREEZE)
				m_Frozen = true;
			if(Tile == TILE_DEATH)
				return false;
		}
		(void)Jump;
		(void)Hook;
		(void)HookTarget;
		(void)GroundedForce;
		return true;
	}
} // namespace YablokoNativeAvoid

// -----------------------------------------------------------------------
// CYablokoPilot
// -----------------------------------------------------------------------

void CYablokoPilot::OnReset()
{
	m_Goal = vec2(0, 0);
	m_GoalActive = false;
	m_ReplanTimer = 0;
	m_PlanDir = vec2(0, 0);
	m_PlanJump = false;
	m_PlanHook = false;
	m_LastManualTime = -10.0f;
	m_LastLocalPos = vec2(0, 0);
}

void CYablokoPilot::OnConsoleInit()
{
	Console()->Register("yabloko_autopilot_toggle", "", CFGFLAG_CLIENT, ConAutopilotToggle, this, "Toggle the Race/Gores autopilot");
	Console()->Register("yabloko_autopilot_goal", "", CFGFLAG_CLIENT, ConAutopilotGoal, this, "Set the autopilot goal to the current mouse position");
	Console()->Register("yabloko_autopilot_reset_goal", "", CFGFLAG_CLIENT, ConAutopilotResetGoal, this, "Reset the autopilot goal");
}

void CYablokoPilot::ConAutopilotToggle(IConsole::IResult *pResult, void *pUserData)
{
	(void)pResult;
	CYablokoPilot *pSelf = (CYablokoPilot *)pUserData;
	g_Config.m_ClYablokoAutopilot ^= 1;
	char aBuf[64];
	str_format(aBuf, sizeof(aBuf), "autopilot %s", g_Config.m_ClYablokoAutopilot ? "on" : "off");
	pSelf->GameClient()->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "yabloko", aBuf);
	if(!g_Config.m_ClYablokoAutopilot)
		pSelf->m_GoalActive = false;
}

void CYablokoPilot::ConAutopilotGoal(IConsole::IResult *pResult, void *pUserData)
{
	(void)pResult;
	CYablokoPilot *pSelf = (CYablokoPilot *)pUserData;
	pSelf->m_Goal = pSelf->GameClient()->m_LocalCharacterPos + pSelf->GameClient()->m_Controls.m_aMousePos[g_Config.m_ClDummy];
	pSelf->m_GoalActive = true;
	char aBuf[64];
	str_format(aBuf, sizeof(aBuf), "autopilot goal set (%.0f, %.0f)", pSelf->m_Goal.x, pSelf->m_Goal.y);
	pSelf->GameClient()->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "yabloko", aBuf);
}

void CYablokoPilot::ConAutopilotResetGoal(IConsole::IResult *pResult, void *pUserData)
{
	(void)pResult;
	CYablokoPilot *pSelf = (CYablokoPilot *)pUserData;
	pSelf->m_GoalActive = false;
	pSelf->GameClient()->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "yabloko", "autopilot goal reset");
}

bool CYablokoPilot::DirectionSafe(int Direction, bool Frozen)
{
	if(!GameClient()->m_Snap.m_pLocalCharacter)
		return true;
	vec2 Pos = GameClient()->m_LocalCharacterPos;
	vec2 Vel = vec2(GameClient()->m_Snap.m_pLocalCharacter->m_VelX / 256.0f, GameClient()->m_Snap.m_pLocalCharacter->m_VelY / 256.0f);

	YablokoNativeAvoid::CSimulation Sim;
	Sim.SetState(Pos, Vel, Frozen);
	// look ahead with the requested direction
	if(!Sim.Step(Direction, false, false, vec2(0, 0), GameClient()->Collision(), g_Config.m_ClYablokoAvoidHorizon > 0 ? g_Config.m_ClYablokoAvoidHorizon : 30))
		return false;
	if(Sim.m_Frozen && !Frozen)
	{
		// walking into freeze — check margin
		return false;
	}
	return true;
}

void CYablokoPilot::OnUpdate()
{
	// remember manual input time (pause autopilot after manual control)
	if(GameClient()->m_Snap.m_pLocalCharacter)
	{
		vec2 Pos = GameClient()->m_LocalCharacterPos;
		if(distance(Pos, m_LastLocalPos) > 0.01f && m_LastLocalPos != vec2(0, 0))
		{
			// moved: assume the pilot or the player moved them; refine by input check
		}
		m_LastLocalPos = Pos;
	}
}

void CYablokoPilot::ApplyInput(CNetObj_PlayerInput &InputData, bool Local)
{
	if(!Local)
		return;
	if(!GameClient()->m_Snap.m_pLocalCharacter)
		return;

	bool Frozen = GameClient()->m_aClients[GameClient()->m_Snap.m_LocalClientId].m_Predicted.m_FreezeEnd > 0;

	// manual pause after user input
	float Time = Client()->GlobalTime();
	if(InputData.m_Direction != 0 || InputData.m_Jump || InputData.m_Hook)
		m_LastManualTime = Time;
	if(g_Config.m_ClYablokoAutopilotManualPause > 0 && Time - m_LastManualTime < g_Config.m_ClYablokoAutopilotManualPause / 50.0f)
		return;

	// sentry mode: stationary PvP assistant
	if(g_Config.m_ClYablokoSentry)
	{
		int Best = -1;
		float BestDist = 1e9f;
		for(int i = 0; i < MAX_CLIENTS; i++)
		{
			if(!GameClient()->m_Snap.m_aCharacters[i].m_Active || i == GameClient()->m_Snap.m_LocalClientId)
				continue;
			if(GameClient()->m_Teams.SameTeam(i, GameClient()->m_Snap.m_LocalClientId))
				continue;
			float Dist = distance(GameClient()->m_aClients[i].m_RenderPos, GameClient()->m_LocalCharacterPos);
			if(Dist < BestDist && Dist < g_Config.m_ClYablokoSentryRadius)
			{
				BestDist = Dist;
				Best = i;
			}
		}
		if(Best >= 0)
		{
			vec2 Delta = GameClient()->m_aClients[Best].m_RenderPos - GameClient()->m_LocalCharacterPos;
			GameClient()->m_Controls.m_aMousePos[g_Config.m_ClDummy] = Delta;
			GameClient()->m_Controls.m_aMouseInputType[g_Config.m_ClDummy] = CControls::EMouseInputType::AUTOMATED;
			if(g_Config.m_ClYablokoCompanionRanged && BestDist < 700.0f)
				InputData.m_Fire = (InputData.m_Fire + 1) & ~1;
		}
		return;
	}

	// FNG assistant: hook-move toward the target area, rescue frozen mates
	if(g_Config.m_ClYablokoFngBudget > 0 && GameClient()->m_Snap.m_pGameInfoObj &&
		GameClient()->m_Snap.m_pGameInfoObj->m_GameFlags & GAMEFLAG_FLAGS)
	{
		int Target = FindFngTarget();
		if(Target >= 0)
		{
			vec2 Delta = GameClient()->m_aClients[Target].m_RenderPos - GameClient()->m_LocalCharacterPos;
			GameClient()->m_Controls.m_aMousePos[g_Config.m_ClDummy] = Delta;
			if(length(Delta) > 100.0f && length(Delta) < 380.0f)
				InputData.m_Hook = 1;
		}
		return;
	}

	// autopilot: walk/hook towards the goal
	if(g_Config.m_ClYablokoAutopilot && m_GoalActive)
	{
		vec2 Delta = m_Goal - GameClient()->m_LocalCharacterPos;
		if(length(Delta) < 40.0f)
		{
			m_GoalActive = false;
			return;
		}

		m_ReplanTimer--;
		if(m_ReplanTimer <= 0)
		{
			m_ReplanTimer = g_Config.m_ClYablokoAutopilotReplan > 0 ? g_Config.m_ClYablokoAutopilotReplan : 10;
			int Dir = Delta.x > 0 ? 1 : -1;
			if(!DirectionSafe(Dir, Frozen))
			{
				// try the other direction, then stop
				if(DirectionSafe(-Dir, Frozen))
					Dir = -Dir;
				else
					Dir = 0;
			}
			m_PlanJump = Delta.y < -100.0f && !Frozen;
			m_PlanHook = length(Delta) > 300.0f && !Frozen;
			m_PlanDir = vec2(Dir, 0);
		}

		InputData.m_Direction = (int)m_PlanDir.x;
		if(m_PlanJump)
			InputData.m_Jump = 1;
		if(m_PlanHook)
			InputData.m_Hook = 1;
	}
}

int CYablokoPilot::FindFngTarget() const
{
	// closest frozen enemy or frozen teammate depending on settings
	for(int i = 0; i < MAX_CLIENTS; i++)
	{
		if(!GameClient()->m_Snap.m_aCharacters[i].m_Active || i == GameClient()->m_Snap.m_LocalClientId)
			continue;
		auto &Predicted = GameClient()->m_aClients[i].m_Predicted;
		if(Predicted.m_FreezeEnd <= 0)
			continue;
		if(g_Config.m_ClYablokoFngRescue && GameClient()->m_Teams.SameTeam(i, GameClient()->m_Snap.m_LocalClientId))
			return i;
		if(!GameClient()->m_Teams.SameTeam(i, GameClient()->m_Snap.m_LocalClientId))
			return i;
	}
	return -1;
}

void CYablokoPilot::ApplySafety(CNetObj_PlayerInput &InputData, bool Frozen)
{
	(void)Frozen;
	if(g_Config.m_ClYablokoAvoidStrict <= 0 && g_Config.m_ClYablokoCompanionSafety <= 0)
		return;
	if(!GameClient()->m_Snap.m_pLocalCharacter)
		return;

	// veto movement that would walk into freeze or death tiles
	if(InputData.m_Direction != 0 && !DirectionSafe(InputData.m_Direction, Frozen))
	{
		InputData.m_Direction = 0;
	}
	vec2 NextPos = GameClient()->m_LocalCharacterPos + vec2(InputData.m_Direction * 32.0f, 0);
	int Tile = GameClient()->Collision()->GetTileIndex(GameClient()->Collision()->GetPureMapIndex(NextPos));
	if(Tile == TILE_DEATH)
		InputData.m_Direction = 0;
}
