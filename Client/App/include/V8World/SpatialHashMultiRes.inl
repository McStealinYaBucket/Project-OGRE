/* V8World/SpatialHashMultiRes.inl -- October 2013 SpatialHash<Primitive,
 * Contact, ContactManager, 4> template bodies.  Included at the bottom of
 * SpatialHashMultiRes.h (see that header for the evidence block).
 *
 * CONVENTIONS (all documented per-use below, not repeated):
 * - RBXASSERT(cond) + `// :line` comments carry the binary's
 *   SpatialHashMultiRes.inl/.cpp assert texts; our macro honors
 *   FLog::Asserts, exactly like the binary's flag check.
 * - Pools-as-new: plain new/delete with the binary's byte values (see the
 *   header note).  All live nodes/trees are tracked for ~SpatialHash.
 * - Removal-path bodies (destroyNode/returnNode/retireTreeNode/
 *   removeTreeNodeChild/removeNodeFromHash/findOtherNodesInLevel0Cell) are
 *   fully decoded and ported below; they run only when prims move or leave
 *   (never in any green path yet).
 */

template<class PrimitiveT, class ContactT, class ManagerT, int MaxLevels>
SpatialHash<PrimitiveT, ContactT, ManagerT, MaxLevels>::SpatialHash(
	World* world, ManagerT* manager, int maxNodes)
	: int0(0)
	, name("")
	, int8(0)
	, maxNodes(maxNodes)
	, treeNodeCount(0)
	, world(world)
	, manager(manager)
	, nodeCount(0)
	, int80(0)
{
	// mac: 0x56CB78: the +28..+75 loop zeroes the four level tables; setup()
	// sizes them.  (Pool-adjacent members omitted -- see header.)
	setup();
}

template<class PrimitiveT, class ContactT, class ManagerT, int MaxLevels>
SpatialHash<PrimitiveT, ContactT, ManagerT, MaxLevels>::~SpatialHash()
{
	// Binary D2 walks its pools; ours tracks everything explicitly.
	// Prims are NOT owned (never deleted here).
	for (size_t i = 0; i < allNodes.size(); ++i)
		delete allNodes[i];
	for (size_t i = 0; i < allTrees.size(); ++i)
		delete allTrees[i];
}

// mac: 0x56B900 (97B).  Per level: buckets.resize(numBuckets) (std::vector,
// value-initialized) + bzero.  Our HashBucket() zeroes head/pad, so
// resize alone reaches the identical state -- no explicit bzero needed.
template<class PrimitiveT, class ContactT, class ManagerT, int MaxLevels>
void SpatialHash<PrimitiveT, ContactT, ManagerT, MaxLevels>::setup()
{
	for (int i = 0; i < MaxLevels; ++i)
		buckets[i].resize((size_t)SpatialHashStatic::numBuckets(i));
}

// mac: 0x5674E4 (814B).  See the session notes for the branch-by-branch mapping;
// the shape: assert answer empty (:894); per level (skip when the bucket
// vector is empty -- vestigial post-setup, kept verbatim) computeMinMax,
// walk every cell in [min,max], collect the cell's prims, skip self,
// dedup via the set, safeExtents + overlapsOrTouches filter, emplace with
// the maxCount early-out (0 = unlimited); assert temp < 200 (:929).
// STUB: PLAYER 0x007b9a50
// STUB: STUDIO 0x008d63f0
template<class PrimitiveT, class ContactT, class ManagerT, int MaxLevels>
void SpatialHash<PrimitiveT, ContactT, ManagerT, MaxLevels>::getPrimitivesTouchingGrids(
	const Extents& extents, const PrimitiveT* ignorePrim, unsigned long maxCount,
	boost::unordered_set<PrimitiveT*>& answer)
{
	RBXASSERT(answer.size() == 0);  // :894 "answer.size() == 0"
	Vector3int32 minCell, maxCell;
	G3D::Array<PrimitiveT*> foundThisGrid;
	for (int level = 0; level < MaxLevels; ++level)
	{
		if (buckets[level].empty())
			continue;
		SpatialHashStatic::computeMinMax(level, extents, minCell, maxCell);
		for (int x = minCell.x; x <= maxCell.x; ++x)
		{
			for (int y = minCell.y; y <= maxCell.y; ++y)
			{
				for (int z = minCell.z; z <= maxCell.z; ++z)
				{
					Vector3int32 cell(x, y, z);
					foundThisGrid.resize(0, false);
					getPrimitivesInGrid(level, cell, foundThisGrid);
					for (int i = 0; i < foundThisGrid.size(); ++i)
					{
						PrimitiveT* prim = foundThisGrid[i];
						if (prim == ignorePrim)
							continue;
						if (answer.find(prim) != answer.end())
							continue;
						Extents safe = SpatialHashStatic::safeExtents(
							prim->getFastFuzzyExtents());
						if (extents.overlapsOrTouches(safe))
						{
							answer.insert(prim);
							if (maxCount != 0 && (unsigned long)answer.size() >= maxCount)
								return;
						}
					}
				}
			}
		}
	}
	RBXASSERT(foundThisGrid.size() < 200);  // :929 "foundThisGrid.size() < 200"
}

