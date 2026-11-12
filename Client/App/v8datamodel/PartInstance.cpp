/* App/v8datamodel/PartInstance.cpp — October 2013 (ctor shell).
   Evidence: init<FWPartInstance> 0x44B86E + FWDictionnary<FWPartInstance>
     traffic; parent chain Instance -> PVInstance -> PartInstance (see
     PVInstance.h); FW chain FWInstance -> FWPVInstance -> FWPartInstance.
   TODO: full decompile. */

#include "stdafx.h"

#include "V8DataModel/PartInstance.h"
#include "V8DataModel/ModelInstance.h"
#include "V8DataModel/FWService.h"
#include "Util/Utilities.h"
#include "Util/Math.h"
#include "G3D/Vector3.h"
#include "G3D/Matrix3.h"
#include "V8DataModel/TouchTransmitter.h"
#include "V8DataModel/Workspace.h"
#include "V8World/ContactManager.h"
#include "V8World/World.h"
#include "V8World/Mechanism.h"
#include "Tool/Dragger.h"
#include "Util/DFFlag.h"
#include "Util/Object.h"
#include "decomp.h"
#include <cfloat>

namespace RBX {
	const char* const sPart = "BasePart";
}

using namespace RBX;

REFLECTION_BEGIN();
const Reflection::EnumPropDescriptor<PartInstance, Material> PartInstance::prop_renderMaterial("Material", category_Appearance, &PartInstance::getRenderMaterial, &PartInstance::setRenderMaterial);
const Reflection::PropDescriptor<PartInstance, BrickColor> PartInstance::prop_BrickColor("BrickColor", category_Appearance, &PartInstance::getColor, &PartInstance::setColor);
const Reflection::PropDescriptor<PartInstance, G3D::Color3> PartInstance::prop_Color("Color", category_Appearance, &PartInstance::getColor3, &PartInstance::setColor3);
const Reflection::PropDescriptor<PartInstance, float> PartInstance::prop_Transparency("Transparency", category_Appearance, &PartInstance::getTransparencyXml, &PartInstance::setTransparency);
const Reflection::PropDescriptor<PartInstance, float> PartInstance::prop_Reflectance("Reflectance", category_Appearance, &PartInstance::getReflectance, &PartInstance::setReflectance);
const Reflection::PropDescriptor<PartInstance, float> PartInstance::prop_AlphaModifier("AlphaModifier", category_Appearance, &PartInstance::getAlphaModifier, &PartInstance::setAlphaModifier);
const Reflection::PropDescriptor<PartInstance, bool> PartInstance::prop_Locked("Locked", category_Behavior, &PartInstance::getPartLocked, &PartInstance::setPartLocked);
const Reflection::PropDescriptor<PartInstance, float> PartInstance::prop_Friction("Friction", category_Data, &PartInstance::getFriction, &PartInstance::setFriction);
const Reflection::PropDescriptor<PartInstance, float> PartInstance::prop_Elasticity("Elasticity", category_Data, &PartInstance::getElasticity, &PartInstance::setElasticity);
const Reflection::PropDescriptor<PartInstance, bool> PartInstance::prop_Anchored("Anchored", category_Behavior, &PartInstance::getAnchored, &PartInstance::setAnchored);
const Reflection::PropDescriptor<PartInstance, bool> PartInstance::prop_Dragging("DraggingV1", category_Behavior, &PartInstance::getDragging, &PartInstance::setDragging);
const Reflection::PropDescriptor<PartInstance, bool> PartInstance::prop_CanCollide("CanCollide", category_Behavior, &PartInstance::getCanCollide, &PartInstance::setCanCollide);
const Reflection::PropDescriptor<PartInstance, G3D::CoordinateFrame> PartInstance::prop_CFrame("CFrame", category_Data, &PartInstance::getCoordinateFrame, &PartInstance::setCoordinateFrameRoot);
const Reflection::PropDescriptor<PartInstance, G3D::Vector3> PartInstance::prop_Position("Position", category_Data, &PartInstance::getTranslationUi, &PartInstance::setTranslationUi);const Reflection::PropDescriptor<PartInstance, G3D::Vector3> PartInstance::prop_Rotation("Rotation", category_Data, &PartInstance::getRotationUi, &PartInstance::setRotationUi);const Reflection::PropDescriptor<PartInstance, G3D::Vector3> PartInstance::prop_Size("Size", category_Data, &PartInstance::getPartSizeUi, &PartInstance::setPartSizeXml);const Reflection::PropDescriptor<PartInstance, bool> PartInstance::prop_NetworkIsSleeping("NetworkIsSleeping", category_Data, &PartInstance::getNetworkIsSleeping, &PartInstance::setNetworkIsSleeping);
REFLECTION_END();

// Class registrar (see Instance.cpp/Service.cpp note).
namespace RBX {
RBX_REGISTER_CLASS(RBX::PartInstance);
}

namespace RBX {

	// Direct Described base init (PartInstance is only an indirect base).
	typedef DescribedNonCreatable<PartInstance, PVInstance, sPart, Reflection::ClassDescriptor::PERSISTENT> PartSuper;

	PartInstance::PartInstance()
		: PartSuper("Part", FWBase::init<FWPartInstance>((FWPartInstance*)NULL))
		, primitive(new Primitive(Primitive::GEOMETRY_BLOCK))
		, gfxPartLink(NULL)
		, sleepingState(0)
		, continousMotion(false)
		, lastWasRemote(false)
		, renderUiStep(0)
		, networkIsSleeping(false)
	{
		initPhysics(G3D::Vector3(4.0f, 1.0f, 2.0f));
	}

	PartInstance::PartInstance(const char* name, FWPartInstance* fw)
		: PartSuper(name, fw != NULL ? fw : FWBase::init<FWPartInstance>((FWPartInstance*)NULL))
		, primitive(new Primitive(Primitive::GEOMETRY_BLOCK))
		, gfxPartLink(NULL)
		, sleepingState(0)
		, continousMotion(false)
		, lastWasRemote(false)
		, renderUiStep(0)
		, networkIsSleeping(false)
	{
	}

	bool PartInstance::disableInterpolation = false;

} // namespace RBX

namespace RBX {
namespace Reflection
{
	/* EnumDesc<Material> 0x441DC4: pair order verbatim + legacy aliases
	   ("Corroded Metal", "Aluminum" — spaces bar addPair, hence legacy). */
	// STUB: PLAYER 0x00722680
	// STUB: STUDIO 0x007bde50
	template<>
	EnumDesc<Material>::EnumDesc()
		:EnumDescriptor("Material", typeid(Material))
	{
		addPair(Plastic, "Plastic");
		addPair(Wood, "Wood");
		addPair(Slate, "Slate");
		addPair(Concrete, "Concrete");
		addPair(CorrodedMetal, "CorrodedMetal");
		addPair(DiamondPlate, "DiamondPlate");
		addPair(Foil, "Foil");
		addPair(Grass, "Grass");
		addPair(Ice, "Ice");
		addPair(Marble, "Marble");
		addPair(Granite, "Granite");
		addPair(Brick, "Brick");
		addPair(Pebble, "Pebble");
		addPair(Sand, "Sand");
		addPair(Fabric, "Fabric");
		addLegacyName("Corroded Metal", CorrodedMetal);
		addLegacyName("Aluminum", Foil);
	}

	template<>
	Material& Variant::convert<Material>(void)
	{
		return genericConvert<Material>();
	}

	template<>
	const Type& Type::getSingleton<Material>()
	{
		return EnumDesc<Material>::singleton();
	}
	template<> EnumRegistrar<Material> EnumRegistrar<Material>::registrar(0);
	template<> TypeRegistrar<Material> TypeRegistrar<Material>::registrar(0);
} // namespace Reflection
} // namespace RBX

namespace RBX {

