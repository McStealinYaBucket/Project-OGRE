/* V8World/Contact.h -- October 2013 (broadphase contact edge).
   Evidence: Contact C2 0x55DCAC / D2 0x55DD38; layout = Edge base + 16B
     (+32=-2, +36=-1, +40=0, +44 heap ptr); SpatialHash<Primitive,Contact,
     ContactManager,4> instantiation (TreeNode/SpatialNode messages).
   +36 is the SleepStage stepping/touching index (IndexArray slot,
   SleepStage::changeContactState 0x59Fxxx fastRemoves/appends by it,
   C2 -1): named steppingIndex. Edge base is now the binary 32B, so
   member offsets below are binary-exact (pads shrunk accordingly).
   DELTAS: member NAMES unmapped except steppingIndex (values exact);
   D2 heap-free unmapped (default dtor; no heap set in tree).
   Original file: /Applications/buildAgent/work/fa0a2141fe3d2f4d/Client/App/v8world/Contact.h (path inferred) */

#pragma once

#include "V8World/Edge.h"
#include "V8Kernel/ContactConnector.h"
#include "Util/FixedArray.h"

namespace RBX {

	class SleepStage;

	// mac: 0x55DFEC (batch250: clamp 0..2 pair + same-side min / split
	// sum-1; 2016-verbatim, disasm-identical; fminf spelled via
	// ternaries per the VS2012 toolchain rule).
	float calculateFriction(float c0, float c1);

	// 12B heap triple written by onPrimitiveContactParametersChanged
	// (0x55E06E, batch248): [0] min override elasticity, [1] scaled by
	// the elastic multiplier, [2] combined friction. Owned by the
	// Contact (news on first generateData, freed in D2).
	struct ContactData {
		float elasticBase;
		float elasticScaled;
		float friction;
	};

	class Contact : public Edge
	{
	public:
		Contact(Primitive* p0, Primitive* p1)
			: Edge(p0, p1)
			, contactFlagA(-2)
			, steppingIndex(-1)
			, numTouchCycles(0)
			, heapData(NULL)
		{}
		virtual ~Contact();

		// Edge vtbl+20 (Primitive::insertEdge 0x590516 routes CONTACT to the
		// contacts lists). Body here so Contact stays concrete under the
		// new pure-virtual base.
		virtual int getEdgeType() const { return CONTACT; }

		// SleepStage IndexArray slot (+36; changeContactState 0x59Fxxx).
		// Private with SleepStage friendship (same pattern as
		// Body::leafBodyIndex/KernelData).
		int getSteppingIndex() const { return steppingIndex; }
		// 2016 shape (IndexArray hook for the Sleep stepping/touching
		// lists; binary instantiation 0x5A0532).
		int& steppingIndexFunc() { return steppingIndex; }
		// Collision-signal gate (SleepStage::changeContactState fires
		// World::onPrimitiveCollided only while this is < 3; 2016 name;
		// binary reads +40 as int16 <= 2 -- same values. Incremented by
		// the contact generate-data path (section 123+); C2 0 here).
		int getNumTouchCycles() const { return numTouchCycles; }
		// mac: 0x55DDBA (108B, batch276: count/getter loop calling reset()
		// per linked connector; recounts each iteration; void return
		// per the return-value DELTA precedent -- binary returns the
		// last count, unobserved by the sole caller
		// (notifyAssemblyPrimitiveMoved)).
		void primitiveMovedExternally();
		// Binary vtbl slots 13/14 (batch283; base holds
		// __cxa_pure_virtual there). Non-pure 0/NULL defaults here
		// (LOUD DELTA): purity would brick direct construction until
		// the Poly era completes the override set (BallPoly/
		// PolyPoly inherit these defaults -- queued with PolyContact
		// + PolyConnector). Our vtable also lacks the 4 stepper
		// virtuals between generateData and these (deleteAllConnectors/
		// stepContact/computeIsCollidingUi/computeIsColliding --
		// stepper era), so slot numbers differ; by-name dispatch is
		// unaffected (Edge.h precedent).
		virtual int numConnectors() const { return 0; }
		virtual GeoPairConnector* getConnector(int i) { (void)i; return NULL; }
		// mac: 0x55E1AC (51B, batch584: getKernel + Kernel::removeConnector
		// + virtual-D0 free if non-null; NON-virtual, direct-called).
		// Requires a staged contact (binary-identical AV on NULL
		// kernel -- Kernel-stage era owns live-delete tests).
		void deleteConnector(ContactConnector* connector);
		// Binary vtbl slot 9 (base pure; BallPoly/PolyPoly inherit this
		// default -- Poly era queued). Empty default (LOUD DELTA vs
		// pure, same precedent as the count/getter pair).
		virtual void deleteAllConnectors() {}
		// mac: 0x562AA8 (127B, batch596: staged-assert :57 + virtual
		// deleteAllConnectors + IPipelined::removeFromKernel).
		// Binary vtbl slot 5 (our position differs -- stepper slots
		// missing; by-name dispatch unaffected). Defined here as in 2013
		// (assert Contact.h:57; the prior tree had it in Contact.cpp).
		// STUB: PLAYER 0x007b8ec0
		// STUB: STUDIO 0x008d5860
		virtual void removeFromKernel()
		{
			RBXASSERT(getKernel() != NULL);  // Contact.h:57 "getKernel()"
			deleteAllConnectors();
			IPipelined::removeFromKernel();
		}
		// mac: 0x55E1E0 (batch275: heap triple alloc + onParam; all
		// per-type overrides except BlockBlock forward here).
		virtual void generateDataForMovingAssemblyStage();
		// mac: 0x55E06E (317B, batch248: friction/elasticity combine into
		// the heap triple; +316/+320 override arms).
		void onPrimitiveContactParametersChanged();
		// Test view of the heap triple (NULL until generateData runs).
		const ContactData* getContactData() const { return heapData; }

