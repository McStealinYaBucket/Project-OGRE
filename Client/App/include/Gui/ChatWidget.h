/* gui/ChatWidget.h -- October 2013 C++-only widget (NO reflection).
   Evidence: C2(string,string) 0x5CB07C; onMenuStateChanged 0x5CB11A;
     process 0x5CB13E (547B: click->chat, needs Players/SoundService).
   Original file: /Applications/buildAgent/work/fa0a2141fe3d2f4d/Client/App/gui/ChatWidget.h */

#pragma once

#include "gui/UnifiedWidget.h"
#include "gui/GuiEvent.h"
#include "gui/GUI.h"
#include "Util/SoundService.h"
#include "Util/SoundWorld.h"
#include "Network/Players.h"
#include <string>

namespace RBX {

	class ChatWidget
		: public UnifiedWidget
		{
	public:
		ChatWidget(const std::string& s1, const std::string& s2)
			: chatText(s2)
		{
			Instance::setName(s1);
		}

		// STUB: PLAYER 0x0096b5e0
		// STUB: STUDIO 0x00a1a0b0
		virtual void onMenuStateChanged()
		{
			if (menuState == HOVER)
				setMenuState(SHOWN);
		}

		// STUB: PLAYER 0x0096b6c0
		// STUB: STUDIO 0x00a1a190
		virtual GuiResponse process(const GuiEvent& event)
		{
			if (!isVisible())
				return UnifiedWidget::process(event);
			if ((unsigned int)(event.eventType - 1) > 8)
				return UnifiedWidget::process(event);
			if (event.eventType != 4)
				return UnifiedWidget::process(event);

			Canvas canvas;
			canvas.x = (float)event.canvasX;
			canvas.y = (float)event.canvasY;
			Rect rect = getMyRect(canvas);

			float mx = (float)event.mouseX;
			if (mx < rect.low.x)
				return UnifiedWidget::process(event);
			if (rect.high.x < mx)
				return UnifiedWidget::process(event);
			float my = (float)event.mouseY;
			if (my < rect.low.y)
				return UnifiedWidget::process(event);
			if (rect.high.y < my)
				return UnifiedWidget::process(event);

			Network::Players* players = ServiceProvider::find<Network::Players>(this);
			if (players == NULL)
				return UnifiedWidget::process(event);

			Soundscape::SoundService* ss = ServiceProvider::create<Soundscape::SoundService>(this);
			ss->playSound(CLICK_SOUND);

			std::string message = chatText;  // binary: message = loc_5CF12C(prefix="") + chatText. Prefix verified empty (0x5CF11C = "").
			players->chat(message);

			return GuiResponse::sunkAndFinished();
		}

	private:
		std::string chatText;  // +116
	};

} // namespace RBX