/* V8World/SpatialHashMultiRes.h -- October 2013 RBX::SpatialHashStatic +
 * RBX::SpatialHash<Primitive, Contact, ContactManager, 4> template.
 *
 * (This header replaces a 0-byte placeholder.)
 *
 * EVIDENCE (symbolled 0.126.0.39503 Studio, idalib; batches 121/123/125):
 *   statics: 0x5A3AE0 getHash / 0x5A3BCC computeMinMax / 0x56E084 safeExtents
 *   0x56CB78  SpatialHash::SpatialHash(World*, ContactManager*, int)  410B
 *   0x5674E4  getPrimitivesTouchingGrids (walk, 814B)
 *   0x56D600 / 0x56D6B8  getPrimitivesInGrid overloads (183B / 100B)
 *   0x5696C2  addNode(Primitive*, Vector3int32 const&, bool)         1517B
 *   0x569CB0  findNode(Primitive*, Vector3int32 const&)               263B
 *   0x56A538  newNode(int, int, Vector3int32 const&)                 381B
 *   0x56A6B6  insertNodeToPrimitive(SpatialNode*, Primitive*,
 *               Vector3int32 const&, int)                            160B
 *   0x56A756  createTreeNode(int, int, Vector3int32 const&)          164B
 *   0x56A9EC  TreeNode::TreeNode()                                   130B
 *   0x568E38  primitiveAdded(Primitive*, bool)                       455B
 *   0x567C88  onPrimitiveAdded(Primitive*, bool)                     192B
 *   (changeMinMax 0x5692A0 + computeLevel 0x568C14: batch126.)
 *   TreeNode pool: boost::object_pool<TreeNode, 48u>; SpatialNode pool:
 *   object_pool<SpatialNode, 32u> (batch122 sweep).
 *
 * MODEL-RELATIVE LAYOUT (binary C2 offsets in comments; our model packs the
 * same members in the same ORDER but without the pool objects and registries
 * we don't need -- sizes differ, names and behavior don't):
 *   +0 int 0 / +4 name"" / +8 int 0 / +12 maxNodes / +16 treeNodeCount /
 *   +20 World* / +24 Manager* / +28..75 four LevelTables (12B each) /
 *   +76 nodeCount / +80 int80 / +84 Array<Primitive*> /
 *   +100/+104 listener begin/end (+108..+120 pool-adjacent -- omitted) /
 *   +124..164 pool params (48/32/32 -- omitted, constants live in code) /
 *   +136..168 pool objects (omitted -- plain new/delete, see below).
 *
 * POOLS-AS-NEW DELTA: the binary allocates TreeNodes (48B) and SpatialNodes
 * (32B) from boost::object_pool (or RBX::Allocator when
 * DFFlag::UseSpatialHashUnorderedPool is set -- the manual-init path, whose
 * byte values this port uses verbatim).  Allocation strategy is unobservable
 * from any reader (buckets, counts, links are identical), so this port uses
 * plain new/delete and skips the flag.  All nodes are additionally tracked
 * for the destructor (the binary pools never free until teardown either).
 *
 * Original files: .../Client/App/v8world/SpatialHashMultiRes.cpp (statics,
 * getHash :72-73, computeMinMax :79) and
 * .../Client/App/include/V8World/SpatialHashMultiRes.h (safeExtents :105/
 * :107, addNode asserts :429/:474/:485/:498/:500/:509/:519, insert :220,
 * newNode :120, findNode :255, changeMinMax :624, primitiveAdded :585/:599,
 * onPrimitiveAdded listeners).
 * Template bodies live in SpatialHashMultiRes.inl (included below). */

#pragma once

#include <vector>

#include <boost/unordered_set.hpp>

#include "Util/Extents.h"
#include "Util/ExtentsInt32.h"
#include "Util/Vector3int32.h"
#include "G3D/Array.h"

