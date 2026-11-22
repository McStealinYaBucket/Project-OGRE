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
#include "Util/BrickColor.h"
#include "rbx/signal.h"
#include "V8DataModel/EventReplicator.h"
#include <boost/function.hpp>
#include <boost/scoped_ptr.hpp>
#include "decomp.h"

namespace RBX {

	extern const char* const sGuiObject;
	class ContentId;
	class GuiDrawImage;

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

		void setZIndex(int value);

		// The getters below are the out-of-line copies the prop descriptors take (PLAYER 0x007bcf30..).
		void setSize(UDim2 value);
		UDim2 getSize() const { return size; }
		void setPosition(UDim2 value);
		UDim2 getPosition() const { return position; }
		void setBackgroundTransparency(float value);
		// FUNCTION: PLAYER 0x007bd050
		// FUNCTION: STUDIO 0x008d9a10
		float getBackgroundTransparency() const { return backgroundTransparency; }
		void setBorderSizePixel(int value);
		// FUNCTION: PLAYER 0x007bcf40
		// FUNCTION: STUDIO 0x008d98f0
		int getBorderSizePixel() const { return borderSizePixel; }
		void setSizeConstraint(SizeConstraint value);
		// FUNCTION: PLAYER 0x007bcf30
		// FUNCTION: STUDIO 0x008d98e0
		SizeConstraint getSizeConstraint() const { return sizeConstraint; }
		void setDraggable(bool value);
		// FUNCTION: PLAYER 0x007bcf50
		// FUNCTION: STUDIO 0x008d9900
		bool getDraggable() const { return draggable; }
		void setVisible(bool value);
		bool getVisible() const { return visible; }
		void setActive(bool value);
		bool getActive() const { return active; }
		void setClipping(bool value);
		// FUNCTION: PLAYER 0x007bcf60
		// FUNCTION: STUDIO 0x008d9910
		bool getClipping() const { return clipping; }
		// FUNCTION: PLAYER 0x007bcf70
		// FUNCTION: STUDIO 0x008d9920
		BrickColor getBorderColor() const { return BrickColor::closest(borderColor); }
		void setBorderColor(BrickColor value);
		// FUNCTION: PLAYER 0x007bcfb0
		// FUNCTION: STUDIO 0x008d9960
		G3D::Color3 getBorderColor3() const { return borderColor; }
		void setBorderColor3(G3D::Color3 value);
		// FUNCTION: PLAYER 0x007bcfe0
		// FUNCTION: STUDIO 0x008d9990
		BrickColor getBackgroundColor() const { return BrickColor::closest(backgroundColor); }
		void setBackgroundColor(BrickColor value);
		// FUNCTION: PLAYER 0x007bd020
		// FUNCTION: STUDIO 0x008d99d0
		G3D::Color3 getBackgroundColor3() const { return backgroundColor; }
		void setBackgroundColor3(G3D::Color3 value);

		static Reflection::PropDescriptor<GuiObject, UDim2> prop_Size;
		static Reflection::PropDescriptor<GuiObject, UDim2> prop_Position;
		static Reflection::EnumPropDescriptor<GuiObject, SizeConstraint> prop_SizeConstraint;
		static Reflection::PropDescriptor<GuiObject, BrickColor> prop_BorderColor;
		static Reflection::PropDescriptor<GuiObject, G3D::Color3> prop_BorderColor3;
		static Reflection::PropDescriptor<GuiObject, BrickColor> prop_BackgroundColor;
		static Reflection::PropDescriptor<GuiObject, G3D::Color3> prop_BackgroundColor3;
		static Reflection::PropDescriptor<GuiObject, bool> prop_Draggable;
		static Reflection::PropDescriptor<GuiObject, bool> prop_Active;
		static Reflection::PropDescriptor<GuiObject, int> prop_ZIndex;
		static Reflection::PropDescriptor<GuiObject, float> prop_Transparency;
		static Reflection::PropDescriptor<GuiObject, bool> prop_ClipsDescendants;
		static Reflection::PropDescriptor<GuiObject, int> prop_BorderSizePixel;
		static Reflection::PropDescriptor<GuiObject, bool> prop_Visible;
		static Reflection::PropDescriptor<GuiObject, float> prop_BackgroundTransparency;

