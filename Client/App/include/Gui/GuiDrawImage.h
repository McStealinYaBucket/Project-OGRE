/* gui/GuiDrawImage.h -- October 2013 (image-draw member).
   Evidence: void C2 (callee of GuiImageButton C2(Verb*)
     0x3F4CDE) + (Adorn*, string, uint) C2 0x3A671A (batch338: 17 zero
     dwords + 2 empty strings + 2 null names, then setImageFromName) +
     setImageFromName 0x5D0CFE (batch339: "Textures/"+name+".png" ->
     ContentId::fromAssets -> setImage); setImage 0x5CFADE (4257B
     texture-proxy loading via Adorn -- render subsystem, queued).
   Member roles TODO-verify (offsets model-relative: MSVC strings are
   32B vs the binary Mach-O 4B COW strings -- see ChatWidget precedent).
   Original file: /Applications/buildAgent/work/fa0a2141fe3d2f4d/Client/App/gui/GuiDrawImage.h (path inferred) */

#pragma once

#include "Util/Name.h"
#include "Util/ContentId.h"

#include <string>
#include <cstring>

// Adorn = RBX::Adorn in the binary: every GUI-family Adorn* mangles
// PNS_5AdornE (batch791: C2 0x3A671A, setImage 0x5CFADE,
// setImageFromName 0x5D0CFE; cf. DataModel::startCoreScripts 0x2D617A,
// GuiBuilder::buildGui 0x39715A).
#include "Gui/Adorn.h"

namespace RBX {

	class GuiDrawImage
	{
	public:
		GuiDrawImage()
			: name1(Name::getNullName())
			, name3(Name::getNullName())
		{
			memset(reserved, 0, sizeof(reserved));
		}
		// mac: 0x3A671A (batch338).
		GuiDrawImage(Adorn* adorn, const std::string& name, unsigned int unknown);
		// mac: 0x5D0CFE (batch339).
		bool setImageFromName(Adorn* adorn, const std::string& name, unsigned int unknown);
		// mac: 0x5CFADE STUB (render era: texture-proxy loading via Adorn).
		// 4th param = G3D::Vector2* per the binary mangle
		// (PN3G3D7Vector2E, batch791) -- was void* unknown2.
		// Loud if image binding is ever attempted in-tree.
		bool setImage(Adorn* adorn, const TextureId& textureId, unsigned int unknown1, G3D::Vector2* unknown2);

	private:
		std::string str0;   // +0 (C2 empty).
		Name name1;         // +4 (C2 null).
		std::string str2;   // +8 (C2 empty).
		Name name3;         // +12 (C2 null).
		int reserved[17];   // +16..+83 (C2 zero; roles TODO).
	};

} // namespace RBX
