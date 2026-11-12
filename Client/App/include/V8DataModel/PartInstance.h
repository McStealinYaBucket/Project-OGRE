/* V8DataModel/PartInstance.h — October 2013 (physical part).
   Evidence: init<FWPartInstance> 0x44B86E + property setters/getters
     (setRenderMaterial 0x448F10, setTransparency 0x448F92, setReflectance
     0x448FFA, setColor 0x449056, setPartLocked 0x448D8C, setAlphaModifier
     0x4427A2, getColor 0x44B49A, getTransparencyXml 0x44B512,
     getRenderMaterial 0x44B4D6, getAlphaModifier 0x44B520, getReflectance
     0x44B52E, getPartLocked 0x44B53C, getColor3 0x44B3EC, setColor3 0x44B418)
     + prop_* descriptor symbols in those bodies.
     sPart = "BasePart": the 0.125 ReflectionMetadata.xml lists "BasePart"
     ("A structural class, not creatable", holding Color/CFrame/CanCollide)
     alongside "Part" ("A plastic building block"). PartInstance is the
     abstract base; the instantiable "Part" is BasicPartInstance
     (FactoryProduct, sBasicPart = "Part" — what Instance.new("Part") and
     the file loaders resolve).
     Parent chain Instance -> PVInstance -> PartInstance; FW chain FWInstance
     -> FWPVInstance -> FWPartInstance (see PVInstance.h).
   Lua prop names follow 2016 ("Material", "BrickColor", "Color",
   "Transparency", "Reflectance", "Locked") except AlphaModifier (2016
   removed it — "AlphaModifier" is a guess TODO vs its PropDescriptor EA).
   TODO: full decompile (physics Primitive wiring, remaining surface).
   Original file: /Applications/buildAgent/work/fa0a2141fe3d2f4d/Client/App/v8datamodel/PartInstance.h

   ---- VTAB MAP (session 101, Studio symbolled; object vptr -> 0xEE1C1C) ----
   Proven by setPartSizeUnjoined 0x4489CC: vptr+0xBC holds the 1-arg void
   callee 0x448870 (setPartSizeXml); the Mach-O vtable header is
   [vptr-8]=offset_to_top(0) [vptr-4]=typeinfo(0xEE1C18) so the object
   pointer is 0xEE1C1C and slot N lives at 0xEE1C1C + 4N.  Offsets:
     +00 Debugable::dump 0x145258      +40 getMinimumUiSizeCustom 0x485E98
     +04 ~PartInstance   0x443CC6      +44 getResizeHandleMask 0x686EA0
     +08 ~PartInstance   0x443C86      +48 getDragUtilitiesSupport 0x485EF0
     +0C initOnDemand    0x444114      +4C setTranslationUi 0x4483DE
     +10 Instance::setIsArchivable    +50 getTranslationUi 0x4484A0
     +14 Instance::destroy             +54 setRotationUi 0x4484C0
     +18 Instance::luaClone           +58 getRotationUi 0x4485B8
     +1C setName         0x4441B4      +5C setPartSizeXml 0x448870
     +20 getPersistentDataCost 0x2CBE40  +60 getResizeIncrement 0x507030
     +24 Instance::canClientCreate    +64 getMinimumResizeIncrement 0x686C30
     +28 onServiceProvider 0x44523E   +68 resize(NormalId,int) 0x44917A
     +2C Instance::createChild        +6C getFormFactor 0x467630
     +30 Instance::writeXml           +70 setCoordinateFrame 0x447A86
     +34 Instance::raiseEventInvocation   +74 setLinearVelocity 0x448650
     +38 verifySetParent              +78 setRotationalVelocity 0x448796
     +3C verifySetAncestor            +7C setFriction 0x4490B6
     +40 verifyAddChild               +80 setElasticity 0x449118
     +44 verifyAddDescendant          +84 getSurface 0x449BF6
     +48 askAddChild                  +88 setCanCollide 0x448A72
     +4C askForbidChild               +8C setAnchored 0x448AC4
     +50 askForbidParent              +90 join 0x44504C
     +54 askSetParent    0x444E5A     +94 destroyJoints 0x444FE0
     +58 onAncestorChanged 0x4456C0   +98 setRenderMaterial 0x448F10
     +5C Instance::onDescendantAdded  +9C setTransparency 0x448F92
     +60 Instance::onDescendantRemoving +A0 setReflectance 0x448FFA
     +64 Instance::onChildAdded      +A4 setColor 0x449056
     +68 Instance::onChildRemoving   +A8 setPartLocked 0x448D8C
     +6C onChildRemoved  0x442D7C     +AC onSleepingChanged 0x444C98
     +70 Instance::onChildChanged    +B0 reportTouches 0x4446A6
     +74 Instance::onPropertyChanged +B4 onClumpChanged 0x444C3E
     +78 PVInstance::readProperty    +B8 onNetworkIsSleepingChanged 0x448A0E
     +7C topHashCode    0x46D4E0     +BC onBuoyancyChanged 0x444DF2
     +80 childHashCode  0x519920     +C0 isInContinousMotion 0x444E3A
     +84 hitTestImpl    0x447594     +C4 onGuidChanged 0x44428A
     +88 computeExtentsWorld 0x442A62 +C8 getLocation 0x37D7D0
     +8C legacyTraverseState 0x4476B0 +CC getRenderLocation 0x2803E0
     +90 getClassName   0x486630     +D0 getRenderSize 0x280400
     +94 hasThreeDimensionalSize 0x37DC60
     +98 getPartType    0x44CA1C     +9C getMinimumUiSize 0x686BA0
   AUDIT (P0, 2026-10-04): virtuals below are in binary slot order
   (batch265 atlas); inherited Instance/PVInstance/NonFactoryProduct
   slots need no decls. Missing bodies (22/33/50/53/54/57) get decls
   when they land (each needs a real body; do not stub).
*/

