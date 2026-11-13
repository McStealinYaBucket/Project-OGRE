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

	protected:
		/*override*/ void onHeartbeat(const Heartbeat& event);

	private:
		bool m_unk0xcc;  // 0xcc  onHeartbeat resizes to windowRect and clears it
		bool m_unk0xcd;  // 0xcd
		std::vector<GuiButton*> m_unk0xd0;  // 0xd0  2016: modalGuiObjects
		std::map<const Name*, InputObject::UserInputState> m_unk0xe0;  // 0xe0
	};

	// Out-of-line copies the original emitted in this class's .obj (the Described ctor its ctors call).
	// clang-format off
	// TEMPLATE: PLAYER 0x008b27e0
	// TEMPLATE: STUDIO 0x00960ae0
	// ??$?0PBD@?$Described@VScreenGui@RBX@@$E?sScreenGui@2@3QBDBV?$FactoryProduct@VScreenGui@RBX@@VGuiLayerCollector@2@$E?sScreenGui@2@3QBDBVInstance@2@@2@$0BL@$0A@@Reflection@RBX@@QAE@PBD@Z
	// clang-format on

} // namespace RBX