// mac: 0x56D600 (183B): assert out empty (:1044); per level query cell>>=1
// (cells double every level: 8<<L).
// STUB: PLAYER 0x007b5010
// STUB: STUDIO 0x008d1280
template<class PrimitiveT, class ContactT, class ManagerT, int MaxLevels>
void SpatialHash<PrimitiveT, ContactT, ManagerT, MaxLevels>::getPrimitivesInGrid(
	const Vector3int32& cell, G3D::Array<PrimitiveT*>& out)
{
	RBXASSERT(out.size() == 0);  // :1044 "found.size() == 0"
	Vector3int32 c = cell;
	for (int level = 0; level < MaxLevels; ++level)
	{
		getPrimitivesInGrid(level, c, out);
		c = c >> 1;
	}
}

// mac: 0x56D6B8 (100B): hash the cell, walk the bucket chain, append exact
// cell matches (cell compare, not overlap -- overlap is the caller's job).
template<class PrimitiveT, class ContactT, class ManagerT, int MaxLevels>
void SpatialHash<PrimitiveT, ContactT, ManagerT, MaxLevels>::getPrimitivesInGrid(
	int level, const Vector3int32& cell, G3D::Array<PrimitiveT*>& out)
{
	int hash = SpatialHashStatic::getHash(level, cell);
	for (SpatialNode* node = buckets[level][hash].head;
		node != NULL;
		node = node->next)
	{
		if (node->cell == cell)
			out.append(node->prim);
	}
}

// mac: 0x569CB0 (263B): level bucket walk matching (prim, cell), or NULL.
// Asserts numBuckets == table size (:255).
template<class PrimitiveT, class ContactT, class ManagerT, int MaxLevels>
typename SpatialHash<PrimitiveT, ContactT, ManagerT, MaxLevels>::SpatialNode*
SpatialHash<PrimitiveT, ContactT, ManagerT, MaxLevels>::findNode(
	PrimitiveT* prim, const Vector3int32& cell)
{
	int level = prim->getSpatialNodeLevel();
	RBXASSERT((int)SpatialHashStatic::numBuckets(level)  // :255
		== (int)buckets[level].size());
	int hash = SpatialHashStatic::getHash(level, cell);
	for (SpatialNode* node = buckets[level][hash].head;
		node != NULL;
		node = node->next)
	{
		if (node->prim == prim && node->cell == cell)
			return node;
	}
	return NULL;
}

// mac: 0x56A538 (381B).  Manual-init values verbatim (the DFFlag pool branch
// does the same bytes via construct).  Level 0: no tree.  Else find (or
// create) the TreeNode, attach, count++, assert level match (:120).
// STUB: PLAYER 0x007bad80
// STUB: STUDIO 0x008d7720
template<class PrimitiveT, class ContactT, class ManagerT, int MaxLevels>
typename SpatialHash<PrimitiveT, ContactT, ManagerT, MaxLevels>::SpatialNode*
SpatialHash<PrimitiveT, ContactT, ManagerT, MaxLevels>::newNode(
	int level, int hash, const Vector3int32& cell)
{
	SpatialNode* node = new SpatialNode();
	node->level = (short)level;
	node->hash = hash;
	node->cell = cell;
	node->prim = NULL;
	node->next = NULL;
	node->treeNode = NULL;
	allNodes.push_back(node);
	if (level == 0)
		return node;
	TreeNode* treeNode = findTreeNode(level, hash, cell);
	if (treeNode == NULL)
		treeNode = createTreeNode(level, hash, cell);
	node->treeNode = treeNode;
	++treeNode->count;
	RBXASSERT(level == treeNode->level);  // :120 "level == answer->treeNode->level"
	return node;
}

// mac: 0x56A6B6 (160B): assert node->level == prim level (:220); fill
// prim/cell/hash.
// STUB: PLAYER 0x007b3d90
// STUB: STUDIO 0x008cfe10
template<class PrimitiveT, class ContactT, class ManagerT, int MaxLevels>
void SpatialHash<PrimitiveT, ContactT, ManagerT, MaxLevels>::insertNodeToPrimitive(
	SpatialNode* node, PrimitiveT* prim, const Vector3int32& cell, int hash)
{
	RBXASSERT(node->level == prim->getSpatialNodeLevel());  // :220
	node->prim = prim;
	node->cell = cell;
	node->hash = hash;
}