	private:
		friend class SleepStage;
		int contactFlagA;    // +32 (-2).
		int steppingIndex;   // +36 (-1).
		int numTouchCycles;  // +40 (0).
		ContactData* heapData;  // +44 (news on generateData; D2 frees).
	};

	// All five concrete contacts below are sized by their construction sites
	// in ContactManager::createContact (session 111/115 evidence) and carry
	// the binary's exact init values.  Offsets are binary-exact (Edge is
	// the binary 32B); VALUES and TOTALS are exact.  Solver/stepper
	// behavior is unported (nothing in the tree steps contacts).

	// BlockBlock pair store (overlays the 0x94 solver block at +48:
	// 76B unmapped + FixedArray<GeoPairConnector*,8> @+76 (whose num
	// doubles as the +108 count) + 32B unmapped + owner @+144).
	// generateData value-inits the whole block (num 0, owner set).
	class BlockBlockContact;
	struct BlockBlockContactData
	{
		char unknown0[76];
		FixedArray<GeoPairConnector*, 8> connectors;
		char unknown112[32];
		BlockBlockContact* owner;  // +144 (owner+44 = ContactParams).
		// mac: 0x55EC88 (389B, batch308: match-hit fastRemove + rebind,
		// else pool-new + ContactConnector C2 + kernel insert).
		GeoPairConnector* findGeoPairConnector(Body* b0, Body* b1,
			GeoPairType pairType, int id0, int id1);
		int numConnectors() const { return (int)connectors.size(); }
		GeoPairConnector* getConnector(int i) { return connectors[i]; }
	};

	// mac: 0x56BF20 (95B) / 0x56BD48 (95B): Contact(p0,p1) + ptr@+48 = NULL.
	// Total 52B (52B CHECK pinned).
	class BallBallContact : public Contact
	{
	public:
		BallBallContact(Primitive* p0, Primitive* p1)
			: Contact(p0, p1)
			, connector48(NULL)
		{}
		// mac: 0x55E4BA (batch275: forwards to the base).
		virtual void generateDataForMovingAssemblyStage()
		{
			Contact::generateDataForMovingAssemblyStage();
		}
		// mac: 0x562B74 (count = ptr != 0) / 0x55E21C (get = ptr, index
		// ignored -- binary-verbatim). Single-connector store at +48
		// (was int48; same 0 bits, pointer-typed per the bodies).
		// FUNCTION: PLAYER 0x007b9050 FOLDED
		// FUNCTION: STUDIO 0x008d59f0 FOLDED
		virtual int numConnectors() const { return connector48 != NULL; }
		// FUNCTION: PLAYER 0x00850a40 FOLDED
		// FUNCTION: STUDIO 0x009a8770 FOLDED
		virtual GeoPairConnector* getConnector(int i) { (void)i; return connector48; }
		// mac: 0x55E228 (42B, batch584: deleteConnector(+48) + null).
		virtual void deleteAllConnectors();
	private:
		GeoPairConnector* connector48;  // +48 (NULL).
	};

	class BallBlockContact : public Contact
	{
	public:
		BallBlockContact(Primitive* p0, Primitive* p1)
			: Contact(p0, p1)
			, connector48(NULL)
		{}
		// Batch275: forwards to the base.
		virtual void generateDataForMovingAssemblyStage()
		{
			Contact::generateDataForMovingAssemblyStage();
		}
		// mac: 0x562BC8 / 0x55E4C4 (batch582: same single-pointer shape as
		// BallBall).
		// FUNCTION: PLAYER 0x007b9050 FOLDED
		// FUNCTION: STUDIO 0x008d59f0 FOLDED
		virtual int numConnectors() const { return connector48 != NULL; }
		// FUNCTION: PLAYER 0x00850a40 FOLDED
		// FUNCTION: STUDIO 0x009a8770 FOLDED
		virtual GeoPairConnector* getConnector(int i) { (void)i; return connector48; }
		// mac: 0x55E4D0 (42B, batch584: identical twin of BallBall's).
		virtual void deleteAllConnectors();
	private:
		GeoPairConnector* connector48;  // +48 (NULL).
	};

