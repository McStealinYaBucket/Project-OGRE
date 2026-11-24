// Placeholder so the Win OBJECT library has a source to build. Delete it once the
// library has real sources (and drop it from CMakeLists.txt).

/* Shell template stand-ins -- NOT original code, carry no marker (PLAYER and STUDIO: both shells
   compile Client/Win).

   Stand in for the 2013 shell code (WindowsClient, ClientBase, Win: ReflectionMetadata,
   FrameRateManagerStatsItem, the views ...) that instantiated these templates. The shell is /GL and
   is linked first, so a COMDAT symbol it also instantiates resolves to its LTCG copy for every
   caller, engine callers included, and VC9's /OPT:ICF never folds that copy with the native one
   (CLAUDE.md, Gotchas). Without the shell, our link folds all of them into one native body and
   reccmp pairs only one of the two originals, so the engine callers of the other lose a call.

   Each entry is a symbol the original's engine functions call at its shell copy and never at a
   native one (tools/shell_twin_callers.py on sweep15: PLAYER shell calls / native calls, example
   caller -> shell copy). STUDIO's shell owns the same ones (56/0 ~shared_ptr<Instance>, 9/0
   signal<bool>, 7/0 StandardOut, 6/0 weak_ptr<DataModel>::lock, ...). Delete an entry when the
   real shell code that instantiates it is written. Contradictory symbols (both counts non-zero)
   are left out. CoreGuiService (shared_from 1/0, = 1/0) is left out too: its header made the shell
   instantiate Creatable<Instance>::create<ScreenGui> and its chain, which 2013 keeps native (-6).
   rbx::signal<void(bool)>::operator() (9/0) is left out: its shell copy 0x00439b20 has a second
   shell twin 0x00456d20 (PropertyDescriptor), ours folds the two, and the Lighting setters lost 7.
   Measured 2026-11-24 on identical objects with and without this TU's stand-in (two regeneration
   rounds each): PLAYER +51/-24, STUDIO +59/-32; every loss is a twin callee pairing.
   Removed 2026-11-24: shared_ptr<DescribedBase>::operator= and enable_shared_from_this<DescribedBase>::
   shared_from_this, which ClientBase/ReflectionMetadata.cpp (the real 2013 shell code) now instantiates
   (both resolve to its object in the PLAYER map).
   Removed 2026-11-24 (wave 11): ~shared_ptr<StandardOut>, which Win/LogManager.cpp (ReportEvent) now
   instantiates. */

#include "V8Tree/Instance.h"
#include "V8DataModel/DataModel.h"
#include "V8DataModel/RunService.h"
#include "V8DataModel/Script.h"
#include "rbx/Tasks/Coordinator.h"

#include <new>
#include <vector>

namespace OgreShellStandIn {

	struct Uses
	{
		boost::shared_ptr<RBX::Instance> instance;              // ~ 73/0 0x00488850 -> 0x00405b50; = 40/0 -> 0x00469520
		std::vector<boost::shared_ptr<RBX::Instance> > instances; // resize 34/0 -> 0x004226f0; _Destroy_range 5/0 -> 0x00427f30; erase 1/0 -> 0x00426170
		boost::weak_ptr<RBX::DataModel> dataModel;              // lock 6/0 0x0063f330 -> 0x00439eb0
		boost::function0<void> function;                        // assign_to_own 3/0 0x007a68d0 -> 0x00437a90
		boost::shared_ptr<RBX::RunService> runService;          // shared_from 2/0, = 1/0 0x0064f210
		boost::shared_ptr<RBX::Script> script;                  // ~ 1/0 0x0088cce0 -> 0x00405b50
		// Copy ctor: Job.obj's vector<shared_ptr<Coordinator>>::_Insert_n 0x009ba5c0 calls the shell
		// copy 0x00422330, Thread.obj's twin 0x009b6af0 (Thread, Job, Job const) the native 0x00576dc0,
		// so the two push_back chains (0x009ba960 / 0x009b7020) stay apart. PLAYER evidence only.
		boost::shared_ptr<RBX::Tasks::Coordinator> coordinator;
	};

	extern "C" void ogreShellStandInUse(Uses& u, Uses& v, RBX::RunService* r, void* raw)
	{
		u.instance = v.instance;
		u.instances.resize(v.instances.size());
		u.instances.erase(u.instances.begin());
		u.dataModel.lock();
		new (raw) boost::function0<void>(v.function);          // copy ctor only: function0::operator= is native-owned (0/1)
		u.runService = RBX::shared_from(r);
		u.script.reset();
#ifndef RBX_STUDIO_BUILD
		new (raw) boost::shared_ptr<RBX::Tasks::Coordinator>(v.coordinator); // copy ctor only
#endif
	}

} // namespace OgreShellStandIn

// Keep it through /OPT:REF so its instantiations stay in the LTCG set.
#pragma comment(linker, "/include:_ogreShellStandInUse")
