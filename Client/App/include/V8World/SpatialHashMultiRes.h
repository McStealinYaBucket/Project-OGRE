/* V8World/SpatialHashMultiRes.h -- October 2013 RBX::NodeBase, RBX::SpatialHashStatic and the
 * SpatialHash<Primitive, Contact, ContactManager, 4> template (bodies in SpatialHashMultiRes.inl).
 * LAYOUT: the 2013 Windows ctor (PLAYER 0x008eef80; ContactManager's ctor 0x007b63a0 news 0xdc):
 *   0x00 ConcurrencyValidator (0x24), 0x24 maxNodes, 0x28 treeNodeCount, 0x2c World*,
 *   0x30 manager, 0x34 four bucket vectors (`eh vector constructor iterator'), 0x74 nodesOut,
 *   0x78 int80, 0x7c G3D::Array<Primitive*>, 0x8c listener vector, 0x9c TreeNode pool,
 *   0xbc SpatialNode pool.
 * The pools are RBX::object_pool over RBX::roblox_allocator (iOS 10-29 symbols): each is a
 * boost::object_pool (0x1c) plus its CallDestructor byte, 0x20; the ctor stores requested size
 * 0x30/0x20, next and start size 0x20, max size 0 (boost 1.53 pool defaults). Nodes come from
 * malloc() and go back through free(); ~SpatialHash's pool destructors run the node destructors
 * (~TreeNode out of line, PLAYER 0x007b36f0; ~SpatialNode inline in the pool dtor 0x00502c50).
 *
 * 2013 asserts in this header: NodeBase::getLevel :51, safeExtents :105/:107, ~TreeNode :287/:288
 * (PLAYER strings; the file is spelled include\V8World/, Include\V8World/ and Include\v8World/
 * by different TUs). Statics: Client/App/v8world/SpatialHashMultiRes.cpp (getHash :72-73,
 * computeMinMax :79). */
#pragma once

#include <vector>

#include <boost/unordered_set.hpp>

#include "Util/Extents.h"
#include "Util/ExtentsInt32.h"
#include "Util/Vector3int32.h"
#include "G3D/Array.h"
#include "Util/ConcurrencyValidator.h"
#include "rbx/object_pool.h"
#include "rbx/memory.h"

namespace RBX {

	// SIZE 0x14: level (a short) 0x00, hash 0x04, cell 0x08. The destructor poisons level and hash
	// (the pool destructors 0x007b36f0 / 0x00502c50 store -2 to both after the derived members).
	class NodeBase
	{
	public:
		~NodeBase()
		{
			level = -2;
			hash = -2;
		}

		short level;
		int hash;
		Vector3int32 cell;

		int getLevel()
		{
			RBXASSERT(level >= -1);
			return level;
		}
	};








	class SpatialHashStatic
	{
	public:
		// mac: 0x5A3AE0 (236B).  Bucket index for a cell at a level:
		//   (0xFFFFu >> level) & (73856093*x ^ 19349663*y ^ 83492791*z)
		// with RBXASSERT(0 <= hash) (:72) and RBXASSERT(hash < numBuckets)
		// (:73), where numBuckets(level) == (0x10000u >> level).  The first
		// parameter arrives as `this` (a bare int level in disguise -- the
		// function is static in spirit and hexrays prints it __cdecl).
		// See v8world/SpatialHashMultiRes.cpp.
		static int getHash(int level, const Vector3int32& cell);
		// numBuckets(level) == (0x10000u >> level): named by the findNode
		// :255 assert text ("SpatialHashStatic::numBuckets(l) ==
		// hashTables[l].size()") and the handoff's level table
		// (65536>>L).  Inline -- pure arithmetic.
		static unsigned long numBuckets(int level) { return 0x10000 >> level; }  // signed shift: getHash 0x007925a0 sar
		// cellMinSize 8: rodata-proved (the "\b" dword computeMinMax
		// multiplies by).  maxLevelForAnchored: loaded but value not yet
		// read from rodata -- 2 is contextual (anchored cap below the
		// default 3), marked TODO-verify.
		static const int cellMinSize = 8;
		static const int maxLevelForAnchored = 2;
		// mac: 0x5A3BCC (342B).  Cell range covering an extents at a level:
		// assert level != -1 (:79); clamp the extents inside the million
		// cube; minOut = floor(lo / (8<<level)), maxOut = floor(hi /
		// (8<<level)).  See v8world/SpatialHashMultiRes.cpp for the
		// million-cube DELTA.
		static void computeMinMax(int level, const Extents& extents,
			Vector3int32& minOut, Vector3int32& maxOut);
		// mac: 0x56E084 (319B).  Inline in the ORIGINAL header (:105/:107), so
		// inline here too.  Assert Extents(e.min(),e.max())==e (:105);
		// return e unless isNanInf, in which case assert "0" (:107) and
		// return Extents::zero().
		// DELTA, shape: the binary is a member-write form (writes the
		// sanitized copy into a caller temp via `this` and returns it).
		// Value-identical as a static; the calling convention is not
		// observable from any caller.
		// STUB: PLAYER 0x004cd9a0
		// STUB: STUDIO 0x00553a30
		static Extents safeExtents(const Extents& e)
		{
			// SpatialHashMultiRes.h:105.
			RBXASSERT(Extents(e.min(), e.max()) == e);
			if (e.isNanInf()) {
				RBXASSERT(0);
				return Extents::zero();
			}
			return e;
		}
	};