// mac: 0x56A756 (164B): alloc + TreeNode() + ++treeNodeCount, then set
// level/hash/cell and link into the bucket's tree chain
// (tree->treeNext = head; head = tree).
template<class PrimitiveT, class ContactT, class ManagerT, int MaxLevels>
typename SpatialHash<PrimitiveT, ContactT, ManagerT, MaxLevels>::TreeNode*
SpatialHash<PrimitiveT, ContactT, ManagerT, MaxLevels>::createTreeNode(
	int level, int hash, const Vector3int32& cell)
{
	TreeNode* treeNode = new TreeNode();
	// TreeNode::TreeNode 0x56A9EC inits (see header): level -1, hash -1,
	// cell 0, children[8] -1, hasChild 0, count 0, +44 0.  The level/hash/
	// cell/treeNext are overwritten below; children/hasChild/count must
	// start clean (uninitialized garbage caused hasChild=-40/count=737M
	// crashes on first level>0 insert -- caught live).
	for (int i = 0; i < 8; ++i)
		treeNode->children[i] = -1;
	treeNode->hasChild = 0;
	treeNode->count = 0;
	treeNode->level = (short)level;
	treeNode->hash = hash;
	treeNode->cell = cell;
	allTrees.push_back(treeNode);
	++treeNodeCount;
	HashBucket& bucket = buckets[level][hash];
	treeNode->treeNext = bucket.treeHead;
	bucket.treeHead = treeNode;
	return treeNode;
}

// mac: 0x568E38 (455B): assert not-inserted (:585); level = computeLevel;
// stash min/max/level on the prim; changeMinMax(add all cells);
// assert level stuck (:599).
// STUB: PLAYER 0x007bbad0
// STUB: STUDIO 0x008d8470
template<class PrimitiveT, class ContactT, class ManagerT, int MaxLevels>
void SpatialHash<PrimitiveT, ContactT, ManagerT, MaxLevels>::primitiveAdded(
	PrimitiveT* prim, bool createContacts)
{
	// :585 "p->getSpatialNodeLevel() == -1".
	RBXASSERT(prim->getSpatialNodeLevel() == -1);
	Extents safe = SpatialHashStatic::safeExtents(prim->getFastFuzzyExtents());
	int level = computeLevel(prim, safe);
	Vector3int32 minCell, maxCell;
	SpatialHashStatic::computeMinMax(level, safe, minCell, maxCell);
	prim->setSpatialCells(minCell, maxCell, level);
	// changeMinMax takes ExtentsInt32 ranges; the two adjacent Vector3int32
	// temps ARE that layout (24B), exactly as the binary reinterprets its
	// stack pair -- so build the real object instead of punning.
	ExtentsInt32 cells(minCell, maxCell);
	changeMinMax(prim, &cells, NULL, &cells, createContacts);
	// :599 "p->getSpatialNodeLevel() == level".
	RBXASSERT(prim->getSpatialNodeLevel() == level);
}

// mac: 0x567C88 (192B): skip MEGACLUSTER (type 10); primitiveAdded; notify the
// listener array (empty in our tree -- the loop is still written out).
// STUB: PLAYER 0x007bbf90
// STUB: STUDIO 0x008d8930
template<class PrimitiveT, class ContactT, class ManagerT, int MaxLevels>
void SpatialHash<PrimitiveT, ContactT, ManagerT, MaxLevels>::onPrimitiveAdded(
	PrimitiveT* prim, bool createContacts)
{
	if (prim->getGeometryType() == 10)
		return;
	primitiveAdded(prim, createContacts);
	int unusedFlag = 0;
	for (size_t i = 0; i < listeners.size(); ++i)
		listeners[i]->onPrimitiveAdded(prim, &unusedFlag);
}

// mac: 0x5692A0 (432B): sync the cells in [newMin,newMax].  inOld = (old==new)
// or (old && old->contains); inNew = (cur==new) or (cur && cur->contains)
// -- the binary's pointer-identity shortcuts are semantically transparent
// (a well-formed range trivially contains its own cells), so the clean
// form below is exact.  !inOld && inNew -> addNode; inOld && !inNew ->
// findNode + assert (:624 "destroyMe") + destroyNode.
// STUB: PLAYER 0x007bb950
// STUB: STUDIO 0x008d82f0
template<class PrimitiveT, class ContactT, class ManagerT, int MaxLevels>
void SpatialHash<PrimitiveT, ContactT, ManagerT, MaxLevels>::changeMinMax(
	PrimitiveT* prim, const ExtentsInt32* newCells, const ExtentsInt32* oldCells,
	const ExtentsInt32* curCells, bool createContacts)
{
	for (int x = newCells->min().x; x <= newCells->max().x; ++x)
	{
		for (int y = newCells->min().y; y <= newCells->max().y; ++y)
		{
			for (int z = newCells->min().z; z <= newCells->max().z; ++z)
			{
				Vector3int32 cell(x, y, z);
				bool inOld = (oldCells == newCells)
					|| (oldCells != NULL && oldCells->contains(cell));
				bool inNew = (curCells == newCells)
					|| (curCells != NULL && curCells->contains(cell));
				if (!inOld && inNew)
				{
					addNode(prim, cell, createContacts);
				}
				else if (inOld && !inNew)
				{
					SpatialNode* node = findNode(prim, cell);
					RBXASSERT(node != NULL);  // :624 "destroyMe"
					destroyNode(node);
				}
			}
		}
	}
}

