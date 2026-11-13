/* V8DataModel/GuiBase2d.h -- October 2013.
   Layout from the PLAYER ctor 0x007bcc40 (STUDIO 0x008d9600): GuiBase(name), then
   absolutePosition +0x7c, absoluteSize +0x84 (0.0f each), zIndex +0x8c = 1 (minZIndex2d),
   guiQueue +0x90 = GUIQUEUE_GENERAL. Primary vftable 0x00b664ac, 35 slots: Instance's 28 with
   askAddChild at +0x3c, then canProcessMeAndDescendants, getZIndex, getGuiQueue, isGuiLeaf,
   getChildRect2D, handleResize, recalculateAbsoluteSizeAndPosition (+0x70..+0x88).
   2016 differs: it adds the *Float members and fireChangedEvent, renames the last slot
   recalculateAbsolutePlacement and adds getCanvasRect. */

#pragma once

#include "V8DataModel/GuiBase.h"
#include "G3D/Vector2.h"
#include "G3D/Rect2D.h"

namespace RBX {

	extern const char* const sGuiBase2d;

	// VTABLE: PLAYER 0x00b664ac RBX::Debugable
	// VTABLE: STUDIO 0x00c33f64 RBX::Debugable
	// VTABLE: PLAYER 0x00b6647c RBX::GuiTarget
	// VTABLE: STUDIO 0x00c33f34 RBX::GuiTarget
	// VTABLE: PLAYER 0x00b66448 RBX::IAdornable
	// VTABLE: STUDIO 0x00c33f00 RBX::IAdornable
	// SIZE 0x94
	class GuiBase2d
		: public DescribedNonCreatable<GuiBase2d, GuiBase, sGuiBase2d, Reflection::ClassDescriptor::PERSISTENT>
	{
		typedef DescribedNonCreatable<GuiBase2d, GuiBase, sGuiBase2d, Reflection::ClassDescriptor::PERSISTENT> Super;
	public:
		GuiBase2d(const char* name);

		// FUNCTION: PLAYER 0x00927cc0 FOLDED
		// FUNCTION: STUDIO 0x008db930 FOLDED
		virtual bool isGuiLeaf() const { return false; }  // vtable+0x7c

		const G3D::Vector2& getAbsolutePosition() const { return absolutePosition; }
		bool setAbsolutePosition(const G3D::Vector2& value);
		const G3D::Vector2& getAbsoluteSize() const { return absoluteSize; }
		bool setAbsoluteSize(const G3D::Vector2& value);

		G3D::Rect2D getRect2D() const;
		// FUNCTION: PLAYER 0x006bd920
		// FUNCTION: STUDIO 0x0083aa70
		virtual G3D::Rect2D getChildRect2D() const { return getRect2D(); }  // vtable+0x80
		virtual void handleResize(const G3D::Rect2D& viewport, bool force);   // vtable+0x84
		virtual bool recalculateAbsoluteSizeAndPosition(const G3D::Rect2D& viewport);  // vtable+0x88

		/*override*/ bool askAddChild(const Instance* instance) const;

		// FUNCTION: PLAYER 0x006be550 FOLDED
		// FUNCTION: STUDIO 0x0078ad10 FOLDED
		/*implement*/ virtual bool canProcessMeAndDescendants() const { return true; }
		// FUNCTION: PLAYER 0x0091c820
		// FUNCTION: STUDIO 0x00800410
		/*implement*/ virtual int getZIndex() const { return zIndex; }
		// FUNCTION: PLAYER 0x009378a0
		// FUNCTION: STUDIO 0x007b6090
		/*implement*/ virtual GuiQueue getGuiQueue() const { return guiQueue; }

		// The shared `xor al,al; ret` (PLAYER 0x00927cc0), marked at isGuiLeaf above.
		/*override*/ bool shouldRender2d() const { return false; }
		// FUNCTION: PLAYER 0x006bd940
		// FUNCTION: STUDIO 0x0083aa90
		/*override*/ bool isVisible(const G3D::Rect2D& rect) const { return rect.intersects(getRect2D()); }

		static Reflection::PropDescriptor<GuiBase2d, G3D::Vector2> prop_AbsoluteSize;
		static Reflection::PropDescriptor<GuiBase2d, G3D::Vector2> prop_AbsolutePosition;

		void recursiveRender2d(RBX::Adorn* adorn);

	protected:
		void setGuiQueue(GuiQueue queue) { guiQueue = queue; }

		G3D::Vector2 absolutePosition;  // 0x7c
		G3D::Vector2 absoluteSize;      // 0x84
		int zIndex;                     // 0x8c
		GuiQueue guiQueue;              // 0x90

	private:
		static void RecursiveRenderChildren(shared_ptr<Instance> instance, RBX::Adorn* adorn);
	};

