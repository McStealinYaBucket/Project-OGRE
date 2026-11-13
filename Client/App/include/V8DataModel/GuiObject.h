/* V8DataModel/GuiObject.h -- October 2013.
   Layout from the PLAYER ctor 0x007c77c0 (1101 bytes; GuiObject(name, active)) and the derived ctors
   (Frame 0x006c2b70 and GuiLabel 0x007cb640 start their members at 0x21c, so sizeof is 0x21c):
     0x94 0, 0x98/0x99/0x9a false, 0x9c/0xa0 0.0f,
     0xa4..0xcc five remote_signal<void(int,int)> (MouseEnter, MouseLeave, MouseMoved,
       MouseWheelForward, MouseWheelBackward; each pairs with the replicator below whose
       connectionSignal points at its +4),
     0xcc..0x16c five {int <event>ConnectionCount; EventReplicator<GuiObject, void(int,int)>} (0x20 each;
       2016's DECLARE_EVENT_REPLICATOR_SIG order: the count, then the 0x1c-byte replicator, vftable
       0x00b66ad0, whose instance the ctor sets to this last),
     0x16c dragStoppedSignal (int,int), 0x174 dragBeginSignal (UDim2), then the DragStopped and
       DragBegin count/replicator pairs (0x17c, 0x19c; the UDim2 replicator's vftable 0x00b66ad8),
     0x1bc serverGuiObject (set by setServerGuiObject 0x007c34e0), 0x1bd false (read by 0x007bcf60),
     0x1c0 0, 0x1c4 size, 0x1d4 position (UDim2 zeroed), 0x1e4 sizeConstraint 0 (read by 0x007bcf30),
     0x1e8 borderSizePixel 1 (read by 0x007bcf40), 0x1ec backgroundColor BrickColor(194).color3(),
     0x1f8 borderColor BrickColor(26).color3(), 0x204 visible true (canProcessMeAndDescendants
     0x006bd990 returns it), 0x205 active (the ctor's bool), 0x208 backgroundTransparency 0.0f
     (read by 0x007bd050), 0x20c..0x21c 0.0f x4 (handleResize 0x007c88f0 stores the window rect).
   Names follow the 2016 member order where the 2013 init values agree (colors, sizeConstraint,
   borderSizePixel, visible/active); the rest are placeholders.
   Primary vftable 0x00b6729c, 41 slots: GuiBase2d's 35, then setTransparencyLegacy,
   renderSelectionBox, setServerGuiObject, processMouseEvent, processKeyEvent, checkForResize. */

#pragma once

#include "V8DataModel/GuiBase2d.h"
#include "Util/UDim.h"
#include "Util/G3DCore.h"
#include "rbx/signal.h"
#include "decomp.h"

namespace RBX {

	extern const char* const sGuiObject;