// STUB: PLAYER 0x007bbc40
// STUB: STUDIO 0x008d85e0
template<class PrimitiveT, class ContactT, class ManagerT, int MaxLevels>
void SpatialHash<PrimitiveT, ContactT, ManagerT, MaxLevels>::primitiveRemoved(
	PrimitiveT* prim)
{
	// :757 "level != -1 || primitiveIsExcludedFromSpatialHash(p)".  The
	// second clause calls ContactManager::primitiveIsExcludedFromSpatialHash
	// (unported; nothing in the tree could satisfy it anyway -- excluded
	// prims are never inserted, so they never reach removal).  With the
	// harness flag off the whole assert is skipped either way; the bare
	// level check preserves the only reachable half.
	RBXASSERT(prim->getSpatialNodeLevel() != -1);
	ExtentsInt32 cells(prim->getSpatialMinCell(), prim->getSpatialMaxCell());
	changeMinMax(prim, &cells, &cells, NULL, true);
	const ExtentsInt32& empty = ExtentsInt32::empty();
	prim->setSpatialCells(empty.min(), empty.max(), -1);
	array84.resize(0, true);
	for (int i = 0; i < prim->contacts.size(); ++i)
		array84.append(prim->contacts.getOther(i));
	for (int k = 0; k < array84.size(); ++k)
		manager->releasePair(prim, array84[k]);
}

// mac: 0x568C14 (181B): smallest L with 512*maxNodes*8^L > (dx+16)(dy+16)(dz+16),
// capped at (anchored||dragging ? maxLevelForAnchored : 3).  pad = 2*8:
// cellMinSize (8, rodata-proved) doubled, exactly as the binary's
// cvtsi2sd+addsd sequence computes it.
template<class PrimitiveT, class ContactT, class ManagerT, int MaxLevels>
int SpatialHash<PrimitiveT, ContactT, ManagerT, MaxLevels>::computeLevel(
	const PrimitiveT* prim, const Extents& extents)
{
	bool lowDetail = prim->getAnchoredProperty() || prim->getDragging();
	int maxLevel = lowDetail ? SpatialHashStatic::maxLevelForAnchored : 3;
	float threshold = (float)(512 * maxNodes);
	float pad = 2.0f * (float)SpatialHashStatic::cellMinSize;
	float dx = extents.max().x - extents.min().x;
	float dy = extents.max().y - extents.min().y;
	float dz = extents.max().z - extents.min().z;
	int level = 0;
	while (true)
	{
		if (threshold > (dx + pad) * (dy + pad) * (dz + pad))
			break;
		threshold *= 8.0f;
		if (++level >= maxLevel)
			return maxLevel;
	}
	return level;
}

