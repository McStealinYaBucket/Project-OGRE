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
   instantiates.
   Added 2026-11-24 (wave 13, A/B on identical engine objects, regenerated CSVs, 0 lost): the
   intrusive_ptr<thread_data_base> dtor, CStringT(const wchar_t*), DebugSettings::singleton (with its
   create chain), the Http temporary (its dtor) and RunningAverage<double>::sample. Tried and left out:
   Security::Context::current (it pulls the engine's own out-of-line current() into the shell, -1) and
   ServiceProvider::find/create<T> plus FactoryProduct<T>::className() for 13 services (-620: find<T>'s
   shared helpers and Described<T>::classDescriptor move into the shell).
   Added 2026-11-26 (wave 20): prime_list_template<unsigned>::value (the users unchanged; it keeps an
   engine TU that declares an unordered_map from supplying it). Tried and left out: a shell ~SoundId
   (a local from SoundChannel::getSoundId): +2 (the ContentId descriptor vftables 0x00b3388c/0x00b33904
   fold again) but equalValues<SoundId> 0x006bc870 then folds into equalValues<ContentId> and loses its
   100. 2013 sends equalValues<SoundId>'s funclets to a second thunk 0x004a2390, so only copyValue<SoundId>
   reaches the shell's 0x0041c8a0 (through ~ContentId, it seems: App's copyValue<SoundId> source). */

#include "V8Tree/Instance.h"
#include "V8DataModel/DataModel.h"
#include "V8DataModel/RunService.h"
#include "V8DataModel/Script.h"
#include "rbx/Tasks/Coordinator.h"
#include "rbx/RunningAverage.h"
#include "Util/Http.h"
#include "V8DataModel/DebugSettings.h"
#include "reflection/Property.h"
#include "rbx/TaskScheduler.h"

#include <boost/thread/thread.hpp>
#include <boost/thread/recursive_mutex.hpp>
#include <boost/unordered/detail/util.hpp>

#include <atlstr.h>

#include <locale>
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
		// ~ 18/0: the engine's unwind funclets (0x00a117ee ...) jump to the shell copy 0x004069d0
		// (STUDIO 0x004256d0); without it the only copy is CrashReporter.obj's unoptimized one.
		boost::intrusive_ptr<boost::detail::thread_data_base> threadData;
	};

	extern "C" void ogreShellStandInUse(Uses& u, Uses& v, RBX::RunService* r, void* raw)
	{
		boost::intrusive_ptr<boost::detail::thread_data_base> threadData(v.threadData); // dtor called from the unwind funclet
		u.instance = v.instance;
		u.instances.resize(v.instances.size());
		u.instances.erase(u.instances.begin());
		u.dataModel.lock();
		new (raw) boost::function0<void>(v.function);          // copy ctor only: function0::operator= is native-owned (0/1)
		u.runService = RBX::shared_from(r);
		u.script.reset();
		// CStringT<char>(const wchar_t*) 5/0: FileSystem::getUserDirectory, redirectCallback and
		// httpGetPostWinHttp call the shell's /GS copy 0x00440d20, which no written shell code builds yet.
		ATL::CStringA narrowed(L"");
		// Engine callers reach these only at their shell copies (tools/shell_twin_callers.py on the
		// wave-13 link D: shell/native calls, the shell copy); ours were native, so built without /GS.
		RBX::DebugSettings::singleton();                                     // 7/0 0x00407700
		RBX::Http http("");                                                  // ctor 5/0 0x00402b50
		static_cast<RBX::RunningAverage<double, double>*>(raw)->sample(1.0); // 5/0 0x004612a0
		// boost::unordered's prime_list_template<unsigned>::value: 2013's /GL shell supplies it (PLAYER
		// 0x00bdafe8, among the shell's LTCG data, after Document's slot vftables; STUDIO 0x00d10920),
		// so the engine's unordered containers (StatsUpdateJob's maps) do not decide where it lands.
		*static_cast<std::size_t*>(raw) = boost::unordered::detail::next_prime(*static_cast<std::size_t*>(raw));
		// function0<void>::swap 2/0: the engine's 0x00540fd0 and 0x0069c690 call the shell copy
		// 0x0043b140, so LTCG keeps it __thiscall (ecx + stack). Those engine functions are not
		// written; taking the address stands in for them (RecordToggleVerb::doIt's operator=).
#ifndef RBX_STUDIO_BUILD
		static void (boost::function0<void>::*volatile swapFn)(boost::function0<void>&) = &boost::function0<void>::swap;
		// Wave-13 twins list (build-sh/w13/twins-D.csv), PLAYER: unique_lock<recursive_mutex>::lock
		// 8/0 -> 0x004366c0, the ConstProperty ctor 3/0 -> 0x00441d40 (its assert spells
		// reflection/Property.h), use_facet<numpunct<char>> 3/0 -> 0x0041aaa0.
		boost::unique_lock<boost::recursive_mutex> recursiveLock(*static_cast<boost::recursive_mutex*>(raw));
		RBX::Reflection::ConstProperty constProperty(*static_cast<const RBX::Reflection::PropertyDescriptor*>(raw),
			static_cast<const RBX::Reflection::DescribedBase*>(raw));
		std::use_facet<std::numpunct<char> >(std::locale());
#endif
#ifndef RBX_STUDIO_BUILD
		new (raw) boost::shared_ptr<RBX::Tasks::Coordinator>(v.coordinator); // copy ctor only
		// weak_ptr<Job>::operator=(shared_ptr<Job>): 2013's _internal_accept_owner<T> copies come in
		// four bodies by (weak_ptr operator=, unwind ~shared_ptr): native/native 0x005baa50, shell/shell
		// 0x005bd080 and 0x007a2e50, native/shell 0x006678b0 (StandardOut). The Job types' copy is
		// shell/shell (sp_pointer_construct<NetworkOwnerJob> ... call 0x005bd080); View.cpp gives Job its
		// shell ~shared_ptr, and this the shell operator= (in 2013 the shell's job creation).
		{ boost::weak_ptr<RBX::TaskScheduler::Job> w; w = *static_cast<boost::shared_ptr<RBX::TaskScheduler::Job>*>(raw); }
#endif
	}

} // namespace OgreShellStandIn

// Keep it through /OPT:REF so its instantiations stay in the LTCG set.
#pragma comment(linker, "/include:_ogreShellStandInUse")

