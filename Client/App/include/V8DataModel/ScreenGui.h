/* V8DataModel/ScreenGui.h -- October 2013.
   Layout from the PLAYER ctors 0x008b2980 (ScreenGui()) and 0x008b2ae0 (ScreenGui(name)), STUDIO
   0x00960c80 / 0x00960de0: GuiLayerCollector (0xb4), the HeartbeatInstance base at +0xb4 (vftable
   0x00b86800: onHeartbeat 0x008b1e20 and the deleting dtor), then windowRect +0xbc ({0, 0, 800, 600}
   by default, zero from the name ctor), two bools +0xcc/+0xcd, a vector +0xd0 and a
   map<const Name*, InputObject::UserInputState> +0xe0 (its _Buynode 0x0091cc50 names the type).
   2016 calls windowRect bufferedViewport and the vector modalGuiObjects (vector<GuiButton*>); the
   2016 renderable flag and the connections map are not 2013's +0xcc and +0xe0. */

#pragma once

#include "V8DataModel/GuiLayerCollector.h"
#include "V8DataModel/InputObject.h"
#include "Util/HeartbeatInstance.h"
#include "G3D/Vector2int16.h"

#include <map>
#include <vector>

namespace RBX {

	extern const char* const sScreenGui;

	class GuiButton;

	// VTABLE: PLAYER 0x00b8686c RBX::Debugable
	// VTABLE: STUDIO 0x00c4753c RBX::Debugable
	// VTABLE: PLAYER 0x00b86840 RBX::GuiTarget
	// VTABLE: STUDIO 0x00c47510 RBX::GuiTarget
	// VTABLE: PLAYER 0x00b8680c RBX::IAdornable
	// VTABLE: STUDIO 0x00c474dc RBX::IAdornable
	// VTABLE: PLAYER 0x00b86800 RBX::HeartbeatInstance
	// VTABLE: STUDIO 0x00c474d0 RBX::HeartbeatInstance
	// SIZE 0xec
	class ScreenGui
		: public DescribedCreatable<ScreenGui, GuiLayerCollector, sScreenGui>
		, public HeartbeatInstance  // 0xb4
	{
		typedef DescribedCreatable<ScreenGui, GuiLayerCollector, sScreenGui> Super;
	public:
		ScreenGui();
		ScreenGui(const char* name);

		G3D::Rect2D windowRect;  // 0xbc

		// Both setters share one 2013 body (PLAYER 0x008b1d90), the linker folded them.
		void setReplicatingAbsoluteSize(G3D::Vector2int16 value);
		void setReplicatingAbsolutePosition(G3D::Vector2int16 value);
		bool isAncestorRenderableGui() const;

		static Reflection::PropDescriptor<ScreenGui, G3D::Vector2int16> prop_ReplicateAbsoluteSize;
		static Reflection::PropDescriptor<ScreenGui, G3D::Vector2int16> prop_ReplicateAbsolutePosition;

	protected:
		/*override*/ void onHeartbeat(const Heartbeat& event);
		/*override*/ void onPropertyChanged(const Reflection::PropertyDescriptor& descriptor);
		/*override*/ void onAncestorChanged(const AncestorChanged& event);

	private:
		bool m_unk0xcc;  // 0xcc  onHeartbeat resizes to windowRect and clears it
		bool renderable;  // 0xcd  isAncestorRenderableGui, kept by onAncestorChanged 0x008b2790
		std::vector<GuiButton*> m_unk0xd0;  // 0xd0  2016: modalGuiObjects
		std::map<const Name*, InputObject::UserInputState> m_unk0xe0;  // 0xe0
	};

	// Out-of-line copies the original emitted in this class's .obj (the Described ctor its ctors call).
	// clang-format off
	// TEMPLATE: PLAYER 0x008b27e0
	// TEMPLATE: STUDIO 0x00960ae0
	// ??$?0PBD@?$Described@VScreenGui@RBX@@$E?sScreenGui@2@3QBDBV?$FactoryProduct@VScreenGui@RBX@@VGuiLayerCollector@2@$E?sScreenGui@2@3QBDBVInstance@2@@2@$0BL@$0A@@Reflection@RBX@@QAE@PBD@Z
	// clang-format on


