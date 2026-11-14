/* Yabloko client — reconstructed from the surviving build.
   Ghost aura implementation. */

#include "yabloko_aimvisuals.h"

#include <engine/graphics.h>
#include <engine/shared/config.h>

#include <game/client/gameclient.h>

#include <cmath>

void CYablokoAimVisuals::OnRender()
{
	if(Client()->State() != IClient::STATE_ONLINE && Client()->State() != IClient::STATE_DEMOPLAYBACK)
		return;
	if(!g_Config.m_ClZzAimbotEnabled || !g_Config.m_ClZzAimbotGhostAura)
		return;

	int Target = GameClient()->m_YablokoZz.AimbotTarget();
	if(Target < 0 || Target >= MAX_CLIENTS || !GameClient()->m_Snap.m_aCharacters[Target].m_Active)
		return;

	vec2 Pos = GameClient()->m_aClients[Target].m_RenderPos;
	float Time = Client()->GlobalTime();
	float Alpha = g_Config.m_ClZzAimbotGhostAuraAlpha / 100.0f;
	float Radius = g_Config.m_ClZzAimbotGhostAuraRadius / 2.0f;
	float Stretch = g_Config.m_ClZzAimbotGhostAuraStretch / 100.0f;
	float Speed = g_Config.m_ClZzAimbotGhostAuraSpeed / 100.0f;
	int Axis = g_Config.m_ClZzAimbotGhostAuraAxis;
	int Direction = g_Config.m_ClZzAimbotGhostAuraDirection == 2 ? -1 : 1;

	ColorRGBA Color;
	if(g_Config.m_ClZzAimbotGhostAuraUseTargetBodyColor)
		Color = GameClient()->m_aClients[Target].m_RenderInfo.m_ColorBody;
	else
		Color = Yabloko::Col(g_Config.m_ClZzAimbotGhostAuraColor, 1.0f);

	Graphics()->BlendAdditive();

	// two rotating rings drawn as point chains
	for(int Ring = 0; Ring < 2; Ring++)
	{
		float RingRadius = Radius * (1.0f + Ring * Stretch);
		float RingTime = Time * Speed * Direction * (Ring == 1 && g_Config.m_ClZzAimbotGhostAuraDirection2 == 2 ? -1 : 1);
		for(int i = 0; i < 32; i++)
		{
			float Angle = RingTime + i * 2.0f * pi / 32;
			vec2 Offset = vec2(std::sin(Angle), std::cos(Angle)) * RingRadius;
			if(Axis == 1)
				Offset.y *= 0.3f;
			else if(Axis == 2)
				Offset.x *= 0.3f;
			float Size = 2.0f + std::sin(Angle * 3.0f) * 1.0f;
			Graphics()->SetColor(Color.r, Color.g, Color.b, Alpha * 0.5f);
			Graphics()->DrawCircle(Pos.x + Offset.x, Pos.y + Offset.y, Size, 6);
		}
	}
	Graphics()->BlendNone();
}
