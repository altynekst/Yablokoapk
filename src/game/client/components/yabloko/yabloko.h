/* Yabloko client — reconstructed from the surviving build.
   Shared helpers for all Yabloko components. */

#ifndef GAME_CLIENT_COMPONENTS_YABLOKO_YABLOKO_H
#define GAME_CLIENT_COMPONENTS_YABLOKO_YABLOKO_H

#include <base/color.h>
#include <base/math.h>
#include <base/system.h>
#include <base/vmath.h>

namespace Yabloko
{
	// Feature scope: 0 = self, 1 = teammates, 2 = everyone
	inline bool ScopeAllowed(int Scope, bool Self, bool Teammate)
	{
		if(Self)
			return true;
		if(Scope == 2)
			return true;
		if(Scope == 1 && Teammate)
			return true;
		return false;
	}

	// Pack config color (0xRRGGBB / 0xAARRGGBB int) into ColorRGBA
	inline ColorRGBA Col(int ConfigColor, float Alpha = 1.0f)
	{
		ColorRGBA Res = color_cast<ColorRGBA>(ColorHSLA(ConfigColor, true, 1.0));
		Res.a *= Alpha;
		return Res;
	}

	// Rainbow hue helper
	inline ColorRGBA Rainbow(float Phase, float Alpha = 1.0f)
	{
		ColorHSLA Hsl(Phase * 255.0f, 1.0, 0.5);
		return color_cast<ColorRGBA>(Hsl).WithAlpha(Alpha);
	}

	// Does the clan/name mark somebody as a Yabloko user?
	inline bool IsYablokoName(const char *pStr)
	{
		if(!pStr)
			return false;
		return str_find_nocase(pStr, "[Yabloko]") != nullptr ||
		       str_find_nocase(pStr, "Yabloko.") != nullptr;
	}

	// Theme constants: 0=classic 1=violet 2=red 3=emerald 4=custom
	inline ColorRGBA ThemeAccent(int Theme, int CustomAccent)
	{
		switch(Theme)
		{
		case 1: return ColorRGBA(0.55f, 0.30f, 0.90f, 1.0f);
		case 2: return ColorRGBA(0.95f, 0.25f, 0.30f, 1.0f);
		case 3: return ColorRGBA(0.20f, 0.85f, 0.45f, 1.0f);
		case 4: return Col(CustomAccent);
		default: return ColorRGBA(0.55f, 1.0f, 0.35f, 1.0f); // apple green
		}
	}

	inline ColorRGBA ThemeBackground(int Theme, int CustomBackground)
	{
		switch(Theme)
		{
		case 1: return ColorRGBA(0.10f, 0.05f, 0.18f, 1.0f);
		case 2: return ColorRGBA(0.18f, 0.03f, 0.05f, 1.0f);
		case 3: return ColorRGBA(0.02f, 0.14f, 0.08f, 1.0f);
		case 4: return Col(CustomBackground);
		default: return ColorRGBA(0.06f, 0.10f, 0.04f, 1.0f);
		}
	}
}

#endif