	// mac: 0x5611DA (127B): the binary chains Edge (skipping Contact C2), but its
	// five words (+32=-2, +36=-1, +40w=0, +44=0, +48=0) are EXACTLY our
	// Contact ctor inits plus connector48 -- same net state through our chain.
	// Total 52B.
	class BlockBlockContact : public Contact
	{
	public:
		BlockBlockContact(Primitive* p0, Primitive* p1)
			: Contact(p0, p1)
			, solverData(NULL)
		{}
		~BlockBlockContact();
		// Batch275: base triple + 148B solver block at +48 (sparse
		// init pattern transcribed verbatim; padding memset-zeroed for
		// determinism -- binary leaves it fresh-heap garbage, which no
		// reader may depend on).
		virtual void generateDataForMovingAssemblyStage();
		// mac: 0x55EBD6 (178B, batch308: myData && contactParams assert,
		// then the Data find; NULL when ungenerated).
		GeoPairConnector* findGeoPairConnector(Body* b0, Body* b1,
			GeoPairType pairType, int id0, int id1);
		// mac: 0x5612CC / 0x55EA20 (batches 302/582: myData null -> 0/NULL;
		// FFlag::BlockBlockNarrowPhaseRefactor true arm reads the
		// refactor-layout count (layout unmapped -- loud RBXASSERT +
		// legacy fallthrough; flag BSS-false). Binary vtbl slots 13/14.
		virtual int numConnectors() const;
		virtual GeoPairConnector* getConnector(int i);
		// mac: 0x562BDA (32B, batch584: FFlag dispatch to Orig/FFlag arms
		// below). Binary vtbl slot 9.
		virtual void deleteAllConnectors();
		static int pairMisses;
		// Typed myData overlay (public: tests reach it the way
		// generate-data code does; binary has no access control).
		BlockBlockContactData* myData()
		{
			return reinterpret_cast<BlockBlockContactData*>(solverData);
		}
		const BlockBlockContactData* myData() const
		{
			return reinterpret_cast<const BlockBlockContactData*>(solverData);
		}
	private:
		// mac: 0x55EA6C Orig / 0x55EB18 FFlag (batch584: per-element
		// deleteConnector + count zero; FFlag arm reads the unmapped
		// refactor layout -- loud assert + Orig fallthrough).
		void deleteAllConnectorsOrig();
		void deleteAllConnectorsFFlag();
		// 148B solver block (binary operator new(0x94) at +48):
		// [18]=0, [30]=[31]=-1 (indices), [36]=this (back-pointer);
		// rest unmapped solver state.
		char* solverData;  // +48 (NULL until generateData).
	};

	// mac: 0x558B6A (293B): Contact(p0,p1) + int@+208 = 0 (+ asserts
	// rbx_static_cast<Ball*>(p0->getGeometry()) :17 and
	// rbx_static_cast<Poly*>(p1->getGeometry()) :18 -- flag-gated, and the
	// Ball/Poly geometry kinds are unmapped, so the asserts are documented,
	// not ported).  Total 212B: 160B of solver data between Contact end
	// (+48) and int208 are unmapped -- pad, never read in-tree.
	// LOUD DELTA: binary derives BallPolyContact from PolyContact (vtable
	// slots 13/14 = PolyContact::numConnectors/getConnector over the
	// +48 FixedArray<PolyConnector*,40> + count at +208); PolyConnector
	// is unmapped (geometry faces era), so this inherits the base 0/NULL
	// defaults -- Poly era queues the intermediate base + overrides.
	class BallPolyContact : public Contact
	{
	public:
		// STUB: PLAYER 0x00968390
		// STUB: STUDIO 0x00a2ca20
		BallPolyContact(Primitive* p0, Primitive* p1)
			: Contact(p0, p1)
			, int208(0)
		{}
		// Batch275: forwards to the base.
		virtual void generateDataForMovingAssemblyStage()
		{
			Contact::generateDataForMovingAssemblyStage();
		}
	private:
		char pad48[160];
		int int208;    // +208 (0).
	};

	// mac: 0x58C194 (116B): Contact(p0,p1) + int@+208 = 0 + int@+212 = 0.
	// Total 216B, same pad caveat as BallPoly (same Poly-era queue).
	class PolyPolyContact : public Contact
	{
	public:
		PolyPolyContact(Primitive* p0, Primitive* p1)
			: Contact(p0, p1)
			, int208(0)
			, int212(0)
		{}
		// Batch275: forwards to the base.
		virtual void generateDataForMovingAssemblyStage()
		{
			Contact::generateDataForMovingAssemblyStage();
		}
	private:
		char pad48[160];
		int int208;    // +208 (0).
		int int212;    // +212 (0).
	};

} // namespace RBX