	// Out-of-line copies the original emitted in this class's .obj (the Described ctor its ctor calls).
	// clang-format off
	// TEMPLATE: PLAYER 0x007bc730
	// TEMPLATE: STUDIO 0x008d9160
	// ?xywh@Rect2D@G3D@@SA?AV12@ABVVector2@2@0@Z
	// TEMPLATE: PLAYER 0x007bc800
	// TEMPLATE: STUDIO 0x008d9230
	// ??$getset@P8GuiBase2d@RBX@@BEABVVector2@G3D@@XZH@?$PropDescriptor@VGuiBase2d@RBX@@VVector2@G3D@@@Reflection@RBX@@SA?AV?$auto_ptr@VGetSet@?$TypedPropertyDescriptor@VVector2@G3D@@@Reflection@RBX@@@std@@P8GuiBase2d@2@BEABVVector2@G3D@@XZH@Z
	// TEMPLATE: PLAYER 0x007bc9d0
	// TEMPLATE: STUDIO 0x008d9330
	// ??$visitChildren@V?$bind_t@XP6AXV?$shared_ptr@VInstance@RBX@@@boost@@ABVRect2D@G3D@@_N@ZV?$list3@U?$arg@$00@boost@@V?$value@VRect2D@G3D@@@_bi@2@V?$value@_N@42@@_bi@2@@_bi@boost@@@Instance@RBX@@QBEXABV?$bind_t@XP6AXV?$shared_ptr@VInstance@RBX@@@boost@@ABVRect2D@G3D@@_N@ZV?$list3@U?$arg@$00@boost@@V?$value@VRect2D@G3D@@@_bi@2@V?$value@_N@42@@_bi@2@@_bi@boost@@@Z
	// TEMPLATE: PLAYER 0x007bcab0
	// TEMPLATE: STUDIO 0x008d9470
	// ??$?0PBD@?$Described@VGuiBase2d@RBX@@$E?sGuiBase2d@2@3QBDBV?$NonFactoryProduct@VGuiBase@RBX@@$E?sGuiBase2d@2@3QBDB@2@$0BL@$0A@@Reflection@RBX@@QAE@PBD@Z
	// TEMPLATE: PLAYER 0x007bcb30
	// TEMPLATE: STUDIO 0x008d94f0
	// ??$?0P8GuiBase2d@RBX@@BEABVVector2@G3D@@XZH@?$PropDescriptor@VGuiBase2d@RBX@@VVector2@G3D@@@Reflection@RBX@@QAE@PBD0P8GuiBase2d@2@BEABVVector2@G3D@@XZHUAttributes@PropertyDescriptor@12@W4Permissions@Security@2@@Z
	// The class descriptor accessor fastDynamicCast<GuiBase2d> calls (kept in another .obj).
	// TEMPLATE: PLAYER 0x00541290
	// TEMPLATE: STUDIO 0x005efe10
	// ?classDescriptor@?$Described@VGuiBase2d@RBX@@$E?sGuiBase2d@2@3QBDBV?$NonFactoryProduct@VGuiBase@RBX@@$E?sGuiBase2d@2@3QBDB@2@$0BL@$0A@@Reflection@RBX@@SAAAVClassDescriptor@23@XZ
	// clang-format on

} // namespace RBX