namespace RBX {

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
		static unsigned long numBuckets(int level) { return 0x10000u >> level; }

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
			if (e.isNanInf())
			{
				// SpatialHashMultiRes.h:107 ("0").
				RBXASSERT(false);
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
		// 32B exactly (pool-confirmed).  Field offsets from newNode 0x56A538
		// (+0W level, +4 hash, +8/+12/+16 cell, +20 prim, +24 next) and
		// insertNodeToPrimitive 0x56A6B6 (same) + newNode's +28 treeNode.
		struct SpatialNode
		{
			short level;
			short padLevel;
			int hash;
			Vector3int32 cell;
			PrimitiveT* prim;
			SpatialNode* next;
			struct TreeNode* treeNode;
		};

		// 48B exactly (pool-confirmed).  From TreeNode::TreeNode 0x56A9EC
		// (+0W level -1, +4 hash -1, +8/+12/+16 cell 0, +20..+35 children
		// WORD[8] -1, +36 hasChild 0, +40 count 0, +44 0) and addNode's
		// children[offset]=prevHash writes (:500) + count++ (:509 area).
		struct TreeNode
		{
			short level;
			short padLevel;
			int hash;
			Vector3int32 cell;
			// WORD[8], unsigned: level-0 child hashes reach 65535, which a
			// signed short would sign-extend to negative bucket indexes
			// (binary reads movzx; the signed spelling crashed walking
			// children >32767 -- caught live, ch=-19317 for a 46219 hash).
			unsigned short children[8];
			char hasChild;
			char padChild[3];
			int count;
			TreeNode* treeNext;   // +44 bucket tree-chain link (was int44)
		};

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
		// + returnNode + --nodeCount.
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
		// mac: 0x569510 (376B).  Same-level: ExtentsInt32 overlap of the stored
		// ranges (:276/:277 level asserts).  Cross-level: shift the finer
		// prim's cells up to the coarser level, then overlap.  Despite the
		// name it reads CURRENT stored cells (they're already the new ones
		// when checkAndReleaseContacts runs) -- ported verbatim, not renamed.
		bool oldExtentsOverlap(PrimitiveT* p0, PrimitiveT* p1);
		// mac: 0x569450 (191B).  Collect every contact-neighbor whose pair no
		// longer overlaps (skipping megacluster prims) into array84, then
		// releasePair each.  Returns the last internal (like the binary);
		// callers ignore it, so ours returns void -- documented, not hidden.
		void checkAndReleaseContacts(PrimitiveT* prim);
		// mac: 0x569000.  In-place cell update for a moved/resized prim (body in
		// the next dump round): recompute cells from the new extents and
		// changeMinMax against the stored range (adds new cells, destroys
		// left ones).  The entry point moves tests will call directly
		// (ContactManager::onPrimitiveExtentsChanged needs the unported
		// checkMegaClusterContact, so it waits).
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

	private:
		SpatialHash(const SpatialHash&);
		SpatialHash& operator=(const SpatialHash&);

		int int0;                        // +0 (0)
		const char* name;                // +4 ("")
		int int8;                        // +8 (0)
		int maxNodes;                    // +12 (100 via C1)
		int treeNodeCount;               // +16 (++ in createTreeNode)
		World* world;                    // +20
		ManagerT* manager;               // +24
		int nodeCount;                   // +76 (++ in addNode)
		int int80;                       // +80 (max coarser-depth stat)
		G3D::Array<PrimitiveT*> array84; // +84
		G3D::Array<SpatialHashListener*> listeners; // +100/+104
		// +108..+172: pool objects/params + registries (omitted -- plain
		// new/delete need no storage; see the header note).

		// Per-level bucket vectors (the binary's 12B level tables).
		std::vector<HashBucket> buckets[MaxLevels];

		// All live nodes/trees (binary pools never free until teardown;
		// this frees them in ~SpatialHash -- same lifetime, no leak).
		std::vector<SpatialNode*> allNodes;
		std::vector<TreeNode*> allTrees;
	};

#include "V8World/SpatialHashMultiRes.inl"

} // namespace RBX