	class Primitive;
	class Contact;
	class ContactManager;
	class World;

	// Minimal listener interface for the +100/+104 registrar array: the
	// binary calls slot 0 as listener(prim, &int-flag) (onPrimitiveAdded
	// mac: 0x567D1B: `call dword ptr [ecx]` with [esp+8]=&flag, [esp+4]=prim).
	// Nothing in the tree registers, so it is never called -- but the slot
	// shape is what makes the array meaningful.
	struct SpatialHashListener
	{
		virtual ~SpatialHashListener() {}
		virtual void onPrimitiveAdded(Primitive* prim, int* flag) = 0;
	};

	// 8-byte bucket entries ([base + hash*8] everywhere): node-chain head
	// (+0, the only dword the grid path touches) + tree-chain head (+4,
	// maintained by createTreeNode/_retireTreeNode).  Defined inside the
	// template (after the node structs).
	// Per-level tables are std::vector<HashBucket> (12B: begin/end/capacity
	// at +28/+32/+36 of the level base this+12*L -- setup() resizes each to
	// numBuckets, and findNode asserts size == numBuckets).

	template<class PrimitiveT, class ContactT, class ManagerT, int MaxLevels>
	class SpatialHash
	{
	public:
		struct SpatialNode;	// 0x20, defined below
		struct TreeNode;	// 0x30

		// Bucket entries must be 8B ([base + hash*8]): node-chain head +
		// tree-chain head.  Levels are std::vector<HashBucket>, which IS
		// the binary's 12B {begin,end,capacity} level table.
		struct HashBucket
		{
			SpatialNode* head;
			TreeNode* treeHead;
			HashBucket() : head(NULL), treeHead(NULL) {}
		};

		SpatialHash(World* world, ManagerT* manager, int maxNodes);
		~SpatialHash();

		// mac: 0x56B900 (97B): per level L, buckets.resize(numBuckets(L))
		// (std::vector value-init zeroes the entries -- the binary's
		// explicit bzero reaches the identical state).
		void setup();

		// mac: 0x5674E4.  Collect grid-touching prims (minus ignore, deduped,
		// overlap-filtered) up to maxCount (0 = unlimited).
		void getPrimitivesTouchingGrids(const Extents& extents,
			const PrimitiveT* ignorePrim, unsigned long maxCount,
			boost::unordered_set<PrimitiveT*>& answer);
		// mac: 0x56D600: assert out empty (:1044), then per level L query
		// cell>>=1 (cells double every level).
		void getPrimitivesInGrid(const Vector3int32& cell,
			G3D::Array<PrimitiveT*>& out);
		// mac: 0x56D6B8: hash the cell, walk the bucket chain, append exact
		// cell matches.
		void getPrimitivesInGrid(int level, const Vector3int32& cell,
			G3D::Array<PrimitiveT*>& out);