	//////////////////////////////////////////////////////////////////////////////////
	// FW-backed physical appearance (EAs in the header comment).
	// Physics live-state calls (Primitive::setSpecificGravity, IAdornable
	// render-dirty) are TODO-skipped: no Primitive is ever attached in
	// bring-up, and the binary touches it unconditionally (in-engine it
	// always exists). The FW + Changed-event behavior below is exact.
	//////////////////////////////////////////////////////////////////////////////////

	/* sub_4435F8 equivalent (called by setRenderMaterial 0x448F10). */
	static float toSpecificGravity(Material material)
	{
		switch (material)
		{
		case Plastic: return 0.7f;
		case Wood: return 0.350f;
		case Marble: return 2.563f;
		case Slate: return 2.691f;
		case Concrete: return 2.403f;
		case Granite: return 2.691f;
		case Brick: return 1.922f;
		case Pebble: return 2.403f;
		case CorrodedMetal: return 7.850f;
		case DiamondPlate: return 7.850f;
		case Foil: return 7.700f;
		case Grass: return 0.9f;
		case Sand: return 1.602f;
		case Fabric: return 0.7f;
		case Ice: return 0.919f;
		default: return 1.0f;
		}
	}

		/* mac: 0x4437AE: PVInstance("Part", fresh FW) + physics init. The default
	   ctor shares it (default size 4x1x2 — TODO verify vs factory path). */
	// STUB: PLAYER 0x007268d0
	// STUB: STUDIO 0x007c1e00
	PartInstance::PartInstance(const G3D::Vector3& initialSize)
		: PartSuper("Part", FWBase::init<FWPartInstance>((FWPartInstance*)NULL))
		, primitive(new Primitive(Primitive::GEOMETRY_BLOCK))
		, gfxPartLink(NULL)
		, sleepingState(0)
		, continousMotion(false)
		, lastWasRemote(false)
		, networkIsSleeping(false)
	{
		initPhysics(initialSize);
	}

	/* mac: 0x4437AE tail: surfaces + size + physical defaults + owner. */
	void PartInstance::initPhysics(const G3D::Vector3& initialSize)
	{
		primitive->setSurfaceType(NORM_Y, STUDS);
		primitive->setSurfaceType(NORM_Y_NEG, INLET);
		primitive->setSize(initialSize);
		primitive->setFriction(0.3f);
		primitive->setElasticity(0.5f);
		primitive->setSpecificGravity(toSpecificGravity(getRenderMaterial()));
		primitive->setAnchoredProperty(false);
		primitive->setPreventCollide(false);
		onGuidChanged();
	}

/* mac: 0x44B4D6 */
	Material PartInstance::getRenderMaterial() const
	{
		return static_cast<const FWPartInstance*>(fw())->material;
	}

	/* mac: 0x448F10: FW compare+set; + Primitive::setSpecificGravity(toSpecificGravity)
	   + raise unk_FB4904 (TODO: unidentified density-family descriptor) + raise.
	   TODO(Primitive): re-enable the guarded physics call when the module lands. */
	// STUB: PLAYER 0x007206b0
	// STUB: STUDIO 0x007bc100
	void PartInstance::setRenderMaterial(Material value)
	{
		FWPartInstance* partFw = static_cast<FWPartInstance*>(fw());
		if (partFw->material != value)
		{
			FWRef ref;
			ref.fwPtr = partFw->sharedSelf();
			FWValue<Material>::set(partFw->material, value, &ref);
			primitive->setSpecificGravity(toSpecificGravity(value));
			this->raisePropertyChanged(prop_renderMaterial);
		}
	}

	/* mac: 0x44B49A */
	BrickColor PartInstance::getColor() const
	{
		return static_cast<const FWPartInstance*>(fw())->brickColor;
	}

	/* mac: 0x449056: FW compare+set; raises BrickColor + Color. */
	// STUB: PLAYER 0x007207d0
	// STUB: STUDIO 0x007bc220
	void PartInstance::setColor(BrickColor value)
	{
		FWPartInstance* partFw = static_cast<FWPartInstance*>(fw());
		if (value != partFw->brickColor)
		{
			FWRef ref;
			ref.fwPtr = partFw->sharedSelf();
			FWValue<BrickColor>::set(partFw->brickColor, value, &ref);
			this->raisePropertyChanged(prop_BrickColor);
			this->raisePropertyChanged(prop_Color);
		}
	}

	/* mac: 0x44B3EC: Color3 of the current BrickColor. */
	G3D::Color3 PartInstance::getColor3() const
	{
		return getColor().color3();
	}

	/* mac: 0x44B418: closest-match BrickColor through the virtual setColor. */
	void PartInstance::setColor3(const G3D::Color3& value)
	{
		setColor(BrickColor::closest(value));
	}

	/* mac: 0x44B512 */
	float PartInstance::getTransparencyXml() const
	{
		return static_cast<const FWPartInstance*>(fw())->transparency;
	}

	/* mac: 0x448F92: FW compare+set + raise. TODO(IAdornable): shouldRenderSetDirty(+108). */
	// STUB: PLAYER 0x00720710
	// STUB: STUDIO 0x007bc160
	void PartInstance::setTransparency(float value)
	{
		FWPartInstance* partFw = static_cast<FWPartInstance*>(fw());
		if (partFw->transparency != value)
		{
			FWRef ref;
			ref.fwPtr = partFw->sharedSelf();
			FWValue<float>::set(partFw->transparency, value, &ref);
			this->raisePropertyChanged(prop_Transparency);
		}
	}

	/* mac: 0x44B52E */
	float PartInstance::getReflectance() const
	{
		return static_cast<const FWPartInstance*>(fw())->reflectance;
	}

	/* mac: 0x448FFA */
	// STUB: PLAYER 0x00720790
	// STUB: STUDIO 0x007bc1e0
	void PartInstance::setReflectance(float value)
	{
		FWPartInstance* partFw = static_cast<FWPartInstance*>(fw());
		if (partFw->reflectance != value)
		{
			FWRef ref;
			ref.fwPtr = partFw->sharedSelf();
			FWValue<float>::set(partFw->reflectance, value, &ref);
			this->raisePropertyChanged(prop_Reflectance);
		}
	}

	/* mac: 0x44B520 */
	float PartInstance::getAlphaModifier() const
	{
		return static_cast<const FWPartInstance*>(fw())->alphaModifier;
	}

	/* mac: 0x4427A2 */
	void PartInstance::setAlphaModifier(float value)
	{
		FWPartInstance* partFw = static_cast<FWPartInstance*>(fw());
		if (partFw->alphaModifier != value)
		{
			FWRef ref;
			ref.fwPtr = partFw->sharedSelf();
			FWValue<float>::set(partFw->alphaModifier, value, &ref);
			this->raisePropertyChanged(prop_AlphaModifier);
		}
	}

	// [77] +0x134 onCameraNear 0x444BBA (93B, batch265).
	void PartInstance::onCameraNear(float distance)
	{
		if (distance < 3.0f)
			setAlphaModifier(0.0f);
		else if (distance >= 6.0f)
			setAlphaModifier(1.0f);
		else
			setAlphaModifier(0.5f);
	}

	// mac: 0x442830 (batch284: :1615 prim assert + prim+92 owner read).
	// STUB: PLAYER 0x0071bf10
	// STUB: STUDIO 0x007b7970
	SystemAddress PartInstance::getNetworkOwner() const
	{
		Primitive* prim = getPartPrimitive();
		RBXASSERT(prim != NULL);  // :1615 "getConstPartPrimitive()"
		return prim->getNetworkOwner();
	}

	// mac: 0x44562A (batch284: on-demand +196 byte, NULL-safe).
	bool PartInstance::getIsCurrentlyStreamRemovingPart() const
	{
		const OnDemandInstance* onDemand = onDemandRead();
		if (onDemand != NULL)
			return static_cast<const OnDemandPartInstance*>(onDemand)->streamRemoving;
		return false;
	}

