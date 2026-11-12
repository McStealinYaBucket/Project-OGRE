/* gui/UnifiedImageWidget.h -- October 2013 image widget base (NO reflection).
   Evidence: C2 0x3A640E (180B, batch337: UnifiedWidget base + 5 vtbls +
     GuiDrawImage member at +116 via the (Adorn*, string, uint) C2 0x3A671A
     + string at +200 + int at +204); D1 0x3A64D6/D0 0x3CB400 (125B: vtbls +
     string/GuiDrawImage/GuiItem teardown, no owned members).
   Original file: /Applications/buildAgent/work/fa0a2141fe3d2f4d/Client/App/gui/UnifiedImageWidget.h (path inferred) */

#pragma once

#include "Gui/UnifiedWidget.h"
#include "Gui/GuiDrawImage.h"

#include <string>

// Adorn = RBX::Adorn (binary mangle: C2 0x3A640E = PNS_5AdornE,
// batch791). Binding comes from GuiDrawImage.h's Gui/Adorn.h include.

namespace RBX {

	class UnifiedImageWidget
		: public UnifiedWidget
	{
	public:
		// mac: 0x3A640E (C2 args proven by the ChatButton factory
		// create<ChatButton>(Adorn*, char const*, int) 0x3A5B30; no
		// out-of-line ChatButton C1/C2 exists DB-wide).
		UnifiedImageWidget(Adorn* adorn, const std::string& imageName, int unknown204)
			: image(adorn, imageName, (unsigned int)unknown204)
			, imageName(imageName)
			, unknown204(unknown204)
		{}
		virtual ~UnifiedImageWidget() {}
		// D1/D0 (0x3A64D6/0x3CB400): member teardown only; implicit
		// bodies match (string + GuiDrawImage + GuiItem chain).

	protected:
		GuiDrawImage image;      // +116 (3-arg C2).
		std::string imageName;   // +200 (copy of the C2 name).
		int unknown204;          // +204 (C2 int arg; role TODO).
	};

} // namespace RBX
