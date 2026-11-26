/* App/v8datamodel/Selection.cpp -- October 2013 STUB. TODO: full decompile. */

#include "stdafx.h"

#include "V8DataModel/Selection.h"
#include "decomp.h"

namespace RBX {
	const char* const sSelection = "Selection";
}

using namespace RBX;

REFLECTION_BEGIN();
REFLECTION_END();

// Class registrar (see Instance.cpp/Service.cpp note).
namespace RBX {
RBX_REGISTER_CLASS(RBX::Selection);
}

namespace RBX {
// Not written yet (vector/set members, a selectionChanged signal, setName("Selection")).
// __declspec(noinline): stand-in until the body lands, so the deleting dtor 0x00869fd0 calls it.
// STUB: PLAYER 0x00869e20
// STUB: STUDIO 0x0079dc50
__declspec(noinline) Selection::~Selection()
{
	STUB(0x00869e20);
}

// Not written yet (578 bytes).
// STUB: PLAYER 0x0086aa30
// STUB: STUDIO 0x0079e860
__declspec(noinline) void Selection::removeFromSelection(Instance* instance)
{
	STUB(0x0086aa30);
}

// FUNCTION: PLAYER 0x0086adb0
// FUNCTION: STUDIO 0x0079ebe0
void Selection::onAncestryChanged(Instance* instance)
{
	if (instance->getRootAncestor() != getRootAncestor())
		removeFromSelection(instance);
}

// STUB: PLAYER 0x0086b070
// STUB: STUDIO 0x0079f000
Selection::Selection()
{
	STUB(0x0086b070);
}
} // namespace RBX

DECOMP_SIZE_ASSERT(RBX::Selection, 0x90);
