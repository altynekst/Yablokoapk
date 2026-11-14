/* Yabloko client — reconstructed from the surviving build.
   Pilot: autopilot, hazard avoidance, sentry mode, FNG assistant.
   Includes YablokoNativeAvoid::CSimulation — a lightweight prediction
   wrapper used to plan safe movement. */

#ifndef GAME_CLIENT_COMPONENTS_YABLOKO_YABLOKO_PILOT_H
#define GAME_CLIENT_COMPONENTS_YABLOKO_YABLOKO_PILOT_H

#include <engine/console.h>

#include <generated/protocol.h>

#include <game/client/component.h>

#include <game/client/components/yabloko/yabloko.h>

namespace YablokoNativeAvoid
{
	// Lightweight simulation wrapper for movement planning.
	class CSimulation
	{
	public:
		vec2 m_Pos;
		vec2 m_Vel;
		bool m_Frozen;

		CSimulation() :
			m_Pos(0, 0), m_Vel(0, 0), m_Frozen(false) {}

		void SetState(vec2 Pos, vec2 Vel, bool Frozen);
		// Step the simulation forward by one tick with the given input.
		// Returns false when the simulated character would die or freeze.
		bool Step(int Direction, bool Jump, bool Hook, vec2 HookTarget, class CCollision *pCollision, int Ticks);
	};

} // namespace YablokoNativeAvoid

class CYablokoPilot : public CComponent
{
public:
	int Sizeof() const override { return sizeof(*this); }

	void OnReset() override;
	void OnConsoleInit() override;
	void OnUpdate() override;

	// Called from CControls::SnapInput: may override movement input when
	// autopilot / sentry / FNG assistant drive the tee.
	void ApplyInput(CNetObj_PlayerInput &InputData, bool Local);

	// Safety layer from CControls::ApplyYablokoSafety: vetoes unsafe inputs
	// (freeze/kill hazards ahead) while protection systems are enabled.
	void ApplySafety(CNetObj_PlayerInput &InputData, bool Frozen);

	vec2 m_Goal;
	bool m_GoalActive;

private:
	int m_ReplanTimer;
	vec2 m_PlanDir;
	bool m_PlanJump;
	bool m_PlanHook;
	float m_LastManualTime;
	vec2 m_LastLocalPos;

	int FindFngTarget() const;
	bool DirectionSafe(int Direction, bool Frozen);
	static void ConAutopilotToggle(IConsole::IResult *pResult, void *pUserData);
	static void ConAutopilotGoal(IConsole::IResult *pResult, void *pUserData);
	static void ConAutopilotResetGoal(IConsole::IResult *pResult, void *pUserData);
};

#endif