	/* mac: 0x44B53C */
	bool PartInstance::getPartLocked() const
	{
		return static_cast<const FWPartInstance*>(fw())->locked;
	}

	/* mac: 0x448D8C: xor-inequality compare + FW set + raise. */
	// STUB: PLAYER 0x00720680
	// STUB: STUDIO 0x007bc0d0
	void PartInstance::setPartLocked(bool value)
	{
		FWPartInstance* partFw = static_cast<FWPartInstance*>(fw());
		if ((value ^ (partFw->locked != 0)) == 1)
		{
			FWRef ref;
			ref.fwPtr = partFw->sharedSelf();
			FWValue<bool>::set(partFw->locked, value, &ref);
			this->raisePropertyChanged(prop_Locked);
		}
	}

} // namespace RBX

namespace RBX {

	/* mac: 0x44278E / 0x44277A / 0x441B30: Primitive live state. */
	float PartInstance::getFriction() const
	{
		return primitive->getFriction();
	}

	float PartInstance::getElasticity() const
	{
		return primitive->getElasticity();
	}

	float PartInstance::getSpecificGravity() const
	{
		return primitive->getSpecificGravity();
	}

	/* mac: 0x4490B6: clamp [0,2], Primitive compare+store + raise. */
	// STUB: PLAYER 0x0071c140
	// STUB: STUDIO 0x007b7ba0
	void PartInstance::setFriction(float value)
	{
		float clamped = 0.0f;
		if (value > 0.0f)
			clamped = value < 2.0f ? value : 2.0f;
		if (clamped != primitive->getFriction())
		{
			primitive->setFriction(clamped);
			this->raisePropertyChanged(prop_Friction);
		}
	}

	/* mac: 0x449118: clamp [0,1], Primitive compare+store + raise. */
	// STUB: PLAYER 0x0071c1b0
	// STUB: STUDIO 0x007b7c10
	void PartInstance::setElasticity(float value)
	{
		float clamped = 0.0f;
		if (value > 0.0f)
			clamped = value < 1.0f ? value : 1.0f;
		if (clamped != primitive->getElasticity())
		{
			primitive->setElasticity(clamped);
			this->raisePropertyChanged(prop_Elasticity);
		}
	}

	/* mac: 0x4427FE (batch252: prim+0xAC deref + prim+253 byte -- the
	anchored flag; Primitive.h +252/+253 labels swapped to match). */
	bool PartInstance::getAnchored() const
	{
		return primitive->getAnchoredProperty();
	}

	/* mac: 0x448AC4 */
	// STUB: PLAYER 0x0071c0f0
	// STUB: STUDIO 0x007b7b50
	void PartInstance::setAnchored(bool value)
	{
		if (value != primitive->getAnchoredProperty())
		{
			primitive->setAnchoredProperty(value);
			this->raisePropertyChanged(prop_Anchored);
		}
	}

	/* [34] +0x88 setCanCollide 0x448A72 (82B). */
	bool PartInstance::getCanCollide() const
	{
		return !primitive->getPreventCollide();
	}
	// STUB: PLAYER 0x0071c0a0
	// STUB: STUDIO 0x007b7b00
	void PartInstance::setCanCollide(bool value)
	{
		bool prevent = !value;
		if (prevent != primitive->getPreventCollide())
		{
			primitive->setPreventCollide(prevent);
			this->raisePropertyChanged(prop_CanCollide);
		}
	}

	bool PartInstance::getDragging() const
	{
		return primitive->getDragging();
	}

	/* mac: 0x442A12 (raise unk_FB4C58 TODO-skipped: descriptor unidentified). */
	// STUB: PLAYER 0x0071be90
	// STUB: STUDIO 0x007b78f0
	void PartInstance::setDragging(bool value)
	{
		if (value != primitive->getDragging())
		{
			primitive->setDragging(value);
		}
	}

	/* mac: 0x4419EC: Primitive live state. */
	const G3D::CoordinateFrame& PartInstance::getCoordinateFrame() const
	{
		return primitive->getCoordinateFrame();
	}

	// [50] +0xC8 getLocation 0x37D7D0 (61B).
	G3D::CoordinateFrame PartInstance::getLocation()
	{
		return getCoordinateFrame();
	}

	// [49] +0xC4 computeExtentsWorld 0x442A62 (37B).
	// STUB: PLAYER 0x0071b7a0
	// STUB: STUDIO 0x007b7220
	Extents PartInstance::computeExtentsWorld() const
	{
		return primitive->getExtentsWorld();
	}

	// [52] +0xD0 legacyTraverseState 0x4476B0 (183B).
	// OPERAND-ORDER NOTE: the call setup pushes (out, argCF, legacyOffset)
	// with no ecx write, so it reads as the source order `cf * (*legacyOffset)`
	// (parent/traverse frame first, local legacy offset second -- standard
	// scene-graph composition, world = parent * local).  The member-vs-free
	// convention is ambiguous in the disasm (member mangling, cdecl shape);
	// in-tree only the member operator* exists, and both spellings compile
	// to the same composition.  If a future Studio-drag test shows mirrored
	// placement, flip the operands.
	// STUB: PLAYER 0x0071bae0
	// STUB: STUDIO 0x007b7540
	void PartInstance::legacyTraverseState(const G3D::CoordinateFrame& cf)
	{
		// PartInstance.cpp:1115 "onDemandRead() && onDemandWrite()->legacyOffset".
		OnDemandPartInstance* od =
			static_cast<OnDemandPartInstance*>(onDemandWrite());
		RBXASSERT(onDemandRead() != NULL && od->legacyOffset != NULL);
		G3D::CoordinateFrame moved = cf * (*od->legacyOffset);
		primitive->setGridCorner(moved);
	}

	// [8] +0x20 getPersistentDataCost 0x2CBE40 (25B).
	// FUNCTION: PLAYER 0x007234f0
	// FUNCTION: STUDIO 0x0079f3a0
	int PartInstance::getPersistentDataCost() const
	{
		return Instance::getPersistentDataCost() + 16;
	}

	// [21] +0x54 askSetParent 0x444E5A (83B).
	// STUB: PLAYER 0x006de360
	// STUB: STUDIO 0x007c08d0
	bool PartInstance::askSetParent(const Instance* parent) const
	{
		if (parent == NULL)
			return false;
		return Instance::fastDynamicCast<ModelInstance>(parent) != NULL;
	}

	// [27] +0x6C onChildRemoved 0x442D7C (61B).  +40 is the on-demand
	// touchTransmitter (see OnDemand.h member map).
	// STUB: PLAYER 0x0071ae90
	// STUB: STUDIO 0x007b6a00
	void PartInstance::onChildRemoved(Instance* child)
	{
		const OnDemandInstance* ro = onDemandRead();
		if (ro != NULL
			&& static_cast<const OnDemandPartInstance*>(ro)->touchTransmitter == child)
			static_cast<OnDemandPartInstance*>(onDemandWrite())->touchTransmitter = NULL;
	}

	// [49] +0xC4 onGuidChanged 0x44428A (136B).  The FW-myWorld assert
	// (PartInstance.cpp:412 "!fw()->myWorld.get()") queues with the FW-world
	// setter lifecycle (who sets myWorld is unmapped); the Primitive side
	// is asserted live inside setGuid (:152, prim worldless).
	// STUB: PLAYER 0x0071cf80
	// STUB: STUDIO 0x007b89d0
	void PartInstance::onGuidChanged()
	{
		primitive->setGuid(guid);
	}