#pragma once

#include "V8DataModel/PVInstance.h"
#include "V8World/Primitive.h"
#include "V8World/IMoving.h"
#include "V8Tree/OnDemand.h"
#include "GfxBase/IAdornable.h"
#include "rbx/signal.h"
#include "Util/BrickColor.h"
#include "Util/Faces.h"
#include "Util/Guid.h"
#include "Util/InterpolatedCFrame.h"
#include "Util/Material.h"

namespace RBX {

	class Surface;

	extern const char* const sPart;

	// Binary order proven by setPartSizeUi 0x44264E: the vtbl+0xCC slot
	// (getFormFactor, 0x467630) result is compared against 2 -> xml.y =
	// ui.y / 0.4f (a "plate" is 0.4 studs) and against 1 -> / 1.2f (the
	// custom-part UI unit). So CUSTOM==1 and BRICK==2, i.e. the 2016 order
	// SYMETRIC, CUSTOM, BRICK, PLATE, CYLINDER, PYRAMID — NOT the
	// SYMETRIC/BRICK/PLATE/CUSTOM order this header previously carried.
	enum FormFactor
	{
		SYMETRIC = 0,
		CUSTOM,
		BRICK,
		PLATE,
		CYLINDER,
		PYRAMID
	};
	enum PartType
	{
		BALL_PART = 0,
		BLOCK_PART,
		CYLINDER_PART,
		TRUSS_PART,
		WEDGE_PART,
		PRISM_PART,
		PYRAMID_PART,
		PARALLELRAMP_PART,
		RIGHTANGLERAMP_PART
	};