		// mac: 0x5696C2.  Insert prim's cell; pair-check coarser levels
		// (getContact gate + onNewPair) and descend the tree when
		// createContacts && level > 0 (addContactFromChildren).
		void addNode(PrimitiveT* prim, const Vector3int32& cell,
			bool createContacts);
		// mac: 0x569CB0: level bucket walk matching (prim, cell), or NULL.
		// Asserts numBuckets == table size (:255).
		SpatialNode* findNode(PrimitiveT* prim, const Vector3int32& cell);
		// mac: 0x56A538: alloc + manual init (pool path values verbatim);
		// level>0 attaches/finds the TreeNode (:120 assert inside).
		SpatialNode* newNode(int level, int hash, const Vector3int32& cell);
		// mac: 0x56A6B6: assert node->level == prim level (:220); fill
		// prim/cell/hash into the node.
		void insertNodeToPrimitive(SpatialNode* node, PrimitiveT* prim,
			const Vector3int32& cell, int hash);
		// mac: 0x56A756: alloc + TreeNode() + ++treeNodeCount.
		TreeNode* createTreeNode(int level, int hash,
			const Vector3int32& cell);
		// findTreeNode(level, hash, cell): tree descent for newNode.
		TreeNode* findTreeNode(int level, int hash,
			const Vector3int32& cell);
		// Removal path (cell-shrink / prim removal).  All decoded; only
		// reachable when prims move or leave (never in any green path --
		// nothing calls primitiveRemoved/primitiveExtentsChanged yet).
		// destroyNode 0x569DB8: unlink + (level>0 ? assert+retire tree
		// (:327) : retire level-0 tree child iff no same-cell nodes remain)
		// + returnNode + --nodesOut.
		void destroyNode(SpatialNode* node);
		// returnNode 0x56A0E2: poison (tree/prim/next 0, level/hash -2) +
		// pool-free.  Ours deletes + untracks (same lifetime, no leak).
		void returnNode(SpatialNode* node);
		// retireTreeNode 0x569E98: --count; if 0 && no children: _retire +
		// removeTreeNodeChild(level, cell) upward.
		void retireTreeNode(TreeNode* treeNode);
		// _retireTreeNode 0x56A308: unlink from bucket tree-chain, null the
		// link, pool-free, --treeNodeCount.
		void _retireTreeNode(TreeNode* treeNode);
		// removeTreeNodeChild 0x569EF4(level, cell): walk levels above,
		// clear the child bit/link (:355/:358 asserts), retire emptied
		// nodes upward, stop at the first non-empty one.
		void removeTreeNodeChild(int level, Vector3int32& cell);
		// removeNodeFromHash 0x56E040: unlink by identity from the bucket.
		void removeNodeFromHash(SpatialNode* node);
		// findOtherNodesInLevel0Cell 0x56DF94: assert level 0 (:295);
		// true iff any same-cell node remains in the level-0 bucket.
		bool findOtherNodesInLevel0Cell(SpatialNode* node);
		// addContactFromChildren(treeNode, prim): level>0 contact creation
		// inside addNode.  Body decoded in a later round; declared so
		// addNode links.
		void addContactFromChildren(TreeNode* treeNode, PrimitiveT* prim);

		// mac: 0x568E38: assert not-inserted (:585); level = computeLevel;
		// stash cells+level on the prim; changeMinMax(add all).
		void primitiveAdded(PrimitiveT* prim, bool createContacts);
		// mac: 0x567D48 (523B).  Production moves entry: megacluster skip, then
		// computeLevel at the fresh extents -- level within {-1,0} of old
		// goes in-place, anything else removes + re-adds (which re-levels).
		// checkMegaClusterContact (voxel) + listener fan-out (event layout
		// unrecoverable, array empty in-tree) are DELTAs -- see the .inl.
		void onPrimitiveExtentsChanged(PrimitiveT* prim);
		bool oldExtentsOverlap(PrimitiveT* p0, PrimitiveT* p1);
		// mac: 0x569450 (191B).  Collect every contact-neighbor whose pair no
		// longer overlaps (skipping megacluster prims) into array84, then
		// releasePair each.  Returns the last internal (like the binary);
		// callers ignore it, so ours returns void -- documented, not hidden.
		void checkAndReleaseContacts(PrimitiveT* prim);
		void primitiveExtentsChanged(PrimitiveT* prim, const Extents& extents);
		// mac: 0x568CCE (361B): assert inserted (:757, minus the unported
		// excluded-prim clause -- see body); changeMinMax with
		// new==old==cells (destroys every cell); reset level to -1 and
		// cells to empty; snapshot contact-neighbors into array84 and
		// releasePair each.
		void primitiveRemoved(PrimitiveT* prim);
		// mac: 0x567C88: skip MEGACLUSTER (type 10); primitiveAdded; notify the
		// (empty-in-our-tree) listener array.
		void onPrimitiveAdded(PrimitiveT* prim, bool createContacts);
		// mac: 0x5692A0: sync cells in [newMin,newMax]: addNode for cells in new
		// but not old, findNode+destroyNode for cells in old but not new
		// (:624 assert inside).  Pointer-identity shortcuts are semantically
		// transparent (a range trivially contains its own cells).
		void changeMinMax(PrimitiveT* prim, const ExtentsInt32* newCells,
			const ExtentsInt32* oldCells, const ExtentsInt32* curCells,
			bool createContacts);
		// mac: 0x568C14: smallest L with 6400*8^L > (dx+16)(dy+16)(dz+16),
		// capped at (anchored||dragging ? maxLevelForAnchored : 3).
		int computeLevel(const PrimitiveT* prim, const Extents& extents);