	// [10] +0x28 onServiceProvider 0x44523E (166B).  fw read is +68
	// (fwBase) -> +12 (holder); register acts on the LIVE +68 slot via
	// fwRef() (layout-identical FWRef -- the dict may redirect it, which is
	// exactly how clearTerrain's unlock lands).  The :738 assert is carried.
	// STUB: PLAYER 0x00726cb0
	// STUB: STUDIO 0x007c21e0
	void PartInstance::onServiceProvider(ServiceProvider* oldProvider,
		ServiceProvider* newProvider)
	{
		(void)oldProvider;
		if (newProvider == NULL)
			return;
		FWService* fwService = ServiceProvider::find<FWService>(newProvider);
		FWBase* f = fw();
		FWDictionnaryBase* holder =
			(f != NULL) ? f->holder.get() : NULL;
		if (holder != NULL)
		{
			// PartInstance.cpp:738 "fwService->isOwnedHolder(...)".
			RBXASSERT(fwService != NULL && fwService->isOwnedHolder(holder));
		}
		else
		{
			RBXASSERT(fwService != NULL);
			if (fwService != NULL)
			{
				fwService->partDict().registerFlyweight(fwRef());
			}
		}
	}

	// [7] +0x1C setName 0x4441B4 (213B).  The two FastLog lines
	// ("PartInstance %p named" / "Name: %s") need the PartInstanceLifetime
	// channel -- absent, so skipped like the other lifetime logs.
	// STUB: PLAYER 0x0071b800
	// STUB: STUDIO 0x007b7280
	void PartInstance::setName(const std::string& value)
	{
		Instance::setName(value);
		if (value == "HumanoidRootPart")
			primitive->setSizeMultiplier(Primitive::SIZE_MULTIPLIER_2);
		else if (value == "Torso")
			primitive->setSizeMultiplier(Primitive::SIZE_MULTIPLIER_1);
		else
			primitive->setSizeMultiplier(Primitive::SIZE_MULTIPLIER_0);
	}

	/* mac: 0x441A08: mechanism-root dispatch under DFFlag (script context).
	   Bring-up: flag false -> direct primitive write, no Changed event (binary
	   raises none here either). Mechanism/NetworkOwner redispatch TODO. */
	void PartInstance::setCoordinateFrameRoot(const G3D::CoordinateFrame& value)
	{
		if (DFFlag::PromoteAssemblyModifications)
		{
			// TODO: script-context check + mechanism-root primitive redispatch.
		}
		primitive->setCoordinateFrame(value);
	}

	/* mac: 0x447A86: compare, sanitize (NaN -> sink far below), orthonormalize,
	   store via Primitive (setPhysics equivalent), raise CFrame.
	   TODO-skipped: InterpolatedCFrame history (+184), onPVChangedFromReflection,
	   unk_FB48AC/FB48D8 raises (hypothesized PositionUi/RotationUi), onDemand signal. */
	// STUB: PLAYER 0x00725430
	// STUB: STUDIO 0x007c09b0
	void PartInstance::setCoordinateFrame(const G3D::CoordinateFrame& value)
	{
		const G3D::CoordinateFrame& current = primitive->getCoordinateFrame();
		if (value.translation != current.translation || !(value.rotation == current.rotation))
		{
			// NOTE: hasNanOrInf is fabsf(f) >= inf: FALSE for NaN, TRUE for Inf
			// (IEEE quirk, binary-faithful — parsetest 42 proves only Inf sinks).
			G3D::CoordinateFrame orthoValue = value;
			if (Math::hasNanOrInf(orthoValue.rotation))
			{
				orthoValue = G3D::CoordinateFrame(G3D::Matrix3::identity(), G3D::Vector3(0.0f, -FLT_MAX, 0.0f));
			}
			else
			{
				Math::orthonormalizeIfNecessary(orthoValue.rotation);
			}
			primitive->setCoordinateFrame(orthoValue);
			interpolatedCFrame.clearHistory();
			this->raisePropertyChanged(prop_CFrame);
			this->raisePropertyChanged(prop_Position);
			this->raisePropertyChanged(prop_Rotation);
			// setCoordinateFrameChanged tail (0x447C06: fire +32 iff OD exists).
			if (onDemandRead())
				static_cast<OnDemandPartInstance*>(onDemandWrite())->coordinateFrameChangedSignal();
		}
	}

	// vtable+0xc4 / +0xc8 (2013 vftable 0x00b4c784). Bodies not written yet.
	// STUB: PLAYER 0x00725580
	// STUB: STUDIO 0x007c0b00
	void PartInstance::setLinearVelocity(const G3D::Vector3& value)
	{
		STUB(0x00725580);
	}

	// STUB: PLAYER 0x007256b0
	// STUB: STUDIO 0x007c0c30
	void PartInstance::setRotationalVelocity(const G3D::Vector3& value)
	{
		STUB(0x007256b0);
	}

	// vtable+0x78 (PVInstance virtual). Body not written yet.
	// STUB: PLAYER 0x00720370
	// STUB: STUDIO 0x007bbdc0
	bool PartInstance::hitTestImpl(const RbxRay& worldRay, G3D::Vector3& worldHitPoint)
	{
		STUB(0x00720370);
		return false;
	}

