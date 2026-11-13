/* App/util/IndexedMesh.cpp -- October 2013. Mac bodies (batch202): C2 0x206020, D2 0x206098, setComputedUpper 0x20627E, setUpper 0x2062CC, setLower 0x20633E,
   attachChildren 0x206492, onParentChanged 0x2064E4, severeChildren 0x206560, getConstIndexedMeshParent 0x2065B6, getComputedUpper 0x20647C, getConstComputedUpper
   0x206642, getIndexedMeshParent 0x206488. 2013 asserts: :19-:23 (D2), :78 (setLower), :173 (getConstIndexedMeshParent), :219 (isUpperRoot, unported). */
#include "stdafx.h"
#include "Util/IndexedMesh.h"
namespace RBX {
// mac: 0x206020 (61B).
IndexedMesh::IndexedMesh()
	: upper(NULL)
	, lower(NULL)
	, computedUpper(NULL)
{
}
// mac: 0x206098 (486B).  Destruction requires a fully unlinked mesh -- the binary asserts rather than severing, so tests must unlink first.
// FUNCTION: PLAYER 0x008f9160
// FUNCTION: STUDIO 0x00a0e220
IndexedMesh::~IndexedMesh()
{
	RBXASSERT(!lower);  // :19 "!lower" (IndexedMesh.cpp:19)
	RBXASSERT(!upper);  // :20 "!upper" (IndexedMesh.cpp:20)
	RBXASSERT(!computedUpper);  // :21 "!computedUpper" (IndexedMesh.cpp:21)
	RBXASSERT(!getTypedParent<IndexedMesh>());  // :22 (IndexedMesh.cpp:22)
	RBXASSERT(numChildren() == 0);  // :23 "numChildren() == 0" (IndexedMesh.cpp:23)
}

// Lower-chain climb for the :173 parent check (decoded from the assert's
// inline walk in getConstIndexedMeshParent 0x2065B6): first ancestor above
// lower with an upper set, answer is that upper (NULL if none).
const IndexedMesh* IndexedMesh::computeParentFromLower() const
{
	if (lower == NULL)
		return NULL;
	const IndexedMesh* node = lower->getConstTypedParent<IndexedMesh>();
	while (node != NULL && node->getConstUpper() == NULL)
		node = node->getConstTypedParent<IndexedMesh>();
	return (node != NULL) ? node->getConstUpper() : NULL;
}

































































































































// mac: 0x2065B6 (139B): typed parent + the :173 cross-check.
// STUB: PLAYER 0x008f9300
// STUB: STUDIO 0x00a0e3c0
const IndexedMesh* IndexedMesh::getConstIndexedMeshParent() const
{
	const IndexedMesh* answer = getConstTypedParent<IndexedMesh>();
	RBXASSERT(answer == computeParentFromLower());  // :173 (IndexedMesh.cpp:173)
	return answer;
}

// mac: 0x206488 (9B): forwards to the const version.
// FUNCTION: PLAYER 0x008f9370
// FUNCTION: STUDIO 0x00a0e430
IndexedMesh* IndexedMesh::getIndexedMeshParent()
{
	return const_cast<IndexedMesh*>(
		static_cast<const IndexedMesh*>(this)->getConstIndexedMeshParent());
}

// mac: 0x20647C (11B) / 0x206642 (11B): plain member reads (no lazy compute).
IndexedMesh* IndexedMesh::getComputedUpper()
{
	return computedUpper;
}

const IndexedMesh* IndexedMesh::getConstComputedUpper() const
{
	return computedUpper;
}

void IndexedMesh::setComputedUpper(IndexedMesh* newComputedUpper)
{
	if (computedUpper != newComputedUpper)
	{
		computedUpper = newComputedUpper;
		for (int i = 0; i < numChildren(); ++i)
		{
			IndexedMesh* child = getTypedChild<IndexedMesh>(i);
			if (child->getUpper() == NULL)
				child->setComputedUpper(newComputedUpper);
		}
	}
}

// STUB: PLAYER 0x008f9590
// STUB: STUDIO 0x00a0e650
void IndexedMesh::setUpper(IndexedMesh* newUpper)
{
	if (upper != newUpper)
	{
		IndexedMesh* oldUpper = upper;
		upper = NULL;
		if (oldUpper != NULL)
			oldUpper->setLower(NULL);
		upper = newUpper;
		IndexedMesh* computed = NULL;
		if (newUpper != NULL)
		{
			newUpper->setLower(this);
			computed = upper;
		}
		if (computed == NULL)
		{
			IndexedMesh* node = getTypedParent<IndexedMesh>();
			while (node != NULL && node->getUpper() == NULL)
				node = node->getTypedParent<IndexedMesh>();
			computed = (node != NULL) ? node->getUpper() : NULL;
		}
		setComputedUpper(computed);
	}
}

// STUB: PLAYER 0x008f9490
// STUB: STUDIO 0x00a0e550
void IndexedMesh::setLower(IndexedMesh* newLower)
{
	if (lower != newLower)
	{
		if (lower != NULL)
		{
			const IndexedMesh* savedParent = getConstIndexedMeshParent();
			setIndexedTreeParent(NULL);
			// Severe back-children onto the saved parent (last to first --
			// removal shrinks the list under the walk).
			for (int i = numChildren() - 1; i >= 0; --i)
			{
				IndexedMesh* backChild = getTypedChild<IndexedMesh>(i);
				RBXASSERT(backChild->getTypedParent<IndexedMesh>() == this);  // :78 (IndexedMesh.cpp:78)
				backChild->setIndexedTreeParent(
					const_cast<IndexedMesh*>(savedParent));
			}
		}
		lower = newLower;
		if (newLower != NULL)
		{
			// Adopt newLower's upper-chain anchor (first ancestor ABOVE
			// newLower with an upper, else NULL) as the tree parent.
			IndexedMesh* anchor = newLower->getTypedParent<IndexedMesh>();
			while (anchor != NULL && anchor->getUpper() == NULL)
				anchor = anchor->getTypedParent<IndexedMesh>();
			IndexedMesh* anchorUpper = (anchor != NULL) ? anchor->getUpper() : NULL;
			setIndexedTreeParent(anchorUpper);
			for (int i = 0; i < newLower->numChildren(); ++i)
				attachChildren(newLower->getTypedChild<IndexedMesh>(i));
		}
	}
}

void IndexedMesh::attachChildren(IndexedMesh* lowerChild)
{
	if (lowerChild->getUpper() != NULL)
	{
		lowerChild->getUpper()->setIndexedTreeParent(this);
		return;
	}
	for (int i = 0; i < lowerChild->numChildren(); ++i)
		attachChildren(lowerChild->getTypedChild<IndexedMesh>(i));
}

void IndexedMesh::severeChildren(IndexedMesh* lowerChild)
{
	if (lowerChild->getUpper() != NULL)
	{
		lowerChild->getUpper()->setIndexedTreeParent(NULL);
		return;
	}
	for (int i = 0; i < lowerChild->numChildren(); ++i)
		severeChildren(lowerChild->getTypedChild<IndexedMesh>(i));
}

// STUB: PLAYER 0x008f9600
// STUB: STUDIO 0x00a0e6c0
void IndexedMesh::onParentChanged(IndexedTree* oldParent)
{
	// NOTE: oldParent is intentionally unused beyond the null check below
	// (binary reads its computedUpper first).  Kept verbatim.
	(void)oldParent;
	if (oldParent != NULL)
	{
		IndexedMesh* oldParentMesh = static_cast<IndexedMesh*>(oldParent);
		IndexedMesh* oldComputed = oldParentMesh->getComputedUpper();
		if (oldComputed != NULL)
		{
			oldComputed->severeChildren(this);
			oldComputed->lowersChanged();
		}
	}
	IndexedMesh* parent = getTypedParent<IndexedMesh>();
	if (parent != NULL)
	{
		IndexedMesh* newComputed = parent->getComputedUpper();
		if (newComputed != NULL)
		{
			newComputed->attachChildren(this);
			newComputed->lowersChanged();
		}
	}
	IndexedMesh* computed = upper;
	if (computed == NULL)
	{
		IndexedMesh* node = getTypedParent<IndexedMesh>();
		while (node != NULL && node->getUpper() == NULL)
			node = node->getTypedParent<IndexedMesh>();
		computed = (node != NULL) ? node->getUpper() : NULL;
	}
	setComputedUpper(computed);
}

} // namespace RBX