		GuiObject* firstAncestorClipping();
		G3D::Rect2D getClippedRect();
		bool mouseIsOver(const G3D::Vector2& point);

		virtual bool askSetParent(const Instance* parent) const;

		/*override*/ GuiResponse process(const GuiEvent& event);  // GuiTarget slot 0

		// GuiObject's own primary vftable slots 35..40 (PLAYER 0x00b6729c).
		// FUNCTION: PLAYER 0x006bd9a0
		// FUNCTION: STUDIO 0x00837080
		virtual void setTransparencyLegacy(float value) { setBackgroundTransparency(value); }  // vtable+0x8c
		virtual void renderSelectionBox(Adorn* adorn);                                       // vtable+0x90
		virtual void setServerGuiObject();                                                   // vtable+0x94
		virtual GuiResponse processMouseEvent(const GuiEvent& event);                        // vtable+0x98
		// FUNCTION: PLAYER 0x007bc510
		// FUNCTION: STUDIO 0x008d8f30
		virtual GuiResponse processKeyEvent(const GuiEvent& event) { return GuiResponse(); } // vtable+0x9c
		virtual void checkForResize();                                                       // vtable+0xa0
		void forceResize();

		/*override*/ void onPropertyChanged(const Reflection::PropertyDescriptor& descriptor);

		// Climbs to the first ScreenGui (or a BillboardGui) and copies its window rect.
		G3D::Rect2D getWindowRect(GuiBase2d* start);

		// The inset rect of a scale-9 frame style (Frame::getChildRect2D).
		static G3D::Rect2D Scale9Rect2D(const G3D::Rect2D& rect, float border, float minSize);
		/*override*/ void handleResize(const G3D::Rect2D& rect, bool flag);

		// The parentless render entry the bubble chat calls (ChatOutput).
		void legacyRender2d(RBX::Adorn* adorn, const G3D::Rect2D& rect);
		void render2dScale9Impl(Adorn* adorn, const ContentId& texId, const G3D::Vector2int16& scaleEdgeSize,
			const G3D::Vector2& minSize, GuiDrawImage& guiImageDraw, G3D::Rect2D& rect, GuiObject* clippingObject);

		// 2016 field names (2013 has the same shape: ~Tweens deletes positionTween, then sizeTween,
		// each ~Tween only destroys its callback at +0x38).
		struct Tween
		{
			UDim2 start;                                   // 0x00
			UDim2 end;                                     // 0x10
			float elapsedTime;                             // 0x20
			float totalTime;                               // 0x24
			TweenEasingDirection style;                    // 0x28
			TweenEasingStyle variance;                     // 0x2c
			float delayTime;                               // 0x30
			boost::function<void(TweenStatus)> callback;   // 0x38
		};

		struct Tweens
		{
			boost::scoped_ptr<Tween> sizeTween;      // 0x00
			boost::scoped_ptr<Tween> positionTween;  // 0x04
		};

	private:
		boost::scoped_ptr<Tweens> tweens;  // 0x94 (deleted by ~GuiObject through ~Tweens 0x006bcc20)
		bool selectionBox;                 // 0x98 (renderSelectionBox 0x007bf080 reads it)
		bool draggable;                    // 0x99 (setDraggable 0x007bd3d0)
		bool dragging;                     // 0x9a
		G3D::Vector2 lastMousePosition;    // 0x9c

	public:
		rbx::remote_signal<void(int, int)> mouseEnterSignal;          // 0xa4
		rbx::remote_signal<void(int, int)> mouseLeaveSignal;          // 0xac
		rbx::remote_signal<void(int, int)> mouseMovedSignal;          // 0xb4
		rbx::remote_signal<void(int, int)> mouseWheelForwardSignal;   // 0xbc
		rbx::remote_signal<void(int, int)> mouseWheelBackwardSignal;  // 0xc4

	private:
		DECLARE_EVENT_REPLICATOR_SIG(GuiObject, MouseEnter, void(int, int))          // 0xcc, 0xd0
		DECLARE_EVENT_REPLICATOR_SIG(GuiObject, MouseLeave, void(int, int))          // 0xec, 0xf0
		DECLARE_EVENT_REPLICATOR_SIG(GuiObject, MouseMoved, void(int, int))          // 0x10c, 0x110
		DECLARE_EVENT_REPLICATOR_SIG(GuiObject, MouseWheelForward, void(int, int))   // 0x12c, 0x130
		DECLARE_EVENT_REPLICATOR_SIG(GuiObject, MouseWheelBackward, void(int, int))  // 0x14c, 0x150