	/* Script-facing Ui views (see header EAs). getTranslationUi is a direct
	   translation read (0x4484A0: single return of CF+36). setTranslationUi per
	   0x4483DE decomp: component-wise translation early-out, then the same
	   break-joints (slot 61) -> virtual setCoordinateFrame (slot 52) -> safeMove
	   -> re-join (slot 60) flow as RotationUi (no extra raises, no degree
	   conversion). RotationUi bodies per 0x4484C0/0x4485B8 decomp:
	   euler-degrees <-> rotation via from/toEulerAnglesXYZ; setter funnels through
	   virtual setCoordinateFrame (which raises CFrame+Position+Rotation).
	   destroyJoints/safeMove/join (slots 61/60) TODO: methods absent from tree. */
	// STUB: PLAYER 0x0071bc70
	// STUB: STUDIO 0x007b76d0
	G3D::Vector3 PartInstance::getTranslationUi() const
	{
		return primitive->getCoordinateFrame().translation;
	}
	// STUB: PLAYER 0x00720db0
	// STUB: STUDIO 0x007bc8d0
	void PartInstance::setTranslationUi(const G3D::Vector3& value)
	{
		G3D::CoordinateFrame current = primitive->getCoordinateFrame();
		if (value != current.translation)
		{
			current.translation = value;
			setCoordinateFrame(current);
		}
	}
	// STUB: PLAYER 0x0071bc80
	// STUB: STUDIO 0x007b76e0
	G3D::Vector3 PartInstance::getRotationUi() const
	{
		float x, y, z;
		primitive->getCoordinateFrame().rotation.toEulerAnglesXYZ(x, y, z);
		return G3D::Vector3(G3D::toDegrees(x), G3D::toDegrees(y), G3D::toDegrees(z));
	}
	// STUB: PLAYER 0x00720e70
	// STUB: STUDIO 0x007bc990
	void PartInstance::setRotationUi(const G3D::Vector3& value)
	{
		G3D::CoordinateFrame current = primitive->getCoordinateFrame();
		G3D::Matrix3 r = G3D::Matrix3::fromEulerAnglesXYZ(G3D::toRadians(value.x), G3D::toRadians(value.y), G3D::toRadians(value.z));
		if (!(r == current.rotation))
		{
			current.rotation = r;
			setCoordinateFrame(current);
		}
	}
	/* Sleep/motion/buoyancy (moverrides EAs; MI wiring TODO, see header).
	   getSleeping reads the +0x68 store; isInContinousMotion (0x444E3A,
	   non-const per mangling) reads the +0x101 flag; onBuoyancyChanged
	   (0x444DF2) fires the +0x144 signal. */
	bool PartInstance::getSleeping() const
	{
		return sleepingState != 0;
	}
	bool PartInstance::isInContinousMotion()
	{
		return continousMotion;
	}
	void PartInstance::onBuoyancyChanged(bool buoyant)
	{
		buoyancyChangedSignal(buoyant);
	}
	/* OnDemand source (C2 0x442DBC, pool 200) + OD-driven bodies.
	   initOnDemand 0x444114; getters 0x44B6A0/0x44B65C (Write+offset);
	   hasTouchTransmitter 0x44BA7C; reportTouches 0x4446A6; onSleepingChanged
	   0x444C98; onClumpChanged 0x444C3E. Gfx paths are null-guarded dead
	   branches this build (gfxPartLink always NULL; GfxPart module TODO). */
	// STUB: PLAYER 0x00726bb0
	// STUB: STUDIO 0x007c20e0
	OnDemandInstance* PartInstance::initOnDemand()
	{
		return new OnDemandPartInstance(this);
	}
	TouchedSignal& PartInstance::getOrCreateTouchedSignal()
	{
		return static_cast<OnDemandPartInstance*>(onDemandWrite())->touchedSignal;
	}
	// mac: 0x44B6E4 (25B): `onDemandWrite(this); add eax, 38h` -- the ended
	// signal lives at OnDemand+56, right after touchedSignal at +48.
	TouchedSignal& PartInstance::getOrCreateTouchedEndedSignal()
	{
		return static_cast<OnDemandPartInstance*>(onDemandWrite())->touchedEndedSignal;
	}
	TouchedSignal& PartInstance::getOrCreateLocalSimulationTouchedSignal()
	{
		return static_cast<OnDemandPartInstance*>(onDemandWrite())->localSimulationTouchedSignal;
	}
	bool PartInstance::hasTouchTransmitter() const
	{
		const OnDemandPartInstance* od = static_cast<const OnDemandPartInstance*>(onDemandRead());
		return od != NULL && od->touchTransmitter != NULL;
	}
	bool PartInstance::reportTouches() const
	{
		// Non-const Write calls from a const method mirror the binary (which calls
		// non-const onDemandWrite 0x543F36 from const reportTouches).
		PartInstance* self = const_cast<PartInstance*>(this);
		if (onDemandRead())
		{
			if (!self->getOrCreateTouchedSignal().empty())
				return true;
		}
		if (onDemandRead())
		{
			if (!self->getOrCreateLocalSimulationTouchedSignal().empty())
				return true;
		}
		if (hasTouchTransmitter())
			return true;
		return findConstFirstChildOfType<TouchTransmitter>() != NULL;
	}
	// mac: 0x44A8F0 (153B).  DELTA, parameter name: the binary names nothing (it
	// is `arg_4`), so `otherPart` follows the TouchPair part0/part1 reading.
	void PartInstance::reportTouch(const shared_ptr<PartInstance>& otherPart)
	{
		if (hasTouchTransmitter())
		{
			OnDemandPartInstance* od = static_cast<OnDemandPartInstance*>(onDemandWrite());
			if (od->touchTransmitter->checkTouch(otherPart))
			{
				// The source spells the second onDemandWrite as a getter
				// call (the binary calls 0x543F36 twice, at 0x44A90D and
				// mac: 0x44A928); the getter returns the +48 signal and the
				// argument goes across BY VALUE as a shared_ptr<Instance>
				// copy, exactly the stack temp the disasm builds.
				getOrCreateTouchedSignal()(shared_ptr<Instance>(otherPart));
			}
		}
	}
	// mac: 0x445590 (153B).  Byte-for-byte the mirror of reportTouch, with
	// checkUntouch and the +56 (touchedEndedSignal) signal.
	void PartInstance::reportUntouch(const shared_ptr<PartInstance>& otherPart)
	{
		if (hasTouchTransmitter())
		{
			OnDemandPartInstance* od = static_cast<OnDemandPartInstance*>(onDemandWrite());
			if (od->touchTransmitter->checkUntouch(otherPart))
			{
				getOrCreateTouchedEndedSignal()(shared_ptr<Instance>(otherPart));
			}
		}
	}
	// STUB: PLAYER 0x00722910
	// STUB: STUDIO 0x007be080
	void PartInstance::onSleepingChanged(bool sleeping)
	{
		// Lifetime guard (binary holds shared_from 0x967EF4 across the body).
		shared_ptr<PartInstance> guard = shared_from(this);
		RBXASSERT(sleeping == getSleeping());  // PartInstance.cpp:606
		if (onDemandRead())
		{
			static_cast<OnDemandPartInstance*>(onDemandWrite())->sleepingSignal(sleeping);
		}
		if (gfxPartLink != NULL)
		{
			// TODO: GfxPart::onSleepingChanged(gfx, sleeping, this) (module + slot unmapped).
		}
		adornable.shouldRenderSetDirty();
		if (sleeping)
			interpolatedCFrame.clearHistory();
	}
	void PartInstance::onClumpChanged()
	{
		if (onDemandRead())
		{
			static_cast<OnDemandPartInstance*>(onDemandWrite())->clumpSignal();
		}
		if (gfxPartLink != NULL)
			{
			// TODO: GfxPart::onClumpChanged(gfx) (module + slot unmapped).
		}
	}
	/* Network-sleeping Changed callback + sampler (moverrides/icfalgo EAs).
	   onNetworkIsSleepingChanged 0x448A0E: primitive CF -> setRenderedFrame
	   (2-arg) -> raise prop. Backing is a local bool (2016 routes through
	   Primitive::get/setNetworkIsSleeping — 2013 bodies unmapped, TODO).
	   addInterpolationSample 0x447962: remote ? (clear-if-new + render) :
	   setValue; latch store; returns the flag. */
	bool PartInstance::getNetworkIsSleeping() const
	{
		return networkIsSleeping;
	}
	void PartInstance::setNetworkIsSleeping(bool value)
	{
		networkIsSleeping = value;
	}
	void PartInstance::onNetworkIsSleepingChanged()
	{
		interpolatedCFrame.setRenderedFrame(primitive->getCoordinateFrame());
		this->raisePropertyChanged(prop_NetworkIsSleeping);
	}
	bool PartInstance::getLastWasRemote() const
	{
		return lastWasRemote;
	}
	// STUB: PLAYER 0x0071b000
	// STUB: STUDIO 0x007b6b40
	bool PartInstance::addInterpolationSample(const G3D::CoordinateFrame& cf, const RemoteTime& rt, bool isRemote)
	{
		if (isRemote)
		{
			if (!lastWasRemote)
				interpolatedCFrame.clearHistory();
			interpolatedCFrame.setRenderedFrame(cf, rt);
		}
		else
			interpolatedCFrame.setValue(this, cf, rt);
		lastWasRemote = isRemote;
		return isRemote;
	}
	/* TouchTransmitter management (touchnotify EAs).
	   add 0x442A88: assert empty (:234), Creatable TouchTransmitter, parent it,
	   store OD+40 iff parented. remove 0x442B6E: Read-gated unparent + clear.
	   setParent used for setParentInternal(child,parent,false) (tree signature differs). */
	// STUB: PLAYER 0x007212f0
	// STUB: STUDIO 0x007bcef0
	void PartInstance::addTouchTransmitter()
	{
		RBXASSERT(onDemandRead() == NULL || !hasTouchTransmitter());  // PartInstance.cpp:234
		shared_ptr<TouchTransmitter> tt = Creatable<Instance>::create<TouchTransmitter>();
		tt->setParent(this);
		if (tt->getParent() == this)
			static_cast<OnDemandPartInstance*>(onDemandWrite())->touchTransmitter = tt.get();
	}
	void PartInstance::removeTouchTransmitter()
	{
		// Read-gated; mutation via Write (mirrors binary).
	if (onDemandRead() != NULL)
		{
			OnDemandPartInstance* od = static_cast<OnDemandPartInstance*>(onDemandWrite());
			if (od->touchTransmitter != NULL)
			{
				od->touchTransmitter->setParent(NULL);
				od->touchTransmitter = NULL;
			}
		}
	}
	/* mac: 0x467630: always SYMETRIC (subclasses override). */
	// STUB: PLAYER 0x0096bde0
	FormFactor PartInstance::getFormFactor() const
	{
		return SYMETRIC;
	}

