/* App/v8tree/Verb.cpp — reconstructed for October 2013.
   Evidence: symbolled Studio 0.126 decompilation
     (F:\2013Decompile\exports\decomp\Refl_DataModel.cpp.txt)
      - DataModel::close 0x2CFF76 calls RBX::VerbContainer::getVerb(this+144, "Exit"),
        throws std::runtime_error "Couldn't find Exit Verb" on NULL, then calls
        vtable slot 7 (doIt) directly: (*(Verb+28))(Verb, 0).
      - DataModel::toggleToolsOff 0x2D00E4 calls getVerb(this+144, "TogglePlayMode"),
        then vtable slot 4 (isChecked); if true, calls slot 7 (doIt).
      - DataModel::completeShutdown 0x2D038C calls getVerb on the workspace
        VerbContainer (+324 into workspace) with "ShutdownClientAndSave"/"ShutdownClient".
     => VerbContainer::getVerb(const std::string&) exists with 2016 semantics
        (whitelist+blacklist search + parent chain). No whitelist/doItWithChecks
        layer in 2013 callers — they use plain getVerb + direct virtual calls.
   2013 adaptations from 2016 source (style template only):
     - Name::declare/lookup return const Name* (2013) not const Name& (2016).
     - Both Verb ctors implemented (2016 only implements the string one).
   Original file: /Applications/buildAgent/work/fa0a2141fe3d2f4d/Client/App/v8tree/Verb.cpp */

#include "stdafx.h"
#include "decomp.h"

#include "V8Tree/Verb.h"

#include "Util/Name.h"

namespace RBX {

Verb::Verb(VerbContainer* container, const std::string& name)
	: name(*Name::declare(name.c_str()))
	, container(container)
{
	if (container)
		container->addVerb(this);
}

Verb::~Verb()
{
	if (container!=NULL)
		container->removeVerb(this);
}

// FUNCTION: PLAYER 0x006f9200
VerbContainer::VerbContainer(VerbContainer* parent)
	: parent(parent)
{
}

void VerbContainer::setVerbParent(VerbContainer* parent)
{
	this->parent = parent;
}

// The 2013 asserts put addVerb's check on line 58 and removeVerb's on line 65 of this file.
// FUNCTION: PLAYER 0x006f9270
// FUNCTION: STUDIO 0x007fa270
void VerbContainer::addVerb(Verb* verb)
{
	RBXASSERT(verbs.find(&verb->getName())==verbs.end());
	verbs[&verb->getName()] = verb;
}
// FUNCTION: PLAYER 0x006f9190
// FUNCTION: STUDIO 0x007fa190
void VerbContainer::removeVerb(Verb* verb) {
	int verEraseCount = static_cast<int>(verbs.erase(&verb->getName()));
	RBXASSERT(verEraseCount==1);
}
// FUNCTION: PLAYER 0x006f90d0
// FUNCTION: STUDIO 0x007fa070
Verb* VerbContainer::getVerb(const std::string& name)
{
	return getVerb(*Name::lookup(name));
}
// FUNCTION: PLAYER 0x006f90f0
// FUNCTION: STUDIO 0x007fa0f0
VerbContainer::~VerbContainer()
{
	// delete any remaining verbs
	while (verbs.begin()!=verbs.end())
		delete verbs.begin()->second;
}

Verb* VerbContainer::getVerb(const Name& name)
{
	if (name.empty())
		return NULL;

	std::map<const Name*, Verb*>::iterator iter = verbs.find(&name);
	if (iter!=verbs.end())
		return iter->second;

	if (parent!=NULL)
		return parent->getVerb(name);
	else
		return NULL;
}

}  // namespace RBX

DECOMP_SIZE_ASSERT(RBX::VerbContainer, 0x14);