	// VTABLE: PLAYER 0x00b4c784 RBX::Debugable
	// VTABLE: STUDIO 0x00c08b8c RBX::Debugable
	class PartInstance
		: public DescribedNonCreatable<PartInstance, PVInstance, sPart, Reflection::ClassDescriptor::PERSISTENT>
		, public IMoving  // +96 MI base (thunks Thn96; 6 overrides below).
	{
		friend class InterpolatedCFrame;  // +73/+257 latch sync (see header note).
public:
		PartInstance();
		PartInstance(const char* name, FWPartInstance* fw);
		// mac: 0x4437AE C2(size): name "Part" + full physics init (see cpp).
		PartInstance(const G3D::Vector3& initialSize);

	private:
		// Shared 0x4437AE tail (surfaces/size/defaults/owner).
		void initPhysics(const G3D::Vector3& initialSize);

	public:
		// ---- Binary slot-ordered virtuals (78-slot vtable 0xEE1C1C;
		// inherited Instance/PVInstance/NonFactoryProduct slots omitted;
		// missing bodies (22/33/50/53/54/57) declared when they land) ----
		// [slot 3]
		virtual OnDemandInstance* initOnDemand();  // 0x444114: news OnDemandPartInstance.
		// [slot 7]
		// [7] +0x1C setName 0x4441B4 (213B): base setName (FastLog lifetime
		// lines TODO -- channel absent), then HumanoidRootPart->mult 2,
		// Torso->mult 1, else mult 0 on the Primitive.
		virtual void setName(const std::string& value);
		// [slot 8]
		// [8] +0x20 getPersistentDataCost 0x2CBE40 (25B): base + 16.
		virtual int getPersistentDataCost() const;
		// [slot 10]
		// [10] +0x28 onServiceProvider 0x44523E (166B): NULL new-provider
		// no-op; else FWService lookup + holder check-or-register (see cpp).
		// Binary returns the provider; ours is void like the base -- no
		// in-tree caller uses the return (same documented-DELTA precedent
		// as checkAndReleaseContacts).
		virtual void onServiceProvider(ServiceProvider* oldProvider,
			ServiceProvider* newProvider);
		// [slot 21]
		// [21] +0x54 askSetParent 0x444E5A (83B): non-NULL parent that isA
		// ModelInstance (the +36 compare renders as pointer-truth: true iff
		// the parent is non-null after the isA gate).
		virtual bool askSetParent(const Instance* parent) const;
		// [slot 27]
		// [27] +0x6C onChildRemoved 0x442D7C (61B): if on-demand exists and
		// its +40 touchTransmitter is the removed child, clear it.
		virtual void onChildRemoved(Instance* child);
		// vtable+0x78 hitTestImpl (PVInstance virtual; 2013 vftable 0x00b4c784).
		/*override*/ virtual bool hitTestImpl(const RbxRay& worldRay, G3D::Vector3& worldHitPoint);
		// [slot 34]
		// [49] +0xC4 computeExtentsWorld 0x442A62 (37B): forwards the
		// Primitive's world extents.  Const like the binary (__ZNK).
		virtual Extents computeExtentsWorld() const;
		// [slot 35]
		// [52] +0xD0 legacyTraverseState 0x4476B0 (183B): assert
		// onDemandRead() && legacyOffset (PartInstance.cpp:1115), then
		// setGridCorner(cf * *legacyOffset).  See the cpp for the operand-
		// order note.
		virtual void legacyTraverseState(const G3D::CoordinateFrame& cf);
		// [slot 37]
		// ---- Size/UI query family (all 78-slot vtable slots, all decoded) ----
		// [37] +0x94 hasThreeDimensionalSize 0x37DC60 -- `mov eax,1`.
		// FUNCTION: PLAYER 0x006be550 FOLDED
		// FUNCTION: STUDIO 0x0078ad10 FOLDED
		virtual bool hasThreeDimensionalSize() const { return true; }
		// [slot 38]
		virtual PartType getPartType() const;
		// [slot 39]
		// [39] +0x9C getMinimumUiSize 0x686BA0 -- pointer to a function-local
		// `static G3D::Vector3 minSize` built under a __cxa_guard.  uiToXmlSize
		// reads it through the returned pointer (maxss [eax+0/+4/+8]).
		virtual const G3D::Vector3* getMinimumUiSize() const;
		// [slot 40]
		// [40] +0xA0 getMinimumUiSizeCustom 0x485E98 -- identical shape, value
		// 0.2f per component (3 dwords of 0x3E4CCCCD written under the guard).
		virtual const G3D::Vector3* getMinimumUiSizeCustom() const;
		// [slot 41]
		// [41] +0xA4 getResizeHandleMask 0x686EA0 -- Faces(63) (all six).
		// FUNCTION: PLAYER 0x007234c0
		// FUNCTION: STUDIO 0x0079f370
		virtual Faces getResizeHandleMask() const { return Faces(63); }
		// [slot 42]
		// [42] +0xA8 getDragUtilitiesSupport 0x485EF0 -- `mov eax,1`.
		// FUNCTION: PLAYER 0x006be550 FOLDED
		// FUNCTION: STUDIO 0x0078ad10 FOLDED
		virtual bool getDragUtilitiesSupport() const { return true; }
		// [slot 43]
		// Script-facing Position/Rotation: translation + Euler-degrees views of the
		// Primitive CFrame (setTranslationUi 0x4483DE / getTranslationUi 0x4484A0,
		// setRotationUi 0x4484C0 / getRotationUi 0x4485B8; slots 43-46).
		// 2013 order is set before get (vtable+0x9c set, +0xa0 get, +0xa4 set, +0xa8 get;
		// vftable 0x00b4c784 and iOS 10-29 agree).
		virtual void setTranslationUi(const G3D::Vector3& value);
		virtual G3D::Vector3 getTranslationUi() const;
		virtual void setRotationUi(const G3D::Vector3& value);
		virtual G3D::Vector3 getRotationUi() const;
		// [slot 47]
		// [28] +0x5C setPartSizeXml 0x448870 (309B): 3-word change-detect,
		// 3D ? value : (x,x,x) splat (Vector3::xxx, spelled directly since
		// vendor G3D is untouchable), legacyOffset corner-save iff
		// on-demand, Primitive::setSize, raise prop_Size (+ unk_FB4AE0
		// second descriptor, EA unknown -- DELTA), frameDirty = 1 (+180)
		// iff on-demand exists.
		virtual void setPartSizeXml(const G3D::Vector3& value);
		// mac: 0x44264E (300B, batch307: destroyJoints + form-factor Y adjust
		// + uiToXmlSize + xml/size compare (setPartSizeXml or
		// unk_FB4AE0 raise DELTA) + safeMove + join + onDemand signal).
		// Not virtual in 2013: no slot between setPartSizeXml (+0xac) and getResizeIncrement (+0xb0).
		void setPartSizeUi(const G3D::Vector3& value);
		// mac: 0x4489CC (66B, batch307: uiToXmlSize + setPartSizeXml + safeMove). Not virtual in 2013.
		void setPartSizeUnjoined(const G3D::Vector3& value);
		// [slot 50] +0xC8 resize 0x44917A (67B, batch304): destroyJoints
		// (slot 61) + resizeImpl + join (slot 60), by name.
		virtual bool resize(NormalId normal, int increment);
		// mac: 0x4491BE (1011B, batch305): face-sign grow + form-factor Y
		// adjust + CF shift + setCoordinateFrame/setPartSizeXml, with
		// the ContactManager/Dragger revert arm (Dragger era DELTA).
		bool resizeImpl(NormalId normal, int increment);
		// [slot 48]
		// [60] +0xF0-side neighbours: [49] +0xC4 getResizeIncrement 0x507030
		// returns 1; [53] +0xD4 getMinimumResizeIncrement 0x686C30 returns
		// 0.01f (fld dword_C683A8).
		// FUNCTION: PLAYER 0x00962a50 FOLDED
		// FUNCTION: STUDIO 0x00892cf0 FOLDED
		virtual int getResizeIncrement() const { return 1; }
		// [slot 49]
		// FUNCTION: PLAYER 0x007234e0
		// FUNCTION: STUDIO 0x0079f390
		virtual float getMinimumResizeIncrement() const { return 0.01f; }
		// [slot 51]
		// Trivial queries (EAs above; subclasses override).
		virtual FormFactor getFormFactor() const;
		// [slot 52]
		virtual void setCoordinateFrame(const G3D::CoordinateFrame& value);
		virtual void setLinearVelocity(const G3D::Vector3& value);      // vtable+0xc4
		virtual void setRotationalVelocity(const G3D::Vector3& value);  // vtable+0xc8
		// [slot 55]
		virtual void setFriction(float value);
		// [slot 56]
		virtual void setElasticity(float value);
		// vtable+0xd4 (PLAYER 0x0071c220, STUDIO 0x007b7c80). Surface is not written yet, so
		// this is declared only (the link-stub generator binds it).
		virtual Surface getSurface(const RbxRay& gridRay, int& surfaceId);
		// [slot 58]
		virtual void setCanCollide(bool value);
		// [slot 59]
		virtual void setAnchored(bool value);
		// [slot 60]
		// vtable slot [60] +0xF0 = 0x44504C. setPartSizeUi 0x44264E calls
		// vtbl+0xF0 after a resize; destroyJoints is slot [61] +0xF4 and is the
		// first thing setPartSizeUi does.
		virtual void join();
		// [slot 61]
		virtual void destroyJoints();
		// [slot 62]
		virtual void setRenderMaterial(Material value);
		// [slot 63]
		virtual void setTransparency(float value);
		// [slot 64]
		virtual void setReflectance(float value);
		// [slot 65]
		virtual void setColor(BrickColor value);
		// [slot 66]
		virtual void setPartLocked(bool value);
		// vtable+0xfc, the last slot PartInstance adds (iOS 10-29 PartInstance::partNeeds3dAdorn;
		// PLAYER 0x00927cc0 is a shared `xor al,al; ret`). BasicPartInstance overrides it.
		virtual bool partNeeds3dAdorn() const { return false; }
		// [slot 67]
		virtual void onSleepingChanged(bool sleeping);  // IMoving slot 0 (0x444C98).: see cpp.
		// [slot 68]
		virtual bool reportTouches() const;  // IMoving slot 1 (0x4446A6).: signal checks + transmitter + child find.
		// [slot 69]
		virtual void onClumpChanged();  // IMoving slot 2 (0x444C3E).: clump signal + gfx dead path.
		// [slot 70]
		virtual void onNetworkIsSleepingChanged();  // IMoving slot 3 (0x448A0E; no Time arg).
		// [slot 71]
		virtual void onBuoyancyChanged(bool buoyant);  // IMoving slot 4 (0x444DF2).
		// [slot 72]
		virtual bool isInContinousMotion();  // IMoving slot 5 (0x444E3A).
		// [slot 73]
		// [49] +0xC4 onGuidChanged 0x44428A (136B): assert + push the +24
		// Guid into the Primitive via setGuid (FW-myWorld half queues with
		// the FW-world lifecycle; the Primitive half is live below).
		virtual void onGuidChanged();
		// [slot 74]
		// [50] +0xC8 getLocation 0x37D7D0 (61B): CoordinateFrame copy of
		// getCoordinateFrame (rotation via Matrix3 copy + translation words
		// -- identical to the copy ctor).  Non-const like the binary
		// (mangled __ZN..., no K).
		virtual G3D::CoordinateFrame getLocation();
		// [slot 75]
		// [51] +0xCC getRenderLocation 0x2803E0 (31B): forwards
		// calcRenderingCoordinateFrame.  Non-const per the mangling.
		virtual G3D::CoordinateFrame getRenderLocation();
		// [slot 76]
		// [52] +0xD0 getRenderSize 0x280400 (31B): forwards getPartSizeUi.
		virtual G3D::Vector3 getRenderSize();
		// [slot 77]
		// [77] +0x134 onCameraNear 0x444BBA (93B, batch265): alpha ramp
		// by distance (<3 -> 0.0, >=6 -> 1.0, else 0.5).
		virtual void onCameraNear(float distance);
		// FW-backed physical appearance (bodies from the EAs above).
		Material getRenderMaterial() const;
		BrickColor getColor() const;
		G3D::Color3 getColor3() const;
		void setColor3(const G3D::Color3& value);  // not virtual in 2013 (no primary slot)
		float getTransparencyXml() const;
		float getReflectance() const;
		float getAlphaModifier() const;
		void setAlphaModifier(float value);  // not virtual in 2013
		bool getPartLocked() const;

		// Physics live state (Primitive-backed; EAs above + PrimGets.txt).
		float getFriction() const;
		float getElasticity() const;
		float getSpecificGravity() const;
		bool getAnchored() const;
		// [34] +0x88 setCanCollide 0x448A72 (82B): prevent = !value; on
		// change, store + raise prop_CanCollide.
		bool getCanCollide() const;
		bool getDragging() const;
		void setDragging(bool value);  // not virtual in 2013

		// Coordinate frame (Primitive-backed; 0x4419EC/0x441A08/0x447A86).
		const G3D::CoordinateFrame& getCoordinateFrame() const;
		void setCoordinateFrameRoot(const G3D::CoordinateFrame& value);  // not virtual in 2013

		// Bring-up view of the +24 Guid (binary: FW-pushed, no getter).
		const Guid& getGuid() const { return guid; }

		// OnDemand-driven IMoving-override helpers (moverrides/ondemand EAs).
		TouchedSignal& getOrCreateTouchedSignal();  // 0x44B6A0: onDemandWrite+48.
		TouchedSignal& getOrCreateTouchedEndedSignal();  // 0x44B6E4: onDemandWrite+56.
		TouchedSignal& getOrCreateLocalSimulationTouchedSignal();  // 0x44B65C: Write+72.
		bool hasTouchTransmitter() const;  // 0x44BA7C: OnDemand+40.
		void addTouchTransmitter();  // 0x442A88: assert-empty + create + parent + store.
		void removeTouchTransmitter();  // 0x442B6E: unparent + clear +40.
		// mac: 0x44A8F0 (153B) / 0x445590 (153B).  NON-virtual, non-const (the
		// mangling carries no const and there is no thunk).  Gate on
		// hasTouchTransmitter, debounce through the transmitter's
		// checkTouch/checkUntouch, and fire touchedSignal (+48) /
		// touchedEndedSignal (+56) with a shared_ptr<Instance> COPY of the
		// argument (the binary copies the pair into a stack temp, bumps
		// the count, and passes it BY VALUE to TouchedSignal::operator()).
		void reportTouch(const shared_ptr<PartInstance>& otherPart);
		void reportUntouch(const shared_ptr<PartInstance>& otherPart);
		// Gfx link (+176, always NULL this build; 2016-shape GfxPart* slot).
		void* gfxPartLink;

		// Sleep/motion/buoyancy state for the IMoving overrides (moverrides spec):
		// sleeping store +0x68 (0 = awake; asserted by onSleepingChanged :606),
		// motion flag +0x101 (isInContinousMotion), buoyancy signal +0x144.
		// Full MI (+96 base + 6 overrides) TODO pending OnDemand/IAdornable wiring.
		bool getSleeping() const;
		bool getNetworkIsSleeping() const;
		void setNetworkIsSleeping(bool value);
		bool addInterpolationSample(const G3D::CoordinateFrame& cf, const RemoteTime& rt, bool isRemote);  // 0x447962 (returns the flag).
		bool getLastWasRemote() const;
		// Bring-up helpers (not in binary): test access to interpolation state.
		void computeInterpolatedForTest(G3D::CoordinateFrame& out) { interpolatedCFrame.computeValue(out, this); }
		float sampleIntervalForTest() const { return interpolatedCFrame.getSampleInterval(); }
		// +0x144 event signal (public like RunService::runTransitionSignal).
		rbx::signals::signal_with_args<1, void(bool)> buoyancyChangedSignal;

		virtual PartInstance* getPrimaryPart();

		// mac: 0x4449D6 (NOT virtual -- absent from the 78-slot vtable; called
		// directly by setPartSizeUi 0x44264E and setPartSizeUnjoined 0x4489CC).
		// switch (getFormFactor()): BRICK -> round(max(v,minUi)) then y *= 0.4f
		// (plate stud); CUSTOM -> same then y *= 1.2f; SYMMETRIC -> the plain
		// rounded max; anything else -> the UNROUNDED max against
		// getMinimumUiSizeCustom().
		G3D::Vector3 uiToXmlSize(const G3D::Vector3& uiSize) const;
		// mac: 0x442602 (28B, __ZNK3RBX12PartInstance14getPartSizeXmlEv): the
		// const& thin -- derefs this+0xAC (the Primitive px our
		// `primitive` shared_ptr holds at its head) and forwards
		// Primitive::getSize.  rarc10 raw: sub esp,8 / mov eax,
		// [this+0AC] / mov [esp],eax / call __ZNK3RBX9Primitive7getSizeEv
		// / add esp,8 / retn.  Callers (renderBubbleImposters 0x5C4CA9
		// staging the imposter VP) deref the return -- x/y as a movsd
		// qword, +8 as the z dword -- so the const& materializes, not
		// a copy (contrast the 47B copy sister below).
		const G3D::Vector3& getPartSizeXml() const;
		// mac: 0x44261E (47B): Vector3 copy of the Primitive's size.  NOT virtual
		// (absent from the 78-slot vtable; getRenderSize calls it directly).
		G3D::Vector3 getPartSizeUi() const;
		// mac: 0x4481A6 (61B): true iff a const mechanism exists and its owner's
		// +224 byte is set.  getConstMechanism() is NULL for every unlinked
		// prim (see Primitive), which short-circuits first -- exact today.
		// The tail (getConstMechanismPrimitive +224) queues with the
		// Mechanism shell; reaching it returns false (no silent pass).
		bool computeNetworkOwnerIsSomeoneElse() const;
		// mac: 0x447C1E (759B): rendering frame into out.  a2==root: interpolated
		// value past the uiStep latch, else snapshot+copy of the current
		// frame.  a2!=root: relative composition (see cpp note).
		void calcRenderingCoordinateFrame(G3D::CoordinateFrame& out,
			PartInstance* root);

		// mac: 0x44787C: RBXASSERT(getPartPrimitive()) (PartInstance.cpp:1161) then
		// Dragger::safeMoveNoDrop({prim}, Vector3::zero(), *cm) when there is a
		// ContactManager. NOT virtual (absent from the 78-slot vtable).
		void safeMove();
		// The PartInstance.cpp:1161 assert reads this+0xAC directly, so the
		// accessor is header-inline in 2013.
		Primitive* getPartPrimitive() const { return primitive.get(); }
		// mac: 0x442830 (batch284: :1615 prim assert + prim+92 owner read).
		SystemAddress getNetworkOwner() const;
		// mac: 0x44562A (batch284: on-demand +196 byte, NULL-safe).
		bool getIsCurrentlyStreamRemovingPart() const;
		// mac: 0x441A7C / 0x441A9C (31B each, batch233): PV velocity reads
		// (non-virtual per 2016 header + direct-call evidence).
		const G3D::Vector3& getLinearVelocity() const;
		const G3D::Vector3& getRotationalVelocity() const;
		// mac: 0x448776 (31B): the PV+48 linear read again -- the render-side
		// entry the chat-bubble arm calls (identical body to
		// getLinearVelocity in the binary; kept as its own symbol).
		const G3D::Vector3& getVelocity() const;

		// mac: 0x443524 (209B, sret): interpolation-enabled + owner-remote +
		// last-sample-local yields the mechanism root's last computed
		// frame (the +0xF4 model-back-cast carries the mechanism-root
		// part); any gate fails -> the primitive's plain frame.
		G3D::CoordinateFrame getRenderingCoordinateFrame() const;

		// mac: 0x447F16 (656B, sret): asserts that this and mechanismRootPart
		// share a mechanism (:1257) and that the root's mechanism
		// primitive is the root itself (:1258), then self-root =>
		// interpolatedCFrame.renderedFrame copy; else the relative
		// composition against a recursive root call.
		G3D::CoordinateFrame getLastComputedRenderingCoordinateFrame(
			PartInstance const* mechanismRootPart) const;

		static const Reflection::EnumPropDescriptor<PartInstance, Material> prop_renderMaterial;
		static const Reflection::PropDescriptor<PartInstance, BrickColor> prop_BrickColor;
		static const Reflection::PropDescriptor<PartInstance, G3D::Color3> prop_Color;
		static const Reflection::PropDescriptor<PartInstance, float> prop_Transparency;
		static const Reflection::PropDescriptor<PartInstance, float> prop_Reflectance;
		static const Reflection::PropDescriptor<PartInstance, float> prop_AlphaModifier;
		static const Reflection::PropDescriptor<PartInstance, bool> prop_Locked;
		// Lua names PE-verified (0.125 string table); categories TODO (2016
		// uses category_Part for Friction/Elasticity — absent in this tree).
		static const Reflection::PropDescriptor<PartInstance, float> prop_Friction;
		static const Reflection::PropDescriptor<PartInstance, float> prop_Elasticity;
		static const Reflection::PropDescriptor<PartInstance, bool> prop_Anchored;
		static const Reflection::PropDescriptor<PartInstance, bool> prop_Dragging;
		// 0xFA8818 (binary descriptor global).  Name PE-verified by the Lua
		// API ("CanCollide"); category follows the other Behavior bools
		// (categories TODO per the note above).
		static const Reflection::PropDescriptor<PartInstance, bool> prop_CanCollide;
		// "CFrame" per 2016 (raised by setCoordinateFrame 0x447A86 alongside
		// "Position" (0xFB48AC, slots 43/44 TranslationUi) + "Rotation" (0xFB48D8,
		// slots 45/46 RotationUi) — InitFunc_284 resolved, all three raised).
		static const Reflection::PropDescriptor<PartInstance, G3D::CoordinateFrame> prop_CFrame;
		static const Reflection::PropDescriptor<PartInstance, G3D::Vector3> prop_Position;
		static const Reflection::PropDescriptor<PartInstance, G3D::Vector3> prop_Rotation;
		static const Reflection::PropDescriptor<PartInstance, bool> prop_NetworkIsSleeping;
		// Binary descriptor global 0xFA8684 ("Size", Data like prop_Position;
		// getter getPartSizeUi, setter setPartSizeXml).
		static const Reflection::PropDescriptor<PartInstance, G3D::Vector3> prop_Size;

	private:
		// +6 dwords/+24: Guid pushed to the Primitive by onGuidChanged
		// mac: 0x44428A (passed by const ref to Primitive::setGuid).
		Guid guid;
		// +43 (dword): physics Primitive (live state — friction/elasticity/
		// anchored/dragging/specificGravity live here, NOT in FW). Created
		// eagerly per part (2016 ctor pattern); creator-EA and exact
		// geometry-type mapping TODO.
		shared_ptr<Primitive> primitive;
		int sleepingState;
		bool lastWasRemote;  // +256 latch (addInterpolationSample 0x447962).
		// +260 (a2[65]): ui-step latch for calcRenderingCoordinateFrame.
		int renderUiStep;
		// killswitch for the interpolation branch (BSS-false in the binary).
		static bool disableInterpolation;
		bool networkIsSleeping;  // get/set trivial; 2013 Primitive routing TODO-verify.
		bool continousMotion;
		// Render-interpolation history (binary: value member at +184; clearHistory
		// called from setCoordinateFrame 0x447A86; full algorithm TODO, see header).
		InterpolatedCFrame interpolatedCFrame;
		// Render adornable (binary subobject +108; member until MI integration).
		IAdornable adornable;
	};

} // namespace RBX