	/* mac: 0x44CA1C: always BLOCK_PART (subclasses override). */
	// FUNCTION: PLAYER 0x00962a50 FOLDED
	// FUNCTION: STUDIO 0x00892cf0 FOLDED
	PartType PartInstance::getPartType() const
	{
		return BLOCK_PART;
	}

	/* mac: 0x44CA30: self (subclasses with children override). */
	PartInstance* PartInstance::getPrimaryPart()
	{
		return this;
	}

	// mac: 0x686BA0 (vtable slot [39] +0x9C).  Both minimum-size accessors return
	// a POINTER to a function-local `static G3D::Vector3 minSize` filled in
	// under a __cxa_guard the first time they are called; the value is three
	// identical dwords written back to back.
	// FUNCTION: PLAYER 0x00723440
	// FUNCTION: STUDIO 0x0079f2f0
	const G3D::Vector3* PartInstance::getMinimumUiSize() const
	{
		static G3D::Vector3 minSize(1.0f, 1.0f, 1.0f);   // 0x3F800000 x3
		return &minSize;
	}

	// mac: 0x485E98 (vtable slot [40] +0xA0).
	// FUNCTION: PLAYER 0x00723480
	// FUNCTION: STUDIO 0x0079f330
	const G3D::Vector3* PartInstance::getMinimumUiSizeCustom() const
	{
		static G3D::Vector3 minSize(0.2f, 0.2f, 0.2f);   // 0x3E4CCCCD x3
		return &minSize;
	}

	// mac: 0x4449D6.  The three-way dispatch on getFormFactor() (vtbl+0xCC) is
	// literally a switch on the returned int: 0 -> SYMMETRIC, 1 -> CUSTOM,
	// 2 -> BRICK, anything else -> the default arm.  Every arm but the
	// default starts from the component-wise max against getMinimumUiSize()
	// (read through the returned POINTER: `maxss xmm0, [eax+8]`) and rounds
	// it with Math::iRoundVector3 (0x22DF8D); BRICK then scales y by 0.4f
	// (dword_C1E558, the plate stud height) and CUSTOM by 1.2f (dword_C68364)
	// -- the exact inverses of setPartSizeUi's `/0.4f` and `/1.2f`.  The
	// default arm (PLATE/CYLINDER/PYRAMID) is NOT rounded and clamps against
	// getMinimumUiSizeCustom() instead.
	// STUB: PLAYER 0x0071d2e0
	// STUB: STUDIO 0x007b8d30
	G3D::Vector3 PartInstance::uiToXmlSize(const G3D::Vector3& uiSize) const
	{
		FormFactor formFactor = getFormFactor();
		if (formFactor == FormFactor::BRICK)                        // 2
		{
			G3D::Vector3 size = Math::iRoundVector3(uiSize.max(*getMinimumUiSize()));
			size.y *= 0.4f;
			return size;
		}
		if (formFactor == FormFactor::CUSTOM)                       // 1
		{
			G3D::Vector3 size = Math::iRoundVector3(uiSize.max(*getMinimumUiSize()));
			size.y *= 1.2f;
			return size;
		}
		if (formFactor == FormFactor::SYMETRIC)                     // 0
		{
			return Math::iRoundVector3(uiSize.max(*getMinimumUiSize()));
		}
		// PLATE / CYLINDER / PYRAMID (and any other FormFactor value): no
		// rounding, and the CUSTOM minimum is the clamp.
		return uiSize.max(*getMinimumUiSizeCustom());
	}

	// mac: 0x44261E (47B): 12-byte copy of the Primitive size.
	G3D::Vector3 PartInstance::getPartSizeUi() const
	{
		return primitive->getSize();
	}

	// mac: 0x442602 (28B, __ZNK3RBX12PartInstance14getPartSizeXmlEv): the
	// const& pass-through twin of the copy above.  rarc10 raw: sub
	// esp,8 / mov eax,[this+0AC] (the Primitive px) / mov [esp],eax /
	// call __ZNK3RBX9Primitive7getSizeEv / add esp,8 / retn.  The
	// bubble-imposter arm (0x5C4CA9) materially derefs the return --
	// x/y qword + z dword into the AB C2's byval size -- so the ImCo
	// source read is a const Vector3&, exactly this forwarding shape.
	const G3D::Vector3& PartInstance::getPartSizeXml() const
	{
		return primitive->getSize();
	}

	// [52] +0xD0 getRenderSize 0x280400 (31B).
	G3D::Vector3 PartInstance::getRenderSize()
	{
		return getPartSizeUi();
	}

	// [51] +0xCC getRenderLocation 0x2803E0 (31B).
	G3D::CoordinateFrame PartInstance::getRenderLocation()
	{
		G3D::CoordinateFrame out;
		calcRenderingCoordinateFrame(out, this);
		return out;
	}

	// [28] +0x5C setPartSizeXml 0x448870 (309B).
	// STUB: PLAYER 0x0071bd70
	// STUB: STUDIO 0x007b77d0
	void PartInstance::setPartSizeXml(const G3D::Vector3& value)
	{
		// NaN-aware 3-word change-detect (NaN always proceeds; Vector3
		// operator== is false on NaN components, matching ucomiss+jnp).
		if (value == primitive->getSize())
			return;
		G3D::Vector3 newSize;
		if (hasThreeDimensionalSize())
			newSize = value;
		else
			newSize = G3D::Vector3(value.x, value.x, value.x);  // xxx() splat
		if (onDemandRead() != NULL
			&& static_cast<OnDemandPartInstance*>(
				onDemandWrite())->legacyOffset != NULL)
		{
			G3D::CoordinateFrame corner = primitive->getGridCorner();
			primitive->setSize(newSize);
			primitive->setGridCorner(corner);
		}
		else
		{
			primitive->setSize(newSize);
		}
		this->raisePropertyChanged(prop_Size);
		// DELTA: second raise of unk_FB4AE0 (descriptor EA unknown).
		if (onDemandRead() != NULL)
			static_cast<OnDemandPartInstance*>(
				onDemandWrite())->frameDirty = true;
	}

	// mac: 0x4491BE (1011B, batch305: face-sign grow + form-factor Y adjust
	// + CF shift + setCoordinateFrame/setPartSizeXml; ContactManager
	// revert arm queues behind Dragger::intersectingWorldOrOthers).
	// STUB: PLAYER 0x0071da70
	// STUB: STUDIO 0x007b94e0
	bool PartInstance::resizeImpl(NormalId normal, int increment)
	{
		if (increment == 0)
			return false;
		Primitive* prim = getPartPrimitive();
		G3D::Vector3 oldSize = prim->getSize();
		G3D::CoordinateFrame cf = prim->getCoordinateFrame();
		G3D::Vector3 delta(0.0f, 0.0f, 0.0f);
		// Face sign (rodata 0xCA2470 {-1.0f, +1.0f} verified 2026-10-05,
		// indexed by (normal < 3): +faces grow +center, -faces -center).
		float sign = (normal < 3) ? 1.0f : -1.0f;
		if (hasThreeDimensionalSize())
		{
			// No Vector3::operator[] in-tree (G3D vendored) -- lane select.
			if (normal % 3 == 0)
				delta.x = (float)increment;
			else if (normal % 3 == 1)
				delta.y = (float)increment;
			else
				delta.z = (float)increment;
		}
		else
		{
			// :1884 assert arm (getFormFactor() == SYMETRIC under the
			// A63E9D flag byte) is dead in-tree -- getFormFactor()
			// always returns SYMETRIC (loud DELTA if that changes).
			delta = G3D::Vector3((float)increment, (float)increment, (float)increment);
		}
		float baseY = oldSize.y;
		FormFactor ff = getFormFactor();
		if (ff == BRICK)
			baseY = oldSize.y / 0.4f;
		else if (ff == CUSTOM)
			baseY = oldSize.y / 1.2f;
		G3D::Vector3 newUiSize(oldSize.x + delta.x, baseY + delta.y, oldSize.z + delta.z);
		G3D::Vector3 xmlSize = uiToXmlSize(newUiSize);
		G3D::Vector3 d = xmlSize - oldSize;
		// offset = R * d, explicit lanes (binary assoc: (m2*z)+((m0*x)+(m1*y))).
		const float* mrows = cf.rotation;
		G3D::Vector3 offset(
			((mrows[2] * d.z) + ((mrows[0] * d.x) + (mrows[1] * d.y))),
			((mrows[5] * d.z) + ((mrows[3] * d.x) + (mrows[4] * d.y))),
			((mrows[8] * d.z) + ((mrows[6] * d.x) + (mrows[7] * d.y))));
		G3D::Vector3 newT = cf.translation + offset * (0.5f * sign);
		setCoordinateFrame(G3D::CoordinateFrame(cf.rotation, newT));
		setPartSizeXml(xmlSize);
		if (Workspace::getContactManagerIfInWorkspace(this) != NULL)
		{
			// DELTA (Dragger era): intersectingWorldOrOthers +
			// setCoordinateFrame/setPartSizeXml revert need
			// Dragger::intersectingWorldOrOthers (only safeMoveNoDrop
			// modeled). Loud if a workspace CM ever exists in-tree.
			RBXASSERT(false && "resizeImpl: Dragger intersectingWorldOrOthers era");
		}
		return true;
	}

