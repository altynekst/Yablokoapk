/* Yabloko client — reconstructed from the surviving build.
   ZZetClient-style gameplay assists: aimbot, autohook, balance bot,
   fake lag/aim, hammer assists, FNG trigger, force zoom. */

#ifndef GAME_CLIENT_COMPONENTS_YABLOKO_YABLOKO_ZZ_H
#define GAME_CLIENT_COMPONENTS_YABLOKO_YABLOKO_ZZ_H

#include <engine/console.h>

#include <generated/protocol.h>

#include <game/client/component.h>

#include <game/client/components/yabloko/yabloko.h>

class CYablokoZz : public CComponent
{
public:
	int Sizeof() const override { return sizeof(*this); }

	void OnReset() override;
	void OnConsoleInit() override;
	void OnUpdate() override;

	// Called from CControls::SnapInput to modify outgoing input
	void ApplyInput(CNetObj_PlayerInput &InputData, bool Local);

	// Find the current aimbot target (-1 = none)
	int AimbotTarget() const { return m_AimbotTarget; }

private:
	int m_AimbotTarget;
	float m_FakeAimAngle;
	bool m_FakeLagFlip;
	float m_FakeLagTimer;
	bool m_CompanionEnabled;

	int FindTarget(float FovDegrees, bool ClosestToPlayer) const;
	static void ConToggleAimbot(IConsole::IResult *pResult, void *pUserData);
	static void ConForceZoomNotice(IConsole::IResult *pResult, void *pUserData);
};

#endif
