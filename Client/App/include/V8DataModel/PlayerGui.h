/* V8DataModel/PlayerGui.h -- October 2013.
   Ctor PLAYER 0x0070c050 (setName("PlayerGui") after the vptrs), no members of its own: the
   FactoryProduct deleting destructor 0x0070c100 goes straight to ~BasePlayerGui. RTTI:
   DescribedCreatable<PlayerGui, BasePlayerGui, sPlayerGui, 5 = INTERNAL_PLAYER, 0>.
   Vftable 0x00b4849c overrides askForbidParent (+0x44, folded with StatusInstance's at 0x006d51a0)
   and askSetParent (+0x48, folded with Players::askAddChild at 0x0070a240).
   Original file: /Applications/buildAgent/work/fa0a2141fe3d2f4d/Client/App/v8datamodel/PlayerGui.h */

#pragma once

#include "V8DataModel/BasePlayerGui.h"

namespace RBX {

	extern const char* const sPlayerGui;

	// VTABLE: PLAYER 0x00b4849c RBX::Debugable
	// VTABLE: STUDIO 0x00bfd78c RBX::Debugable
	// SIZE 0x74
	class PlayerGui
		: public DescribedCreatable<PlayerGui, BasePlayerGui, sPlayerGui, Reflection::ClassDescriptor::INTERNAL_PLAYER>
	{
	public:
		PlayerGui();

	protected:
		/*override*/ bool askSetParent(const Instance* instance) const;
		/*override*/ bool askForbidParent(const Instance* instance) const { return !askSetParent(instance); }
	};

} // namespace RBX

// SYNTHETIC: PLAYER 0x007087f0
// SYNTHETIC: STUDIO 0x007955f0
// RBX::PlayerGui::~PlayerGui