	// VTABLE: PLAYER 0x00b6729c RBX::Debugable
	// VTABLE: STUDIO 0x00c34d54 RBX::Debugable
	// VTABLE: PLAYER 0x00b6726c RBX::GuiTarget
	// VTABLE: STUDIO 0x00c34d24 RBX::GuiTarget
	// VTABLE: PLAYER 0x00b67238 RBX::IAdornable
	// VTABLE: STUDIO 0x00c34cf0 RBX::IAdornable
	// SIZE 0x21c
	class GuiObject
		: public DescribedNonCreatable<GuiObject, GuiBase2d, sGuiObject, Reflection::ClassDescriptor::PERSISTENT>
	{
		typedef DescribedNonCreatable<GuiObject, GuiBase2d, sGuiObject, Reflection::ClassDescriptor::PERSISTENT> Super;
	public:
		enum TweenStatus
		{
			TWEEN_CANCELED = 0,
			TWEEN_COMPLETED = 1,
		};

		enum TweenEasingStyle
		{
			EASING_STYLE_LINEAR = 0,
			EASING_STYLE_SINE = 1,
			EASING_STYLE_BACK = 2,
			EASING_STYLE_QUAD = 3,
			EASING_STYLE_QUART = 4,
			EASING_STYLE_QUINT = 5,
			EASING_STYLE_BOUNCE = 6,
			EASING_STYLE_ELASTIC = 7,
		};

		enum TweenEasingDirection
		{
			EASING_DIRECTION_IN = 0,
			EASING_DIRECTION_OUT = 1,
			EASING_DIRECTION_IN_OUT = 2,
		};

		// GuiObject::SizeConstraint (mangle __ZN3RBX9GuiObject17setSizeConstraintENS0_14SizeConstraintE).
		enum SizeConstraint
		{
			RelativeXY,
			RelativeXX,
			RelativeYY
		};

		GuiObject(const char* name = NULL, bool active = false);  // defaults: the unported subclasses default-construct

		void setZIndex(int value) { zIndex = value; }

		void setSize(const UDim2& value)
		{
			if (size != value)
				size = value;
		}
		UDim2 getSize() const { return size; }
		void setPosition(const UDim2& value)
		{
			if (position != value)
				position = value;
		}
		UDim2 getPosition() const { return position; }
		void setBackgroundTransparency(float value);
		float getBackgroundTransparency() const { return backgroundTransparency; }
		void setBorderSizePixel(int value);
		int getBorderSizePixel() const { return borderSizePixel; }
		void setSizeConstraint(SizeConstraint value)
		{
			if (sizeConstraint != value)
				sizeConstraint = value;
		}
		void setVisible(bool value);
		bool getVisible() const { return visible; }
		void setClipping(bool value);
		bool getClipping() const { return clipping; }

		static Reflection::PropDescriptor<GuiObject, bool> prop_ClipsDescendants;
		static Reflection::PropDescriptor<GuiObject, int> prop_BorderSizePixel;
		static Reflection::PropDescriptor<GuiObject, bool> prop_Visible;
		static Reflection::PropDescriptor<GuiObject, float> prop_BackgroundTransparency;

		GuiObject* firstAncestorClipping();
		G3D::Rect2D getClippedRect();
		bool mouseIsOver(const G3D::Vector2& point);

		virtual bool askSetParent(const Instance* parent) const;

		// Climbs to the first ScreenGui (or a BillboardGui) and copies its window rect.
		G3D::Rect2D getWindowRect(GuiBase2d* start);

		// The inset rect of a scale-9 frame style (Frame::getChildRect2D).
		static G3D::Rect2D Scale9Rect2D(const G3D::Rect2D& rect, float border, float minSize);
		/*override*/ void handleResize(const G3D::Rect2D& rect, bool flag);

		// The parentless render entry the bubble chat calls (ChatOutput).
		void legacyRender2d(RBX::Adorn* adorn, const G3D::Rect2D& rect);

	private:
		undefined4 m_unk0x94;           // 0x94
		bool m_unk0x98;                 // 0x98
		bool m_unk0x99;                 // 0x99
		bool m_unk0x9a;                 // 0x9a
		G3D::Vector2 m_unk0x9c;         // 0x9c

	public:
		rbx::remote_signal<void(int, int)> mouseEnterSignal;          // 0xa4
		rbx::remote_signal<void(int, int)> mouseLeaveSignal;          // 0xac
		rbx::remote_signal<void(int, int)> mouseMovedSignal;          // 0xb4
		rbx::remote_signal<void(int, int)> mouseWheelForwardSignal;   // 0xbc
		rbx::remote_signal<void(int, int)> mouseWheelBackwardSignal;  // 0xc4

	private:
		int varMouseEnterConnectionCount;                       // 0xcc
		undefined m_unk0xd0[0xec - 0xd0];                       // 0xd0 eventReplicatorMouseEnter
		int varMouseLeaveConnectionCount;                       // 0xec
		undefined m_unk0xf0[0x10c - 0xf0];                      // 0xf0 eventReplicatorMouseLeave
		int varMouseMovedConnectionCount;                       // 0x10c
		undefined m_unk0x110[0x12c - 0x110];                    // 0x110 eventReplicatorMouseMoved
		int varMouseWheelForwardConnectionCount;                // 0x12c
		undefined m_unk0x130[0x14c - 0x130];                    // 0x130 eventReplicatorMouseWheelForward
		int varMouseWheelBackwardConnectionCount;               // 0x14c
		undefined m_unk0x150[0x16c - 0x150];                    // 0x150 eventReplicatorMouseWheelBackward

	public:
		rbx::remote_signal<void(int, int)> dragStoppedSignal;   // 0x16c
		rbx::remote_signal<void(UDim2)> dragBeginSignal;        // 0x174

	private:
		int varDragStoppedConnectionCount;                      // 0x17c
		undefined m_unk0x180[0x19c - 0x180];                    // 0x180 eventReplicatorDragStopped
		int varDragBeginConnectionCount;                        // 0x19c
		undefined m_unk0x1a0[0x1bc - 0x1a0];                    // 0x1a0 eventReplicatorDragBegin

		bool serverGuiObject;           // 0x1bc
		bool clipping;                  // 0x1bd
		undefined4 m_unk0x1c0;          // 0x1c0
		UDim2 size;                     // 0x1c4
		UDim2 position;                 // 0x1d4
		SizeConstraint sizeConstraint;  // 0x1e4
		int borderSizePixel;            // 0x1e8
		G3D::Color3 backgroundColor;    // 0x1ec
		G3D::Color3 borderColor;        // 0x1f8
		bool visible;                   // 0x204
		bool active;                    // 0x205
		float backgroundTransparency;   // 0x208
		G3D::Rect2D windowRectCache;    // 0x20c
	};

	// Out-of-line copies the original emitted in this class's .obj (the Described ctor its ctor calls).
	// clang-format off
	// TEMPLATE: PLAYER 0x007c4940
	// TEMPLATE: STUDIO 0x008e14e0
	// ??$?0PBD@?$Described@VGuiObject@RBX@@$E?sGuiObject@2@3QBDBV?$NonFactoryProduct@VGuiBase2d@RBX@@$E?sGuiObject@2@3QBDB@2@$0BL@$0A@@Reflection@RBX@@QAE@PBD@Z
	// clang-format on

} // namespace RBX