		// SIZE 0x20
		struct SpatialNode : public NodeBase
		{
			PrimitiveT* prim;		// 0x14
			SpatialNode* next;		// 0x18 (bucket chain)
			TreeNode* treeNode;		// 0x1c
			~SpatialNode()
			{
				treeNode = NULL;
				prim = NULL;
				next = NULL;
			}
		};

		// SIZE 0x30. children are unsigned: level-0 child hashes reach 65535 (movzx reads).
		struct TreeNode : public NodeBase
		{
			unsigned short children[8];	// 0x14
			char hasChild;				// 0x24
			int refByPrimitives;		// 0x28
			TreeNode* next;				// 0x2c (bucket tree chain)
			~TreeNode()
			{
				RBXASSERT(refByPrimitives == 0);
				RBXASSERT(next == NULL);
				next = NULL;
			}
		};

	private:
		SpatialHash(const SpatialHash&);
		SpatialHash& operator=(const SpatialHash&);

		// 2013 layout from the ctor (PLAYER 0x008eef80; ContactManager news 0xdc). Same order as the
		// Mac model; the pools are RBX::object_pool (0x20 each with its CallDestructor byte).
		ConcurrencyValidator concurrencyValidator;		// 0x00
		int maxNodes;									// 0x24 (100 from ContactManagerSpatialHash)
		int treeNodeCount;								// 0x28
		World* world;									// 0x2c
		ManagerT* manager;								// 0x30
		std::vector<HashBucket> buckets[MaxLevels];		// 0x34
		int nodesOut;									// 0x74
		int int80;										// 0x78 (max coarser-depth stat)
		G3D::Array<PrimitiveT*> array84;				// 0x7c
		std::vector<SpatialHashListener*> listeners;	// 0x8c
		RBX::object_pool<TreeNode, RBX::roblox_allocator> treeNodePool;		// 0x9c (requested size 0x30)
		RBX::object_pool<SpatialNode, RBX::roblox_allocator> nodePool;		// 0xbc (requested size 0x20)
	};

	// Out-of-line node and pool destructors of the ContactManager instantiation (run by ~SpatialHash).
	// clang-format off
	// TEMPLATE: PLAYER 0x007b95d0
	// TEMPLATE: STUDIO 0x008d5f70
	// ??1?$SpatialHash@VPrimitive@RBX@@VContact@2@VContactManager@2@$03@RBX@@QAE@XZ

	// TEMPLATE: PLAYER 0x007b36f0
	// TEMPLATE: STUDIO 0x008cf770
	// ??1TreeNode@?$SpatialHash@VPrimitive@RBX@@VContact@2@VContactManager@2@$03@RBX@@QAE@XZ

	// TEMPLATE: PLAYER 0x007b7450
	// TEMPLATE: STUDIO 0x008d4400
	// ??1?$object_pool@UTreeNode@?$SpatialHash@VPrimitive@RBX@@VContact@2@VContactManager@2@$03@RBX@@Vroblox_allocator@3@@boost@@QAE@XZ

	// TEMPLATE: PLAYER 0x00502c50
	// TEMPLATE: STUDIO 0x00592e90
	// ??1?$object_pool@USpatialNode@?$SpatialHash@VPrimitive@RBX@@VContact@2@VContactManager@2@$03@RBX@@Vroblox_allocator@3@@boost@@QAE@XZ
	// clang-format on

#include "v8World/SpatialHashMultiRes.inl"	// the 2013 __FILE__ spelling (include\v8World/SpatialHashMultiRes.inl)

} // namespace RBX