	public:
		rbx::remote_signal<void(int, int)> dragStoppedSignal;   // 0x16c
		rbx::remote_signal<void(UDim2)> dragBeginSignal;        // 0x174

	private:
		DECLARE_EVENT_REPLICATOR_SIG(GuiObject, DragStopped, void(int, int))  // 0x17c, 0x180
		DECLARE_EVENT_REPLICATOR_SIG(GuiObject, DragBegin, void(UDim2))       // 0x19c, 0x1a0

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

	// The implicit destructors, emitted out of line in ImageLabel's .obj (the first user of the vftable).
	// SYNTHETIC: PLAYER 0x006bcc20
	// SYNTHETIC: STUDIO 0x00838e10
	// RBX::GuiObject::Tweens::~Tweens
	// SYNTHETIC: PLAYER 0x006bd630
	// SYNTHETIC: STUDIO 0x0083a890
	// RBX::GuiObject::~GuiObject

	// Out-of-line copies the original emitted in this class's .obj (the Described ctor its ctor calls).
	// clang-format off
	// TEMPLATE: PLAYER 0x007c4940
	// TEMPLATE: STUDIO 0x008e14e0
	// ??$?0PBD@?$Described@VGuiObject@RBX@@$E?sGuiObject@2@3QBDBV?$NonFactoryProduct@VGuiBase2d@RBX@@$E?sGuiObject@2@3QBDB@2@$0BL@$0A@@Reflection@RBX@@QAE@PBD@Z
	// clang-format on


