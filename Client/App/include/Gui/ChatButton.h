/* gui/ChatButton.h -- October 2013 chat button widget (NO reflection).
   Evidence: D1 0x5CB560/D0 0x5CB5EA (batch300: vtbls + string@+200 +
     GuiDrawImage@+116 + GuiItem teardown; MI thunks Thn32/Thn36) +
     vtable atlas (batches 334-335: GuiItem primary + UnifiedImageWidget
     secondaries) + factory create<ChatButton>(Adorn*, char const*, int)
     0x3A5B30 (ctor args; no out-of-line C1/C2 DB-wide -- inline
     base-forward like the 2016 GeoPairConnector precedent).
   isVisible 0x5CB036 queues behind the Players+188 member identity.
   Original file: /Applications/buildAgent/work/fa0a2141fe3d2f4d/Client/App/gui/ChatButton.h (path inferred) */

#pragma once

#include "Gui/UnifiedImageWidget.h"

namespace RBX {

	class ChatButton
		: public UnifiedImageWidget
	{
	public:
		// Inline base-forward (factory-proven signature; no out-of-line
		// C2 exists to carry a body).
		ChatButton(Adorn* adorn, const char* name, int unknown)
			: UnifiedImageWidget(adorn, name == NULL ? "" : name, unknown)
		{}
		// D1/D0 out-of-line in the binary (member teardown + GuiItem
		// chain + plain operator delete); implicit bodies match.
		virtual ~ChatButton() {}
		// mac: 0x5CB036 isVisible queues behind Players+188 identity.
	};

} // namespace RBX
