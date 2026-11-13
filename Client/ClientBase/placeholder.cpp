// Placeholder so the ClientBase OBJECT library has a source to build. Not original code and carries
// no marker. Delete it once the library has real sources (and drop it from CMakeLists.txt).
//
// Stand-in for the 2013 shell's own Instance subclasses (CRenderSettingsItem,
// FrameRateManagerStatsItem, ...). Both shells are /GL with debug info and link /LTCG /DEBUG, and
// under that combination VC9's linker keeps LTCG-generated COMDATs apart from the engine's native
// ones: /OPT:ICF folds identical bodies inside each set but never across the two (probe 2026-11-13:
// an IL `ret 4` and a native `ret 4` fold without /Z7 + /DEBUG and stay apart with both). The shell's
// classes emit Instance's vftables in IL, so the original keeps a shell copy of every trivial
// Instance inline virtual (PLAYER verifyAddChild 0x004027c0, canClientCreate 0x00402730,
// onServiceProvider 0x00402740, GuidItem<Instance>::onGuidChanged 0x0040b950; STUDIO 0x0040eb90,
// 0x0040c110, 0x004fad60) next to the engine's copy of the same body (0x00575d00, 0x00927cc0,
// 0x008c8450, 0x00528720). This class makes our shell emit the same COMDATs until the real shell
// classes do.

#include "V8Tree/Instance.h"

namespace
{
	class ShellInstanceStandIn : public RBX::Instance
	{
	public:
		ShellInstanceStandIn();
		/*override*/ const RBX::Name& getClassName() const;
	};

	ShellInstanceStandIn::ShellInstanceStandIn()
	{
	}

	const RBX::Name& ShellInstanceStandIn::getClassName() const
	{
		return RBX::Name::getNullName();
	}
}

RBX::Instance* ogre_shell_instance_stand_in()
{
	return new ShellInstanceStandIn();
}