	// Out-of-line template and implicit-member copies the original emitted for this class, paired by
	// identical masked bytes (tools/unpaired_twins.py on a 2026-11-13 build).
	// clang-format off
	// TEMPLATE: PLAYER 0x006bc300
	// ??1?$EventReplicatorBase@VGuiObject@RBX@@$$A6AXHH@Z@RBX@@QAE@XZ
	// TEMPLATE: PLAYER 0x006bc3a0
	// ??1?$EventReplicatorBase@VGuiObject@RBX@@$$A6AXVUDim2@2@@Z@RBX@@QAE@XZ
	// TEMPLATE: PLAYER 0x006bcbe0
	// TEMPLATE: STUDIO 0x00838dd0
	// ??1?$scoped_ptr@UTween@GuiObject@RBX@@@boost@@QAE@XZ
	// TEMPLATE: PLAYER 0x006bd550
	// ??1?$scoped_ptr@UTweens@GuiObject@RBX@@@boost@@QAE@XZ
	// TEMPLATE: PLAYER 0x006be380
	// ??1?$NonFactoryProduct@VGuiObject@RBX@@$E?sGuiLabel@2@3QBDB@RBX@@UAE@XZ
	// TEMPLATE: PLAYER 0x006bf300
	// ??0?$TypedPropertyDescriptor@VColor3@G3D@@@Reflection@RBX@@IAE@AAVClassDescriptor@12@PBD1V?$auto_ptr@VGetSet@?$TypedPropertyDescriptor@VColor3@G3D@@@Reflection@RBX@@@std@@UAttributes@PropertyDescriptor@12@W4Permissions@Security@2@@Z
	// TEMPLATE: PLAYER 0x007bd7d0
	// ?fireItem@?$signal_with_args@$01$$A6AXW4CombinedSignalType@Instance@RBX@@PBVICombinedSignalData@23@@Z@signals@rbx@@CAXPAVslot@?$signal@$$A6AXW4CombinedSignalType@Instance@RBX@@PBVICombinedSignalData@23@@Z@23@W4CombinedSignalType@Instance@RBX@@PBVICombinedSignalData@78@@Z
	// TEMPLATE: PLAYER 0x007bd7f0
	// TEMPLATE: STUDIO 0x008da190
	// ?fireItem@?$signal_with_args@$00$$A6AXVUDim2@RBX@@@Z@signals@rbx@@CAXPAVslot@?$signal@$$A6AXVUDim2@RBX@@@Z@23@VUDim2@RBX@@@Z
	// TEMPLATE: PLAYER 0x007bd8f0
	// TEMPLATE: STUDIO 0x008277e0
	// ?destruct_func@?$typed_holder@VWeakFunctionRef@Lua@RBX@@@implementation@rbx@@SAXPAD@Z
	// TEMPLATE: PLAYER 0x007bd9d0
	// ??_9GuiObject@RBX@@$BIM@AE
	// TEMPLATE: PLAYER 0x007bd9e0
	// TEMPLATE: STUDIO 0x007ff280
	// ??_9GuiBase2d@RBX@@$BHE@AE
	// TEMPLATE: PLAYER 0x007beea0
	// ??$getset@P8GuiBase2d@RBX@@BEHXZP8GuiObject@2@AEXH@Z@?$PropDescriptor@VGuiObject@RBX@@H@Reflection@RBX@@SA?AV?$auto_ptr@VGetSet@?$TypedPropertyDescriptor@H@Reflection@RBX@@@std@@P8GuiBase2d@2@BEHXZP8GuiObject@2@AEXH@Z@Z
	// TEMPLATE: PLAYER 0x007bef10
	// TEMPLATE: STUDIO 0x008db940
	// ?setValue@?$GetSetImpl@P8GuiObject@RBX@@BE?AVUDim2@2@XZP812@AEXV32@@Z@?$PropDescriptor@VGuiObject@RBX@@VUDim2@2@@Reflection@RBX@@UBEXPAVDescribedBase@34@ABVUDim2@4@@Z
	// TEMPLATE: PLAYER 0x007bef60
	// TEMPLATE: STUDIO 0x008db990
	// ?getValue@?$GetSetImpl@P8GuiObject@RBX@@BE?AVUDim2@2@XZP812@AEXV32@@Z@?$PropDescriptor@VGuiObject@RBX@@VUDim2@2@@Reflection@RBX@@UBE?AVUDim2@4@PBVDescribedBase@34@@Z
	// TEMPLATE: PLAYER 0x007bf320
	// TEMPLATE: STUDIO 0x008dbd30
	// ?listenerConnectionAdded@?$EventReplicatorBase@VGuiObject@RBX@@$$A6AXHH@Z@RBX@@IAEXXZ
	// TEMPLATE: PLAYER 0x007bf380
	// TEMPLATE: STUDIO 0x008dbd90
	// ??0?$TypedPropertyDescriptor@VUDim2@RBX@@@Reflection@RBX@@IAE@AAVClassDescriptor@12@PBD1V?$auto_ptr@VGetSet@?$TypedPropertyDescriptor@VUDim2@RBX@@@Reflection@RBX@@@std@@UAttributes@PropertyDescriptor@12@W4Permissions@Security@2@@Z
	// TEMPLATE: PLAYER 0x007bf470
	// ??$getset@P8GuiObject@RBX@@BE?AVUDim2@2@XZP812@AEXV32@@Z@?$PropDescriptor@VGuiObject@RBX@@VUDim2@2@@Reflection@RBX@@SA?AV?$auto_ptr@VGetSet@?$TypedPropertyDescriptor@VUDim2@RBX@@@Reflection@RBX@@@std@@P8GuiObject@2@BE?AVUDim2@2@XZP852@AEXV62@@Z@Z
	// TEMPLATE: PLAYER 0x007bf4d0
	// ??$getset@P8GuiObject@RBX@@BEHXZP812@AEXH@Z@?$PropDescriptor@VGuiObject@RBX@@H@Reflection@RBX@@SA?AV?$auto_ptr@VGetSet@?$TypedPropertyDescriptor@H@Reflection@RBX@@@std@@P8GuiObject@2@BEHXZP852@AEXH@Z@Z
	// TEMPLATE: PLAYER 0x007bf530
	// ??$getset@P8GuiObject@RBX@@BE?AW4SizeConstraint@12@XZP812@AEXW4312@@Z@?$PropDescriptor@VGuiObject@RBX@@W4SizeConstraint@12@@Reflection@RBX@@SA?AV?$auto_ptr@VGetSet@?$TypedPropertyDescriptor@W4SizeConstraint@GuiObject@RBX@@@Reflection@RBX@@@std@@P8GuiObject@2@BE?AW4SizeConstraint@52@XZP852@AEXW4652@@Z@Z
	// TEMPLATE: PLAYER 0x007bf590
	// ??$getset@P8GuiObject@RBX@@BE?AVBrickColor@2@XZP812@AEXV32@@Z@?$PropDescriptor@VGuiObject@RBX@@VBrickColor@2@@Reflection@RBX@@SA?AV?$auto_ptr@VGetSet@?$TypedPropertyDescriptor@VBrickColor@RBX@@@Reflection@RBX@@@std@@P8GuiObject@2@BE?AVBrickColor@2@XZP852@AEXV62@@Z@Z
	// TEMPLATE: PLAYER 0x007bf5f0
	// ??$getset@P8GuiObject@RBX@@BE?AVColor3@G3D@@XZP812@AEXV34@@Z@?$PropDescriptor@VGuiObject@RBX@@VColor3@G3D@@@Reflection@RBX@@SA?AV?$auto_ptr@VGetSet@?$TypedPropertyDescriptor@VColor3@G3D@@@Reflection@RBX@@@std@@P8GuiObject@2@BE?AVColor3@G3D@@XZP852@AEXV67@@Z@Z
	// TEMPLATE: PLAYER 0x007bf650
	// ??$getset@P8GuiObject@RBX@@BEMXZP812@AEXM@Z@?$PropDescriptor@VGuiObject@RBX@@M@Reflection@RBX@@SA?AV?$auto_ptr@VGetSet@?$TypedPropertyDescriptor@M@Reflection@RBX@@@std@@P8GuiObject@2@BEMXZP852@AEXM@Z@Z
	// TEMPLATE: PLAYER 0x007bf6b0
	// ??$getset@P8GuiObject@RBX@@BE_NXZP812@AEX_N@Z@?$PropDescriptor@VGuiObject@RBX@@_N@Reflection@RBX@@SA?AV?$auto_ptr@VGetSet@?$TypedPropertyDescriptor@_N@Reflection@RBX@@@std@@P8GuiObject@2@BE_NXZP852@AEX_N@Z@Z
	// TEMPLATE: PLAYER 0x007bf8d0
	// TEMPLATE: STUDIO 0x008dc330
	// ?copyValue@?$TypedPropertyDescriptor@VUDim2@RBX@@@Reflection@RBX@@UBEXPBVDescribedBase@23@PAV423@@Z
	// TEMPLATE: PLAYER 0x007bf910
	// TEMPLATE: STUDIO 0x008dc370
	// ?equalValues@?$TypedPropertyDescriptor@VUDim2@RBX@@@Reflection@RBX@@UBE_NPBVDescribedBase@23@0@Z
	// The signal ctors the GuiObject ctor 0x007c77c0 calls for its remote_signal members.
	// TEMPLATE: PLAYER 0x007bf990
	// TEMPLATE: STUDIO 0x008dc3f0
	// ??0?$signal@$$A6AXHH@Z@signals@rbx@@QAE@XZ
	// TEMPLATE: PLAYER 0x007bfa70
	// TEMPLATE: STUDIO 0x008dc4d0
	// ??0?$signal@$$A6AXVUDim2@RBX@@@Z@signals@rbx@@QAE@XZ
	// TEMPLATE: PLAYER 0x007c01f0
	// TEMPLATE: STUDIO 0x008dcde0
	// ?call@?$callable@Vslot@?$signal@$$A6AXVUDim2@RBX@@@Z@signals@rbx@@V?$bind_t@XV?$mf1@XV?$EventReplicatorImpl@$00VGuiObject@RBX@@$$A6AXVUDim2@2@@Z@RBX@@VUDim2@2@@_mfi@boost@@V?$list2@V?$value@PAV?$EventReplicatorImpl@$00VGuiObject@RBX@@$$A6AXVUDim2@2@@Z@RBX@@@_bi@boost@@U?$arg@$00@3@@_bi@3@@_bi@boost@@$00$$A6AXVUDim2@RBX@@@Z@rbx@@UAEXVUDim2@RBX@@@Z
	// TEMPLATE: PLAYER 0x007c02c0
	// TEMPLATE: STUDIO 0x008dceb0
	// ?reset@?$scoped_array@VGuiDrawImage@RBX@@@boost@@QAEXPAVGuiDrawImage@RBX@@@Z
	// TEMPLATE: PLAYER 0x007c06f0
	// TEMPLATE: STUDIO 0x008dd360
	// ?getVariant@?$TypedPropertyDescriptor@VUDim2@RBX@@@Reflection@RBX@@UBEXPBVDescribedBase@23@AAVVariant@23@@Z
	// TEMPLATE: PLAYER 0x007c1f90
	// TEMPLATE: STUDIO 0x008deb10
	// ?setVariant@?$TypedPropertyDescriptor@VUDim2@RBX@@@Reflection@RBX@@UBEXPAVDescribedBase@23@ABVVariant@23@@Z
	// TEMPLATE: PLAYER 0x007c26e0
	// TEMPLATE: STUDIO 0x008df260
	// ??$connect@V?$bind_t@XV?$mf1@XV?$EventReplicatorImpl@$00VGuiObject@RBX@@$$A6AXVUDim2@2@@Z@RBX@@VUDim2@2@@_mfi@boost@@V?$list2@V?$value@PAV?$EventReplicatorImpl@$00VGuiObject@RBX@@$$A6AXVUDim2@2@@Z@RBX@@@_bi@boost@@U?$arg@$00@3@@_bi@3@@_bi@boost@@@?$signal@$$A6AXVUDim2@RBX@@@Z@signals@rbx@@QAE?AVconnection@12@ABV?$bind_t@XV?$mf1@XV?$EventReplicatorImpl@$00VGuiObject@RBX@@$$A6AXVUDim2@2@@Z@RBX@@VUDim2@2@@_mfi@boost@@V?$list2@V?$value@PAV?$EventReplicatorImpl@$00VGuiObject@RBX@@$$A6AXVUDim2@2@@Z@RBX@@@_bi@boost@@U?$arg@$00@3@@_bi@3@@_bi@boost@@@Z
	// setListenerMode bodies called by setServerGuiObject 0x007c34e0 and GuiButton's 0x007c35a0 (one per
	// instantiation; the 2013 linker folded the GuiObject/GuiButton twins the same way ours does).
	// TEMPLATE: PLAYER 0x007c2e80
	// TEMPLATE: STUDIO 0x008dfa00
	// ?setListenerMode@?$EventReplicatorBase@VGuiObject@RBX@@$$A6AXHH@Z@RBX@@QAEX_N@Z
	// TEMPLATE: PLAYER 0x007c2f60
	// TEMPLATE: STUDIO 0x008dfae0
	// ?setListenerMode@?$EventReplicatorBase@VGuiObject@RBX@@$$A6AXVUDim2@2@@Z@RBX@@QAEX_N@Z
	// TEMPLATE: PLAYER 0x007c3040
	// TEMPLATE: STUDIO 0x008dfbc0
	// ?setListenerMode@?$EventReplicatorBase@VGuiButton@RBX@@$$A6AXXZ@RBX@@QAEX_N@Z
	// TEMPLATE: PLAYER 0x007c3120
	// TEMPLATE: STUDIO 0x008dfca0
	// ?setListenerMode@?$EventReplicatorBase@VGuiButton@RBX@@$$A6AXHH@Z@RBX@@QAEX_N@Z
	// TEMPLATE: PLAYER 0x007c3200
	// TEMPLATE: STUDIO 0x008dfde0
	// ?signalProducedIncremented@?$EventReplicatorImpl@$01VGuiObject@RBX@@$$A6AXHH@Z@RBX@@IAEXHH@Z
	// TEMPLATE: PLAYER 0x007c3240
	// TEMPLATE: STUDIO 0x008dfd80
	// ?signalProducedIncremented@?$EventReplicatorImpl@$00VGuiObject@RBX@@$$A6AXVUDim2@2@@Z@RBX@@IAEXVUDim2@2@@Z
	// TEMPLATE: PLAYER 0x007c62f0
	// TEMPLATE: STUDIO 0x008e2e90
	// ??$?0PAV?$signal@$$A6AXHH@Z@signals@rbx@@@?$callable@Vslot@?$signal@$$A6AXHH@Z@signals@rbx@@V?$function@$$A6AXHH@Z@boost@@$01$$A6AXHH@Z@rbx@@QAE@ABV?$function@$$A6AXHH@Z@boost@@PAV?$signal@$$A6AXHH@Z@signals@1@@Z
	// TEMPLATE: PLAYER 0x007c6970
	// ??$?0PAV?$signal@$$A6AXVUDim2@RBX@@@Z@signals@rbx@@@?$callable@Vslot@?$signal@$$A6AXVUDim2@RBX@@@Z@signals@rbx@@V?$function@$$A6AXVUDim2@RBX@@@Z@boost@@$00$$A6AXVUDim2@RBX@@@Z@rbx@@QAE@ABV?$function@$$A6AXVUDim2@RBX@@@Z@boost@@PAV?$signal@$$A6AXVUDim2@RBX@@@Z@signals@1@@Z
	// TEMPLATE: PLAYER 0x007c69e0
	// TEMPLATE: STUDIO 0x008e33e0
	// ??R?$function1@XVUDim2@RBX@@@boost@@QBEXVUDim2@RBX@@@Z
	// TEMPLATE: PLAYER 0x007c6a60
	// TEMPLATE: STUDIO 0x008e3460
	// ?call@?$callable@Vslot@?$signal@$$A6AXVUDim2@RBX@@@Z@signals@rbx@@V?$function@$$A6AXVUDim2@RBX@@@Z@boost@@$00$$A6AXVUDim2@RBX@@@Z@rbx@@UAEXVUDim2@RBX@@@Z
	// TEMPLATE: PLAYER 0x007c9d60
	// ?connectGeneric@?$EventDescImpl@$01VGuiObject@RBX@@$$A6AXHH@ZV?$remote_signal@$$A6AXHH@Z@rbx@@PQ12@V34@@Reflection@RBX@@UBE?AVconnection@signals@rbx@@PAVDescribedBase@23@V?$shared_ptr@VGenericSlotWrapper@Reflection@RBX@@@boost@@@Z
	// TEMPLATE: PLAYER 0x007ca7c0
	// ??_E?$NonFactoryProduct@VGuiObject@RBX@@$E?sGuiLabel@2@3QBDB@RBX@@UAEPAXI@Z
	// clang-format on

} // namespace RBX