	// mac: 0x44917A (67B, batch304: slot-61 destroyJoints + resizeImpl +
	// slot-60 join, by name per the IMoving-call precedent).
	// STUB: PLAYER 0x00720810
	// STUB: STUDIO 0x007bc260
	bool PartInstance::resize(NormalId normal, int increment)
	{
		destroyJoints();
		bool ok = resizeImpl(normal, increment);
		join();
		return ok;
	}

	// mac: 0x44264E (300B, batch307).
	// STUB: PLAYER 0x00722a90
	// STUB: STUDIO 0x007be200
	void PartInstance::setPartSizeUi(const G3D::Vector3& value)
	{
		destroyJoints();  // slot 61.
		G3D::Vector3 adjusted = value;
		FormFactor ff = getFormFactor();  // slot 51.
		if (ff == BRICK)
			adjusted.y = value.y / 0.4f;
		else if (ff == CUSTOM)
			adjusted.y = value.y / 1.2f;
		G3D::Vector3 xmlSize = uiToXmlSize(adjusted);
		// NaN-aware compares (ucomiss+jnp: NaN always differs -- same
		// convention as setPartSizeXml).
		if (xmlSize == primitive->getSize())
		{
			if (!(xmlSize == value))
				this->raisePropertyChanged(prop_Size);  // DELTA: binary raises unk_FB4AE0
		}
		else
		{
			setPartSizeXml(xmlSize);  // slot 47.
		}
		safeMove();
		join();  // slot 60.
		// +36 signal arm (0x442739: partSizeChangedSignal, named by
		// firer -- TODO-verify; dead when on-demand is NULL).
		if (onDemandRead() != NULL)
			static_cast<OnDemandPartInstance*>(
				onDemandWrite())->partSizeChangedSignal();
	}

	// mac: 0x4489CC (66B, batch307).
	// STUB: PLAYER 0x00720f50
	// STUB: STUDIO 0x007bca70
	void PartInstance::setPartSizeUnjoined(const G3D::Vector3& value)
	{
		setPartSizeXml(uiToXmlSize(value));  // slot 47.
		safeMove();
	}

	// mac: 0x4481A6 (61B).
	bool PartInstance::computeNetworkOwnerIsSomeoneElse() const
	{
		if (primitive->getConstMechanism() == NULL)
			return false;
		// Unreachable today (see header tripwire): the tail reads
		// (getConstMechanismPrimitive(mech)+244)+224.  False keeps the
		// callers' short-circuit exact.
		return false;
	}

	// mac: 0x447C1E (759B).
	// Asserts :1236 (same-mechanism, both NULL today -- passes) carried;
	// :1237 needs getConstMechanismPrimitive (Mechanism shell) and is
	// unexecutable while both are NULL (the binary would deref NULL too),
	// so it queues with the shell.
	// The a2!=root branch composes out = rendering(root) * rel with
	// rel = rootCF.inverse() * thisCF, assembled from the binary's own
	// ops (transpose/negate/row-dots/compose -- see the note below), NOT
	// via CoordinateFrame::inverse (no such call in the binary).
	// STUB: PLAYER 0x0071d640
	// STUB: STUDIO 0x007b90b0
	void PartInstance::calcRenderingCoordinateFrame(G3D::CoordinateFrame& out,
		PartInstance* root)
	{
		RBXASSERT(primitive->getConstMechanism()
			== root->primitive->getConstMechanism());  // :1236
		if (root == this)
		{
			Workspace* ws = Workspace::getWorkspaceIfInWorkspace(this);
			World* world = (ws != NULL) ? ws->getWorld() : NULL;
			// DELTA guards: the binary derefs ws/world unconditionally
			// (callers always run in-workspace); the harness drives free
			// parts, so NULL falls through to the copy branch instead of
			// faulting.  Revisit if free-part rendering ever matters.
			if (!disableInterpolation && ws != NULL && world != NULL
				&& world->getUiStepId() > renderUiStep
				&& computeNetworkOwnerIsSomeoneElse()
				&& !lastWasRemote)
			{
				renderUiStep = world->getUiStepId();
				// Binary passes &interpolatedCFrame as the part arg; our
				// computeValue accepts NULL for "no latch sync", which is
				// the binary's observable (its arg is ignored -- see the
				// ICF header note).
				interpolatedCFrame.computeValue(out, NULL);
			}
			else
			{
				const G3D::CoordinateFrame& cf =
					primitive->getCoordinateFrame();
				interpolatedCFrame.setRenderedFrame(cf);
				out = cf;
			}
			return;
		}
		// Relative branch.  rel.rot = transpose(rootCF.rot) (binary v27),
		// rel.trans = -rel.rot * rootCF.trans (the v22-negate + dot loop),
		// composed = rel * thisCF (binary v17), out = rootRendering *
		// composed (final operator*).  Verified at identity + single
		// translation (see section 105); a two-part mechanism fixture for
		// full proof queues with the Mechanism shell.
		const G3D::CoordinateFrame& rootCF =
			root->primitive->getCoordinateFrame();
		const G3D::CoordinateFrame& thisCF = primitive->getCoordinateFrame();
		G3D::Matrix3 relRot = rootCF.rotation.transpose();
		G3D::Matrix3 negRel = -relRot;
		G3D::Vector3 relTrans(
			negRel.row(0).dot(rootCF.translation),
			negRel.row(1).dot(rootCF.translation),
			negRel.row(2).dot(rootCF.translation));
		G3D::CoordinateFrame rel(relRot, relTrans);
		G3D::CoordinateFrame composed = rel * thisCF;
		G3D::CoordinateFrame rootRendering;
		root->calcRenderingCoordinateFrame(rootRendering, root);
		out = rootRendering * composed;
	}

	// mac: 0x44504C: Workspace::getWorldIfInWorkspace(this) then
	// World::createAutoJoints(getPartPrimitive()) when there is a World.
	// STUB: PLAYER 0x0071bac0
	// STUB: STUDIO 0x007b7520
	void PartInstance::join()
	{
		World* world = Workspace::getWorldIfInWorkspace(this);
		if (world != NULL)
			world->createAutoJoints(getPartPrimitive());
	}

	// mac: 0x444FE0: same shape with World::destroyAutoJoints(prim, true).
	// STUB: PLAYER 0x0071ba60
	// STUB: STUDIO 0x007b74c0
	void PartInstance::destroyJoints()
	{
		World* world = Workspace::getWorldIfInWorkspace(this);
		if (world != NULL)
			world->destroyAutoJoints(getPartPrimitive(), true);
	}

