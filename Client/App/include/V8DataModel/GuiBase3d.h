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

		// FUNCTION: PLAYER 0x00952c00
		// FUNCTION: STUDIO 0x009869e0
		BrickColor getColor() const { return color; }
		void setColor(BrickColor value);
		float getTransparency() const { return transparency; }
		void setTransparency(float value);
		bool getVisible() const { return visible; }
		void setVisible(bool value);

		static Reflection::PropDescriptor<GuiBase3d, BrickColor> prop_Color;
		static Reflection::PropDescriptor<GuiBase3d, float> prop_Transparency;
		static Reflection::PropDescriptor<GuiBase3d, bool> prop_Visible;

		/*override*/ bool shouldRender3dAdorn() const { return getVisible(); }

		/*implement*/ virtual bool canProcessMeAndDescendants() const { return false; }
		/*implement*/ virtual int getZIndex() const { return -1; }
		/*implement*/ virtual GuiQueue getGuiQueue() const { return GUIQUEUE_GENERAL; }

	protected:
		BrickColor color;    // 0x7c
		float transparency;  // 0x80
		bool visible;        // 0x84
	};


	// Out-of-line template and implicit-member copies the original emitted for this class, paired by
	// identical masked bytes (tools/unpaired_twins.py on a 2026-11-13 build).
	// clang-format off
	// TEMPLATE: PLAYER 0x00952c90
	// ??$getset@P8GuiBase3d@RBX@@BE?AVBrickColor@2@XZP812@AEXV32@@Z@?$PropDescriptor@VGuiBase3d@RBX@@VBrickColor@2@@Reflection@RBX@@SA?AV?$auto_ptr@VGetSet@?$TypedPropertyDescriptor@VBrickColor@RBX@@@Reflection@RBX@@@std@@P8GuiBase3d@2@BE?AVBrickColor@2@XZP852@AEXV62@@Z@Z
	// TEMPLATE: PLAYER 0x00952cf0
	// ??$getset@P8GuiBase3d@RBX@@BEMXZP812@AEXM@Z@?$PropDescriptor@VGuiBase3d@RBX@@M@Reflection@RBX@@SA?AV?$auto_ptr@VGetSet@?$TypedPropertyDescriptor@M@Reflection@RBX@@@std@@P8GuiBase3d@2@BEMXZP852@AEXM@Z@Z
	// TEMPLATE: PLAYER 0x00952d50
	// ??$getset@P8GuiBase3d@RBX@@BE_NXZP812@AEX_N@Z@?$PropDescriptor@VGuiBase3d@RBX@@_N@Reflection@RBX@@SA?AV?$auto_ptr@VGetSet@?$TypedPropertyDescriptor@_N@Reflection@RBX@@@std@@P8GuiBase3d@2@BE_NXZP852@AEX_N@Z@Z
	// clang-format on

} // namespace RBX

// Template copies the original emitted in this class's .obj (identical bodies, tools/unpaired_twins.py).
// clang-format off
// TEMPLATE: STUDIO 0x00986a70
// ??$getset@P8GuiBase3d@RBX@@BE?AVBrickColor@2@XZP812@AEXV32@@Z@?$PropDescriptor@VGuiBase3d@RBX@@VBrickColor@2@@Reflection@RBX@@SA?AV?$auto_ptr@VGetSet@?$TypedPropertyDescriptor@VBrickColor@RBX@@@Reflection@RBX@@@std@@P8GuiBase3d@2@BE?AVBrickColor@2@XZP852@AEXV62@@Z@Z
// TEMPLATE: STUDIO 0x00986ad0
// ??$getset@P8GuiBase3d@RBX@@BEMXZP812@AEXM@Z@?$PropDescriptor@VGuiBase3d@RBX@@M@Reflection@RBX@@SA?AV?$auto_ptr@VGetSet@?$TypedPropertyDescriptor@M@Reflection@RBX@@@std@@P8GuiBase3d@2@BEMXZP852@AEXM@Z@Z
// TEMPLATE: STUDIO 0x00986b30
// ??$getset@P8GuiBase3d@RBX@@BE_NXZP812@AEX_N@Z@?$PropDescriptor@VGuiBase3d@RBX@@_N@Reflection@RBX@@SA?AV?$auto_ptr@VGetSet@?$TypedPropertyDescriptor@_N@Reflection@RBX@@@std@@P8GuiBase3d@2@BE_NXZP852@AEX_N@Z@Z
// clang-format on