// mac: 0x5696C2 (1517B).  Assert level set (:429); hash; ++nodeCount; node =
// newNode; insertNodeToPrimitive; bucket prepend.  Then the coarser-level
// loop (L = level+1 .. 3): halve the cell, hash, pair-check every node in
// the matching cell (skip self; same-cell only with the :485 gridId assert;
// getContact gate; onNewPair on miss; the WHOLE bucket is walked, no
// break), tree find-or-create with children[offset]=prevHash + hasChild
// bits (:498/:500), first-flag cleared on a fresh bit; break out early iff
// past own level with first still set.  Finally, when createContacts &&
// addedNode->level > 0: assert (:519) + addContactFromChildren; int80 =
// max(int80, coarserCount).
// STUB: PLAYER 0x007bafa0
// STUB: STUDIO 0x008d7940
template<class PrimitiveT, class ContactT, class ManagerT, int MaxLevels>
void SpatialHash<PrimitiveT, ContactT, ManagerT, MaxLevels>::addNode(
	PrimitiveT* prim, const Vector3int32& cell, bool createContacts)
{
	// :429 "p->getSpatialNodeLevel() != -1".
	RBXASSERT(prim->getSpatialNodeLevel() != -1);
	int level = prim->getSpatialNodeLevel();
	int hash = SpatialHashStatic::getHash(level, cell);
	++nodeCount;
	SpatialNode* node = newNode(level, hash, cell);
	insertNodeToPrimitive(node, prim, cell, hash);
	HashBucket& ownBucket = buckets[level][hash];
	node->next = ownBucket.head;
	ownBucket.head = node;

	Vector3int32 coarse = cell;
	int coarserCount = 1;
	int prevHash = hash;
	bool first = true;
	// Own bucket first (binary's first loop iteration, L == level: no
	// halving, no tree work -- just the pair check over the bucket the new
	// node was prepended to; self skipped by the prim check).
	if (createContacts)
	{
		for (SpatialNode* other = buckets[level][hash].head;
			other != NULL;
			other = other->next)
		{
			if (other->prim == prim || !(other->cell == cell))
			{
				// :485 "g != tryNode->gridId".
				RBXASSERT(other->cell == cell);
				continue;
			}
			if (PrimitiveT::getContact(prim, other->prim) == NULL)
				manager->onNewPair(prim, other->prim);
		}
	}
	for (int l = level + 1; l < MaxLevels; ++l)
	{
		// :474 "l < MAX_LEVELS" (vacuous in this spelling -- kept verbatim).
		RBXASSERT(l < MaxLevels);
		coarse = coarse >> 1;
		int ch = SpatialHashStatic::getHash(l, coarse);
		if (createContacts)
		{
			for (SpatialNode* other = buckets[l][ch].head;
				other != NULL;
				other = other->next)
			{
				if (other->prim == prim || !(other->cell == coarse))
				{
					// :485 "g != tryNode->gridId" -- fires in debug on hash
					// collisions (distinct cells sharing a bucket); skipped
					// with the flag off like every assert here.
					RBXASSERT(other->cell == coarse);
					continue;
				}
				if (PrimitiveT::getContact(prim, other->prim) == NULL)
					manager->onNewPair(prim, other->prim);
			}
		}
		// Tree descent for the new node's ancestry at this level.
		TreeNode* treeNode = findTreeNode(l, ch, coarse);
		if (treeNode == NULL)
			treeNode = createTreeNode(l, ch, coarse);
		// Child index from the ORIGINAL cell, bit (l-level-1) per axis:
		// which octant of the level-l node holds the prim's cell.
		int shift = l - level - 1;
		int childBase = ((cell.x >> shift) & 1)
			| (((cell.y >> shift) & 1) << 1)
			| (((cell.z >> shift) & 1) << 2);
		// :498 "0<=offset && offset<=7 && prevHash >= 0".
		RBXASSERT(childBase >= 0 && childBase <= 7 && prevHash >= 0);
		int bit = 1 << childBase;
		if ((treeNode->hasChild & bit) != 0)
		{
			// :500 -- the link must already point at our previous hash.
			RBXASSERT(treeNode->children[childBase] == (unsigned short)prevHash);
		}
		else
		{
			treeNode->hasChild = (char)(treeNode->hasChild | bit);
			treeNode->children[childBase] = (unsigned short)prevHash;
			first = false;
		}
		prevHash = ch;
		++coarserCount;
		if (first)
			break;
	}
	if (createContacts && node->level > 0)
	{
		// :519 "addedNode && addedNode->treeNode->level..." (second half).
		RBXASSERT(node->treeNode != NULL && node->level == node->treeNode->level);
		addContactFromChildren(node->treeNode, prim);
	}
	if (int80 < coarserCount)
		int80 = coarserCount;
}

// mac: 0x56A1D8 (304B): walk the level's bucket tree chain asserting hash
// (:133 "tn->hashId == hash") and level (:134 "tn->level == level"),
// returning the first cell match (or NULL).
// STUB: PLAYER 0x007b3b70
// STUB: STUDIO 0x008cfbf0
template<class PrimitiveT, class ContactT, class ManagerT, int MaxLevels>
typename SpatialHash<PrimitiveT, ContactT, ManagerT, MaxLevels>::TreeNode*
SpatialHash<PrimitiveT, ContactT, ManagerT, MaxLevels>::findTreeNode(
	int level, int hash, const Vector3int32& cell)
{
	for (TreeNode* treeNode = buckets[level][hash].treeHead;
		treeNode != NULL;
		treeNode = treeNode->treeNext)
	{
		RBXASSERT(treeNode->hash == hash);    // :133 "tn->hashId == hash"
		RBXASSERT(treeNode->level == level);  // :134 "tn->level == level"
		if (treeNode->cell == cell)
			return treeNode;
	}
	return NULL;
}

// mac: 0x572DF4 (662B).  For each populated child octant of the tree node:
// rebuild the child cell (2*cell + bit), walk that cell's level-1 bucket,
// pair-check (skip self; same-cell only with the :412 gridId assert;
// getContact gate; onNewPair on miss), then recurse into the child tree
// (found by level-1 hash, asserted "c" :420) when level >= 2.
// STUB: PLAYER 0x007b9fd0
// STUB: STUDIO 0x008d6970
template<class PrimitiveT, class ContactT, class ManagerT, int MaxLevels>
void SpatialHash<PrimitiveT, ContactT, ManagerT, MaxLevels>::addContactFromChildren(
	TreeNode* treeNode, PrimitiveT* prim)
{
	Vector3int32 base(
		2 * treeNode->cell.x, 2 * treeNode->cell.y, 2 * treeNode->cell.z);
	for (int i = 0; i < 8; ++i)
	{
		if ((treeNode->hasChild & (1 << i)) == 0)
			continue;
		Vector3int32 g(
			base.x + (i & 1),
			base.y + ((i & 2) != 0 ? 1 : 0),
			base.z + ((i & 4) != 0 ? 1 : 0));
		// :401 "tn->level - 1 >= 0".
		RBXASSERT(treeNode->level - 1 >= 0);
		HashBucket& bucket = buckets[treeNode->level - 1][treeNode->children[i]];
		for (SpatialNode* node = bucket.head;
			node != NULL;
			node = node->next)
		{
			if (node->prim == prim || !(node->cell == g))
			{
				// :412 "g != tryNode->gridId".
				RBXASSERT(node->cell == g);
				continue;
			}
			if (PrimitiveT::getContact(prim, node->prim) == NULL)
				manager->onNewPair(prim, node->prim);
		}
		if (treeNode->level >= 2)
		{
			TreeNode* child = findTreeNode(treeNode->level - 1,
				treeNode->children[i], g);
			RBXASSERT(child != NULL);  // :420 "c"
			addContactFromChildren(child, prim);
		}
	}
}

