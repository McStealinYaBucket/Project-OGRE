/* V8DataModel/GuiBase3d.h -- October 2013.
   Layout from the PLAYER ctor 0x00953230 (STUDIO 0x00987010): GuiBase(name), then
   +0x7c = 23 (BrickColor::brick_23, 2016's brickBlue), +0x80 = 0.0f, +0x84 = true.
   2013 stores a BrickColor where 2016 stores a Color3. The GuiBase overriders are the 2013
   vftable 0x00ba9a0c slots +0x70..+0x78 (0x00927cc0 return false, 0x0092a470 return -1,
   0x0096bde0 return 0); shouldRender3dAdorn is IAdornable slot 1 (0x0092a460, reads +0x84). */

#pragma once

#include "V8DataModel/GuiBase.h"
#include "Util/BrickColor.h"

namespace RBX {

	extern const char* const sGuiBase3d;

	// VTABLE: PLAYER 0x00ba9a0c RBX::Debugable
	// VTABLE: STUDIO 0x00c4b584 RBX::Debugable
	// SIZE 0x88
	class GuiBase3d
		: public DescribedNonCreatable<GuiBase3d, GuiBase, sGuiBase3d>
	{
		typedef DescribedNonCreatable<GuiBase3d, GuiBase, sGuiBase3d> Super;
	public:
		GuiBase3d(const char* name = NULL);

		bool getVisible() const { return visible; }

		/*override*/ bool shouldRender3dAdorn() const { return getVisible(); }

		/*implement*/ virtual bool canProcessMeAndDescendants() const { return false; }
		/*implement*/ virtual int getZIndex() const { return -1; }
		/*implement*/ virtual GuiQueue getGuiQueue() const { return GUIQUEUE_GENERAL; }

	protected:
		BrickColor color;    // 0x7c
		float transparency;  // 0x80
		bool visible;        // 0x84
	};

} // namespace RBX