	// Out-of-line template and implicit-member copies the original emitted for this class, paired by
	// identical masked bytes (tools/unpaired_twins.py on a 2026-11-13 build).
	// clang-format off
	// TEMPLATE: PLAYER 0x008b1ed0
	// ??_E?$DescribedCreatable@VScreenGui@RBX@@VGuiLayerCollector@2@$E?sScreenGui@2@3QBDB$0BL@$0A@@RBX@@WBM@AEPAXI@Z
	// TEMPLATE: PLAYER 0x008b1f00
	// TEMPLATE: STUDIO 0x009602e0
	// ?getValue@?$GetSetImpl@P8GuiBase2d@RBX@@BEABVVector2@G3D@@XZP8ScreenGui@2@AEXVVector2int16@4@@Z@?$PropDescriptor@VScreenGui@RBX@@VVector2int16@G3D@@@Reflection@RBX@@UBE?AVVector2int16@G3D@@PBVDescribedBase@34@@Z
	// TEMPLATE: PLAYER 0x008b1ff0
	// ??1?$FactoryProduct@VScreenGui@RBX@@VGuiLayerCollector@2@$E?sScreenGui@2@3QBDBVInstance@2@@RBX@@MAE@XZ
	// TEMPLATE: PLAYER 0x008b2130
	// ??0?$TypedPropertyDescriptor@VVector2int16@G3D@@@Reflection@RBX@@IAE@AAVClassDescriptor@12@PBD1V?$auto_ptr@VGetSet@?$TypedPropertyDescriptor@VVector2int16@G3D@@@Reflection@RBX@@@std@@UAttributes@PropertyDescriptor@12@W4Permissions@Security@2@@Z
	// TEMPLATE: PLAYER 0x008b2200
	// ??$getset@P8GuiBase2d@RBX@@BEABVVector2@G3D@@XZP8ScreenGui@2@AEXVVector2int16@4@@Z@?$PropDescriptor@VScreenGui@RBX@@VVector2int16@G3D@@@Reflection@RBX@@SA?AV?$auto_ptr@VGetSet@?$TypedPropertyDescriptor@VVector2int16@G3D@@@Reflection@RBX@@@std@@P8GuiBase2d@2@BEABVVector2@G3D@@XZP8ScreenGui@2@AEXVVector2int16@7@@Z@Z
	// TEMPLATE: PLAYER 0x008b2330
	// ?getVariant@?$TypedPropertyDescriptor@VVector2int16@G3D@@@Reflection@RBX@@UBEXPBVDescribedBase@23@AAVVariant@23@@Z
	// TEMPLATE: PLAYER 0x008b24e0
	// ?setVariant@?$TypedPropertyDescriptor@VVector2int16@G3D@@@Reflection@RBX@@UBEXPAVDescribedBase@23@ABVVariant@23@@Z
	// TEMPLATE: PLAYER 0x008b2690
	// ??1?$FactoryProduct@VGuiMain@RBX@@VScreenGui@2@$E?sGuiMain@2@3QBDBVInstance@2@@RBX@@MAE@XZ
	// TEMPLATE: PLAYER 0x008b2a90
	// ??_EScreenGui@RBX@@WBI@AEPAXI@Z
	// TEMPLATE: PLAYER 0x008b2aa0
	// ??_EScreenGui@RBX@@WLE@AEPAXI@Z
	// TEMPLATE: PLAYER 0x008b2e80
	// ??_E?$FactoryProduct@VGuiMain@RBX@@VScreenGui@2@$E?sGuiMain@2@3QBDBVInstance@2@@RBX@@OBI@AEPAXI@Z
	// TEMPLATE: PLAYER 0x008b2e90
	// ??_EGuiMain@RBX@@WLE@AEPAXI@Z
	// TEMPLATE: PLAYER 0x008b45a0
	// ??_EGuiLayerCollector@RBX@@UAEPAXI@Z
	// clang-format on

} // namespace RBX

// Template copies the original emitted in this class's .obj (identical bodies, tools/unpaired_twins.py).
// clang-format off
// TEMPLATE: STUDIO 0x009604e0
// ??0?$TypedPropertyDescriptor@VVector2int16@G3D@@@Reflection@RBX@@IAE@AAVClassDescriptor@12@PBD1V?$auto_ptr@VGetSet@?$TypedPropertyDescriptor@VVector2int16@G3D@@@Reflection@RBX@@@std@@UAttributes@PropertyDescriptor@12@W4Permissions@Security@2@@Z
// TEMPLATE: STUDIO 0x009605b0
// ??$getset@P8GuiBase2d@RBX@@BEABVVector2@G3D@@XZP8ScreenGui@2@AEXVVector2int16@4@@Z@?$PropDescriptor@VScreenGui@RBX@@VVector2int16@G3D@@@Reflection@RBX@@SA?AV?$auto_ptr@VGetSet@?$TypedPropertyDescriptor@VVector2int16@G3D@@@Reflection@RBX@@@std@@P8GuiBase2d@2@BEABVVector2@G3D@@XZP8ScreenGui@2@AEXVVector2int16@7@@Z@Z
// clang-format on