// mac: 0x569DB8 (213B): unlink + (level>0 ? assert tree (:327) + retire it :
// retire the level-0 tree child iff no same-cell nodes remain) +
// returnNode + --nodeCount.
template<class PrimitiveT, class ContactT, class ManagerT, int MaxLevels>
void SpatialHash<PrimitiveT, ContactT, ManagerT, MaxLevels>::destroyNode(
	SpatialNode* node)
{
	removeNodeFromHash(node);
	if (node->level > 0)
	{
		RBXASSERT(node->treeNode != NULL);  // :327 "destroy->treeNode"
		retireTreeNode(node->treeNode);
	}
	else
	{
		if (!findOtherNodesInLevel0Cell(node))
			removeTreeNodeChild(0, node->cell);
	}
	returnNode(node);
	--nodeCount;
}

// mac: 0x56A0E2 (132B): poison (tree/prim/next 0, level/hash -2) + pool-free.
// Ours deletes + untracks (same lifetime, no leak).
template<class PrimitiveT, class ContactT, class ManagerT, int MaxLevels>
void SpatialHash<PrimitiveT, ContactT, ManagerT, MaxLevels>::returnNode(
	SpatialNode* node)
{
	node->treeNode = NULL;
	node->prim = NULL;
	node->next = NULL;
	node->level = -2;
	node->hash = -2;
	for (size_t i = 0; i < allNodes.size(); ++i)
	{
		if (allNodes[i] == node)
		{
			allNodes[i] = allNodes.back();
			allNodes.pop_back();
			break;
		}
	}
	delete node;
}

// mac: 0x569E98 (82B): --count; if 0 && no children: _retire +
// removeTreeNodeChild(level, cell) upward.
template<class PrimitiveT, class ContactT, class ManagerT, int MaxLevels>
void SpatialHash<PrimitiveT, ContactT, ManagerT, MaxLevels>::retireTreeNode(
	TreeNode* treeNode)
{
	if (--treeNode->count == 0 && treeNode->hasChild == 0)
	{
		// Copy out BEFORE _retireTreeNode deletes the object (use-after-free
		// caught live: the old code read treeNode->level/cell after free,
		// corrupting the upward cleanup and leaving stale hasChild bits
		// that later asserted in addContactFromChildren :420).
		int level = treeNode->level;
		Vector3int32 cell = treeNode->cell;
		_retireTreeNode(treeNode);
		removeTreeNodeChild(level, cell);
	}
}

// mac: 0x56A308 (152B): unlink from the bucket tree chain, null the link,
// pool-free, --treeNodeCount.  Ours deletes + untracks.
template<class PrimitiveT, class ContactT, class ManagerT, int MaxLevels>
void SpatialHash<PrimitiveT, ContactT, ManagerT, MaxLevels>::_retireTreeNode(
	TreeNode* treeNode)
{
	HashBucket& bucket = buckets[treeNode->level][treeNode->hash];
	TreeNode** link = &bucket.treeHead;
	while (*link != treeNode)
		link = &(*link)->treeNext;
	*link = treeNode->treeNext;
	treeNode->treeNext = NULL;
	for (size_t i = 0; i < allTrees.size(); ++i)
	{
		if (allTrees[i] == treeNode)
		{
			allTrees[i] = allTrees.back();
			allTrees.pop_back();
			break;
		}
	}
	delete treeNode;
	--treeNodeCount;
}