// Template copies the original emitted in this class's .obj (identical bodies, tools/unpaired_twins.py).
// clang-format off
// TEMPLATE: PLAYER 0x006bc9e0
// TEMPLATE: STUDIO 0x00838db0
// ??1?$EventReplicator@VGuiObject@RBX@@$$A6AXHH@Z@RBX@@QAE@XZ
// TEMPLATE: PLAYER 0x006bc9f0
// TEMPLATE: STUDIO 0x00838dc0
// ??1?$EventReplicatorImpl@$00VGuiObject@RBX@@$$A6AXVUDim2@2@@Z@RBX@@QAE@XZ
// TEMPLATE: PLAYER 0x006bd570
// TEMPLATE: STUDIO 0x00839dd0
// ??1?$signal_with_args@$01$$A6AXHH@Z@signals@rbx@@QAE@XZ
// TEMPLATE: PLAYER 0x006bd580
// TEMPLATE: STUDIO 0x00839de0
// ??1?$signal_with_args@$00$$A6AXVUDim2@RBX@@@Z@signals@rbx@@QAE@XZ
// TEMPLATE: PLAYER 0x006bd590
// TEMPLATE: STUDIO 0x00839df0
// ??1?$remote_signal@$$A6AXHH@Z@rbx@@QAE@XZ
// TEMPLATE: PLAYER 0x006bd5e0
// TEMPLATE: STUDIO 0x00839e40
// ??1?$remote_signal@$$A6AXVUDim2@RBX@@@Z@rbx@@QAE@XZ
// TEMPLATE: STUDIO 0x008db8c0
// ??$getset@P8GuiBase2d@RBX@@BEHXZP8GuiObject@2@AEXH@Z@?$PropDescriptor@VGuiObject@RBX@@H@Reflection@RBX@@SA?AV?$auto_ptr@VGetSet@?$TypedPropertyDescriptor@H@Reflection@RBX@@@std@@P8GuiBase2d@2@BEHXZP8GuiObject@2@AEXH@Z@Z
// TEMPLATE: STUDIO 0x008dbe60
// ??$getset@P8GuiObject@RBX@@BE?AVUDim2@2@XZP812@AEXV32@@Z@?$PropDescriptor@VGuiObject@RBX@@VUDim2@2@@Reflection@RBX@@SA?AV?$auto_ptr@VGetSet@?$TypedPropertyDescriptor@VUDim2@RBX@@@Reflection@RBX@@@std@@P8GuiObject@2@BE?AVUDim2@2@XZP852@AEXV62@@Z@Z
// TEMPLATE: STUDIO 0x008dbec0
// ??$getset@P8GuiObject@RBX@@BEHXZP812@AEXH@Z@?$PropDescriptor@VGuiObject@RBX@@H@Reflection@RBX@@SA?AV?$auto_ptr@VGetSet@?$TypedPropertyDescriptor@H@Reflection@RBX@@@std@@P8GuiObject@2@BEHXZP852@AEXH@Z@Z
// TEMPLATE: STUDIO 0x008dbf20
// ??$getset@P8GuiObject@RBX@@BE?AW4SizeConstraint@12@XZP812@AEXW4312@@Z@?$PropDescriptor@VGuiObject@RBX@@W4SizeConstraint@12@@Reflection@RBX@@SA?AV?$auto_ptr@VGetSet@?$TypedPropertyDescriptor@W4SizeConstraint@GuiObject@RBX@@@Reflection@RBX@@@std@@P8GuiObject@2@BE?AW4SizeConstraint@52@XZP852@AEXW4652@@Z@Z
// TEMPLATE: STUDIO 0x008dbf80
// ??$getset@P8GuiObject@RBX@@BE?AVBrickColor@2@XZP812@AEXV32@@Z@?$PropDescriptor@VGuiObject@RBX@@VBrickColor@2@@Reflection@RBX@@SA?AV?$auto_ptr@VGetSet@?$TypedPropertyDescriptor@VBrickColor@RBX@@@Reflection@RBX@@@std@@P8GuiObject@2@BE?AVBrickColor@2@XZP852@AEXV62@@Z@Z
// TEMPLATE: STUDIO 0x008dbfe0
// ??$getset@P8GuiObject@RBX@@BE?AVColor3@G3D@@XZP812@AEXV34@@Z@?$PropDescriptor@VGuiObject@RBX@@VColor3@G3D@@@Reflection@RBX@@SA?AV?$auto_ptr@VGetSet@?$TypedPropertyDescriptor@VColor3@G3D@@@Reflection@RBX@@@std@@P8GuiObject@2@BE?AVColor3@G3D@@XZP852@AEXV67@@Z@Z
// TEMPLATE: STUDIO 0x008dc040
// ??$getset@P8GuiObject@RBX@@BEMXZP812@AEXM@Z@?$PropDescriptor@VGuiObject@RBX@@M@Reflection@RBX@@SA?AV?$auto_ptr@VGetSet@?$TypedPropertyDescriptor@M@Reflection@RBX@@@std@@P8GuiObject@2@BEMXZP852@AEXM@Z@Z
// TEMPLATE: STUDIO 0x008dc0a0
// ??$getset@P8GuiObject@RBX@@BE_NXZP812@AEX_N@Z@?$PropDescriptor@VGuiObject@RBX@@_N@Reflection@RBX@@SA?AV?$auto_ptr@VGetSet@?$TypedPropertyDescriptor@_N@Reflection@RBX@@@std@@P8GuiObject@2@BE_NXZP852@AEX_N@Z@Z
// TEMPLATE: PLAYER 0x007c66e0
// TEMPLATE: STUDIO 0x008e30e0
// ??$connect@V?$function@$$A6AXHH@Z@boost@@@?$signal@$$A6AXHH@Z@signals@rbx@@QAE?AVconnection@12@ABV?$function@$$A6AXHH@Z@boost@@@Z
// TEMPLATE: STUDIO 0x008e3370
// ??$?0PAV?$signal@$$A6AXVUDim2@RBX@@@Z@signals@rbx@@@?$callable@Vslot@?$signal@$$A6AXVUDim2@RBX@@@Z@signals@rbx@@V?$function@$$A6AXVUDim2@RBX@@@Z@boost@@$00$$A6AXVUDim2@RBX@@@Z@rbx@@QAE@ABV?$function@$$A6AXVUDim2@RBX@@@Z@boost@@PAV?$signal@$$A6AXVUDim2@RBX@@@Z@signals@1@@Z
// clang-format on
