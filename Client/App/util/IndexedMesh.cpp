/* App/util/IndexedMesh.cpp -- October 2013. Mac bodies (batch202): C2 0x206020, D2 0x206098, setComputedUpper 0x20627E, setUpper 0x2062CC, setLower 0x20633E,
   attachChildren 0x206492, onParentChanged 0x2064E4, severeChildren 0x206560, getConstIndexedMeshParent 0x2065B6, getComputedUpper 0x20647C, getConstComputedUpper
   0x206642, getIndexedMeshParent 0x206488. 2013 asserts: :19-:23 (D2), :78 (setLower), :173 (getConstIndexedMeshParent), :219 (isUpperRoot). */
#include "stdafx.h"
#include "Util/IndexedMesh.h"
namespace RBX {
// FUNCTION: PLAYER 0x008f9050
// FUNCTION: STUDIO 0x00a0e110
IndexedMesh::IndexedMesh()
	: upper(NULL)
	, lower(NULL)
	, computedUpper(NULL)
{
}
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
	// 2013 (inlined in getConstIndexedMeshParent 0x008f9300): no NULL check on lower.
	const IndexedMesh* node = lower->getConstTypedParent<IndexedMesh>();
	while (node != NULL && node->getConstUpper() == NULL)
		node = node->getConstTypedParent<IndexedMesh>();
	return (node != NULL) ? node->getConstUpper() : NULL;

}




























// FUNCTION: PLAYER 0x008f9490
// FUNCTION: STUDIO 0x00a0e550
void IndexedMesh::setLower(IndexedMesh* newLower)
{
	if (newLower != lower)
	{
		if (lower) {
			IndexedMesh* oldParent = getIndexedMeshParent();
			setIndexedTreeParent(NULL);
			while (numChildren() > 0)
			{
				IndexedMesh* backChild = getTypedChild<IndexedMesh>(numChildren() - 1);
				RBXASSERT(backChild->getTypedParent<IndexedMesh>() == this);
				backChild->setIndexedTreeParent(oldParent);
			}
		}
		lower = newLower;
		if (newLower) {
			if (IndexedMesh* lowerParent = newLower->getTypedParent<IndexedMesh>()) {
				setIndexedTreeParent(computeUpper(lowerParent));
			}
			for (int i = 0; i < newLower->numChildren(); ++i) {
				attachChildren(newLower->getTypedChild<IndexedMesh>(i));
			}
		}
	}
}










































































// mac: 0x2065B6 (139B): typed parent + the :173 cross-check.
// FUNCTION: PLAYER 0x008f9300
// FUNCTION: STUDIO 0x00a0e3c0
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







// STUB: PLAYER 0x008f9080
// STUB: STUDIO 0x00a0e140
const IndexedMesh* IndexedMesh::computeConstUpper(const IndexedMesh* lower)
{
	if (lower->getConstUpper()) {
		return lower->getConstUpper();
	}
	const IndexedMesh* parent = lower->getConstTypedParent<IndexedMesh>();
	return parent ? computeConstUpper(parent) : NULL;
}

// STUB: PLAYER 0x008f90b0
// STUB: STUDIO 0x00a0e170
bool IndexedMesh::isUpperRoot(const IndexedMesh* lower)
{
	bool answer = (lower && lower->getConstUpper());
	RBXASSERT(answer == (lower && (computeConstUpper(lower)->lower == lower)));
	return answer;
}

// 2013: setUpper/setLower/onParentChanged inline one level of the recursion and call the out-of-line
// copy at 0x008f9080, which is a loop there (here: a tail jump; isUpperRoot calls it without inlining).
IndexedMesh* IndexedMesh::computeUpper(IndexedMesh* lower)
{
	return const_cast<IndexedMesh*>(computeConstUpper(lower));
}

// FUNCTION: PLAYER 0x008f93a0
// FUNCTION: STUDIO 0x00a0e460
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

// FUNCTION: PLAYER 0x008f9590
// FUNCTION: STUDIO 0x00a0e650
void IndexedMesh::setUpper(IndexedMesh* newUpper)
{
	if (newUpper != upper)
	{
		IndexedMesh* oldUpper = upper;
		upper = NULL;
		if (oldUpper) {
			oldUpper->setLower(NULL);
		}
		upper = newUpper;
		if (upper) {
			upper->setLower(this);
		}
		setComputedUpper(computeUpper(this));
	}
}


// FUNCTION: PLAYER 0x008f93f0
// FUNCTION: STUDIO 0x00a0e4b0
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

// FUNCTION: PLAYER 0x008f9440
// FUNCTION: STUDIO 0x00a0e500
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
	if (oldParent) {
		IndexedMesh* oldComputed = static_cast<IndexedMesh*>(oldParent)->getComputedUpper();
		if (oldComputed) {
			oldComputed->severeChildren(this);
			oldComputed->lowersChanged();
		}
	}
	if (IndexedMesh* parent = getTypedParent<IndexedMesh>()) {
		IndexedMesh* newComputed = parent->getComputedUpper();
		if (newComputed) {
			newComputed->attachChildren(this);
			newComputed->lowersChanged();
		}
	}
	setComputedUpper(computeUpper(this));
}

} // namespace RBX