// mac: 0x569EF4 (493B): walk tree levels above `level`, clearing the child
// bit/link for the child hash with the :355/:358 asserts, retiring emptied
// nodes upward and stopping at the first non-empty one.  Offset bits come
// from (cell >> (l-level-1)); the expected child hash is recomputed the
// same way addNode stored it (getHash(l-1, cell>>(l-level-1))).
// STUB: PLAYER 0x007b9e40
// STUB: STUDIO 0x008d67e0
template<class PrimitiveT, class ContactT, class ManagerT, int MaxLevels>
void SpatialHash<PrimitiveT, ContactT, ManagerT, MaxLevels>::removeTreeNodeChild(
	int level, Vector3int32& cell)
{
	for (int l = level + 1; l <= 3; ++l)
	{
		int rel = l - level;
		Vector3int32 coarse(cell.x >> rel, cell.y >> rel, cell.z >> rel);
		int hash = SpatialHashStatic::getHash(l, coarse);
		TreeNode* treeNode = findTreeNode(l, hash, coarse);
		RBXASSERT(treeNode != NULL);  // :355 "tn"
		int childShift = rel - 1;
		int offset = ((cell.x >> childShift) & 1)
			| (((cell.y >> childShift) & 1) << 1)
			| (((cell.z >> childShift) & 1) << 2);
		int childHash = SpatialHashStatic::getHash(l - 1,
			Vector3int32(cell.x >> childShift, cell.y >> childShift, cell.z >> childShift));
		// :358 "tn->children[offset] == childHash".
		RBXASSERT(treeNode->children[offset] == (unsigned short)childHash);
		treeNode->hasChild = (char)(treeNode->hasChild & ~(1 << offset));
		treeNode->children[offset] = -1;
		if (treeNode->count != 0 || treeNode->hasChild != 0)
			return;
		_retireTreeNode(treeNode);
	}
}

// mac: 0x56E040 (54B): unlink by identity from the level bucket (infinite loop
// if absent -- same as the binary; callers always pass live nodes).
template<class PrimitiveT, class ContactT, class ManagerT, int MaxLevels>
void SpatialHash<PrimitiveT, ContactT, ManagerT, MaxLevels>::removeNodeFromHash(
	SpatialNode* node)
{
	HashBucket& bucket = buckets[node->level][node->hash];
	SpatialNode** link = &bucket.head;
	while (*link != node)
		link = &(*link)->next;
	*link = node->next;
}

// mac: 0x56DF94 (169B): assert level 0 (:295); true iff any same-cell node
// remains in the level-0 bucket (the removed node itself is already out).
template<class PrimitiveT, class ContactT, class ManagerT, int MaxLevels>
bool SpatialHash<PrimitiveT, ContactT, ManagerT, MaxLevels>::findOtherNodesInLevel0Cell(
	SpatialNode* node)
{
	// :295 "destroy->getLevel() == 0".
	RBXASSERT(node->level == 0);
	for (SpatialNode* other = buckets[0][node->hash].head;
		other != NULL;
		other = other->next)
	{
		if (other->cell == node->cell)
			return true;
	}
	return false;
}

// mac: 0x569510 (376B).  Same level: ExtentsInt32 overlap of the stored ranges
// (:276/:277 level asserts).  Cross-level: shift the FINER prim's cells up
// to the coarser level (higher number = coarser: bigger cells), then
// overlap.  Despite the name it reads CURRENT stored cells (already the new
// ones when checkAndReleaseContacts runs) -- ported verbatim, not renamed.
// STUB: PLAYER 0x007b3880
// STUB: STUDIO 0x008cf900
template<class PrimitiveT, class ContactT, class ManagerT, int MaxLevels>
bool SpatialHash<PrimitiveT, ContactT, ManagerT, MaxLevels>::oldExtentsOverlap(
	PrimitiveT* p0, PrimitiveT* p1)
{
	int level0 = p0->getSpatialNodeLevel();
	int level1 = p1->getSpatialNodeLevel();
	RBXASSERT(level0 >= 0);  // :276 "myLevel >= 0"
	RBXASSERT(level1 >= 0);  // :277 "otherLevel >= 0"
	if (level0 == level1)
	{
		ExtentsInt32 e0(p0->getSpatialMinCell(), p0->getSpatialMaxCell());
		ExtentsInt32 e1(p1->getSpatialMinCell(), p1->getSpatialMaxCell());
		return ExtentsInt32::overlapsOrTouches(e0, e1);
	}
	PrimitiveT* coarse = (level0 > level1) ? p0 : p1;
	PrimitiveT* fine = (coarse == p0) ? p1 : p0;
	unsigned int shift = (unsigned int)(coarse->getSpatialNodeLevel()
		- fine->getSpatialNodeLevel());
	ExtentsInt32 fineCoarse(
		fine->getSpatialMinCell() >> shift,
		fine->getSpatialMaxCell() >> shift);
	ExtentsInt32 coarseCells(
		coarse->getSpatialMinCell(), coarse->getSpatialMaxCell());
	return ExtentsInt32::overlapsOrTouches(coarseCells, fineCoarse);
}

