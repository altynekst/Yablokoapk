/* Yabloko client — reconstructed from the surviving build.
   HUD subsystem: notifications, hook duration bar, glass destra panels,
   radar, HUD editor, server widget, back button, active features list. */

#ifndef GAME_CLIENT_COMPONENTS_YABLOKO_YABLOKO_TOOLKIT_H
#define GAME_CLIENT_COMPONENTS_YABLOKO_YABLOKO_TOOLKIT_H

#include <game/client/component.h>

#include <game/client/ui_rect.h>

#include <game/client/components/yabloko/yabloko.h>

#include <string>
#include <vector>

class CYablokoToolkit : public CComponent
{
public:
	int Sizeof() const override { return sizeof(*this); }

	void OnReset() override;
	void OnStateChange(int NewState, int OldState) override;
	void OnRender() override;
	bool OnInput(const IInput::CEvent &Event) override;

	// Notifications
	struct SNotification
	{
		std::string m_Text;
		float m_Time;
	};
	std::vector<SNotification> m_vNotifications;

	void Notify(const char *pText);
	void NotifyJoin(const char *pName);
	void NotifyLeave(const char *pName);
	void NotifyMention(const char *pName, const char *pText);
	void NotifyWhisper(const char *pName, const char *pText);

	// HUD editor
	bool m_HudEditActive;
	vec2 m_HudEditDragOffset;
	int m_HudEditPanel;

	void StartHudEdit();
	void StopHudEdit();

	// Panel rects: x,y in percent, w,h in percent
	struct SPanelRect
	{
		int *m_pX;
		int *m_pY;
		int *m_pWidth;
		int *m_pHeight;
		const char *m_pName;
	};

	void GetPanelRect(int PanelIndex, vec4 &Rect) const;
	void SetPanelRect(int PanelIndex, const vec4 &Rect);
	static constexpr int NUM_PANELS = 10;
	static const char *const ms_apPanelNames[NUM_PANELS];

	// Where the destra panels live, in percent of screen
	void RenderAll();

private:
	void RenderNotifications();
	void RenderHookDurationBar();
	void RenderBackButton();
	void RenderServerWidget();
	void RenderDestraPanels();
	void RenderDestraPanel(CUIRect Rect, float Alpha);
	void RenderTopPanel(CUIRect Rect);
	void RenderPlayersPanel(CUIRect Rect);
	void RenderResultsPanel(CUIRect Rect);
	void RenderVotePanel(CUIRect Rect);
	void RenderChatPanel(CUIRect Rect);
	void RenderMusicPanel(CUIRect Rect);
	void RenderActiveFeatures();
	void RenderRadar();
	void RenderPathPrediction();
	void RenderInputOverlay();

	float m_BackButtonPress;
	vec2 m_BackButtonPos;

public:
	bool BackButtonPressed() const { return m_BackButtonPress > 0.0f; }
};

#endif
