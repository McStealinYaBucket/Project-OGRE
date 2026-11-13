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
#include "rbx/signal.h"
#include <boost/shared_ptr.hpp>

namespace RBX {

	class TextureProxyBase;

	// Layout from the PLAYER default ctor 0x006bc570 (inline in this header, emitted out of line;
	// Frame's ctor 0x006c2b70 runs it on Frame+0x220): two TextureIds, seven zeroed proxy refs at
	// 0x40..0x78, size 0x78 = (0, 0), a 4-byte connection at 0x80 (Frame's sizeof 0x2a4 - 0x220 = 0x84).
	// Member names from 2016 GuiDraw.h (same order and sizes).
	// SIZE 0x84
	class GuiDrawImage
	{
	public:
		// FUNCTION: PLAYER 0x006bc570
		// FUNCTION: STUDIO 0x007ab370
		GuiDrawImage()
			: size(0.0f, 0.0f)
		{}
		GuiDrawImage(Adorn* adorn, const std::string& name, unsigned int unknown);
		bool setImageFromName(Adorn* adorn, const std::string& name, unsigned int unknown);
		bool setImage(Adorn* adorn, const TextureId& textureId, unsigned int unknown1, G3D::Vector2* unknown2);
	private:
		TextureId currentTexture;                              // 0x00
		TextureId loadingTexture;                              // 0x20
		boost::shared_ptr<TextureProxyBase> disable;           // 0x40
		boost::shared_ptr<TextureProxyBase> normal;            // 0x48
		boost::shared_ptr<TextureProxyBase> hover;             // 0x50
		boost::shared_ptr<TextureProxyBase> down;              // 0x58
		boost::shared_ptr<TextureProxyBase> selected;          // 0x60
		boost::shared_ptr<TextureProxyBase> selectedHover;     // 0x68
		boost::shared_ptr<TextureProxyBase> selectedDown;      // 0x70
		G3D::Vector2 size;                                     // 0x78
		rbx::signals::scoped_connection unbindResourceSignalHint;  // 0x80
	};

} // namespace RBX