// mac: 0x569450 (191B).  Collect every contact-neighbor whose pair no longer
// overlaps (skipping megacluster prims) into array84, then releasePair each.
// Returns the last internal like the binary; callers ignore it, so ours
// returns void -- documented, not hidden.
// STUB: PLAYER 0x007b4e60
// STUB: STUDIO 0x008d1100
template<class PrimitiveT, class ContactT, class ManagerT, int MaxLevels>
void SpatialHash<PrimitiveT, ContactT, ManagerT, MaxLevels>::checkAndReleaseContacts(
	PrimitiveT* prim)
{
	array84.resize(0, true);
	for (ContactT* contact = prim->getFirstContact();
		contact != NULL;
		contact = prim->getNextContact(contact))
	{
		PrimitiveT* other = (contact->getPrim0() == prim)
			? contact->getPrim1() : contact->getPrim0();
		if (other->getGeometryType() == PrimitiveT::GEOMETRY_MEGACLUSTER)
			continue;
		if (!oldExtentsOverlap(prim, other))
			array84.append(other);
	}
	for (int i = 0; i < array84.size(); ++i)
		manager->releasePair(prim, array84[i]);
}

// mac: 0x569000 (661B).  In-place cell update for a moved/resized prim: recompute
// cells at the STORED level (never re-leveled here -- only the wrapper
// re-levels via remove+re-add); if unchanged, return (binary skips even the
// release check); else store the new cells and sync: overlapping ranges go
// through changeMinMax(union, old, new), disjoint ranges destroy the old
// range then add the new one.  The union/old/new role assignment was
// resolved by reading the disasm call setups, not the decompiler's temp
// names (which swap two ranges) -- see the session notes.
// mac: 0x567D48 (523B).  Production moves entry (see header for the DELTAs).
// STUB: PLAYER 0x007bc080
// STUB: STUDIO 0x008d8a20
template<class PrimitiveT, class ContactT, class ManagerT, int MaxLevels>
void SpatialHash<PrimitiveT, ContactT, ManagerT, MaxLevels>::onPrimitiveExtentsChanged(
	PrimitiveT* prim)
{
	if (prim->getGeometryType() == PrimitiveT::GEOMETRY_MEGACLUSTER)
		return;
	// DELTA: checkMegaClusterContact(manager, prim, 1, 1, 0) (voxel terrain;
	// unlinked pairs are no-ops in-tree either way -- see releasePair note).
	Extents safe = SpatialHashStatic::safeExtents(prim->getFastFuzzyExtents());
	int newLevel = computeLevel(prim, safe);
	int oldLevel = prim->getSpatialNodeLevel();
	// Binary: (unsigned)(newLevel + 1 - oldLevel) < 2 -- i.e. new-old in
	// {-1, 0}.  Spelled directly (proven equal, including the unsigned
	// wraparound on big drops).
	if (newLevel == oldLevel || newLevel == oldLevel - 1)
		primitiveExtentsChanged(prim, safe);
	else
	{
		primitiveRemoved(prim);
		primitiveAdded(prim, true);
	}
	// DELTA: listener fan-out over the +100/+104 array (event triple
	// {flag, oldLevel}+old/new cell ranges; layout unrecoverable and the
	// array is empty in-tree -- revisit with a registrant).
}

// STUB: PLAYER 0x007bbd90
// STUB: STUDIO 0x008d8730
template<class PrimitiveT, class ContactT, class ManagerT, int MaxLevels>
void SpatialHash<PrimitiveT, ContactT, ManagerT, MaxLevels>::primitiveExtentsChanged(
	PrimitiveT* prim, const Extents& extents)
{
	// :717 "p->getSpatialNodeLevel() != -1".
	RBXASSERT(prim->getSpatialNodeLevel() != -1);
	int level = prim->getSpatialNodeLevel();
	Vector3int32 newMin, newMax;
	SpatialHashStatic::computeMinMax(level, extents, newMin, newMax);
	Vector3int32 oldMin = prim->getSpatialMinCell();
	Vector3int32 oldMax = prim->getSpatialMaxCell();
	if (newMin == oldMin && newMax == oldMax)
		return;
	prim->setSpatialCells(newMin, newMax, level);
	ExtentsInt32 newCells(newMin, newMax);
	ExtentsInt32 oldCells(oldMin, oldMax);
	if (newCells.overlapsOrTouches(oldCells))
	{
		ExtentsInt32 unionCells(
			Vector3int32(
				newMin.x < oldMin.x ? newMin.x : oldMin.x,
				newMin.y < oldMin.y ? newMin.y : oldMin.y,
				newMin.z < oldMin.z ? newMin.z : oldMin.z),
			Vector3int32(
				newMax.x > oldMax.x ? newMax.x : oldMax.x,
				newMax.y > oldMax.y ? newMax.y : oldMax.y,
				newMax.z > oldMax.z ? newMax.z : oldMax.z));
		changeMinMax(prim, &unionCells, &oldCells, &newCells, true);
	}
	else
	{
		changeMinMax(prim, &oldCells, &oldCells, NULL, true);
		changeMinMax(prim, &newCells, NULL, &newCells, true);
	}
	checkAndReleaseContacts(prim);
}