	// mac: 0x44787C: assert the primitive, then a no-drop safe move of just this
	// part through Dragger when a ContactManager is reachable.
	// STUB: PLAYER 0x00720cd0
	// STUB: STUDIO 0x007bc7f0
	void PartInstance::safeMove()
	{
		RBXASSERT(getPartPrimitive());   // PartInstance.cpp:1161
		ContactManager* contactManager = Workspace::getContactManagerIfInWorkspace(this);
		if (contactManager != NULL)
		{
			G3D::Array<Primitive*, 10, 32> prims;
			prims.append(getPartPrimitive());
			Dragger::safeMoveNoDrop(prims, G3D::Vector3::zero(), *contactManager);
		}
	}

	template<> bool StringConverter<Material>::convertToValue(const std::string& text, Material& value)
	{
		return Reflection::EnumDesc<Material>::singleton().convertToValue(text.c_str(), value);
	}
	template<> std::string StringConverter<Material>::convertToString(const Material& value)
	{
		return Reflection::EnumDesc<Material>::singleton().convertToString(value);
	}

	// mac: 0x441A7C (31B, batch233): PV+48 linear.
	const G3D::Vector3& PartInstance::getLinearVelocity() const
	{
		return getPartPrimitive()->getPV()->velocity.linear;
	}

	// mac: 0x441A9C (batch233): PV+60 rotational.
	const G3D::Vector3& PartInstance::getRotationalVelocity() const
	{
		return getPartPrimitive()->getPV()->velocity.rotational;
	}

	// mac: 0x448776 (31B) __ZNK3RBX12PartInstance11getVelocityEv: PV+48 linear
	// read (identical 31B body to getLinearVelocity; the render-side
	// alias the chat-bubble arm calls).
	const G3D::Vector3& PartInstance::getVelocity() const
	{
		return getPartPrimitive()->getPV()->velocity.linear;
	}

	// mac: 0x443524 (209B, sret)
	// __ZNK3RBX12PartInstance27getRenderingCoordinateFrameEv.  2026-10-20
	// raw re-pin: the interpolation arm requires !lastWasRemote (the last
	// sample LOCAL) -- the earlier arm/simple polarity note was a misread.
	// ICF arm: (mechPrim = mechanism->getConstMechanismPrimitive()) then
	// BOTH pointer hops guarded in the binary (mechPrim-null at 0x4435xx,
	// the +0xF4 model slot's px-null after) -> other = (md - 0x60) and the
	// answer = other->getLastComputedRenderingCoordinateFrame(other) (the
	// other->other self-root arm).  Simple arm = the primitive's plain
	// frame copy.
	// STUB: PLAYER 0x0071d900
	// STUB: STUDIO 0x007b9370
	G3D::CoordinateFrame PartInstance::getRenderingCoordinateFrame() const
	{
		if (!disableInterpolation)
		{
			const Mechanism* mechanism = primitive->getConstMechanism();
			if (mechanism != NULL
				&& computeNetworkOwnerIsSomeoneElse()
				&& !lastWasRemote)
			{
				Primitive const* mechPrim =
					mechanism->getConstMechanismPrimitive();
				if (mechPrim != NULL)
				{
					// +0xF4 = the prim->PartInstance owner back-slot (typed
					// member DELTA: unpoured in Primitive).
					void* modelRef = *(void**)((char*)mechPrim + 0xF4);
					if (modelRef != NULL)
					{
						PartInstance const* other =
							reinterpret_cast<PartInstance const*>(
								reinterpret_cast<const char*>(modelRef) - 0x60);
						return other->getLastComputedRenderingCoordinateFrame(
							other);
					}
				}
			}
		}
		return primitive->getCoordinateFrame();
	}

	// mac: 0x447F16 (656B, sret)
	// __ZNK3RBX12PartInstance39getLastComputedRenderingCoordinateFrameEPKS0_:
	//   flag-first asserts (:1257 0x4E9 / :1258 0x4EA) landed as the
	//   corpus _debugHook expansion (LuaArguments.h idiom; Debug.h
	//   RBX_LOG_ASSERT = the same GCC shape) with the binary's fully
	//   baked strings -- build-agent path + the 2013 source line, and
	//   the 2013 accessor name getConstPartPrimitive (our landed
	//   accessor = getPartPrimitive(); name DELTA).  Self-root
	//   (this == mechanismRootPart): the +0xB8 copy =
	//   interpolatedCFrame.renderedFrame (ICF first member at
	//   PartInstance+184).  Relative arm mirrors
	//   calcRenderingCoordinateFrame's relative branch (cpp :1102-1121)
	//   verbatim -- incl. the negated-transpose dot loop (float assoc
	//   accepted as landed) -- with the root self-recursion in place of
	//   calc's root self-call.
	// STUB: PLAYER 0x0071d7d0
	// STUB: STUDIO 0x007b9240
	G3D::CoordinateFrame PartInstance::getLastComputedRenderingCoordinateFrame(
		PartInstance const* mechanismRootPart) const
	{
		if (FLog::Asserts != 0)
		{
			// :1257
			if (!(getPartPrimitive()->getConstMechanism()
					== mechanismRootPart->getPartPrimitive()
						->getConstMechanism())
				&& (RBX::_internal::_debugHook == 0
					|| RBX::_internal::_debugHook(
						"this->getConstPartPrimitive()->getConstMechanism() == mechanismRootPart->getConstPartPrimitive()->getConstMechanism()",
						"/Applications/buildAgent/work/fa0a2141fe3d2f4d/Client/App/v8datamodel/PartInstance.cpp",
						1257) == 0))
			{
				ReleaseAssert(FLog::Asserts,
					"this->getConstPartPrimitive()->getConstMechanism() == mechanismRootPart->getConstPartPrimitive()->getConstMechanism() "
					"file: /Applications/buildAgent/work/fa0a2141fe3d2f4d/Client/App/v8datamodel/PartInstance.cpp line: 1257");
			}
			// :1258
			if (!(mechanismRootPart->getPartPrimitive()->getConstMechanism()
					->getConstMechanismPrimitive()
				== mechanismRootPart->getPartPrimitive())
				&& (RBX::_internal::_debugHook == 0
					|| RBX::_internal::_debugHook(
						"mechanismRootPart->getConstPartPrimitive()->getConstMechanism()->getConstMechanismPrimitive() == mechanismRootPart->getConstPartPrimitive()",
						"/Applications/buildAgent/work/fa0a2141fe3d2f4d/Client/App/v8datamodel/PartInstance.cpp",
						1258) == 0))
			{
				ReleaseAssert(FLog::Asserts,
					"mechanismRootPart->getConstPartPrimitive()->getConstMechanism()->getConstMechanismPrimitive() == mechanismRootPart->getConstPartPrimitive() "
					"file: /Applications/buildAgent/work/fa0a2141fe3d2f4d/Client/App/v8datamodel/PartInstance.cpp line: 1258");
			}
		}

		if (this != mechanismRootPart)
		{
			const G3D::CoordinateFrame& rootCF =
				mechanismRootPart->primitive->getCoordinateFrame();
			const G3D::CoordinateFrame& thisCF =
				primitive->getCoordinateFrame();
			G3D::Matrix3 relRot = rootCF.rotation.transpose();
			G3D::Matrix3 negRel = -relRot;
			G3D::Vector3 relTrans(
				negRel.row(0).dot(rootCF.translation),
				negRel.row(1).dot(rootCF.translation),
				negRel.row(2).dot(rootCF.translation));
			G3D::CoordinateFrame rel(relRot, relTrans);
			G3D::CoordinateFrame composed = rel * thisCF;
			G3D::CoordinateFrame rootRendering =
				mechanismRootPart->getLastComputedRenderingCoordinateFrame(
					mechanismRootPart);
			return rootRendering * composed;
		}
		return interpolatedCFrame.renderedFrame;
	}

} // namespace RBX
