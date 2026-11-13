/* V8DataModel/GuiBase.h -- October 2013.
   Layout from the PLAYER ctor 0x007bc4b0 (STUDIO 0x008d8ed0): the Described ctor (Instance(name,
   NULL)), then GuiTarget's vftable at +0x60, IAdornable's members at +0x68..+0x74 (-1, -1, -1, NULL)
   and a float at +0x78. RTTI (GuiBase COL): GuiTarget@0x60, IAdornable@0x64.
   2016 differs: no GuiTarget base (2016 GuiBase::process takes an InputObject) and no member. */

#pragma once

#include "V8Tree/Instance.h"
#include "Gui/GuiEvent.h"
#include "GfxBase/IAdornable.h"

namespace RBX {

	extern const char* const sGuiBase;

	// Render-queue ids (GuiBase2d::getGuiQueue).
	enum GuiQueue
	{
		GUIQUEUE_GENERAL = 0,
		GUIQUEUE_TEXT,
		GUIQUEUE_COUNT
	};

	// VTABLE: PLAYER 0x00b66284 RBX::Debugable
	// VTABLE: STUDIO 0x00c33d3c RBX::Debugable
	// VTABLE: PLAYER 0x00b66254 RBX::GuiTarget
	// VTABLE: STUDIO 0x00c33d0c RBX::GuiTarget
	// VTABLE: PLAYER 0x00b66220 RBX::IAdornable
	// VTABLE: STUDIO 0x00c33cd8 RBX::IAdornable
	// SIZE 0x7c
	class GuiBase
		: public DescribedNonCreatable<GuiBase, Instance, sGuiBase, Reflection::ClassDescriptor::PERSISTENT>
		, public GuiTarget   // 0x60
		, public IAdornable  // 0x64
	{
		typedef DescribedNonCreatable<GuiBase, Instance, sGuiBase, Reflection::ClassDescriptor::PERSISTENT> Super;
	public:
		GuiBase(const char* name);

		static int minZIndex() { return 0; }
		static int maxZIndex() { return 10; }
		static int minZIndex2d() { return minZIndex() + 1; }
		static int maxZIndex2d() { return maxZIndex(); }

		virtual bool canProcessMeAndDescendants() const = 0;  // vtable+0x70
		virtual int getZIndex() const = 0;                    // vtable+0x74
		virtual GuiQueue getGuiQueue() const = 0;             // vtable+0x78

	private:
		float m_unk0x78;  // 0x78  ctor stores 0.0f
	};

	// Out-of-line copies the original emitted in this class's .obj (the Described ctor its ctor calls).
	// clang-format off
	// TEMPLATE: PLAYER 0x007bc440
	// TEMPLATE: STUDIO 0x008d8e60
	// ??$?0PBD@?$Described@VGuiBase@RBX@@$E?sGuiBase@2@3QBDBV?$NonFactoryProduct@VInstance@RBX@@$E?sGuiBase@2@3QBDB@2@$0BL@$0A@@Reflection@RBX@@QAE@PBD@Z
	// clang-format on

} // namespace RBX
