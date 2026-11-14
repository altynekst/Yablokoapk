/* Yabloko client — reconstructed from the surviving build.
   Dynamic world effects: spring grid, plasma shield, tesla hook,
   glass shatter, afterburner, freeze tile glow. */

#ifndef GAME_CLIENT_COMPONENTS_YABLOKO_YABLOKO_DYNAMICS_H
#define GAME_CLIENT_COMPONENTS_YABLOKO_YABLOKO_DYNAMICS_H

#include <game/client/component.h>

#include <game/client/components/yabloko/yabloko.h>

class CYablokoDynamics : public CComponent
{
public:
	int Sizeof() const override { return sizeof(*this); }

	void OnReset() override;
	void OnRender() override;

	// spring grid nodes
	struct SGridNode
	{
		vec2 m_Offset;
		vec2 m_Vel;
	};

	// glass shards
	struct SShard
	{
		vec2 m_Pos;
		vec2 m_Vel;
		float m_Time;
		float m_Lifetime;
		float m_Size;
	};
	std::vector<SShard> m_vShards;

	// afterburner flames
	struct SFlame
	{
		vec2 m_Pos;
		vec2 m_Vel;
		float m_Time;
	};
	std::vector<SFlame> m_vFlames;
	float m_LastBoostSpeed;
	int m_LastRocketState;

	std::vector<SGridNode> m_aGridNodes;
	int m_GridCols;
	int m_GridRows;
	vec2 m_GridOrigin;

	void SpawnShatter(vec2 Pos, const ColorRGBA &Color);

private:
	void RenderGrid();
	void RenderFreezeTiles();
	void RenderPlasma();
	void RenderTesla();
	void RenderShatter();
	void RenderAfterburner();
};

#endif
